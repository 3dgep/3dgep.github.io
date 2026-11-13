# Migration plan: 3dgep.com (WordPress) → Starlight on GitHub Pages

Goal: move every article from www.3dgep.com to this repository (Markdown, Astro + Starlight, GitHub Pages),
keep every article URL working, then point the 3dgep.com domain here and retire WordPress.

## Decisions so far

| Topic | Decision |
|---|---|
| Generator | Astro + Starlight (MyST / Jupyter Book 2 was prototyped and compared; Starlight chosen for code blocks, page size, URL/redirect control and easier customization) |
| Comments | Giscus (GitHub Discussions). Existing WordPress comments are **not** migrated |
| URLs | Same slugs as WordPress, with trailing slash: `/learning-directx-12-1/` |
| Code snippets | Imported from the tutorial sources at the tagged release (`sources/`), not copied |
| External links | Open in a new tab; internal links stay in the same tab |
| Link checking | `starlight-links-validator` fails the build on broken internal links/anchors |

## Inventory (from the WordPress REST API)

45 posts and 6 pages. Done: DirectX 12 lessons 1–4, Introduction to DirectX 11, Texturing and Lighting in
DirectX 11, Forward+, Volume Tiled Forward Shading, C++ Fast Track (16 parts).

| Series / group | Posts | Notes |
|---|---|---|
| Learning DirectX 12 | 4 (all done) | Sources at `LearningDirectX12` tags v0.0.1–v0.0.4 |
| C++ Fast Track for Games Programming | 17 (all done, part 14 removed) | Ported as-is (parts 1–2 already updated for VS 2026); template downloads point to `3dgep/tmpl8` (zip of `main`). A fuller rewrite is still planned (Phase 6) |
| DirectX 9/11, Forward+, Volume Tiled Forward | 5 (4 done) | Introduction to DirectX 9 remains. Sources: `LearningDirectX11` v1.0.0/v2.0.0, `3dgep/ForwardPlus` @94f71f9 |
| OpenGL / GLSL / Cg / MD5 models | 5 | Self-hosted WebGL demos via script (like Lesson 2) in 3 posts |
| CUDA / OpenCL | 7 | Some text in `[brackets]` looks like shortcodes (`[index]`, `[gtid]`) but is code; verify |
| Math (primers, view matrix, quaternions) | 6 | Math-heavy: up to 230 equations per post; check KaTeX coverage |
| Misc (DLL plug-ins, C++ templates) | 2 | |
| Pages | 6 | About, Courses, GRP3, GRP4, AGT6, Privacy and Cookie Policy: decide which to keep |

Totals across all posts: ~1,280 code blocks, ~440 images, ~1,020 equations, 126 alert boxes,
367 captions, 42 tables, 73 file downloads (zip/exe), 8 script-built embeds.

## Phase 1: Converter hardening

- [x] **Original line numbers and highlighted lines** from the raw post markup (`/wp-json/wp/v2/posts/<id>`,
      passed as `RAW_POST`): `<pre class="brush:cpp;first-line:25">` / `<pre class="lang:c++ start-line:… mark:…">`,
      matched one-to-one with the rendered Crayon blocks. Used when a snippet isn't found in the source;
      `mark:` becomes Expressive Code line markers (`{3,5-7}`). The converter reports where the source
      numbering differs from what the article showed.
- [ ] Cross-page anchors in WordPress's other format (`#Pipeline_State_Object-2`) aren't resolved against the
      target article's headings yet (Lesson 4 needed two hand fixes); the link validator catches them.
- [ ] Fetch all posts once into a local cache (the inventory script already does this) and run a **dry-run
      conversion of all 45 posts**; collect warnings (unknown markup, unresolved anchors, missing reference
      anchors, unmatched snippets).
- [x] Desmos embeds (keep their original size), data tables (also with malformed HTML).
- [x] `<video>` (self-hosted under `public/videos/`), named reference anchors (`<a name="WNDCLASSEX">` → `#ref-wndclassex`,
      `cite_N` → `cite-N`), links into other ported articles' sections (resolved against their headings via
      `SITE_DOCS`), `?p=<id>` links (`WP_POSTS`), renamed source files (`SOURCE_ALIASES`), `	ext{A_B}` escaped for
      KaTeX, and broken original markup (`href="#x>`, `href="⏎#x"`, anchors without `#`).
- [ ] Handle remaining markup: `<video>`,
      script-built demo embeds (generalize the Lesson 2 handling), file downloads (zip/exe → GitHub release
      links where possible instead of copying binaries).
- [x] GitHub mark next to repository links: light/dark variants (`public/images/github-mark*.svg`), chosen by
      theme via `.light-only`/`.dark-only`; the converter emits them automatically.
- [ ] Check other dark images that are hard to see in dark mode (use the same `.light-only`/`.dark-only` classes).
- [ ] Remove the legacy `myst` target from `tools/wp2md/convert.mjs`.

## Phase 2: Port the content

