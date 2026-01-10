#pragma once

#include <array>

// Blend modes for layer compositing
enum class BlendMode {
  Additive, // Add layer colors
  Multiply, // Multiply colors
  Screen,   // Screen blend (1 - (1-a)*(1-b))
  Overlay,  // Overlay blend
  Normal,   // Standard alpha blend
  Count
};

// Layer transform - position and size as normalized values (0.0-1.0)
// Allows each visualizer to be positioned and sized like a window
struct LayerTransform {
  // Position (0.0 = left/top, 1.0 = right/bottom)
  float x = 0.0f; // Normalized X position
  float y = 0.0f; // Normalized Y position

  // Size (0.0-1.0 as fraction of screen)
  float width = 1.0f;  // Normalized width
  float height = 1.0f; // Normalized height

  // Anchor point for positioning (0=top-left, 0.5=center, 1=bottom-right)
  float anchorX = 0.0f;
  float anchorY = 0.0f;

  // Z-depth for 3D offset effect
  float depth = 0.0f; // -1.0 to 1.0 (negative = closer)

  // Rotation in degrees
  float rotation = 0.0f;

  // Lock aspect ratio when resizing
  bool lockAspect = false;

  // Is this layer visible?
  bool visible = true;

  // Preset layout positions
  static LayerTransform FullScreen() {
    return LayerTransform{0, 0, 1, 1, 0, 0, 0, 0, false, true};
  }

  static LayerTransform TopLeft() {
    return LayerTransform{0, 0, 0.5f, 0.5f, 0, 0, 0, 0, false, true};
  }

  static LayerTransform TopRight() {
    return LayerTransform{0.5f, 0, 0.5f, 0.5f, 0, 0, 0, 0, false, true};
  }

  static LayerTransform BottomLeft() {
    return LayerTransform{0, 0.5f, 0.5f, 0.5f, 0, 0, 0, 0, false, true};
  }

  static LayerTransform BottomRight() {
    return LayerTransform{0.5f, 0.5f, 0.5f, 0.5f, 0, 0, 0, 0, false, true};
  }

  static LayerTransform Centered(float w, float h) {
    return LayerTransform{0.5f - w / 2, 0.5f - h / 2, w, h, 0, 0, 0, 0,
                          false,        true};
  }

  static LayerTransform QuadLayout(int index) {
    switch (index % 4) {
    case 0:
      return TopLeft();
    case 1:
      return TopRight();
    case 2:
      return BottomLeft();
    case 3:
      return BottomRight();
    default:
      return FullScreen(); // Should not happen with % 4
    }
  }
};

// Post-processing effects configuration per layer
// Designed for easy expansion - add new FX here as needed
struct PostProcessConfig {
  // === LAYER TRANSFORM (position, size) ===
  LayerTransform transform;
  // === BLOOM ===
  bool bloomEnabled = false;
  float bloomThreshold = 0.8f; // Brightness threshold for bloom
  float bloomIntensity = 1.0f; // Bloom strength
  float bloomRadius = 5.0f;    // Blur radius for bloom

  // === GLOW ===
  bool glowEnabled = false;
  float glowColor[3] = {1.0f, 1.0f, 1.0f}; // RGB glow tint
  float glowIntensity = 0.5f;
  float glowSize = 3.0f;

  // === COLOR ADJUSTMENTS ===
  float brightness = 1.0f;                 // 0.0 - 2.0
  float contrast = 1.0f;                   // 0.0 - 2.0
  float saturation = 1.0f;                 // 0.0 - 2.0
  float hueShift = 0.0f;                   // 0.0 - 360.0 degrees
  float tintColor[3] = {1.0f, 1.0f, 1.0f}; // Color tint
  float tintStrength = 0.0f;               // 0.0 - 1.0

  // === DISTORTION ===
  bool distortionEnabled = false;
  float distortionAmount = 0.0f; // Wave/ripple distortion
  float distortionFreq = 10.0f;  // Frequency of distortion
  float distortionSpeed = 1.0f;  // Animation speed

