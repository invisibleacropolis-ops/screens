# SSOT Paths Design

## Objective

Enforce a single source-of-truth output root so generated files never pollute repository root.

## SSOT Root

All generated files must live under `.out/`.

### Structure

- `.out/build/` - CMake configure/build tree
- `.out/logs/` - runtime and diagnostic logs
- `.out/artifacts/` - optional packaged releases
- `.out/toolchains/` - optional local tool state (for example vcpkg clone)

## Rules

1. Presets must configure into `.out/build`.
2. Runtime logging must write into `.out/logs`.
3. CI and local commands must use the same preset contract.
4. Root-level generated paths are migration leftovers only and must not be recreated.
