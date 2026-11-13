# 3D Game Engine Programming

Helping you build your dream game engine.

**3D Game Engine Programming** (3dgep) is an educational resource for teaching new (and old) game developers
about game engine programming. The primary focus is graphics programming with APIs such as DirectX, OpenGL
and Vulkan, along with other game engine topics such as audio, physics and general engine architecture.

The articles from [3dgep.com](https://www.3dgep.com) (WordPress) are being moved to this site, written in
Markdown and built with [Astro](https://astro.build) + [Starlight](https://starlight.astro.build).
See [MIGRATION.md](MIGRATION.md) for the plan.

> 📝 This website is under development: [https://3dgep.github.io](https://3dgep.github.io)

## Working on the site

Requires Node.js 22.23.4 or newer.

```sh
npm install
npm run dev       # local dev server with live reload: http://localhost:4321
npm run build     # production build into dist/ (also fails on broken internal links)
npm run preview   # serve the production build
```

## Repository layout

| Path | What it is |
|---|---|
| `src/content/docs/` | Articles (`.md`/`.mdx`). The file name is the URL: `learning-directx-12-1.md` → `/learning-directx-12-1/` |
| `src/assets/` | Article images (optimized to WebP at build time) |
| `public/` | Files served as-is: `images/` (small inline images), `demos/` (interactive WebGL demos), `audio/` |
| `sources/` | Tutorial source code at the tagged release each article is based on (code snippets are imported from here) |
| `src/components/` | Starlight overrides: `Footer.astro` (Giscus comments + hover previews), `Giscus.astro`, `HoverPreview.astro` |
| `src/plugins/remark-code-from-source.mjs` | Fills code blocks from `sources/` at build time |
| `src/styles/custom.css` | Site styling (links, inline code, captions, TOC, embeds) |
| `tools/wp2md/` | WordPress → Markdown converter used for the migration |
| `.github/workflows/deploy.yml` | Builds and deploys to GitHub Pages on pushes to the `publish` branch |

## Writing articles

**Code from the tutorial sources.** Instead of copying code into the article, import it from `sources/`. The
title (file name) and line numbers are filled in automatically, and the build fails if the range doesn't exist:

````md
```cpp source="LearningDirectX12-v0.0.1/main.cpp#L145-L159"
```
````

Expressive Code options still work alongside `source`, e.g. `{3-4}` to highlight lines or `ins={5}` / `del={6}`
for diffs. Code that isn't from the sources is written inline as usual (`title="..."`, `showLineNumbers`).

**Math** uses KaTeX: `$...$` inline and `$$...$$` for display equations.

**Asides:** `:::note`, `:::tip`, `:::caution`, `:::danger` … `:::`.

**References:** cite as `[\[6\]](#cite-6)` and give the reference entry an anchor:
`<span id="cite-6"></span>\[6\] Author, "Title", ...`. Hovering a citation shows the entry.

**Links** to other articles are root-relative (`/learning-directx-12-1/#create-a-fence`). External links open in a
new tab automatically. Broken internal links and `#anchors` fail the build.

## Comments

Comments use [Giscus](https://giscus.app): each article has a GitHub Discussion in this repository's
**Announcements** category, matched by the page path and created with the first comment or reaction. Readers sign
in with GitHub to comment. Settings are in `src/components/Giscus.astro`.

## Deploying

GitHub Pages deploys from the `publish` branch (repository Settings → Pages → Source: **GitHub Actions**).
Write on `main`, then merge into `publish` to go live. A deploy can also be started manually from the Actions tab.
