// Dump every rendered code block (title, first line number, code text) from a built Starlight page,
// so the output can be compared before and after a change.
// Usage: node snapshot-code.cjs <page.html> <out.json>
const fs = require('fs');
const h = fs.readFileSync(process.argv[2], 'utf8');
const unescape = (s) => s.replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&quot;/g, '"').replace(/&#39;/g, "'").replace(/&amp;/g, '&');
const blocks = [...h.matchAll(/<figure class="frame[\s\S]*?<\/figure>/g)].map((m) => {
  const f = m[0];
  const title = /<span class="title">([^<]*)<\/span>/.exec(f)?.[1] ?? '';
  const lines = f.split('<div class="ec-line').slice(1);
  const nums = lines.map((l) => /<div class="ln"[^>]*>(\d+)</.exec(l)?.[1] ?? null);
  const text = lines.map((l) => {
    const code = /<div class="code">([\s\S]*)$/.exec(l)?.[1] ?? '';
    return unescape(code.replace(/<[^>]+>/g, ''));
  });
  return { title, first: nums[0], count: lines.length, text: text.join('\n') };
});
fs.writeFileSync(process.argv[3], JSON.stringify(blocks, null, 1));
console.log(`${blocks.length} code blocks saved`);
