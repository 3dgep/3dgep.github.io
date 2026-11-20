// Starlight route middleware: search and sharing metadata for every page.
// - og:image / twitter:image: the article's first image from /images/ (most articles start with a thumbnail),
//   or the site logo. The thumbnails are small and square, so the Twitter card is "summary" (image beside
//   the text) instead of Starlight's default "summary_large_image".
// - JSON-LD structured data: TechArticle for the articles, AboutPage for About, WebSite for the home page.
import { defineRouteMiddleware } from '@astrojs/starlight/route-data';

const SITE_NAME = '3D Game Engine Programming';
const AUTHOR = { '@type': 'Person', name: 'Jeremiah van Oosten', url: '/about-3d-game-engine-programming/' };
const FALLBACK_IMAGE = '/images/3dgep-og.png';

export const onRequest = defineRouteMiddleware((context) => {
	const route = context.locals.starlightRoute;
	const site = (context.site?.href ?? 'https://www.3dgep.com/').replace(/\/$/, '');
	const absolute = (path: string) => (path.startsWith('http') ? path : site + path);
	const { entry } = route;
	const isHome = entry.id === 'index' || entry.id === '';
	const pageUrl = absolute(context.url.pathname);

	const firstImage = entry.body?.match(/<img src="(\/images\/[^"]+)"/)?.[1];
	const image = absolute(firstImage ?? FALLBACK_IMAGE);

	// Replace Starlight's large-image card; small square thumbnails look better as a "summary" card.
	route.head = route.head.filter((tag) => tag.attrs?.name !== 'twitter:card');
	route.head.push(
		{ tag: 'meta', attrs: { name: 'twitter:card', content: 'summary' } },
		{ tag: 'meta', attrs: { property: 'og:image', content: image } },
		{ tag: 'meta', attrs: { name: 'twitter:image', content: image } },
	);

	const author = { ...AUTHOR, url: absolute(AUTHOR.url) };
	const publisher = {
		'@type': 'Organization',
		name: SITE_NAME,
		url: site + '/',
		logo: { '@type': 'ImageObject', url: absolute(FALLBACK_IMAGE) },
	};
	const lastUpdated = entry.data.lastUpdated instanceof Date ? entry.data.lastUpdated.toISOString() : undefined;
	const data = isHome
		? { '@context': 'https://schema.org', '@type': 'WebSite', name: SITE_NAME, url: site + '/', author, publisher }
		: {
				'@context': 'https://schema.org',
				'@type': entry.id === 'about-3d-game-engine-programming' ? 'AboutPage' : 'TechArticle',
				headline: entry.data.title,
				description: entry.data.description,
				url: pageUrl,
				mainEntityOfPage: pageUrl,
				image,
				...(lastUpdated ? { dateModified: lastUpdated } : {}),
				author,
				publisher,
				isPartOf: { '@type': 'WebSite', name: SITE_NAME, url: site + '/' },
			};
	if (entry.id === '404') return; // no structured data for the error page
	// "<" can't end the script element early when it is escaped.
	route.head.push({
		tag: 'script',
		attrs: { type: 'application/ld+json' },
		content: JSON.stringify(data).replace(/</g, '\\u003c'),
	});
});
