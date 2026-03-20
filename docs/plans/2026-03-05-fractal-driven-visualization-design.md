# Fractal-Driven Visualization Design

## Goal

Map system monitor metrics into a bounded fractal control system that drives animated 3D forms, instead of direct 1:1 metric-to-geometry mapping.

## Recommended Approach

Use an fBm + domain-warp fractal surface visualizer as Phase 1. It fits GLSL 330 and the current layer compositor architecture while staying performant.

## Architecture

1. Metric preprocessing outputs stable normalized signals and burst derivatives.
2. Fractal parameter mapping converts signals into bounded controls (octaves, gain, lacunarity, warp amount, warp speed, amplitude, palette phase).
3. A new FractalSurface visualizer evaluates domain-warped fBm each frame and produces 3D mesh displacement and color.
4. Existing layer compositing and post effects apply on top.

## Constraints

- Maintain recognizable visual identity under metric spikes.
- Clamp and slew-limit all fractal controls.
- Keep preview mode lightweight and stable.

## Verification

- Build with CMake presets and run `/s`, `/c`, `/p` smoke checks.
- Validate that CPU/RAM/Disk/Network change fractal behavior coherently without jitter.
