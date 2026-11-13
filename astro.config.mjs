// @ts-check
// 3D Game Engine Programming (3dgep.com): Astro + Starlight site configuration.
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import remarkMath from 'remark-math';
import rehypeKatex from 'rehype-katex';
import { unified } from '@astrojs/markdown-remark';
import rehypeExternalLinks from 'rehype-external-links';
import { fileURLToPath } from 'node:url';
import remarkCodeFromSource from './src/plugins/remark-code-from-source.mjs';
import starlightLinksValidator from 'starlight-links-validator';

export default defineConfig({
	site: 'https://3dgep.github.io',
	trailingSlash: 'always', // WordPress URLs end in a slash: /learning-directx-12-1/
	// Astro 7 defaults to the Sätteri Markdown processor; use the unified (remark/rehype) pipeline for KaTeX math.
	markdown: {
		processor: unified({
			remarkPlugins: [
				remarkMath,
				// ```cpp source="LearningDirectX12-v0.0.1/main.cpp#L145-L159": code is read from the tutorial sources.
				[remarkCodeFromSource, { root: fileURLToPath(new URL('./sources', import.meta.url)) }],
			],
			rehypePlugins: [
				rehypeKatex,
				// External links open in a new tab (internal links stay in the same tab).
				[rehypeExternalLinks, { target: '_blank', rel: ['noopener', 'noreferrer'] }],
			],
		}),
	},
	// Old WordPress URLs that no longer exist. GitHub Pages has no server-side redirects, so Astro emits
	// small meta-refresh pages for these. (These aliases are still linked from older articles.)
	redirects: {
		'/learning-directx12-1/': '/learning-directx-12-1/',
		'/learning-directx12-2/': '/learning-directx-12-2/',
		'/learning-directx12-3/': '/learning-directx-12-3/',
		'/learning-directx12-4/': '/learning-directx-12-4/',
	},
	integrations: [
		starlight({
			title: '3D Game Engine Programming',
			// Fail the build on broken internal links and #anchors (pages, headings, citations, figures).
			plugins: [starlightLinksValidator()],
			favicon: '/favicon.png',
			logo: { src: './src/assets/logo.png', alt: '3DGEP' },
			social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/3dgep/3dgep.github.io' }],
			customCss: ['katex/dist/katex.min.css', './src/styles/custom.css'],
			head: [
				{
					// External links in theme UI (e.g. the GitHub icon) also open in a new tab; article links are
					// handled at build time by rehype-external-links.
					tag: 'script',
					content: `document.addEventListener('DOMContentLoaded', () => {
						for (const a of document.querySelectorAll('a[href^="http"]')) {
							if (a.host !== location.host) { a.target = '_blank'; a.rel = 'noopener noreferrer'; }
						}
					});`,
				},
			],
			// Show h2–h4 in "On this page" (default stops at h3); converted articles use h2–h4.
			tableOfContents: { minHeadingLevel: 2, maxHeadingLevel: 4 },
			components: {
				// Wrap the default footer to append Giscus comments on every article page.
				Footer: './src/components/Footer.astro',
			},
			sidebar: [
				{
					label: 'DirectX 12',
					items: [
						{ label: 'Lesson 1 – Initialize DirectX 12', slug: 'learning-directx-12-1' },
						{ label: 'Lesson 2 – Rendering', slug: 'learning-directx-12-2' },
						{ label: 'Lesson 3 – Framework', slug: 'learning-directx-12-3' },
						{ label: 'Lesson 4 – Textures', slug: 'learning-directx-12-4' },
					],
				},
				{
					label: 'DirectX 11',
					items: [
						{ label: 'Introduction to DirectX 11', slug: 'introduction-to-directx-11' },
						{ label: 'Texturing and Lighting', slug: 'texturing-lighting-directx-11' },
					],
				},
				{
					label: 'Rendering Techniques',
					items: [
						{ label: 'Forward vs Deferred vs Forward+', slug: 'forward-plus' },
						{ label: 'Volume Tiled Forward Shading', slug: 'volume-tiled-forward-shading' },
					],
				},
			],
		}),
	],
});
