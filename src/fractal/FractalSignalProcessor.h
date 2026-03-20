#pragma once

#include "../Config.h"
#include "../SystemMonitor.h"
#include "FractalParams.h"

class FractalSignalProcessor {
public:
  void Reset();
  void Update(float dt, const SystemMonitor &monitor, const Config &config);
  const FractalParams &GetParams() const { return m_params; }

private:
  float m_cpu = 0.0f;
  float m_ram = 0.0f;
  float m_disk = 0.0f;
  float m_net = 0.0f;

  float m_prevCpu = 0.0f;
  float m_prevRam = 0.0f;
  float m_prevDisk = 0.0f;
  float m_prevNet = 0.0f;

  bool m_initialized = false;
  FractalParams m_params;
};
