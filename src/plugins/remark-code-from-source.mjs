// remark plugin: fill code blocks from the tutorial source code at build time (like MyST's literalinclude).
//
//   ```cpp source="LearningDirectX12-v0.0.1/main.cpp#L145-L159"
//   ```
//
// - `source` is "<path relative to the sources root>#L<first>-L<last>" (or "#L<n>" for one line, or no
//   range for the whole file).
// - The block's code is replaced by those lines, so snippets can never drift from the real code.
// - Unless the block says otherwise, the title becomes the file name, line numbers are shown, and they
//   start at the first imported line.
// - A missing file or an out-of-range line fails the build with the article and block named.
import fs from 'node:fs';
import path from 'node:path';
import { visit } from 'unist-util-visit';

/** @param {{ root: string }} options  Directory that `source` paths are relative to. */
export default function remarkCodeFromSource({ root }) {
	// Cache the lines of each file, but read it again when it changed (the dev server keeps this plugin alive).
	const cache = new Map();
	const readLines = (file) => {
		const mtime = fs.statSync(file).mtimeMs;
		const cached = cache.get(file);
		if (cached?.mtime === mtime) return cached.lines;
		const lines = fs.readFileSync(file, 'utf8').replace(/\r\n?/g, '\n').split('\n');
		cache.set(file, { mtime, lines });
		return lines;
	};

	return (tree, vfile) => {
		visit(tree, 'code', (node) => {
			const match = /(?:^|\s)source="([^"#]+)(?:#L(\d+)(?:-L(\d+))?)?"/.exec(node.meta ?? '');
			if (!match) return;
			const [attr, rel, from, to] = match;
			const where = `${vfile.path}:${node.position?.start.line ?? '?'}`;
			const file = path.resolve(root, rel);
			if (!file.startsWith(path.resolve(root) + path.sep)) throw new Error(`${where}: source "${rel}" is outside ${root}`);
			if (!fs.existsSync(file)) throw new Error(`${where}: source file not found: ${rel}`);

			const lines = readLines(file);
			const first = from ? +from : 1;
			const last = to ? +to : from ? +from : lines.length;
			if (first < 1 || last > lines.length || first > last) {
				throw new Error(`${where}: lines ${first}-${last} are outside ${rel} (${lines.length} lines)`);
			}
			node.value = lines.slice(first - 1, last).join('\n');

			let meta = (node.meta ?? '').replace(attr, ' ').trim();
			if (!/\btitle=/.test(meta)) meta += ` title="${path.basename(rel)}"`;
			if (!/\bshowLineNumbers\b/.test(meta)) meta += ' showLineNumbers';
			if (!/\bstartLineNumber=/.test(meta) && first > 1) meta += ` startLineNumber=${first}`;
			node.meta = meta.trim();
		});
	};
}
