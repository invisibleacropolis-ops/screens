#pragma once

#include <pdh.h>
#include <pdhmsg.h>
#include <string>
#include <vector>
#include <windows.h>

class SystemMonitor {
public:
  SystemMonitor();
  ~SystemMonitor();

  void Initialize();
  void Update();

  double GetCpuUsage() const { return cpuUsage; }
  double GetRamUsage() const { return ramUsage; }
  double GetDiskUsage() const { return diskUsage; } // % Disk Time
  double GetNetworkBytesPerSec() const { return networkBytesPerSec; }

  // New Spectrum Metrics
  double GetContextSwitches() const { return contextSwitches; }
  double GetInterrupts() const { return interrupts; }
  double GetSystemCalls() const { return systemCalls; }
  double GetPageFaults() const { return pageFaults; }
  double GetProcessCount() const { return processCount; }
  double GetThreadCount() const { return threadCount; }
  double GetHandleCount() const { return handleCount; }
  double GetReadBytes() const { return readBytes; }
  double GetWriteBytes() const { return writeBytes; }

  // Helper to normalize into 0-1 range for visualizer bands (0-9)
  float GetSpectrumBand(int index) const;

private:
  PDH_HQUERY cpuQuery;
  PDH_HCOUNTER cpuTotal;
  PDH_HCOUNTER ctxtTotal;    // Context Switches
  PDH_HCOUNTER intrTotal;    // Interrupts
  PDH_HCOUNTER sysCallTotal; // System Calls

  PDH_HQUERY memQuery;
  PDH_HCOUNTER pageFaultTotal;

  PDH_HQUERY diskQuery;
  PDH_HCOUNTER diskTotal;
  PDH_HCOUNTER diskReadTotal;
  PDH_HCOUNTER diskWriteTotal;

  PDH_HQUERY sysQuery; // For Process/Thread/Handle counts
  PDH_HCOUNTER processCounter;
  PDH_HCOUNTER threadCounter;
  PDH_HCOUNTER handleCounter;

  PDH_HQUERY netQuery;
  std::vector<PDH_HCOUNTER> netCounters;

  // Smoothed Values
  double cpuUsage = 0.0;
  double ramUsage = 0.0;
  double diskUsage = 0.0;
  double networkBytesPerSec = 0.0;

  double contextSwitches = 0.0;
  double interrupts = 0.0;
  double systemCalls = 0.0;
  double pageFaults = 0.0;
  double processCount = 0.0;
  double threadCount = 0.0;
  double handleCount = 0.0;
  double readBytes = 0.0;
  double writeBytes = 0.0;

  // Raw Values for Smoothing
  double rawCpuUsage = 0.0;
  double rawRamUsage = 0.0;
  double rawDiskUsage = 0.0;
  double rawNetworkBytesPerSec = 0.0;

  double rawContextSwitches = 0.0;
  double rawInterrupts = 0.0;
  double rawSystemCalls = 0.0;
  double rawPageFaults = 0.0;
  double rawProcessCount = 0.0;
  double rawThreadCount = 0.0;
  double rawHandleCount = 0.0;
  double rawReadBytes = 0.0;
  double rawWriteBytes = 0.0;

  bool smoothingInitialized = false;
  double smoothingAlpha = 0.2;
};
