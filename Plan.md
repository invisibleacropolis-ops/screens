# Visualizer Upgrade Plan

## 1. Current State Assessment
*   **CPU**: `Wireframe 3D Waveform`. **[DONE]**
    *   Status: Fully functional. height sensitivity is 1.5x. Scroll speed modulated by Disk Usage.
    *   Verdict: Excellent "Hero" visual.
*   **RAM**: `Pulsing Cube`. **[DONE]**
    *   Status: Simple scaling cube. Effective but basic.
    *   Upgrade Potential: Could be a field of "floating bits" or a "tank" filling up.
*   **Disk**: `Rotating Shape`. **[BASIC]**
    *   Status: Just rotates based on usage. Too generic.
    *   Verdict: Needs a distinct metaphor (e.g., Spinning Platter or Sector Map).
*   **Network**: **[MISSING]**
    *   Status: Not implemented.
    *   Verdict: Needs to be added. Good candidate for "particle flow".

## 2. Upgrade Proposals

### A. The "Data Ring" (Disk Upgrade)
Replace the simple rotating ring with a **segmented sector ring**.
*   **Visual**: A ring made of 16-32 distinct segments.
*   **Action**: Segments flash or pop out randomly when disk usage is high.
*   **Metaphor**: Reading/Writing data sectors.

### B. "Packet Flow" (Network Implementation)
Implement `NetworkVisualizer` as a stream of particles.
*   **Visual**: Two nodes (Earth/Internet). Particles fly between them.
*   **Data**: `BytesReceived` (Down stream) and `BytesSent` (Up stream).
*   **Metaphor**: Traffic flow.

### C. "Digital Bloom" (Post-Processing)
Add a post-processing step to make the wireframes and neon colors glow.
*   **Tech**: Gaussian Blur pass added to the compositor.
*   **Effect**: Gives that premium "Screensaver" aesthetic.

## 3. Implementation Tasks

- [ ] **Network Visualizer**
    - [ ] Create `NetworkVisualizer` class.
    - [ ] Implement particle system for "Packets".
    - [ ] Hook up to `SystemMonitor::GetNetworkBytesPerSec()`.
    
- [ ] **Disk Visualizer Upgrade**
    - [ ] Refactor `DiskVisualizer` to use instanced rendering or multiple draw calls for "Sectors".
    - [ ] Implement random flashing logic based on usage intensity.

- [ ] **Post-Processing (Bloom)**
    - [ ] Update `PostProcess` shader to include Bloom threshold/blur.
    - [ ] Add intermediate FBO for blur pass.

- [ ] **RAM Polish (Optional)**
    - [ ] Change single cube to a 3x3x3 grid of smaller cubes that "light up" in proportion to memory usage.

## 4. Outstanding TODOs
- [ ] **Code Cleanup**: Remove unused meshes/headers in `Engine.cpp`.
- [ ] **Config**: Ensure new visualizers expose their colors/speeds to `SettingsDialog`.
