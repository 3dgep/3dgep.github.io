# TODO: web-based demos

Several articles had a downloadable demo (a Visual Studio 2008–2012 project zipped on Google Drive) that no
longer downloads, or only shows a video. Port these to demos that run in the browser and embed them in the
article, like the existing Understanding Quaternions and Understanding the View Matrix demos.

## Conventions (from the existing demos)

- One page per demo under `public/demos/` (e.g. `public/demos/md5-animation/index.html`), embedded with
  `<iframe class="embed embed-demo-tall" src="/demos/..." title="Interactive demo" loading="lazy" allowfullscreen></iframe>`
  under an `## Interactive Demo` heading, with a short note on what the demo shows and the controls.
- Follow the parent page's light/dark theme (read `data-theme` from the parent document, as
  `public/demos/understanding-quaternions.html` does), and keep the controls usable at phone width.
- Use the article's own code and names where possible, so readers can match the demo to the text.
- Keep the source of every demo in a repository (one per demo, or one `3dgep/web-demos` repo) and, for C++, link to it
  from the article as the new "Download the Source".

## Choosing the technology

| Approach | Use when | Notes |
|---|---|---|
| WebGL 2 in plain JavaScript (single HTML file, no libraries) | Small, visual demos | What the math demos use. No build step |
| C++ compiled with Emscripten (WebGL 2 via OpenGL ES 3.0) | The article *is* about the C++/OpenGL code | Readers can build the same code natively. GLSL `#version 330 core` → `#version 300 es` (add `precision` qualifiers); GLUT/freeglut/SDL 1.2 → SDL2 or GLFW (both ship with Emscripten); GLEW not needed; SOIL → stb_image; `boost::filesystem` → `std::filesystem` with `--preload-file` for the assets; the main loop via `emscripten_set_main_loop` |
| WebGPU compute (WGSL) in JavaScript | CUDA / OpenCL articles | CUDA and OpenCL can't run in a browser, and WebGL has no compute shaders. WebGPU's workgroups, `var<workgroup>` memory and `workgroupBarrier()` map directly to CUDA blocks, `__shared__` and `__syncthreads()`. Fall back to a message (or a video) where WebGPU is unavailable |

Emscripten build: decide between building in the deploy workflow (`setup-emsdk` action, output copied into `public/demos/`)
or committing the built `.js`/`.wasm` files from the demo's own repository. Building in CI keeps binaries out of the repo.

## Demos to port

- [ ] **Texturing and Lighting with OpenGL and GLSL** (`OpenGL_TexturingAndLighting.zip`, needs a Google sign-in).
  The article currently embeds the old three.js demo (`public/demos/lighting_demo.html`, three.js r65 with
  `public/demos/Three/`): it shows the scene, but not the article's code. Port the article's C++/GLSL (sun, earth and moon,
  textures, per-pixel lighting and materials) with Emscripten, reusing `public/demos/Textures/`. The exercises
  (night-time texture, clouds, specular mask, starfield) could become toggles. Then remove the three.js copy if Lesson 2's
  demo (`rotating_cube_smooth_shading.html`) no longer needs it.
- [ ] **Loading and Animating MD5 Models** (`MD5ModelLoader.zip`: the VS2008 link is gone, the VS2010 one needs a
  sign-in) and **GPU Skinning of MD5 Models** (`GPUAnimation.zip`, gone). One demo for both articles: load the
  `.md5mesh`/`.md5anim`, play the animation, show the skeleton, and switch between CPU skinning (article 1) and GPU
  skinning in the vertex shader (article 2; the Cg effect becomes GLSL ES). The CPU path must use vertex buffers instead of
  the original client-side vertex arrays (not in WebGL). Emscripten fits best, since both articles walk through
  `MD5Model.cpp`/`MD5Animation.cpp`.
  Check the license of the "Bob with Lamp" model (der_ton, from katsbits.com) before redistributing it, or use another
  MD5 model.
- [ ] **CUDA Memory Model** (`CUDA_Examples.zip`, needs a sign-in): matrix multiply with global memory only vs tiled with
  shared memory, as a WebGPU compute demo with timings for both and a choice of matrix and tile size. Also restore the
  CUDA source itself (modern CUDA + CMake on GitHub) as the download.
- [ ] **CUDA Case Study – N-Body Simulation** (no demo; the article walks through NVIDIA's SDK sample): all-pairs N-body
  in a WebGPU compute shader with workgroup tiles, as in the article; body count and tile size as controls.
- [ ] **Introduction to OpenCL** (`FluidSim.zip` still downloads): the smoke/fluid solver as a WebGPU compute demo
  (click and drag to add density), keeping the zip link for the original.
- [ ] **CUDA Thread Execution Model** (optional, no original demo): an interactive occupancy calculator (block size,
  registers and shared memory per thread → active warps per SM), in plain JavaScript.
- [ ] **DLL Plug-Ins** (`CreatingDLLs.zip` still downloads): not a web demo. Optionally modernize the sample as a CMake
  project on GitHub (Windows `LoadLibrary` and Linux/macOS `dlopen`) and link it as the download.

When a demo is done, update the article's note about the missing download and the list in `MIGRATION.md`.
