// Back-references for bibliographies: every reference entry gets ↩ links to the places where it is cited.
//
// A reference entry is an empty anchor (<span id="cite-6"></span>, <span id="ref-wndclassex"></span>) directly
// followed by text that starts with "[N]", as the converter writes them. Citations are in-page links to that
// anchor ([6](#cite-6)); each gets an id (cite-6-use-1, ...) so the entry can link back to it. One citation gives
// a single ↩; several give "↩ a b c" (as on Wikipedia). The links go at the end of the entry: before the next
// <br> or next entry when entries share a paragraph, otherwise at the end of the paragraph.

/** Depth-first walk calling fn(node). */
function walk(node, fn) {
	fn(node);
	for (const child of node.children ?? []) walk(child, fn);
}

/**
 * The id of an empty anchor starting at kids[i], and the index just after it. Inline HTML in Markdown reaches
 * rehype as raw nodes: <span id="x"> and </span> are two of them (or one, written as <span id="x"></span>).
 */
function emptyAnchorAt(kids, i) {
	const n = kids[i];
	if (n?.type === 'element' && n.tagName === 'span' && typeof n.properties?.id === 'string' && !n.children?.length)
		return { id: n.properties.id, next: i + 1 };
	if (n?.type !== 'raw') return null;
	const value = n.value.trim();
	const whole = /^<span id="([^"]+)"><\/span>$/.exec(value);
	if (whole) return { id: whole[1], next: i + 1 };
	const open = /^<span id="([^"]+)">$/.exec(value);
	if (open && kids[i + 1]?.type === 'raw' && kids[i + 1].value.trim() === '</span>') return { id: open[1], next: i + 2 };
	return null;
}

export default function rehypeCitationBackrefs() {
	return (tree) => {
		// 1. Reference entries: id -> { parent, anchor node }.
		const entries = new Map();
		walk(tree, (node) => {
			const kids = node.children;
			if (!kids) return;
			kids.forEach((kid, i) => {
				const a = emptyAnchorAt(kids, i);
				const next = a && kids[a.next];
				if (next?.type === 'text' && /^\s*\[\d+\]/.test(next.value)) entries.set(a.id, { parent: node, anchor: kid });
			});
		});
		if (!entries.size) return;

		// 2. Citations, in document order.
		const uses = new Map(); // entry id -> [citation ids]
		walk(tree, (node) => {
			if (node.type !== 'element' || node.tagName !== 'a') return;
			const href = node.properties?.href;
			if (typeof href !== 'string' || !href.startsWith('#')) return;
			const id = decodeURIComponent(href.slice(1));
			if (!entries.has(id)) return;
			const list = uses.get(id) ?? [];
			if (!node.properties.id) node.properties.id = `${id}-use-${list.length + 1}`;
			list.push(node.properties.id);
			uses.set(id, list);
		});

		// 3. Back-links at the end of each cited entry.
		const anchors = new Set([...entries.values()].map((e) => e.anchor));
		for (const [id, list] of uses) {
			const { parent, anchor } = entries.get(id);
			const kids = parent.children;
			const start = kids.indexOf(anchor);
			let end = kids.findIndex((n, i) => i > start && ((n.type === 'element' && n.tagName === 'br') || anchors.has(n)));
			if (end === -1) end = kids.length;
			const link = (use, text, label) => ({
				type: 'element',
				tagName: 'a',
				properties: { href: `#${use}`, ariaLabel: label },
				children: [{ type: 'text', value: text }],
			});
			const children =
				list.length === 1
					? [link(list[0], '↩︎', 'Back to the citation')]
					: [
							{ type: 'text', value: '↩︎ ' },
							...list.flatMap((use, i) => [
								...(i ? [{ type: 'text', value: ' ' }] : []),
								link(use, i < 26 ? String.fromCharCode(97 + i) : String(i + 1), `Back to citation ${i + 1}`),
							]),
						];
			kids.splice(end, 0, { type: 'text', value: ' ' }, { type: 'element', tagName: 'span', properties: { className: ['backrefs'] }, children });
		}
	};
}
