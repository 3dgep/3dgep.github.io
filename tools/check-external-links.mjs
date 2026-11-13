// Check external links in the articles: follow redirects, report dead links, and (with --write) replace
// redirected URLs with their final destination.
//
// Usage: node tools/check-external-links.mjs [--write | --apply] [files...]   (default: src/content/docs/**/*.md(x))
//   --write  check, then replace redirected URLs
//   --apply  replace redirected URLs using the results of the previous run (tools/external-links.json)
//
// - Every unique http(s) URL is fetched (GET, redirects followed manually so the chain is visible).
// - "ok":         200 at the same URL (http -> https upgrades count as redirects and are fixed).
// - "redirected": ends at 200 elsewhere -> replaced with the final URL in --write mode. Tracking parameters such
//                 as ?redirectedfrom=MSDN are dropped; the original #fragment is kept.
// - "suspicious": redirect ends on a generic page (site root, search, "page not found" title, or the original
//                 #anchor is missing on the final page) -> reported, not replaced.
// - "broken":     4xx/5xx, DNS errors, timeouts -> reported, not replaced.
// A results file (external-links.json) is written next to this script for review.
import fs from 'node:fs';
import path from 'node:path';

const args = process.argv.slice(2);
const APPLY = args.includes('--apply');
const WRITE = args.includes('--write') || APPLY;
const root = path.resolve(path.dirname(new URL(import.meta.url).pathname.replace(/^\/(\w:)/, '$1')), '..');
const files = args.filter((a) => !a.startsWith('--'));
if (!files.length) {
  const dir = path.join(root, 'src/content/docs');
  for (const f of fs.readdirSync(dir, { recursive: true })) if (/\.mdx?$/.test(f)) files.push(path.join(dir, f));
}