For each article:
1. Add the tutorial sources (if any) to `sources/<repo>-<tag>/` (the tag the article's download links use).
2. Convert with `SOURCE_REF=<repo>-<tag>` and `RAW_POST=<raw post json>` (see `tools/wp2md/convert.mjs`).
3. Run `tools/wp2md/renumber-line-refs.mjs <article.md> <raw post json>` (dry run, then `--write`): when the
   article numbered a snippet differently from the tagged source, it shifts the prose's "line N" references to the
   source numbering and lists references that need a manual look.
4. Run `node tools/check-external-links.mjs` (dry run), review broken/suspicious links, then `--write` to replace
   redirected URLs with their final address (e.g. MSDN → learn.microsoft.com, http → https). Download links on
   GitHub are kept; anchors that no longer exist on Learn enum pages become `#constants`. Learn links are kept
   without a locale (`learn.microsoft.com/windows/...`, not `/en-us/`), so readers get their own language.
5. Build (the link validator flags broken internal anchors) and review in the browser.

1. [x] DirectX 12 lessons 3–4 (sources: `LearningDirectX12` v0.0.3, v0.0.4)
2. [x] DirectX 11 / Forward+ / Volume Tiled Forward (Introduction to DirectX 9 still to do)
3. [ ] Math articles (KaTeX check: `\begin{bmatrix}`, `\mathbf`, aligned environments)
4. [ ] OpenGL / Cg / MD5 (self-host the WebGL demos under `public/demos/`)
5. [ ] CUDA / OpenCL, DLL plug-ins, C++ templates
6. [x] C++ Fast Track (part 14 "Fixed Point" removed; see below)
6. [ ] Pages: About (+ Privacy policy if still needed)

Site structure as content grows:
- [ ] Sidebar groups per series (DirectX 12, DirectX 11, OpenGL, GPGPU, Math, C++) and a landing page per series.
- [ ] Blog features: dates, tags, RSS (evaluate the `starlight-blog` community plugin).
- [ ] Image zoom for large diagrams (evaluate a community plugin).
- [ ] Better HLSL highlighting (Shiki's HLSL grammar is minimal; consider using the C++ grammar for `hlsl`).

## Phase 3: Keep old URLs working

GitHub Pages has no server-side redirects. Astro's `redirects` (in `astro.config.mjs`) generates small
meta-refresh pages, which handle path-based URLs:

- [ ] Article slugs are unchanged, so most URLs need nothing.
- [ ] Old aliases that WordPress redirected (e.g. `learning-directx12-1` → `learning-directx-12-1`, already added).
      Collect the rest from WordPress (Redirection plugin / Yoast) or Search Console before the switch.
- [ ] Category pages `/category/<slug>/` → series landing pages.
- [ ] RSS: provide a feed at `/feed/` (existing subscribers use it).
- [ ] Not covered: `/?p=<id>` shortlinks and other query-string URLs. Consider a small `404.astro` script that
      maps known `?p=` ids to slugs on the client.
- [ ] `/wp-content/uploads/...` image URLs (hotlinked from elsewhere) will break. Optional: copy the most-linked
      uploads to `public/wp-content/uploads/` at the same paths.

## Phase 4: Comments

- [x] Discussions enabled on `3dgep/3dgep.github.io`, giscus app installed, comments use the **Announcements**
      category (only maintainers/giscus start discussions; anyone can comment). Configured in
      `src/components/Giscus.astro`.
- [ ] Optional: create a dedicated `Comments` category (announcement type) in the repo's Discussions settings and
      switch `category`/`categoryId` to it.
- [ ] Optional: export the WordPress comments (Tools → Export) and keep the file privately for reference.

## Phase 5: Domain switch (3dgep.com → GitHub Pages)

Do this only when all articles are ported and reviewed on `https://3dgep.github.io`.

1. [ ] A few days before: lower the DNS TTL for 3dgep.com and www.3dgep.com (e.g. to 300 s).
2. [ ] Add `public/CNAME` containing `www.3dgep.com` (or `3dgep.com`, whichever should be canonical) and set
       `site` in `astro.config.mjs` to the same URL. Set the custom domain in repo Settings → Pages.
3. [ ] DNS at the registrar: apex `3dgep.com` A records `185.199.108.153`, `185.199.109.153`,
       `185.199.110.153`, `185.199.111.153` (+ AAAA `2606:50c0:8000::153` … `8003::153`); `www` CNAME
       `3dgep.github.io`. **Keep existing MX/TXT records** (email, verification).
4. [ ] Verify the domain in GitHub (org settings) to prevent takeover; enable "Enforce HTTPS" once the
       certificate is issued.
5. [ ] Check a sample of old URLs, the redirects, RSS and search; submit the new sitemap
       (`/sitemap-index.xml`) in Google Search Console.
6. [ ] Keep WordPress (read-only) for a few weeks as a fallback, then cancel hosting. Export the full WXR
       backup and the `wp-content/uploads` folder first.

## Phase 6: C++ Fast Track rewrite

The 17-part series (2019–2020) is outdated. Treat it as new content rather than a port:

- [ ] Decide on the target toolchain/standard (e.g. C++20/23, CMake, current Visual Studio) and the template
      project; keep the code in its own repository with a tag per part.
- [ ] Write parts directly in Markdown, importing code from the tagged repository (`source="…"`), so the
      text and the code never drift apart.
- [ ] Redirect the old `cpp-fast-track-*` URLs to the new parts (or keep the old parts with an "outdated" note).

## External links (DirectX 12 lessons 1–4, checked 2026-11-13)

All ~500 unique external links resolve. Redirects were replaced by their final URLs (mostly MSDN/docs →
learn.microsoft.com). Links that needed a judgment call:

- `3dgep.com/cmake-visual-studio-2017/` (Lesson 1) no longer exists: now points to the Wayback Machine copy
  (Dec 2019). Consider porting or rewriting that article.
- `git.io/vApW1` (Lesson 2; git.io was shut down) → `DX12Lib/src/Application.cpp` at v0.0.2 (the link text said
  "the application class on GitHub").
- `d3dx12.h` moved from DirectX-Graphics-Samples to `microsoft/DirectX-Headers` (current version, not the 2018 one).
- `LearningDirectX12/blob/master/...` links (Lesson 4) pinned to `v0.0.4`.
- Old MSDN links to individual enum values now go to the enum page's `#constants` section (Learn has no
  per-value anchors).
- freeimage.sourceforge.io and blender.org block scripts (HTTP 403) but work in a browser.
- Links to 3dgep.com articles that aren't ported yet (math primers, view matrix) still point to WordPress and
  must be updated when those articles move (Phase 3).

## Known content issues (found during the prototype)

- Lesson 1: citation [23] has no reference entry; "`g_NumFrames` … line 44" should be 43; "`As` … line 198" should be 197.
- Lesson 2: "On line 75, the DirectX 12 device is queried" should be 77.
- Lesson 3: the article numbered `UploadBuffer.h/.cpp` and one `DynamicDescriptorHeap.h` snippet without the
  files' header comments; 6 prose references were shifted to the v0.0.3 numbering. `ResourceStateTracker::Unlock`
  in the article releases the mutex after clearing the flag (v0.0.3 does it before); kept as in the article.
- Lesson 4: the article's `CommandList.cpp` snippets come from a different revision than v0.0.4 (e.g. `MakeSRGB`,
  `TrackObject`, `GenerateMips_UAV(Texture&, bool isSRGB)`); those 9 snippets are kept inline with the article's
  original line numbers. 13 prose references were shifted to the v0.0.4 numbering (12 automatically, "line 326"
  → 342 by hand). Two in-page links had no target in the original and now point to "The Compute Shader".
