# CPU Surreal Visualizer Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the current functional-but-flat CPU wireframe with a surreal, expressive sci-fi mesh visualizer driven by layered metric modulation.

**Architecture:** Keep the existing CPU layer ownership (`CPUVisualizer`) but split behavior into three stages: (1) signal shaping, (2) surreal heightfield synthesis, and (3) dedicated shading/material response. Preserve compatibility with current layer/compositor flow while adding CPU-specific shaders.

**Tech Stack:** C++17, OpenGL 3.3, GLSL 330, existing `SystemMonitor`, `Engine`, `LayerCompositor`.

---

### Task 1: Add CPU surreal shader pair

**Files:**
- Create: `assets/shaders/cpu_surreal.vert`
- Create: `assets/shaders/cpu_surreal.frag`
- Modify: `src/engine/Engine.h`
- Modify: `src/engine/Engine.cpp`

**Step 1: Write the failing test**

Create a manual shader-load expectation:
- `Engine::SetupShaders` should load `cpu_surreal.vert/frag` and report success in logs.

**Step 2: Run test to verify it fails**

Run: `cmake --build --preset release`
Expected: FAIL or missing-shader log if shader files are not yet wired.

**Step 3: Write minimal implementation**

- Implement CPU surreal vertex/fragment shaders with:
  - model/view/projection transforms,
  - height/slope/rim driven color composition,
  - uniforms: `uTime`, `uEnergy`, `uBurst`, `uPalettePhase`, `uColor`.
- Load shader in `Engine::SetupShaders` and keep pointer in `Engine`.

**Step 4: Run test to verify it passes**

Run: `cmake --build --preset release`
Expected: PASS and shader loaded without compile/link errors.

**Step 5: Commit**

```bash
git add assets/shaders/cpu_surreal.vert assets/shaders/cpu_surreal.frag src/engine/Engine.h src/engine/Engine.cpp
git commit -m "feat: add dedicated surreal CPU shader pipeline"
```

### Task 2: Convert CPU geometry from wireframe grid to shaded mesh

**Files:**
- Modify: `src/visualizers/CPUVisualizer.h`

**Step 1: Write the failing test**

Define behavior expectation:
- CPU visualizer must generate triangle indices and valid per-vertex normals.

**Step 2: Run test to verify it fails**

Run: `cmake --build --preset release`
Expected: existing draw path still uses `GL_LINES` and flat normals.

**Step 3: Write minimal implementation**

- Replace line index topology with triangle index topology.
- Compute normals from neighboring heights each update.
- Keep optional low-opacity line overlay as a second draw pass (toggleable constant).

**Step 4: Run test to verify it passes**

Run: `cmake --build --preset release`
Expected: PASS, CPU visualizer renders solid surface.

**Step 5: Commit**

```bash
git add src/visualizers/CPUVisualizer.h
git commit -m "update cpu visualizer to shaded mesh topology"
```

### Task 3: Add surreal motion synthesis (macro/meso/micro)

**Files:**
- Modify: `src/visualizers/CPUVisualizer.h`
- Modify: `src/SystemMonitor.h`
- Modify: `src/SystemMonitor.cpp`

**Step 1: Write the failing test**

Define behavior expectation:
- CPU motion should separate into slow drift, structural morph, and burst accents.

**Step 2: Run test to verify it fails**

Run: `cmake --build --preset release`
Expected: current behavior uses mostly direct spectrum + sine, lacking layered motion grammar.

**Step 3: Write minimal implementation**

- Add light-weight CPU burst metric (derivative envelope) in monitor or visualizer.
- In CPU mesh synthesis:
  - macro term from smoothed CPU,
  - meso term from spectrum band interpolation,
  - micro term from burst with decay.
- Apply shaping (`smoothstep`/pow-like curve) and clamp/slew limits.

**Step 4: Run test to verify it passes**

Run: `cmake --build --preset release`
Expected: PASS and visibly layered motion behavior.

**Step 5: Commit**

```bash
git add src/visualizers/CPUVisualizer.h src/SystemMonitor.h src/SystemMonitor.cpp
git commit -m "feat: add layered surreal modulation for cpu mesh"
```

### Task 4: Route CPU layer to surreal shader and uniforms

**Files:**
- Modify: `src/engine/Engine.cpp`
- Modify: `src/visualizers/CPUVisualizer.h`

**Step 1: Write the failing test**

Define behavior expectation:
- CPU layer should use `m_cpuShader` instead of generic `m_mainShader`.

**Step 2: Run test to verify it fails**

Run: `cmake --build --preset release`
Expected: CPU still uses generic shader path.

**Step 3: Write minimal implementation**

- In per-layer render loop, choose CPU shader for CPU layer.
- Push surreal uniforms (`uTime`, `uEnergy`, `uBurst`, `uPalettePhase`) from CPU visualizer state.

**Step 4: Run test to verify it passes**

Run: `cmake --build --preset release`
Expected: PASS and CPU layer visibly uses surreal shading.

**Step 5: Commit**

```bash
git add src/engine/Engine.cpp src/visualizers/CPUVisualizer.h
git commit -m "update cpu layer to use dedicated surreal shader"
```

### Task 5: Add CPU surreal tuning controls (minimal)

**Files:**
- Modify: `src/Config.h`
- Modify: `src/Config.cpp`
- Modify: `src/settings/MetricsPanelLogic.cpp`
- Modify: `src/resource.h`

**Step 1: Write the failing test**

Define behavior expectation:
- config should persist CPU surreal tuning values.

**Step 2: Run test to verify it fails**

Run: `cmake --build --preset release`
Expected: no persisted surreal-specific CPU controls exist yet.

**Step 3: Write minimal implementation**

- Add 2-3 CPU surreal controls only (YAGNI):
  - `cpuSurrealIntensity`
  - `cpuSurrealWarp`
  - `cpuBurstAccent`
- Wire load/save and settings dialog controls.

**Step 4: Run test to verify it passes**

Run: `cmake --build --preset release`
Expected: PASS and values round-trip via config file.

**Step 5: Commit**

```bash
git add src/Config.h src/Config.cpp src/settings/MetricsPanelLogic.cpp src/resource.h
git commit -m "feat: add persisted cpu surreal tuning controls"
```

### Task 6: Visual verification and polish

**Files:**
- Modify: `README.md`
- Modify: `Arch.md`

**Step 1: Write the failing test**

Define behavior expectation:
- docs should describe CPU surreal visual design and tuning behavior.

**Step 2: Run test to verify it fails**

Run: `cmake --build --preset release`
Expected: build passes, but docs do not reflect new CPU path.

**Step 3: Write minimal implementation**

- Document shader files, behavior mapping, and tuning controls.

**Step 4: Run test to verify it passes**

Run:
- `cmake --build --preset release`
- `.out/build/Release/ScreenSaver.scr /s`
- `.out/build/Release/ScreenSaver.scr /c`

Expected:
- Build succeeds.
- CPU layer appears surreal/expressive and stable.
- Settings changes are reflected and persisted.

**Step 5: Commit**

```bash
git add README.md Arch.md
git commit -m "docs: describe surreal cpu visualizer pipeline"
```