// --- collect URLs (Markdown link targets with balanced parentheses, autolinks, and href="...") ---------------
function extractUrls(md) {
  const urls = [];
  for (let i = md.indexOf(']('); i !== -1; i = md.indexOf('](', i + 2)) {
    let j = i + 2, depth = 0;
    for (; j < md.length; j++) {
      const c = md[j];
      if (c === '(') depth++;
      else if (c === ')') { if (depth === 0) break; depth--; }
      else if (c === '\n' || c === ' ') break;
    }
    const url = md.slice(i + 2, j);
    if (/^https?:\/\//.test(url)) urls.push(url);
  }
  for (const m of md.matchAll(/<(https?:\/\/[^>\s]+)>/g)) urls.push(m[1]);
  for (const m of md.matchAll(/href="(https?:\/\/[^"]+)"/g)) urls.push(m[1]);
  return urls;
}
const occurrences = new Map(); // url -> [file]
for (const f of files) for (const u of extractUrls(fs.readFileSync(f, 'utf8'))) {
  if (!occurrences.has(u)) occurrences.set(u, new Set());
  occurrences.get(u).add(path.relative(root, f));
}
// Links to the site itself (3dgep.com articles that aren't ported yet) are checked too.

// --- check -----------------------------------------------------------------------------------------------------
const UA = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0 Safari/537.36';
const TRACKING = /^(redirectedfrom|utm_[a-z]+|ref|source)$/i;
const cleanUrl = (u, fragment) => {
  const url = new URL(u);
  for (const k of [...url.searchParams.keys()]) if (TRACKING.test(k)) url.searchParams.delete(k);
  // Learn redirects locale-free URLs to the reader's language (/nl-nl/, /de-de/, ...): never pin a locale.
  // Some of Microsoft's own redirects also leave a double slash in the path (previous-versions//bb205146).
  // ?view=msvc-170 etc. is added when no version is given: leave it off so readers get the current version.
  if (url.host === 'learn.microsoft.com') {
    url.pathname = url.pathname.replace(/^\/[a-z]{2}-[a-z]{2}\//, '/').replace(/\/{2,}/g, '/');
    url.searchParams.delete('view');
  }
  url.hash = fragment ?? url.hash;
  return url.toString();
};

// Redirects into these hosts are temporary download URLs (signed, expiring): keep the original link.
const DOWNLOAD_HOSTS = /^(release-assets\.githubusercontent\.com|objects\.githubusercontent\.com|codeload\.github\.com)$/;
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const isRoot = (u) => /^\/([a-z]{2}-[a-z]{2}\/?)?$/i.test(new URL(u).pathname);

/** One request; retries throttling (403/429) and connection errors with backoff. */
async function request(url) {
  for (let attempt = 0; ; attempt++) {
    try {
      const res = await fetch(url, { redirect: 'manual', headers: { 'User-Agent': UA, Accept: 'text/html,*/*', 'Accept-Language': 'en-US,en' }, signal: AbortSignal.timeout(25000) });
      if ((res.status === 403 || res.status === 429) && attempt < 4) { await sleep(2000 * 2 ** attempt); continue; }
      return res;
    } catch (e) {
      if (attempt < 3) { await sleep(2000 * 2 ** attempt); continue; }
      throw e;
    }
  }
}

async function check(original) {
  const fragment = new URL(original).hash;
  let url = original.replace(/#.*$/, '');
  const chain = [];
  for (let hop = 0; hop < 12; hop++) {
    let res;
    try {
      res = await request(url);
    } catch (e) {
      return { status: 'broken', reason: e.cause?.code ?? e.name ?? String(e), chain };
    }
    chain.push(`${res.status} ${url}`);
    if (res.status >= 300 && res.status < 400 && res.headers.get('location')) {
      const next = new URL(res.headers.get('location'), url).toString();
      if (DOWNLOAD_HOSTS.test(new URL(next).host)) return { status: 'ok', final: original, note: 'download link (kept)', chain };
      url = next;
      continue;
    }
    if (res.status >= 400) return { status: 'broken', reason: `HTTP ${res.status}`, chain };
    const html = /html/.test(res.headers.get('content-type') ?? '') ? await res.text() : '';
    const title = decodeEntities(/<title[^>]*>([^<]*)<\/title>/i.exec(html)?.[1]?.trim() ?? '');
    let final = cleanUrl(url, fragment);
    // Keep a ?view= that the original link chose deliberately (e.g. view=vs-2015 for archived docs).
    const view = new URL(original).searchParams.get('view');
    if (view && new URL(final).host === 'learn.microsoft.com') {
      const f = new URL(final);
      f.searchParams.set('view', view);
      final = f.toString();
    }
    const changed = final !== original;
    // A specific page that now lands on a site root, a search page or a "not found" page.
    const generic = changed && !isRoot(original) && (isRoot(url) || /(^|\/)(search|404|notfound|not-found)(\/|$|\?)|page not found/i.test(new URL(url).pathname + ' ' + title));
    if (generic) return { status: 'suspicious', reason: `redirects to a generic page ("${title}")`, final, chain };
    // #anchors: GitHub renders #L.. line anchors client-side, so they can't be checked in the HTML.
    const anchor = decodeURIComponent(fragment.slice(1));
    let note;
    if (anchor && html && !/github\.com$/.test(new URL(url).host) && !hasId(html, anchor)) {
      if (hasId(html, 'constants')) { final = cleanUrl(url, '#constants'); note = `#${anchor} -> #constants (enum values are listed there)`; }
      else { final = cleanUrl(url, ''); note = `#${anchor} no longer exists; anchor dropped`; }
    }
    return { status: final !== original ? 'redirected' : 'ok', final, title, note, chain };
  }
  return { status: 'broken', reason: 'too many redirects', chain };
}
function hasId(html, id) {
  return new RegExp(`(id|name)=["']${id.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}["']`, 'i').test(html);
}
function decodeEntities(s) {
  return s.replace(/&#(\d+);/g, (_m, n) => String.fromCharCode(+n)).replace(/&amp;/g, '&').replace(/&quot;/g, '"').replace(/&#39;|&apos;/g, "'");
}

const urls = [...occurrences.keys()];
console.log(`${urls.length} unique external URLs in ${files.length} files`);
const results = APPLY ? JSON.parse(fs.readFileSync(path.join(root, 'tools/external-links.json'), 'utf8')) : {};
let done = 0;
// Microsoft rate-limits aggressively: check its hosts one at a time, everything else 6 at a time.
const isMicrosoft = (u) => /(^|\.)microsoft\.com$/.test(new URL(u).host);
async function worker(queue, delay) {
  while (queue.length) {
    const u = queue.shift();
    results[u] = { ...(await check(u)), files: [...occurrences.get(u)] };
    if (++done % 50 === 0) console.log(`  ${done}/${urls.length}`);
    if (delay) await sleep(delay);
  }
}
const ms = APPLY ? [] : urls.filter(isMicrosoft), other = APPLY ? [] : urls.filter((u) => !isMicrosoft(u));
await Promise.all([worker(ms, 400), ...Array.from({ length: 6 }, () => worker(other, 0))]);

const by = (s) => Object.entries(results).filter(([, r]) => r.status === s);
if (!APPLY) fs.writeFileSync(path.join(root, 'tools/external-links.json'), JSON.stringify(results, null, 1));
console.log(`ok ${by('ok').length}, redirected ${by('redirected').length}, suspicious ${by('suspicious').length}, broken ${by('broken').length}`);
for (const [u, r] of by('broken')) console.log(`BROKEN     ${u}\n           ${r.reason}  (${r.files.join(', ')})`);
for (const [u, r] of by('suspicious')) console.log(`SUSPICIOUS ${u}\n           -> ${r.final}\n           ${r.reason}  (${r.files.join(', ')})`);

if (WRITE) {
  let replaced = 0;
  for (const f of files) {
    let md = fs.readFileSync(f, 'utf8');
    for (const [u, r] of by('redirected')) {
      // Replace whole URLs only: as a link target, link text, autolink or attribute. A plain substring
      // replacement would also rewrite longer URLs that start with this one (e.g. the same page + #anchor).
      const re = new RegExp(`(?<=[(\\[<"\\s]|^)${u.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}(?=[)\\]>"\\s]|$)`, 'gm');
      md = md.replace(re, () => { replaced++; return r.final; });
    }
    fs.writeFileSync(f, md);
  }
  console.log(`replaced ${replaced} occurrence(s) of ${by('redirected').length} redirected URL(s)`);
}
