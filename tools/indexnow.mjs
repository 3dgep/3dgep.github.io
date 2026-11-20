// Tells search engines (Bing, Yandex, … via IndexNow) which pages changed in a deploy, so they recrawl them
// right away instead of waiting for the next crawl. Run by the deploy workflow after a push to `publish`:
//   node tools/indexnow.mjs <before-sha> <after-sha> [--dry-run]
// A page counts as changed when its Markdown changed, a code source it imports (`source="dir/…"`) changed,
// or a demo it embeds (`/demos/name/`) changed. The key file is public/<KEY>.txt (the key is public by design).
import { execFileSync } from 'node:child_process';
import { existsSync, readFileSync, readdirSync } from 'node:fs';
import path from 'node:path';

const KEY = '85ac7da1e51c0789664e12f4acaf1c44';
const HOST = 'www.3dgep.com';
const DOCS = 'src/content/docs';

const [before, after, flag] = process.argv.slice(2);
const dryRun = flag === '--dry-run';
if (!before || !after || /^0+$/.test(before)) {
	console.log('IndexNow: no previous commit to compare with; nothing to submit.');
	process.exit(0);
}

const changed = execFileSync('git', ['diff', '--name-only', before, after], { encoding: 'utf8' })
	.split('\n')
	.filter(Boolean);

// slug of every article, and its text (to find what it imports and embeds)
const docs = readdirSync(DOCS)
	.filter((f) => f.endsWith('.md') || f.endsWith('.mdx'))
	.map((f) => {
		const text = readFileSync(path.join(DOCS, f), 'utf8');
		const slug = text.match(/^slug:\s*["']?([^"'\n]+?)["']?\s*$/m)?.[1] ?? f.replace(/\.mdx?$/, '');
		return { file: `${DOCS}/${f}`, slug: slug === 'index' ? '' : slug, text };
	});

const pages = new Set();
for (const file of changed) {
	if (!existsSync(file)) continue; // deleted
	for (const doc of docs) {
		if (file === doc.file) pages.add(doc.slug);
		const source = file.match(/^sources\/([^/]+)\//)?.[1];
		if (source && doc.text.includes(`source="${source}/`)) pages.add(doc.slug);
		const demo = file.match(/^public\/demos\/([^/]+)\//)?.[1];
		if (demo && doc.text.includes(`/demos/${demo}/`)) pages.add(doc.slug);
	}
}

const urlList = [...pages].sort().map((slug) => `https://${HOST}/${slug ? slug + '/' : ''}`);
if (!urlList.length) {
	console.log('IndexNow: no pages changed.');
	process.exit(0);
}
console.log(`IndexNow: ${urlList.length} changed page(s):\n  ${urlList.join('\n  ')}`);
if (dryRun) process.exit(0);

const response = await fetch('https://api.indexnow.org/indexnow', {
	method: 'POST',
	headers: { 'Content-Type': 'application/json; charset=utf-8' },
	body: JSON.stringify({ host: HOST, key: KEY, keyLocation: `https://${HOST}/${KEY}.txt`, urlList }),
});
// 200 OK and 202 Accepted both mean the list was received.
console.log(`IndexNow: ${response.status} ${response.statusText}`);
if (!response.ok) process.exitCode = 1;
