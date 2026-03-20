#pragma once

struct FractalParams {
  int octaves = 5;
  float lacunarity = 2.0f;
  float gain = 0.5f;

  float baseScale = 1.0f;
  float amplitude = 1.0f;
  float warpAmount = 0.2f;
  float warpSpeed = 0.8f;

  float ridgeMix = 0.25f;
  float palettePhase = 0.0f;
  float energy = 0.0f;
};
