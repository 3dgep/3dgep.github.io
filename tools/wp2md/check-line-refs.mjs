// Check "line N" / "lines N-M" references in a converted (Starlight) article against the source files.
// Each reference is checked against the nearest preceding code block that has a file title.
// Usage: node check-line-refs.mjs <article.md> <sourceDir>
import fs from 'node:fs';
import path from 'node:path';

const [, , mdFile, sourceDir] = process.argv;
const md = fs.readFileSync(mdFile, 'utf8');
const sources = Object.fromEntries(fs.readdirSync(sourceDir).map((f) => [f, fs.readFileSync(path.join(sourceDir, f), 'utf8').split(/\r?\n/)]));

const blocks = [...md.matchAll(/^(`{3,})(\w+)([^\n]*)\n([\s\S]*?)^\1$/gm)].map((m) => {
  // Imported blocks: source="<dir>/<file>#L<first>-L<last>" (the code itself is filled in at build time).
  const imp = /source="(?:[^"#]*\/)?([^"#/]+)#L(\d+)(?:-L(\d+))?"/.exec(m[3]);
  if (imp) return { index: m.index, end: m.index + m[0].length, title: imp[1], start: +imp[2], length: +(imp[3] ?? imp[2]) - +imp[2] + 1 };
  const title = /title="([^"]+)"/.exec(m[3])?.[1];
  const start = +(/startLineNumber=(\d+)/.exec(m[3])?.[1] ?? 1);
  return { index: m.index, end: m.index + m[0].length, title, start, length: m[4].split('\n').length };
});

let inRange = 0, outOfRange = 0;
for (const m of md.matchAll(/\blines? (\d+)(?:\s*(?:-|–|and|to)\s*(\d+))?/g)) {
  const a = +m[1], b = +(m[2] ?? m[1]);
  const block = blocks.filter((k) => k.end <= m.index && sources[k.title]).at(-1);
  if (!block) continue;
  const ok = a >= block.start && b < block.start + block.length;
  ok ? inRange++ : outOfRange++;
  const sentence = md.slice(Math.max(md.lastIndexOf('\n', m.index), m.index - 90), m.index + 50).replace(/\s+/g, ' ').trim();
  const src = sources[block.title];
  console.log(`${ok ? '  ' : '!!'} ${block.title}:${a}${b !== a ? '-' + b : ''} (block ${block.start}-${block.start + block.length - 1}) » ${sentence}`);
  for (let i = a; i <= Math.min(b, a + 2); i++) console.log(`      ${i}: ${(src[i - 1] ?? '<past end of file>').trim().slice(0, 100)}`);
}
console.log(`\n${inRange} references inside the preceding snippet, ${outOfRange} outside (marked !!)`);
