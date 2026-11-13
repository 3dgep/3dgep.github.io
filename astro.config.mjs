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
import rehypeCitationBackrefs from './src/plugins/rehype-citation-backrefs.mjs';
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
				// Reference entries link back to their citations (↩).
				rehypeCitationBackrefs,
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
		'/cpp-fast-track-8-addresses/': '/cpp-fast-track-8-memory-addresses/',
		// Part 14 (Fixed Point) was removed from the series; old links go to the next part.
		'/cpp-fast-track-14-fixed-point/': '/cpp-fast-track-14-fileio/',
		'/cpp-fast-track-14-fixedpoint/': '/cpp-fast-track-14-fileio/',
		// Parts 15-17 were renumbered to 14-16 after that.
		'/cpp-fast-track-15-fileio/': '/cpp-fast-track-14-fileio/',
		'/cpp-fast-track-16-physics/': '/cpp-fast-track-15-physics/',
		'/cpp-fast-track-17-ai/': '/cpp-fast-track-16-ai/',
		// Obsolete articles that were not migrated: send readers to the closest current article.
		'/introduction-to-directx-for-game-programmers/': '/introduction-to-directx-11/', // DirectX 9 + D3DX
		'/introduction-to-shader-programming-with-cg-3-1/': '/texturing-and-lighting-with-opengl-and-glsl/', // Cg (discontinued)
		'/using-opengl-vertex-buffer-objects/': '/texturing-and-lighting-with-opengl-and-glsl/', // client-side vertex arrays
		'/introduction-to-cuda-using-visual-studio-2008/': '/cuda-thread-execution-model/', // VS 2008 / CUDA 4 setup
		'/opengl-interoperability-with-cuda/': '/cuda-thread-execution-model/', // legacy GL + old interop API
		// Pages that were removed (course pages and the WordPress privacy policy).
		'/courses/': '/',
		'/graphics-programming-3-grp3/': '/',
		'/graphics-programming-4-grp4/': '/',
		'/advanced-game-technology-6-agt6/': '/',
		'/privacy-and-cookie-policy/': '/',
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
					label: 'C++ Fast Track',
					collapsed: true,
					items: [
						{ label: 'Part 1: Getting Started', slug: 'cpp-fast-track-1-getting-started' },
						{ label: 'Part 2: The Template', slug: 'cpp-fast-track-2-template' },
						{ label: 'Part 3: Variables and Functions', slug: 'cpp-fast-track-3-variables' },
						{ label: 'Part 4: Sprites and Loops', slug: 'cpp-fast-track-4-sprites-and-loops' },
						{ label: 'Part 5: Conditions', slug: 'cpp-fast-track-5-conditions' },
						{ label: 'Part 6: Floats', slug: 'cpp-fast-track-6-floats' },
						{ label: 'Part 7: Debugging', slug: 'cpp-fast-track-7-debugging' },
						{ label: 'Part 8: Memory Addresses', slug: 'cpp-fast-track-8-memory-addresses' },
						{ label: 'Part 9: Colours', slug: 'cpp-fast-track-9-colours' },
						{ label: 'Part 10: Arrays', slug: 'cpp-fast-track-10-arrays' },
						{ label: 'Part 11: Tiles', slug: 'cpp-fast-track-11-tiles' },
						{ label: 'Part 12: Classes', slug: 'cpp-fast-track-12-classes' },
						{ label: 'Part 13: Data Structures', slug: 'cpp-fast-track-13-data-structures' },
						{ label: 'Part 14: File I/O', slug: 'cpp-fast-track-14-fileio' },
						{ label: 'Part 15: Physics', slug: 'cpp-fast-track-15-physics' },
						{ label: 'Part 16: AI', slug: 'cpp-fast-track-16-ai' },
					],
				},
				{
					label: 'C++',
					items: [
						{ label: 'C++ Template Programming', slug: 'beginning-cpp-template-programming' },
						{ label: 'DLL Plug-Ins', slug: 'using-dynamic-link-libraries-dll-to-create-plug-ins' },
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
					label: 'OpenGL',
					items: [
						{ label: 'Texturing and Lighting with GLSL', slug: 'texturing-and-lighting-with-opengl-and-glsl' },
						{ label: 'Loading and Animating MD5 Models', slug: 'loading-and-animating-md5-models-with-opengl' },
						{ label: 'GPU Skinning of MD5 Models', slug: 'gpu-skinning-of-md5-models-in-opengl-and-cg' },
					],
				},
				{
					label: 'Rendering Techniques',
					items: [
						{ label: 'Forward vs Deferred vs Forward+', slug: 'forward-plus' },
						{ label: 'Volume Tiled Forward Shading', slug: 'volume-tiled-forward-shading' },
					],
				},
				{
					label: 'Math',
					items: [
						{ label: 'Coordinate Systems', slug: '3d-math-primer-for-game-programmers' },
						{ label: 'Vector Operations', slug: '3d-math-primer-for-game-programmers-vector-operations' },
						{ label: 'Matrices', slug: '3d-math-primer-for-game-programmers-matrices' },
						{ label: 'Understanding the View Matrix', slug: 'understanding-the-view-matrix' },
						{ label: 'Understanding Quaternions', slug: 'understanding-quaternions' },
					],
				},
				{
					label: 'GPGPU',
					items: [
						{ label: 'CUDA Thread Execution Model', slug: 'cuda-thread-execution-model' },
						{ label: 'CUDA Memory Model', slug: 'cuda-memory-model' },
						{ label: 'Optimizing CUDA Applications', slug: 'optimizing-cuda-applications' },
						{ label: 'CUDA Case Study: N-Body Simulation', slug: 'cuda-case-study-n-body-simulation' },
						{ label: 'Introduction to OpenCL', slug: 'introduction-to-opencl' },
					],
				},
				{ label: 'About', slug: 'about-3d-game-engine-programming' },
			],
		}),
	],
});
