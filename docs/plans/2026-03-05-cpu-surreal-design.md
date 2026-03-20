# CPU Surreal Visual Design

## Goal

Redesign the CPU visualizer into a surreal, expressive 3D dreamscape that responds to system behavior without trying to imitate realistic terrain.

## Visual Direction

- Style: dreamlike sci-fi landscape, not geological realism.
- Shape language: fluid ridges, floating folds, and impossible depth cues.
- Motion language:
  - slow macro drift (ambient breathing),
  - medium structural morphing (sustained load),
  - short pulse accents (bursts/spikes).

## Data Mapping Principles

- CPU usage drives global amplitude and silhouette scale.
- CPU derivative/burst drives crest emissive pulses and local deformation spikes.
- Context switches/interrupts drive warp turbulence and fold density.
- Mapping uses shaping curves and slew limits to avoid jitter.

## Material and Color Principles

- Use a stylized procedural palette with constrained hue drift (cosine palette).
- Favor cool-to-electric transitions for sci-fi mood.
- Keep base tones darker and reserve bright accents for event peaks.
- Lighting stack: directional + hemisphere + rim + soft emissive accents.

## Geometry and Rendering

- CPU becomes a shaded triangle mesh surface (not wireframe-only).
- Per-frame normals are derived from neighboring heights.
- Optional sparse contour/wire overlay remains for technical readability.
- Depth fog and subtle bloom integration add atmospheric separation.

## Quality and Stability Constraints

- Preview mode must stay smooth and lightweight.
- Adaptive mesh resolution tied to quality tier.
- No hard flicker, no full-scene strobing under burst input.

## Success Criteria

1. Idle state feels alive and intentional.
2. Sustained CPU load produces coherent large-form evolution.
3. Burst events generate brief surreal accent events without chaos.
4. CPU visual identity feels premium and clearly distinct from RAM/Disk layers.
