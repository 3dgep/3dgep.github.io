// Convert a WordPress (Crayon/urvanov highlighter) post export into Markdown for MyST or Starlight.
// Usage: node convert.mjs <post.json> <outDir> <myst|starlight> <assetPrefix> [sourceDir]
// Env:   PORTED="slug,alias=slug,..."  WordPress slugs that exist on the new site; links to them become
//        internal links (aliases cover old URLs that WordPress redirected, e.g. learning-directx12-1).
import fs from 'node:fs';
import path from 'node:path';
import { parseHTML } from 'linkedom';
import TurndownService from 'turndown';
import { gfm } from 'turndown-plugin-gfm';

const [, , postFile, outDir, target, assetPrefix, sourceDir] = process.argv;
const MYST = target === 'myst';
const post = JSON.parse(fs.readFileSync(postFile, 'utf8'))[0];
// Repair a broken attribute seen in the originals: href="#cite_19>[19]</a> (missing closing quote).
const html = post.content.rendered.replace(/href="(#[\w-]+)>/g, 'href="$1">');
const { document } = parseHTML(`<!doctype html><html><body>${html}</body></html>`);

const ENT = { lt: '<', gt: '>', quot: '"', '#039': "'", '#39': "'", '#038': '&', amp: '&', nbsp: ' ' };
const decode = (s) => s.replace(/&(lt|gt|quot|#0?39|#038|amp|nbsp);/g, (_m, e) => ENT[e]);

// Ported articles: WordPress slug (or old alias) -> new slug.
const ported = new Map();
for (const entry of (process.env.PORTED ?? '').split(',').filter(Boolean)) {
  const [from, to = from] = entry.split('=');
  ported.set(from, to);
}

const assets = new Map(); // remote url -> local file name
function localAsset(url) {
  url = url.replace(/\?.*$/, '');
  const name = decodeURIComponent(path.basename(new URL(url).pathname)).replace(/[^\w.-]+/g, '_');
  assets.set(url, name);
  return `${assetPrefix}${name}`;
}
function imageUrl(img) {
  // Prefer the full-size upload linked by the surrounding anchor, else strip the -WxH thumbnail suffix.
  const a = img.closest('a');
  const href = a?.getAttribute('href') || '';
  const url = /3dgep\.com\/wp-content\/uploads\/.*\.(png|jpe?g|gif|webp)$/i.test(href) ? href : img.getAttribute('src');
  return url.replace(/-\d+x\d+(\.\w+)$/, '$1');
}
const slugify = (s) => s.toLowerCase().replace(/_/g, '-').replace(/[^\w-]+/g, '');

const LANG = { 'C++': 'cpp', HLSL: 'hlsl', 'C#': 'csharp', CMake: 'cmake', Batch: 'bat', XML: 'xml', 'Plain Text': 'text' };
// A file name in the code block title is a better hint than Crayon's language (HLSL was often tagged C++).
const EXT_LANG = { hlsl: 'hlsl', hlsli: 'hlsl', glsl: 'glsl', vert: 'glsl', frag: 'glsl', cpp: 'cpp', h: 'cpp', hpp: 'cpp', c: 'c', cs: 'csharp', cmake: 'cmake', txt: 'text', bat: 'bat', json: 'json', xml: 'xml' };
const ADMONITION = { sab_info: 'note', sab_warning: 'warning', sab_danger: 'danger', sab_success: 'tip' };
const ASIDE = { note: 'note', warning: 'caution', danger: 'danger', tip: 'tip' };
const fence = (code) => '`'.repeat(Math.max(3, ...[...code.matchAll(/`{3,}/g)].map((m) => m[0].length + 1)));

// Optional: directory with the tutorial's source files (e.g. main.cpp, Helpers.h at the tagged release).
// Code blocks titled with a file name get line numbers matching that file.
const sources = new Map();
if (sourceDir) for (const f of fs.readdirSync(sourceDir)) sources.set(f, fs.readFileSync(path.join(sourceDir, f), 'utf8').split(/\r?\n/));
// Env SOURCE_ALIASES="ArticleName.cpp=RealName.cpp,...": files that were renamed after the article was written.
const sourceAliases = new Map((process.env.SOURCE_ALIASES ?? '').split(',').filter(Boolean).map((e) => e.split('=')));
const lastPos = new Map(); // file -> line index after the previous matched snippet (the article walks the file in order)
const norm = (l) => l.trim().replace(/\s+/g, ' ');

/** Find the 1-based start line of `code` in `file`; returns { start, exact } or null. */
function locate(code, file) {
  const src = sources.get(file);
  if (!src) return null;
  const snip = code.split('\n').map(norm);
  const lines = src.map(norm);
  // Score every alignment by the number of non-blank snippet lines that equal the file line at the same
  // offset, so small edits in the article (a changed argument, reordered statements) don't prevent a match.
  const significant = snip.filter((l) => l && l !== '{' && l !== '}').length || 1;
  let best = null;
  for (let i = 0; i < lines.length; i++) {
    let score = 0;
    for (let k = 0; k < snip.length; k++) if (snip[k] && snip[k] !== '{' && snip[k] !== '}' && lines[i + k] === snip[k]) score++;
    if (!score) continue;
    // Prefer more matching lines, then positions after the previous snippet, then the earliest.
    const after = i >= (lastPos.get(file) ?? 0);
    if (!best || score > best.score || (score === best.score && after && !best.after)) best = { i, score, after };
  }
  if (!best || best.score < significant / 2) return null;
  lastPos.set(file, best.i + snip.length);
  best.score = snip.filter((l, k) => l === lines[best.i + k]).length;
  // "exact" is stricter than the score: indentation must match too (tabs count as 4 spaces), so a
  // snippet is only replaced by an import when the imported code would look the same.
  const layout = (l) => (l ?? '').replace(/\t/g, '    ').replace(/\s+$/, '');
  const raw = code.split('\n');
  const exact = raw.every((l, k) => layout(l) === layout(src[best.i + k]));
  return { start: best.i + 1, exact, matched: best.score, total: snip.length };
}
const lineReport = [];

// Optional: the raw post (/wp-json/wp/v2/posts/<id>, env RAW_POST) has every code block as
// <pre class="brush:cpp;first-line:25"> / <pre class="lang:c++ start-line:25 mark:3,5-7">, in the same order
// as the rendered Crayon blocks: the line numbers (and highlighted lines) the article originally showed.
// Blocks marked crayon:false were never highlighted by Crayon (they stay plain <pre> in the rendered post), so
// they are skipped here to keep the one-to-one pairing.
const rawBlocks = process.env.RAW_POST
  ? [...JSON.parse(fs.readFileSync(process.env.RAW_POST, 'utf8')).content.rendered.matchAll(/<pre([^>]*)>/g)]
      .filter((m) => !/crayon:\s*false/.test(m[1]))
      .map((m) => ({
        first: +(/(?:first|start)-line:\s*(\d+)/.exec(m[1])?.[1] ?? 1),
        mark: /mark:\s*([\d,\s-]+)/.exec(m[1])?.[1]?.replace(/\s/g, ''),
      }))
  : null;
const startDisagreements = [];

// 1. Pull Crayon code blocks out *before* turndown (it collapses whitespace) and leave placeholders.
const codeBlocks = [];
const crayonBlocks = document.querySelectorAll('div.urvanov-syntax-highlighter-syntax');
if (rawBlocks && rawBlocks.length !== crayonBlocks.length) {
  throw new Error(`RAW_POST has ${rawBlocks.length} code blocks, the rendered post ${crayonBlocks.length}: not the same post?`);
}
for (const n of crayonBlocks) {
  const code = decode(n.querySelector('textarea.urvanov-syntax-highlighter-plain').textContent).replace(/^\n/, '').replace(/\s+$/, '');
  const title = n.querySelector('.crayon-title')?.textContent.trim() || '';
  const ext = /\.(\w+)$/.exec(title)?.[1]?.toLowerCase();
  let lang = EXT_LANG[ext] ?? LANG[n.querySelector('.crayon-language')?.textContent.trim()] ?? 'text';
  if (/^\s*(method|function) /m.test(code) && /^\s*end /m.test(code)) lang = 'text'; // pseudocode
  const f = fence(code);
  // SOURCE_ALIASES maps a file name used in the article to the file's current name in the sources.
  const file = sourceAliases.get(title) ?? title;
  const loc = locate(code, file);
  const raw = rawBlocks?.[codeBlocks.length];
  if (sources.has(file)) lineReport.push({ block: codeBlocks.length + 1, title, ...(loc ?? { start: null }) });
  if (loc && raw && loc.start !== raw.first) startDisagreements.push({ block: codeBlocks.length + 1, title, found: loc.start, original: raw.first, exact: loc.exact });
  // Line numbers: where the snippet is found in the source, that's authoritative; otherwise use the line
  // number the original article showed (if known).
  const start = (loc?.start ?? raw?.first ?? 1) > 1 ? (loc?.start ?? raw.first) : 0;
  const mark = raw?.mark ? `{${raw.mark}}` : '';
  let md;
  if (MYST) {
    const opts = [title && `:caption: ${title}`, ':linenos:', start && `:lineno-start: ${start}`].filter(Boolean).join('\n');
    md = `${f}{code-block} ${lang}\n${opts}\n\n${code}\n${f}`;
  } else if (loc?.exact && process.env.SOURCE_REF) {
    // Snippet is an exact copy of the tagged source: import it at build time instead of copying it
    // (remark-code-from-source fills in the code, title and line numbers).
    const last = loc.start + loc.total - 1;
    md = `${f}${lang} source="${process.env.SOURCE_REF}/${file}#L${loc.start}-L${last}"${mark ? ' ' + mark : ''}\n${f}`;
  } else {
    const meta = [title && `title="${title}"`, 'showLineNumbers', start && `startLineNumber=${start}`, mark].filter(Boolean).join(' ');
    md = `${f}${lang} ${meta}\n${code}\n${f}`;
  }
  const p = document.createElement('p');
  p.textContent = `CODEBLOCK${codeBlocks.push(md) - 1}PLACEHOLDER`;
  n.replaceWith(p);
}
// Plain <pre> blocks that Crayon left alone (class="crayon:false"), e.g. grammar notation like
// "<strong>requires</strong> { <em>requirement-seq</em> }": kept as an HTML <pre class="grammar"> with only its
// bold/italic markup, so keywords and placeholders stay distinguishable (Markdown code blocks can't do that).
// Turndown would otherwise treat a <pre> without <code> as text and collapse its line breaks.
const plainBlocks = [];
for (const n of document.querySelectorAll('pre')) {
  if (n.closest('div.urvanov-syntax-highlighter-syntax')) continue;
  const inner = n.innerHTML
    .replace(/\r/g, '')
    .replace(/<(\/?)(?:strong|b)\b[^>]*>/g, '\u0001$1strong\u0002')
    .replace(/<(\/?)(?:em|i)\b[^>]*>/g, '\u0001$1em\u0002')
    .replace(/<br\s*\/?>/g, '\n')
    .replace(/<[^>]+>/g, '') // any other markup
    .replace(/\u0001/g, '<').replace(/\u0002/g, '>')
    .replace(/^(?:[ \t]*\n)+/, '').replace(/\s+$/, ''); // drop leading blank lines and trailing whitespace
  const md = `<pre class="grammar">${inner}</pre>`;
  const p = document.createElement('p');
  p.textContent = `CODEBLOCK${codeBlocks.push(md) - 1}PLACEHOLDER`;
  plainBlocks.push(n);
  n.replaceWith(p);
}

// 2. Anchor targets: empty <a id="cite-6"></a> / <a name="cite_6"></a> / <a name="WNDCLASSEX"></a> (reference
//    entries) -> placeholders (turndown drops empty anchors). cite_N/cite-N become "cite-N"; other names become
//    "ref-<name>" so they can't collide with heading ids.
const namedAnchors = new Map(); // anchorKey(name) -> id
function anchorKey(s) { return s.toLowerCase().replace(/[^\p{L}\p{N}]+/gu, ''); }
for (const a of document.querySelectorAll('a[id], a[name]')) {
  if (a.getAttribute('href') || a.textContent.trim() || a.querySelector('img')) continue;
  const name = a.getAttribute('id') || a.getAttribute('name');
  const cite = /^cite[-_=]?(\d+)$/i.exec(name);
  const id = cite ? `cite-${cite[1]}` : `ref-${slugify(name)}`;
  namedAnchors.set(anchorKey(name), id);
  a.replaceWith(document.createTextNode(`CITETARGET${id}END`));
}
// Ids on other inline/text elements (e.g. footnote markers <sup id="q1">, answers <p id="a1">) are kept as an
// anchor at the start of the element. Headings, captions and code blocks get their ids elsewhere.
for (const el of document.querySelectorAll('sup[id], sub[id], span[id], p[id], li[id], td[id], em[id], strong[id]')) {
  const name = el.getAttribute('id');
  if (/^(caption-attachment|attachment|urvanov|crayon|video|audio)/.test(name) || namedAnchors.has(anchorKey(name))) continue;
  const id = `ref-${slugify(name)}`;
  namedAnchors.set(anchorKey(name), id);
  el.insertBefore(document.createTextNode(`CITETARGET${id}END`), el.firstChild);
}

// 3. Math (QuickLaTeX/MathJax style \( \) and \[ \]) -> placeholders, so turndown doesn't escape the LaTeX.
//    Code blocks are already placeholders, so only prose is scanned. WordPress inserts <br /> in display math.
const math = [];
// KaTeX (unlike MathJax) rejects a bare "_" inside \text{...}: escape it (\text{BLOCK_SIZE} -> \text{BLOCK\_SIZE}).
// WordPress "texturized" the LaTeX too (v' -> v’, a-b -> a–b, ... -> …): undo that. MathJax also accepted the
// typo \begin{array}{1} for {l}.
const TEXTURIZED = { '’': "'", '‘': '`', '“': "''", '”': "''", '–': '-', '—': '-', '…': '\\ldots ', '×': '\\times ' };
const katexSafe = (tex) =>
  tex
    .replace(/\\text\{([^{}]*)\}/g, (_m, t) => `\\text{${t.replace(/(?<!\\)_/g, '\\_')}}`)
    .replace(/[’‘“”–—…×]/g, (c) => TEXTURIZED[c])
    .replace(/\\begin\{array\}\{([1lcr|]+)\}/g, (_m, cols) => `\\begin{array}{${cols.replace(/1/g, 'l')}}`);
const bodyHtml = document.body.innerHTML
  .replace(/\\\[([\s\S]+?)\\\]/g, (_m, tex) => {
    const clean = decode(tex.replace(/<br\s*\/?>/g, '').replace(/<\/?p>/g, '')).trim();
    return `MATHDISPLAY${math.push(katexSafe(clean)) - 1}END`;
  })
  .replace(/\\\((.+?)\\\)/g, (_m, tex) => `MATHINLINE${math.push(katexSafe(decode(tex).trim())) - 1}END`);
document.body.innerHTML = bodyHtml;

// 4. Heading anchors. WordPress anchors are inconsistent (#CommandQueue::GetCommandList,
//    #Tutorial2UpdateBufferResource, #Shader_Register_Register_Spaces for "Shader Register & Register Spaces",
//    #Pipeline_State_Object_1 for the 2nd heading with that text), so resolve each anchor to the heading it
//    means and emit the id that the target generator actually produces:
//    - Starlight (github-slugger): lowercase, drop punctuation, each space -> "-", duplicates get "-1", "-2".
//    - MyST: lowercase, runs of non-alphanumerics -> "-"; it does not de-duplicate, so repeated headings get
//      an explicit label. MyST labels are global to the site, so they are prefixed with the article slug.
const LABEL_PREFIX = MYST ? `${post.slug}-` : '';
const label = (id) => LABEL_PREFIX + id;
const key = (s) => s.toLowerCase().replace(/[^\p{L}\p{N}]+/gu, '');
const githubSlug = (s) => s.toLowerCase().replace(/[^\p{L}\p{N}\s_-]/gu, '').replace(/ /g, '-');
const mystSlug = (s) => s.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '');
const headingIds = new Map(); // key(text) + "#" + occurrence -> id
const headingLabels = new Map(); // key(text) + "#" + occurrence -> explicit MyST label (duplicates only)
{
  const seen = new Map();
  const used = new Map();
  for (const h of document.querySelectorAll('h1, h2, h3, h4, h5')) {
    const text = h.textContent.trim();
    const k = key(text);
    const occurrence = seen.get(k) ?? 0;
    seen.set(k, occurrence + 1);
    let id;
    if (MYST) {
      id = mystSlug(text);
      if (occurrence > 0) { id = label(`${id}-${occurrence}`); headingLabels.set(`${k}#${occurrence}`, id); }
    } else {
      const base = githubSlug(text);
      const n = used.get(base) ?? 0;
      used.set(base, n + 1);
      id = n ? `${base}-${n}` : base;
    }
    headingIds.set(`${k}#${occurrence}`, id);
  }
}
/** Resolve a WordPress in-page anchor (without "#") to the target id, or null. */
function resolveAnchor(anchor) {
  const named = namedAnchors.get(anchorKey(anchor));
  if (named) return label(named);
  const cite = /^cite[-_=]?(\d+)$/i.exec(anchor);
  if (cite) return label(`cite-${cite[1]}`);
  const attach = /^attachment[-_](\d+)$/.exec(anchor);
  if (attach) return label(`attachment-${attach[1]}`);
  const dup = /^(.*?)_(\d+)$/.exec(anchor);
  return headingIds.get(`${key(anchor)}#0`)
    ?? (dup && headingIds.get(`${key(dup[1])}#${dup[2]}`))
    ?? null;
}
const unresolvedAnchors = new Set();

const td = new TurndownService({ headingStyle: 'atx', codeBlockStyle: 'fenced', bulletListMarker: '-', emDelimiter: '*' });
td.use(gfm);
// A Markdown table row is one line: a <br> inside a cell would end the row, so keep it as HTML.
td.addRule('cellBreak', {
  filter: (n) => n.nodeName === 'BR' && !!n.parentNode?.closest?.('td, th'),
  replacement: () => '<br />',
});

td.addRule('alerts', {
  filter: (n) => n.nodeName === 'DIV' && n.className.includes('simple-alert-boxes'),
  replacement: (content, n) => {
    const kind = ADMONITION[n.className.split(/\s+/).find((c) => ADMONITION[c])] ?? 'note';
    return MYST ? `\n\n:::{${kind}}\n${content.trim()}\n:::\n\n` : `\n\n:::${ASIDE[kind]}\n${content.trim()}\n:::\n\n`;
  },
});

td.addRule('captions', {
  filter: (n) => n.nodeName === 'DIV' && n.className.includes('wp-caption'),
  replacement: (_c, n) => {
    const img = n.querySelector('img');
    const captionText = () => td.turndown(n.querySelector('.wp-caption-text')?.innerHTML || '').trim();
    if (!img) {
      // Captioned embed. Lesson 2 builds the iframe from a script: the WebGL demo, or a YouTube video when
      // WebGL is unavailable. Every current browser has WebGL, so use the first (demo) iframe.
      const iframeSrc = n.querySelector('iframe')?.getAttribute('src')
        ?? /<iframe[^>]*\ssrc="([^"]+)"/.exec(n.querySelector('script')?.textContent ?? '')?.[1];
      if (!iframeSrc) return `\n\n${captionText()}\n\n`;
      const cap = captionText();
      if (MYST) return embed(iframeSrc).replace(':width: 100%\n', `:width: 100%\n\n${cap}\n`);
      return `${embed(iframeSrc)}<div class="caption">\n\n${cap}\n\n</div>\n\n`;
    }
    const src = localAsset(imageUrl(img));
    const alt = (img.getAttribute('alt') || '').replace(/[[\]]/g, '');
    const caption = td.turndown(n.querySelector('.wp-caption-text')?.innerHTML || '').trim()
      // Bare URLs: wrap in <...> so GFM autolinking doesn't swallow surrounding ")" or "\]".
      .replace(/(^|[\s(])(https?:\/\/[^\s)<>\\]+)/g, '$1<$2>');
    const label = n.id ? resolveAnchor(n.id) ?? slugify(n.id) : '';
    const thumb = thumbWidth(img);
    if (MYST) return `\n\n:::{figure} ${src}\n${label ? `:label: ${label}\n` : ''}:alt: ${alt}\n${thumb ? `:width: ${thumb}px\n:align: left\n` : ''}\n${caption}\n:::\n\n`;
    if (thumb) return `\n\n${label ? `<span id="${label}"></span>` : ''}<img src="${thumbSrc(src)}" alt="${alt}" width="${thumb}" class="thumb" />\n\n`;
    return `\n\n${label ? `<span id="${label}"></span>\n\n` : ''}![${alt}](${src})\n\n<div class="caption">

${caption}

</div>\n\n`;
  },
});

// WordPress thumbnails (≤ 200px, usually floated left) keep their size instead of rendering full width.
// Astro does not process raw <img> in Markdown, so Starlight thumbnails are served from public/images/.
const thumbSrc = (src) => (MYST ? src : `/images/${path.basename(src)}`);
function thumbWidth(img) {
  const w = +img.getAttribute('width');
  return w && w <= 200 ? w : 0;
}

td.addRule('images', {
  filter: 'img',
  replacement: (_c, n) => {
    const alt = (n.getAttribute('alt') || '').replace(/[[\]"]/g, '');
    // The black GitHub mark is invisible in dark mode: use the Octicons mark in a light and a dark variant
    // (public/images/github-mark*.svg; .light-only/.dark-only are toggled by the theme in custom.css).
    if (/GitHub-Mark/i.test(n.getAttribute('src') || '')) {
      const w = thumbWidth(n) || 120;
      return `<img src="/images/github-mark.svg" alt="GitHub" width="${w}" class="thumb light-only" />`
        + `<img src="/images/github-mark-white.svg" alt="GitHub" width="${w}" class="thumb dark-only" />`;
    }
    const src = localAsset(imageUrl(n));
    const thumb = thumbWidth(n);
    if (!thumb) return `![${alt}](${src})`;
    return `<img src="${thumbSrc(src)}" alt="${alt}" width="${thumb}" class="thumb" />`;
  },
});

td.addRule('imageLinks', {
  filter: (n) => n.nodeName === 'A' && n.querySelector('img') && !n.textContent.trim() && /wp-content\/uploads/.test(n.getAttribute('href') || ''),
  replacement: (content) => content,
});

// WordPress [video] shortcodes: self-host the MP4 under /videos/ (copy the files listed at the end).
const videos = [];
td.addRule('video', {
  filter: 'video',
  replacement: (_c, n) => {
    const url = (n.querySelector('source')?.getAttribute('src') || n.getAttribute('src') || '').replace(/\?.*$/, '');
    const name = path.basename(new URL(url).pathname);
    videos.push(url);
    const w = n.getAttribute('width'), h = n.getAttribute('height');
    const attrs = [n.hasAttribute('loop') && 'loop', 'controls', 'preload="metadata"', 'playsinline'].filter(Boolean).join(' ');
    const size = w && h ? ` width="${w}" height="${h}"` : '';
    return `\n\n<video class="embed-video-file" src="/videos/${name}"${size} ${attrs}></video>\n\n`;
  },
});

td.addRule('audio', {
  filter: 'audio',
  replacement: (_c, n) => {
    const src = localAsset(n.querySelector('source').getAttribute('src'));
    return MYST ? `\n\n[Play: ${path.basename(src)}](${src})\n\n` : `\n\n<audio controls preload="none" src="${src.replace('../../assets/', '/audio/')}"></audio>\n\n`;
  },
});

// Embedded iframes: demos hosted on 3dgep.com are self-hosted under /demos/; YouTube uses the
// privacy-enhanced youtube-nocookie.com domain.
const embeds = [];
td.addRule('iframes', {
  filter: 'iframe',
  replacement: (_c, n) => embed(n.getAttribute('src') || '', n.getAttribute('width'), n.getAttribute('height')),
});
function embed(src, width, height) {
  let kind = 'embed';
  const yt = /youtube\.com\/embed\/([\w-]+)/.exec(src);
  if (yt) { src = `https://www.youtube-nocookie.com/embed/${yt[1]}?rel=0`; kind = 'video'; }
  const demo = /3dgep\.com\/(demos\/.+)$/.exec(src);
  if (demo) { src = `/${demo[1]}`; kind = 'demo'; embeds.push(demo[1]); }
  if (/desmos\.com/.test(src)) kind = 'desmos';
  const title = { video: 'YouTube video', demo: 'Interactive demo', desmos: 'Desmos graph' }[kind] ?? 'Embedded content';
  if (MYST) return `\n\n:::{iframe} ${src}\n:width: 100%\n:::\n\n`;
  // Other embeds (e.g. Desmos) keep their original size, scaled down on narrow screens.
  const w = parseInt(width, 10), h = parseInt(height, 10);
  const size = kind !== 'video' && kind !== 'demo' && w && h ? ` style="max-width: ${w}px; aspect-ratio: ${w} / ${h}"` : '';
  return `\n\n<iframe class="embed embed-${kind}" src="${src}" title="${title}"${size} loading="lazy" allowfullscreen></iframe>\n\n`;
}

// Links to other 3dgep.com articles that have been ported become internal links.
// Env WP_POSTS=<posts.json from /wp-json/wp/v2/posts> resolves "?p=<id>" links to slugs.
const postSlugs = new Map(process.env.WP_POSTS ? JSON.parse(fs.readFileSync(process.env.WP_POSTS, 'utf8')).map((p) => [String(p.id), p.slug]) : []);
// Env SITE_DOCS=<src/content/docs>: anchors into other ported articles are resolved against that article's
// headings (same rules as in-page anchors), e.g. #Materials_Properties -> #materials-properties.
const otherHeadings = new Map(); // slug -> Map(key#occurrence -> id)
function headingsOf(slug) {
  if (!otherHeadings.has(slug)) {
    const ids = new Map();
    const file = process.env.SITE_DOCS && path.join(process.env.SITE_DOCS, `${slug}.md`);
    if (file && fs.existsSync(file)) {
      const seen = new Map(), used = new Map();
      const body = fs.readFileSync(file, 'utf8').replace(/^(`{3,})[\s\S]*?^\1$/gm, ''); // skip code blocks
      for (const m of body.matchAll(/^#{2,6} (.+)$/gm)) {
        const text = m[1].replace(/\\(.)/g, '$1').replace(/`/g, '').trim();
        const k = key(text), occurrence = seen.get(k) ?? 0;
        seen.set(k, occurrence + 1);
        const base = githubSlug(text), n = used.get(base) ?? 0;
        used.set(base, n + 1);
        ids.set(`${k}#${occurrence}`, n ? `${base}-${n}` : base);
      }
    }
    otherHeadings.set(slug, ids);
  }
  return otherHeadings.get(slug);
}
function internalHref(href) {
  const m = /^https?:\/\/(?:www\.)?3dgep\.com\/(?:\?p=(\d+)|([\w-]+)\/?)(#.*)?$/.exec(href);
  if (!m) return null;
  const wpSlug = m[1] ? postSlugs.get(m[1]) : m[2];
  if (!wpSlug) return null;
  if (!ported.has(wpSlug)) return m[1] ? `https://www.3dgep.com/${wpSlug}/${m[3] ?? ''}` : null; // ?p= -> real URL
  const slug = ported.get(wpSlug);
  let hash = '';
  if (m[3]) {
    const anchor = decodeURIComponent(m[3].slice(1));
    const ids = MYST ? new Map() : headingsOf(slug);
    const dup = /^(.*?)[-_](\d+)$/.exec(anchor);
    const id = ids.get(`${key(anchor)}#0`) ?? (dup && ids.get(`${key(dup[1])}#${dup[2] - (/-/.test(anchor.slice(-dup[2].length - 1, -dup[2].length)) ? 1 : 0)}`));
    if (!id && ids.size) unresolvedAnchors.add(`${slug}#${anchor}`);
    hash = '#' + (id ?? slugify(anchor));
  }
  return MYST ? `${slug}.md${hash}` : `/${slug}/${hash}`;
}

// <code><a>x</a></code> -> [`x`](url) so the link survives.
// Code containing links, e.g. <code><a>D3D12_COMMAND_LIST_TYPE</a> Type</code>: Markdown can't put a
// link inside a code span, so split into adjacent spans: [`D3D12_COMMAND_LIST_TYPE`](url) `Type`.
td.addRule('codeLinks', {
  filter: (n) => n.nodeName === 'CODE' && n.parentNode?.nodeName !== 'PRE' && !!n.querySelector('a'),
  replacement: (_c, n) => {
    const tick = (s) => {
      const t = s.trim();
      if (!t) return s.replace(/\s+/g, ' ');
      const lead = /^\s/.test(s) ? ' ' : '', trail = /\s$/.test(s) ? ' ' : '';
      return `${lead}\`${t}\`${trail}`;
    };
    return [...n.childNodes].map((c) => (c.nodeName === 'A'
      ? `[\`${c.textContent.trim()}\`](${fixHref(c.getAttribute('href'))})`
      : tick(c.textContent))).join('');
  },
});

// In-page anchors (#Check_for_Tearing_Support, #cite-6, cite-9 / XInput without "#") -> the target's ids.
// `text` is the link text: a citation like "[62]" whose anchor doesn't exist falls back to that reference entry.
function fixHref(href, text = '') {
  href = href.trim(); // e.g. href="\n#DrawIndexed"
  const internal = internalHref(href);
  if (internal) return internal;
  const bare = !/^[a-z][\w+.-]*:|^[/.?#]/i.test(href) && /^[\w-]+$/.test(href); // "XInput": anchor without "#"
  const anchor = href.startsWith('#') ? href.slice(1) : (bare && (namedAnchors.has(anchorKey(href)) || /^cite-\d+$/.test(href))) ? href : null;
  if (anchor === null) return href;
  const id = resolveAnchor(decodeURIComponent(anchor));
  if (id) return '#' + id;
  const cite = /^\\?\[(\d+)\\?\]$/.exec(text.trim());
  if (cite) return '#' + label(`cite-${cite[1]}`);
  unresolvedAnchors.add(anchor);
  return '#' + slugify(anchor);
}
// Files in WordPress uploads disappear with the domain: links to them become /downloads/<file> (copy the
// files listed at the end). An image linked to its own upload (click to enlarge) is just the image.
const downloads = new Map(); // upload URL -> local path
function uploadHref(href) {
  if (!/^https?:\/\/(?:www\.)?3dgep\.com\/wp-content\/uploads\//.test(href)) return null;
  const url = href.replace(/[?#].*$/, '');
  const name = decodeURIComponent(path.basename(new URL(url).pathname));
  if (/\.(mp4|webm|mov)$/i.test(name)) { videos.push(url); return `/videos/${name}`; } // same place as <video>
  const local = `/downloads/${name}`;
  downloads.set(url, local);
  return local;
}
// Note: turndown gives rules added later priority, so this rule also covers image-only links.
td.addRule('links', {
  filter: (n) => n.nodeName === 'A' && n.getAttribute('href'),
  // Line breaks (<br>) inside link text: WordPress used them to stack an icon above its label; they'd split
  // the Markdown link over several lines, so drop them.
  replacement: (content, n) => {
    const href = n.getAttribute('href');
    if (n.querySelector('img') && !n.textContent.trim() && /wp-content\/uploads/.test(href)) return content;
    const target = uploadHref(href) ?? fixHref(href, n.textContent);
    return `[${content.replace(/ {2}\n/g, ' ').replace(/\s+/g, ' ').replace(/(\/>) /g, '$1').trim()}](${target.replace(/[()]/g, encodeURIComponent)})`;
  },
});

// WordPress uses <h1> for top-level sections; the page title is the only H1 in the new site.
// MyST: repeated headings get an explicit (prefixed) label, since MyST doesn't de-duplicate heading ids.
const headingSeen = new Map();
td.addRule('headings', {
  filter: ['h1', 'h2', 'h3', 'h4', 'h5'],
  replacement: (content, n) => {
    const k = key(n.textContent.trim());
    const occurrence = headingSeen.get(k) ?? 0;
    headingSeen.set(k, occurrence + 1);
    const explicit = headingLabels.get(`${k}#${occurrence}`);
    return `\n\n${explicit ? `(${explicit})=\n` : ''}${'#'.repeat(+n.nodeName[1] + 1)} ${content.trim()}\n\n`;
  },
});

const anchorsAdded = [];
let md = td.turndown(document.body.innerHTML);
md = md
  .replace(/CODEBLOCK(\d+)PLACEHOLDER/g, (_m, i) => codeBlocks[+i])
  .replace(/CITETARGET([\w-]+)END\s*/g, (_m, id) => (MYST ? `(${label(id)})=\n` : `<span id="${id}"></span>`))
  // Reference entries ("[11] Microsoft, ...") that the original never gave an anchor: add one, so
  // links like [11](#cite-11) have a target.
  // (Entries that do have one start with <span id=…> in Starlight, or follow a "(label)=" line in MyST.)
  .replace(/^\\\[(\d+)\\\] /gm, (m, n, offset, all) => {
    if (/\)=\n$/.test(all.slice(Math.max(0, offset - 3), offset))) return m;
    anchorsAdded.push(n);
    return MYST ? `(${label(`cite-${n}`)})=\n${m}` : `<span id="cite-${n}"></span>${m}`;
  })
  // Reference lists written as one paragraph with <br> between entries: one paragraph per entry, so the hover
  // preview of a citation shows just that entry.
  .replace(/ {2}\n(?=<span id="(?:cite|ref)-)/g, '\n\n')
  .replace(/MATHDISPLAY(\d+)END/g, (_m, i) => `\n\n$$\n${math[+i]}\n$$\n\n`)
  .replace(/MATHINLINE(\d+)END/g, (_m, i) => `$${math[+i]}$`)
  .replace(/\n{3,}/g, '\n\n');

// (A <div> wrapper: excerpts are <p>…</p> themselves, and a <p> inside a <p> would end up empty.)
const text = (html) => parseHTML(`<div>${html}</div>`).document.querySelector('div').textContent.trim();
const title = text(post.title.rendered);
// Description: the WordPress excerpt (first sentences), falling back to the first paragraph.
// Theme excerpts end with "… Continue reading" (a link to the post): drop it.
let description = text((post.excerpt?.rendered ?? '').replace(/<a [^>]*>\s*(?:Continue|Read more)[\s\S]*?<\/a>/gi, '')).replace(/\s*\[(…|&hellip;)\]\s*$/, '').replace(/\s+/g, ' ');
if (!description) description = text(document.querySelector('p')?.innerHTML ?? '');
if (description.length > 200) description = description.slice(0, 200).replace(/\s+\S*$/, '') + '…';
description = description.replace(/"/g, '\\"');
const front = MYST
  ? `---\ntitle: "${title}"\ndescription: "${description}"\ndate: ${post.date.slice(0, 10)}\nauthors:\n  - Jeremiah van Oosten\ntags: [DirectX 12, Graphics Programming]\n---\n\n`
  : `---\ntitle: "${title}"\ndescription: "${description}"\nslug: ${post.slug}\nlastUpdated: ${post.modified.slice(0, 10)}\n---\n\n`;

fs.mkdirSync(outDir, { recursive: true });
fs.writeFileSync(path.join(outDir, `${post.slug}.md`), front + md.trim() + '\n');
fs.writeFileSync(path.join(outDir, 'assets.json'), JSON.stringify([...assets], null, 2));
console.log(`wrote ${post.slug}.md: ${codeBlocks.length} code blocks, ${assets.size} assets, ${math.length} math, ${embeds.length} self-hosted demos`);
if (embeds.length) console.log(`  copy these demos (and their scripts) into the site: ${embeds.join(', ')}`);

if (lineReport.length) {
  const bad = lineReport.filter((r) => !r.exact);
  console.log(`line numbers: ${lineReport.length - bad.length}/${lineReport.length} snippets match the source exactly`);
  for (const r of bad) console.log(`  block ${r.block} (${r.title}): ${r.start ? `start ${r.start}, ${r.matched}/${r.total} lines match` : 'not found'}`);
}
if (anchorsAdded.length) console.log(`  added missing anchors for reference entries: ${anchorsAdded.map((n) => `[${n}]`).join(', ')}`);
if (unresolvedAnchors.size) console.log(`  in-page links without a target in the original: ${[...unresolvedAnchors].map((a) => '#' + a).join(', ')}`);
if (startDisagreements.length) {
  console.log(`  start lines that differ from the original article (${startDisagreements.length}):`);
  for (const d of startDisagreements) console.log(`    block ${d.block} (${d.title}): found at ${d.found}, article showed ${d.original}${d.exact ? '' : ' (snippet differs from source)'}`);
}
if (videos.length) console.log(`  copy these videos to public/videos/: ${videos.join(', ')}`);
if (downloads.size) console.log(`  copy these files to public/downloads/: ${[...downloads.keys()].join(', ')}`);
