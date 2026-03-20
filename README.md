# Windows OpenGL Screensaver Project

This project contains the source code for a native Windows Screensaver (`.scr`) using OpenGL.

## Build SOP

### Prerequisites
- CMake 3.20 or later
- Visual Studio 2022 (or compatible C++ compiler) with Windows SDK

### Build Instructions
1.  Open a terminal (PowerShell is recommended).
2.  Navigate to the project root: `cd c:\GITHUB\screens`
3.  Configure the project:
    ```powershell
    cmake -S . -B .out/build
    ```
4.  Build the project (Release mode recommended for distribution/usage):
    ```powershell
    cmake --build .out/build --config Release
    ```
5.  The output executable will be located at:
    `.out\build\Release\ScreenSaver.scr`

### Build With Presets (Recommended)

The repository includes `CMakePresets.json` for consistent local/CI builds.

```powershell
cmake --preset windows-msvc
cmake --build --preset release
```

### SSOT Paths Policy

Generated files must live under `.out/` only.

- `.out/build/`: CMake binary tree and intermediate build state.
- `.out/logs/`: runtime and diagnostic logs (including `screensaver_debug.log`).
- `.out/artifacts/`: packaged outputs (optional release bundles).
- `.out/toolchains/`: local tool state such as a project-local vcpkg clone.

Forbidden root-level redundant paths:

- `build/`
- `vcpkg/`
- `designdocs/`
- `build_log.txt`
- `build_error_log.txt`
- `screensaver_debug.log`

Run SSOT verification:

```powershell
./scripts/verify-ssot-paths.ps1
```

The verifier is strict:

- It enforces a top-level allowlist for repo root paths.
- It fails on any unexpected root folder/file.
- It fails if build output exists outside `.out/build`.
- It is run in CI on every build workflow.

### Installing / Testing
- **Test**: Run `.out\build\Release\ScreenSaver.scr /s` or right-click the file and select "Test".
- **Install**: Right-click `.out\build\Release\ScreenSaver.scr` and select "Install".

## Development Notes

- **Entry Point**: `src/main.cpp` handles the Windows standard screensaver arguments (`/s`, `/c`, `/p`).
- **Rendering**: `src/renderer.cpp` contains the OpenGL logic. Modify `DrawScene()` to change the visualization.
- **Dependencies**: The project currently uses raw Win32 API and WGL (`opengl32.lib`, `glu32.lib`). No external package manager is required.

## CI

- GitHub Actions workflow: `.github/workflows/windows-build.yml`
- Runs configure/build on Windows for pushes to `master` and pull requests.

## Agent Protocol

**IMPORTANT**: Every AI agent working on this folder MUST document their work in `EngineerLog.md`.

### Engineer Log Format
At the end of every session, append an entry to `EngineerLog.md` with:
- **Date**: Current date/time.
- **Agent**: (Identity/Name).
- **Task**: Brief description of what was worked on.
- **Changes**: Key files modified or created.
- **Completion %**: Estimate of how complete the current specific task is.
- **Remaining TODO**: What should be picked up by the next agent/session.

This ensures continuity and clarity across different sessions.
