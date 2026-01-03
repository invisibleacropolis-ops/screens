#pragma once

#include <pdh.h>
#include <pdhmsg.h>
#include <string>
#include <windows.h>


class SystemMonitor {
public:
  SystemMonitor();
  ~SystemMonitor();

  void Initialize();
  void Update();

  double GetCpuUsage() const { return cpuUsage; }
  double GetRamUsage() const { return ramUsage; } // % used
  double GetDiskUsage() const { return diskUsage; }
  double GetNetworkBytesPerSec() const { return networkBytesPerSec; }

private:
  PDH_HQUERY cpuQuery;
  PDH_HCOUNTER cpuTotal;

  PDH_HQUERY diskQuery;
  PDH_HCOUNTER diskTotal;

  PDH_HQUERY netQuery;
  PDH_HCOUNTER netTotal;

  double cpuUsage = 0.0;
  double ramUsage = 0.0;
  double diskUsage = 0.0;
  double networkBytesPerSec = 0.0; // Aggregated
};
