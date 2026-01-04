# Engineer Log

## Session: 2026-01-02
**Agent**: Antigravity
**Task**: Initial Project Scaffolding
**Changes**:
- Created directory structure (`src/`).
- Created `CMakeLists.txt` for building a `.scr` executable.
- Implemented `src/main.cpp` with Win32 screensaver argument handling (`/s`, `/c`, `/p`).
- Implemented `src/renderer.cpp` with a basic OpenGL WGL context and rotating triangle demo.
- Verified build using CMake.
- Created `README.md` with SOPs.

**Completion %**: 100% (of Scaffolding)
**Remaining TODO**:
- Implement actual visual effects (currently just a demo triangle).
- Add configuration dialog logic (currently a stub).

**Update**: Installed GitHub CLI (`gh`), authenticated, and pushed repository to `https://github.com/invisibleacropolis-ops/screens`.

**Task**: Real-Time Metrics & Visualization
**Changes**:
- Implemented `SystemMonitor` class using Windows PDH API to read CPU, Disk, and Memory usage.
- Implemented `renderer.cpp` visuals:
    - **CPU**: Pulsing red/blue wireframe sphere.
    - **RAM**: 10x10 Grid of cubes that fill up based on memory load.
    - **Disk**: Glowing ring orbiting the CPU.
- Verified build with `pdh.lib`.
**Completion %**: 100% (Core Metrics), 50% (Network metric is stubbed)
**Remaining TODO**:
- Implement complex Network Interface aggregation (currently stubbed).
- Polish visual aesthetics (add shaders, better lighting).

## Session: 2026-01-04
**Agent**: ChatGPT
**Task**: HDR Post-Processing Pipeline
**Changes**:
- Added HDR framebuffer targets with floating-point color attachments and a resize-aware post-processing pipeline in `src/renderer.cpp`.
- Implemented a fullscreen quad path for bloom extraction, multi-pass Gaussian blur, tone mapping, and optional FXAA.
- Added new post-processing shader programs (`assets/shaders/fullscreen.vert`, `assets/shaders/bloom_extract.frag`, `assets/shaders/bloom_blur.frag`, `assets/shaders/tonemap.frag`, `assets/shaders/fxaa.frag`).
- Extended the shader utility with a `SetVec2` helper for post-processing uniforms.

**Completion %**: 100% (HDR Pipeline)
**Remaining TODO**:
- Expose bloom/FXAA toggles via user configuration if needed.

## Session: 2026-01-06
**Agent**: ChatGPT
**Task**: Instanced Particle System Driven by SystemMonitor
**Changes**:
- Added `src/Particles.h`/`src/Particles.cpp` with a documented particle module that uses CPU simulation plus instanced rendering. The module smooths SystemMonitor inputs with rise/fall rates to avoid jitter and documents where GPU compute/transform feedback can slot in when OpenGL 4.3+ is available.
- Integrated the particle system into `src/renderer.cpp`:
    - Initialized the particle system and per-frame timing.
    - Updated and rendered particles each frame using smoothed CPU/RAM/Disk/Network metrics.
    - Cleaned up particle resources on shutdown.
- Added new particle shaders (`assets/shaders/particles.vert` and `assets/shaders/particles.frag`) with SceneData UBO bindings and soft point sprite falloff.
- Updated `CMakeLists.txt` to compile the new particle sources.

**Completion %**: 100% (Particle System)
**Remaining TODO**:
- Consider GPU compute/transform feedback simulation for larger particle counts when targeting OpenGL 4.3+.
- Tune visual parameters (spawn rates, colors, sizes) based on desired aesthetic.
