# Screensaver Architecture Documentation

This document provides a technical overview of the "Simple OpenGL Screensaver" project. It is intended as a reference for engineers working on the codebase, detailing the architectural patterns, core subsystems, and data flow.

## 1. High-Level Architecture

The application follows a **Modular Engine Architecture**, separating platform-specific window management from the core rendering and logic loop. The rendering pipeline is **Layer-Based**, allowing for independent visualization of different system metrics (CPU, RAM, Disk, Network) which are then composited together.

### Core Components

*   **Platform Layer (`main.cpp`)**: Handles the Windows entry point (`wWinMain`), command-line parsing (screensaver modes `/s`, `/c`, `/p`), window creation, and the main message loop.
*   **Engine (`Engine.cpp/h`)**: The central orchestrator. It manages the OpenGL context, initializes subsystems, and runs the update/render loop.
*   **System Monitor (`SystemMonitor.cpp/h`)**: A standalone module that polls Windows Performance Counters (PDH) to gather real-time system metrics.
*   **Graphics Subsystem**:
    *   **Layers (`VisualizerLayer`)**: Encapsulates a framebuffer (FBO) and a specific visualizer instance.
    *   **Compositor (`LayerCompositor`)**: Blends multiple layers onto a single output texture using configurable blending modes (Additive, Multiply, etc.).
    *   **Abstraction Wrappers**: `Shader`, `Mesh`, `Texture` classes wrap raw OpenGL calls.

## 2. Directory Structure

```text
src/
├── main.cpp                # Entry point & Window management
├── Config.h/cpp            # Configuration data structures & loading/saving
├── Logger.h                # Simple file-based logging utility
├── engine/
│   ├── Engine.h/cpp        # Core engine class
│   ├── SystemMonitor.h/cpp # PDH-based system metrics polling
│   ├── DebugUtils.h        # OpenGL error checking helpers
│   └── Math.h/cpp          # Math utilities (Matrices, Vectors)
├── graphics/
│   ├── LayerCompositor.h   # Multi-layer compositing logic
│   ├── VisualizerLayer.h   # Encapsulation of a render layer (FBO + Visualizer)
│   ├── Shader.h/cpp        # GLSL Shader program wrapper
│   ├── Mesh.h/cpp          # Geometry generation (Sphere, Cube, Quad)
│   └── PostProcessConfig.h # Configuration struct for layer FX
├── visualizers/
│   ├── IVisualizer.h       # Interface for all metric visualizers
│   ├── CPUVisualizer.h     # Specific implementation for CPU
│   ├── RAMVisualizer.h     # Specific implementation for RAM
│   └── ...
└── glad/                   # OpenGL function loader
```

## 3. Data Flow

### 3.1. Initialization
1.  **Entry**: `wWinMain` parses command line arguments to determine the mode:
    *   **Saver (`/s`)**: Runs the screensaver fullscreen.
    *   **Config (`/c`)**: Open settings dialog (not fully implemented in Engine yet).
    *   **Preview (`/p`)**: Renders to a child window inside the Windows Display Properties dialog.
2.  **Engine Setup**: `Engine::Initialize` creates the OpenGL context (WGL), loads function pointers (GLAD), and initializes `SystemMonitor`.
3.  **Asset Loading**: Key assets (Shaders, Meshes) are loaded.
4.  **Layer Setup**: 4 Layers are created (CPU, RAM, Disk, Network), each assigned a specific `Visualizer` implementation and a default `PostProcessConfig`.

### 3.2. Examples of the Render Loop (`Engine::Render`)
Each frame follows this sequence:

1.  **Update Metrics**: `SystemMonitor::Update` runs, refreshing CPU/RAM/Disk stats using Windows PDH.
2.  **Update Scene**: Camera rotation and global variables are updated based on `dt` (delta time).
3.  **Render Layers** (`Engine::RenderToLayers`):
    *   Iterates through all active layers.
    *   Binds the Layer's **Framebuffer (FBO)**.
    *   Calls the specific `IVisualizer::Draw` method.
    *   The visualizer renders its content (e.g., a spinning cube for RAM usage) into the FBO's texture.
4.  **Composite** (`Engine::CompositeLayers` -> `LayerCompositor::Composite`):
    *   Layers are sorted by Z-order.
    *   The **Compositor FBO** is bound.
    *   Each layer's texture is drawn as a fullscreen quad using the `passthrough` shader.
    *   Blending is applied based on `BlendMode` (e.g., Additive for "holographic" looks).
5.  **Present** (`LayerCompositor::Present`):
    *   The final Compositor FBO texture is drawn to the Default Framebuffer (Screen) using a fullscreen quad.
    *   `SwapBuffers` is called to display the frame.

## 4. Key Systems Detail

### System Monitor
*   Uses `pdh.lib`.
*   Maintain open queries for `\Processor(_Total)\% Processor Time`, `\Memory\% Committed Bytes In Use`, etc.
*   Provides smoothed values (updates heavily damped to prevent jitter).

### Graphics Pipeline
*   **OpenGL 3.3 Core Profile**.
*   **Shaders**:
    *   `basic.vert/frag`: Standard lit geometry (used by Visualizers).
    *   `passthrough.vert/frag`: Fullscreen quad rendering (used by Compositor).
*   **Debugging**:
    *   `DEBUG_OPENGL` define enables comprehensive logging using `DebugUtils.h`.
    *   Start-up logs written to `screensaver_debug.log`.

## 5. Adding a New Feature

### Adding a New Visualizer
1.  Create a class inheriting from `IVisualizer` (e.g., `GPUVisualizer`).
2.  Implement `Init`, `Update`, and `Draw`.
3.  In `Engine::SetupLayers`, assign this new visualizer to a target Layer.

### Adding a New Post-Process Effect
1.  Modify `PostProcessConfig.h` to include parameters (e.g., `blurStrength`).
2.  Update `LayerCompositor.h` to use a new shader (e.g., `blur.frag`) instead of `passthrough.frag` when drawing that layer.
