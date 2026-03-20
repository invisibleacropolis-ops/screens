#include "FractalSignalProcessor.h"

#include <algorithm>
#include <cmath>

namespace {
float Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

float NormalizeNetwork(double bytesPerSec) {
  constexpr double kMaxExpected = 120.0 * 1024.0 * 1024.0;
  return Clamp01(static_cast<float>(bytesPerSec / kMaxExpected));
}
} // namespace

void FractalSignalProcessor::Reset() {
  m_initialized = false;
  m_cpu = 0.0f;
  m_ram = 0.0f;
  m_disk = 0.0f;
  m_net = 0.0f;
  m_prevCpu = 0.0f;
  m_prevRam = 0.0f;
  m_prevDisk = 0.0f;
  m_prevNet = 0.0f;
  m_params = FractalParams{};
}

void FractalSignalProcessor::Update(float dt, const SystemMonitor &monitor,
                                    const Config &config) {
  const float safeDt = (dt > 0.0001f) ? dt : 0.016f;

  const float cpuRaw = Clamp01(static_cast<float>(monitor.GetCpuUsage() / 100.0));
  const float ramRaw = Clamp01(static_cast<float>(monitor.GetRamUsage() / 100.0));
  const float diskRaw =
      Clamp01(static_cast<float>(monitor.GetDiskUsage() / 100.0));
  const float netRaw = NormalizeNetwork(monitor.GetNetworkBytesPerSec());

  constexpr float kAlpha = 0.2f;
  if (!m_initialized) {
    m_cpu = cpuRaw;
    m_ram = ramRaw;
    m_disk = diskRaw;
    m_net = netRaw;
    m_prevCpu = cpuRaw;
    m_prevRam = ramRaw;
    m_prevDisk = diskRaw;
    m_prevNet = netRaw;
    m_initialized = true;
  } else {
    m_cpu = m_cpu + (cpuRaw - m_cpu) * kAlpha;
    m_ram = m_ram + (ramRaw - m_ram) * kAlpha;
    m_disk = m_disk + (diskRaw - m_disk) * kAlpha;
    m_net = m_net + (netRaw - m_net) * kAlpha;
  }

  const float dCpu = Clamp01(std::fabs(m_cpu - m_prevCpu) / safeDt * 0.3f);
  const float dRam = Clamp01(std::fabs(m_ram - m_prevRam) / safeDt * 0.3f);
  const float dDisk = Clamp01(std::fabs(m_disk - m_prevDisk) / safeDt * 0.3f);
  const float dNet = Clamp01(std::fabs(m_net - m_prevNet) / safeDt * 0.3f);

  m_prevCpu = m_cpu;
  m_prevRam = m_ram;
  m_prevDisk = m_disk;
  m_prevNet = m_net;

  const float flowEnergy = Clamp01((m_cpu + m_net + dDisk) / 3.0f);
  const float structureEnergy = Clamp01((m_ram + m_disk + dCpu) / 3.0f);
  const float burstEnergy = Clamp01((dCpu + dDisk + dNet + dRam) * 0.4f);
  const float response = std::clamp(config.fractalResponse, 0.1f, 3.0f);
  const float warpScale = std::clamp(config.fractalWarp, 0.1f, 3.0f);
  const float speedScale = std::clamp(config.fractalSpeed, 0.1f, 3.0f);

  int baseOctaves = 5;
  if (config.quality == QualityTier::Low)
    baseOctaves = 4;
  else if (config.quality == QualityTier::High)
    baseOctaves = 6;

  m_params.octaves =
      std::clamp(baseOctaves + (flowEnergy > 0.8f ? 1 : 0), 3, 7);
  m_params.lacunarity = 1.8f + (m_net * 1.2f);
  m_params.gain = 0.35f + (m_ram * 0.45f);
  m_params.baseScale = 0.9f + (structureEnergy * 2.0f);
  m_params.amplitude = (0.2f + (structureEnergy * 1.8f)) * response;
  m_params.warpAmount = (0.04f + flowEnergy * 0.35f + burstEnergy * 0.2f) * warpScale;
  m_params.warpSpeed = (0.2f + m_disk * 1.0f + dNet * 0.9f) * speedScale;
  m_params.ridgeMix = Clamp01(0.15f + m_disk * 0.5f + burstEnergy * 0.4f);
  m_params.energy = Clamp01(0.5f * structureEnergy + 0.5f * flowEnergy);
  m_params.palettePhase += safeDt * (0.15f + m_net * 0.9f + burstEnergy * 0.7f);
  if (m_params.palettePhase > 1000.0f) {
    m_params.palettePhase = std::fmod(m_params.palettePhase, 1000.0f);
  }
}