- Several snippets in Lessons 1–2 differ slightly from the tagged sources (kept inline, not imported).
- Introduction to DirectX 11: reference [62] (`ID3D11DeviceContext::DrawIndexed`) had lost its text in WordPress (only
  the URL was left); restored in the format of the neighbouring entries. Two citation links were broken in the
  original HTML (`href="XInput"`, `href="⏎#DrawIndexed"`) and are fixed. 6 prose line references were shifted to
  the v1.0.0 numbering (5 automatically, "lines 243-244" → 246-247 by hand).
- Forward+: the only public source is a 2024 snapshot of the repository; 20 of 72 snippets differ from it and are kept
  inline with the article's original line numbers. Dead references (Crytek downloads, UTK optics, shinvision) point
  to Wayback Machine copies from 2015.
- Volume Tiled Forward Shading: the thesis PDF (28 MB) is now part of the site at
  `/downloads/volume-tiled-forward-shading.pdf` (was in WordPress uploads).
- C++ Fast Track: the 2019 `TheTemplate.zip` (404 on WordPress) was replaced everywhere by the `3dgep/tmpl8` repository
  (link + zip of `main`); text mentions now say "the template". The "Previous Part / Next Part" lines were removed
  (the sidebar lists the series, and Starlight adds previous/next links at the bottom of each page). `nc2tiles.png` (exercise download) is
  self-hosted under `/downloads/`. Old slugs `cpp-fast-track-8-addresses` and `cpp-fast-track-14-fixedpoint` redirect.
  Parts 3–17 still describe Visual Studio 2019 and the 2019 template in places.
- C++ Fast Track part 14 "Fixed Point" was removed: fixed-point arithmetic is no longer relevant for a beginner course,
  and no other part depends on it. Its URLs (`/cpp-fast-track-14-fixed-point/`, `/cpp-fast-track-14-fixedpoint/`) redirect to
  File I/O. The following parts were renumbered, URLs included: File I/O, Physics and AI are now parts 14–16
  (`/cpp-fast-track-14-fileio/`, `/cpp-fast-track-15-physics/`, `/cpp-fast-track-16-ai/`); the old URLs
  (`-15-fileio`, `-16-physics`, `-17-ai`) redirect to them.