  // === CHROMATIC ABERRATION ===
  bool chromaticEnabled = false;
  float chromaticOffset = 2.0f;  // Pixel offset for RGB split
  float chromaticFalloff = 0.5f; // Edge falloff

  // === VIGNETTE ===
  bool vignetteEnabled = false;
  float vignetteIntensity = 0.3f;
  float vignetteRadius = 0.8f;
  float vignetteColor[3] = {0.0f, 0.0f, 0.0f};

  // === SCAN LINES ===
  bool scanLinesEnabled = false;
  float scanLinesDensity = 100.0f;
  float scanLinesIntensity = 0.2f;
  float scanLinesSpeed = 0.0f; // Moving scan lines

  // === NOISE/GRAIN ===
  bool noiseEnabled = false;
  float noiseAmount = 0.05f;
  bool noiseAnimated = true;

  // === PIXELATION ===
  bool pixelateEnabled = false;
  float pixelateSize = 4.0f; // Pixel block size

  // === EDGE GLOW / OUTLINE ===
  bool edgeGlowEnabled = false;
  float edgeGlowColor[3] = {1.0f, 1.0f, 1.0f};
  float edgeGlowWidth = 2.0f;
  float edgeGlowIntensity = 1.0f;

  // === MOTION BLUR ===
  bool motionBlurEnabled = false;
  float motionBlurAmount = 0.5f;
  int motionBlurSamples = 8;

  // === TRAILS / PERSISTENCE ===
  bool trailsEnabled = false;
  float trailsFade = 0.9f; // How fast trails fade (0-1)

  // === LAYER COMPOSITING ===
  float opacity = 1.0f; // Layer opacity (0-1)
  BlendMode blendMode = BlendMode::Additive;
  int renderOrder = 0; // Lower = rendered first (background)
};

// Default FX presets for each metric type
namespace FXPresets {

inline PostProcessConfig CPUDefault() {
  PostProcessConfig cfg;
  cfg.bloomEnabled = true;
  cfg.bloomIntensity = 0.8f;
  cfg.glowEnabled = true;
  cfg.glowColor[0] = 0.2f;
  cfg.glowColor[1] = 0.5f;
  cfg.glowColor[2] = 1.0f; // Blue
  cfg.glowIntensity = 0.6f;
  cfg.renderOrder = 0;
  return cfg;
}

inline PostProcessConfig RAMDefault() {
  PostProcessConfig cfg;
  cfg.bloomEnabled = true;
  cfg.bloomIntensity = 0.5f;
  cfg.glowEnabled = true;
  cfg.glowColor[0] = 0.2f;
  cfg.glowColor[1] = 1.0f;
  cfg.glowColor[2] = 0.3f; // Green
  cfg.glowIntensity = 0.5f;
  cfg.scanLinesEnabled = true;
  cfg.scanLinesIntensity = 0.1f;
  cfg.renderOrder = 1;
  return cfg;
}

inline PostProcessConfig DiskDefault() {
  PostProcessConfig cfg;
  cfg.bloomEnabled = true;
  cfg.bloomIntensity = 1.0f;
  cfg.glowEnabled = true;
  cfg.glowColor[0] = 1.0f;
  cfg.glowColor[1] = 0.8f;
  cfg.glowColor[2] = 0.2f; // Yellow/Gold
  cfg.glowIntensity = 0.7f;
  cfg.motionBlurEnabled = true;
  cfg.motionBlurAmount = 0.3f;
  cfg.renderOrder = 2;
  return cfg;
}

inline PostProcessConfig NetworkDefault() {
  PostProcessConfig cfg;
  cfg.chromaticEnabled = true;
  cfg.chromaticOffset = 3.0f;
  cfg.glowEnabled = true;
  cfg.glowColor[0] = 0.8f;
  cfg.glowColor[1] = 0.2f;
  cfg.glowColor[2] = 1.0f; // Purple
  cfg.glowIntensity = 0.6f;
  cfg.trailsEnabled = true;
  cfg.trailsFade = 0.85f;
  cfg.renderOrder = 3;
  return cfg;
}

} // namespace FXPresets
