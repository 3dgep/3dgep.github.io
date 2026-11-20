// RSS feed of all articles, newest (last updated) first, at /rss.xml. The old WordPress feed URLs
// (/feed/, /comments/feed/) redirect here (meta-refresh pages, plus the old-URL script in astro.config.mjs).
import { getCollection } from 'astro:content';
import type { APIContext } from 'astro';

const escape = (s: string) =>
	s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');

export async function GET(context: APIContext) {
	const site = context.site!.href.replace(/\/$/, '');
	const docs = (await getCollection('docs'))
		.filter((doc) => doc.id !== 'index' && doc.id !== '404' && doc.data.lastUpdated instanceof Date)
		.sort((a, b) => (b.data.lastUpdated as Date).getTime() - (a.data.lastUpdated as Date).getTime());

	const items = docs
		.map((doc) => {
			const link = `${site}/${doc.id}/`;
			return `
		<item>
			<title>${escape(doc.data.title)}</title>
			<link>${link}</link>
			<guid>${link}</guid>
			<pubDate>${(doc.data.lastUpdated as Date).toUTCString()}</pubDate>${
				doc.data.description ? `\n\t\t\t<description>${escape(doc.data.description)}</description>` : ''
			}
		</item>`;
		})
		.join('');

	const xml = `<?xml version="1.0" encoding="UTF-8"?>
<rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">
	<channel>
		<title>3D Game Engine Programming</title>
		<link>${site}/</link>
		<description>Tutorials on game engine programming, graphics programming and C++.</description>
		<language>en</language>
		<atom:link href="${site}/rss.xml" rel="self" type="application/rss+xml" />${items}
	</channel>
</rss>
`;
	return new Response(xml, { headers: { 'Content-Type': 'application/rss+xml; charset=utf-8' } });
}
