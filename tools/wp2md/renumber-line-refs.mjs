// Shift "line N" references in a converted article from the original WordPress numbering to the source
// numbering. Use after converting with SOURCE_REF + RAW_POST, when the original article numbered a snippet
// differently from the tagged source (e.g. the article was written against a file without its license header).
//
// A reference is shifted only when it falls inside the preceding code block under the article's numbering
// but not under the source numbering; it moves by that block's offset (source start - article start).
// Everything else is left alone; references that fit neither numbering are listed for manual review.
//
// Usage: node renumber-line-refs.mjs <article.md> <raw-post.json> [--write]
import fs from 'node:fs';

const [, , mdFile, rawFile, flag] = process.argv;
let md = fs.readFileSync(mdFile, 'utf8');
const raw = [...JSON.parse(fs.readFileSync(rawFile, 'utf8')).content.rendered.matchAll(/<pre([^>]*)>/g)]
  .map((m) => +(/(?:first|start)-line:\s*(\d+)/.exec(m[1])?.[1] ?? 1));

const blocks = [...md.matchAll(/^(`{3,})(\w+)([^\n]*)\n([\s\S]*?)^\1$/gm)].map((m, k) => {
  const imp = /source="(?:[^"#]*\/)?([^"#/]+)#L(\d+)(?:-L(\d+))?"/.exec(m[3]);
  const title = imp?.[1] ?? /title="([^"]+)"/.exec(m[3])?.[1];
  const start = imp ? +imp[2] : +(/startLineNumber=(\d+)/.exec(m[3])?.[1] ?? 1);
  const length = imp ? +(imp[3] ?? imp[2]) - +imp[2] + 1 : m[4].split('\n').length;
  return { end: m.index + m[0].length, title, start, length, article: raw[k], k: k + 1 };
});
if (blocks.length !== raw.length) throw new Error(`${blocks.length} code blocks in the article, ${raw.length} in the raw post`);

const edits = [];
const review = [];
for (const m of md.matchAll(/\b(lines?) (\d+)(?:(\s*(?:-|–|and|to)\s*)(\d+))?/g)) {
  const a = +m[2], b = +(m[4] ?? m[2]);
  const block = blocks.filter((x) => x.end <= m.index && x.title && /\.\w+$/.test(x.title)).at(-1);
  if (!block) continue;
  const inSrc = a >= block.start && b < block.start + block.length;
  const inArt = a >= block.article && b < block.article + block.length;
  const context = md.slice(Math.max(md.lastIndexOf('\n', m.index), m.index - 60), m.index + m[0].length + 30).replace(/\s+/g, ' ').trim();
  if (inArt && !inSrc) {
    const d = block.start - block.article;
    const text = `${m[1]} ${a + d}${m[4] ? m[3] + (b + d) : ''}`;
    edits.push({ index: m.index, length: m[0].length, text, note: `block ${block.k} ${block.title}: "${m[0]}" -> "${text}"  » ${context}` });
  } else if (!inSrc && !inArt) {
    review.push(`block ${block.k} ${block.title} (lines ${block.start}-${block.start + block.length - 1}): "${m[0]}"  » ${context}`);
  }
}

for (const e of edits.slice().reverse()) md = md.slice(0, e.index) + e.text + md.slice(e.index + e.length);
console.log(`${edits.length} reference(s) renumbered to the source numbering:`);
for (const e of edits) console.log(`  ${e.note}`);
if (review.length) {
  console.log(`${review.length} reference(s) outside the preceding snippet in both numberings (check by hand):`);
  for (const r of review) console.log(`  ${r}`);
}
if (flag === '--write') fs.writeFileSync(mdFile, md);
else console.log('(dry run; pass --write to update the file)');
