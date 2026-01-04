#include "SystemMonitor.h"
#include <iostream>
#include <vector>
#include <cwchar>

#pragma comment(lib, "pdh.lib")

SystemMonitor::SystemMonitor() {
  cpuQuery = NULL;
  cpuTotal = NULL;
  diskQuery = NULL;
  diskTotal = NULL;
  netQuery = NULL;
}

SystemMonitor::~SystemMonitor() {
  if (cpuQuery)
    PdhCloseQuery(cpuQuery);
  if (diskQuery)
    PdhCloseQuery(diskQuery);
  if (netQuery)
    PdhCloseQuery(netQuery);
}

void SystemMonitor::Initialize() {
  // Initialize CPU Query
  PdhOpenQueryW(NULL, 0, &cpuQuery);
  // English counter names are generally preferred for portability if possible,
  // but standard Windows installs "Processor" should work.
  // Creating a wildcard query or specific total query.
  PdhAddEnglishCounterW(cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0,
                        &cpuTotal);
  PdhCollectQueryData(cpuQuery);

  // Initialize Disk Query
  PdhOpenQueryW(NULL, 0, &diskQuery);
  PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\% Disk Time", 0,
                        &diskTotal);
  PdhCollectQueryData(diskQuery);

  // Initialize Network Query
  PdhOpenQueryW(NULL, 0, &netQuery);
  DWORD bufferSize = 0;
  PDH_STATUS netStatus = PdhExpandWildCardPathW(
      NULL, L"\\Network Interface(*)\\Bytes Total/sec", NULL, &bufferSize, 0);
  if (netStatus == PDH_MORE_DATA && bufferSize > 0) {
    std::vector<wchar_t> buffer(bufferSize);
    netStatus = PdhExpandWildCardPathW(
        NULL, L"\\Network Interface(*)\\Bytes Total/sec", buffer.data(),
        &bufferSize, 0);
    if (netStatus == ERROR_SUCCESS) {
      const wchar_t *cursor = buffer.data();
      while (*cursor != L'\0') {
        PDH_HCOUNTER counter = NULL;
        if (PdhAddEnglishCounterW(netQuery, cursor, 0, &counter) ==
            ERROR_SUCCESS) {
          netCounters.push_back(counter);
        }
        cursor += wcslen(cursor) + 1;
      }
    }
  }
  if (!netCounters.empty()) {
    PdhCollectQueryData(netQuery);
  }
}

void SystemMonitor::Update() {
  PDH_FMT_COUNTERVALUE counterVal;

  // CPU
  if (cpuQuery) {
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    rawCpuUsage = counterVal.doubleValue;
  }

  // Disk
  if (diskQuery) {
    PdhCollectQueryData(diskQuery);
    PdhGetFormattedCounterValue(diskTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    rawDiskUsage = counterVal.doubleValue;
    // Clamp to 100 as disk time can exceed 100% physically (multiple
    // disks/queues)
    if (rawDiskUsage > 100.0)
      rawDiskUsage = 100.0;
  }

  // RAM (No PDH needed for basic usage)
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
  // dwMemoryLoad is physically used %
  rawRamUsage = (double)memInfo.dwMemoryLoad;

  // Network
  rawNetworkBytesPerSec = 0.0;
  if (netQuery && !netCounters.empty()) {
    PdhCollectQueryData(netQuery);
    for (PDH_HCOUNTER counter : netCounters) {
      if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL,
                                      &counterVal) == ERROR_SUCCESS) {
        rawNetworkBytesPerSec += counterVal.doubleValue;
      }
    }
  }

  // Apply exponential moving average smoothing to stabilize visual response.
  if (!smoothingInitialized) {
    cpuUsage = rawCpuUsage;
    ramUsage = rawRamUsage;
    diskUsage = rawDiskUsage;
    networkBytesPerSec = rawNetworkBytesPerSec;
    smoothingInitialized = true;
  } else {
    cpuUsage =
        smoothingAlpha * rawCpuUsage + (1.0 - smoothingAlpha) * cpuUsage;
    ramUsage =
        smoothingAlpha * rawRamUsage + (1.0 - smoothingAlpha) * ramUsage;
    diskUsage =
        smoothingAlpha * rawDiskUsage + (1.0 - smoothingAlpha) * diskUsage;
    networkBytesPerSec = smoothingAlpha * rawNetworkBytesPerSec +
                         (1.0 - smoothingAlpha) * networkBytesPerSec;
  }
}
