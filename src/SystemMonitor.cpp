#include "SystemMonitor.h"
#include <iostream>

#pragma comment(lib, "pdh.lib")

SystemMonitor::SystemMonitor() {
  cpuQuery = NULL;
  cpuTotal = NULL;
  diskQuery = NULL;
  diskTotal = NULL;
  netQuery = NULL;
  netTotal = NULL;
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
  // Note: Network interface wildcard is tricky because we want the sum.
  // For simplicity in this V1, we might just grab the first interface or try to
  // sum them. PdhAddEnglishCounter supports wildcards but reading them requires
  // parsing arrays. Simplification: Let's try to add the Total for Network
  // Interface if it exists, otherwise we might need to iterate. "Network
  // Interface" usually requires specific instance names. Let's stick to a safe
  // approach: Just init the query, we will need to handle multiple counters if
  // we want accuracy. For MVP, let's try grabbing a common one or skip complex
  // network agg for this exact moment if it risks crash. Actually, `\Network
  // Interface(*)\Bytes Total/sec` is valid for PdhExpandWildCardPath, but
  // complex to read. Let's use a simpler heuristic or just skip strict network
  // for the VERY FIRST compile to ensure stability, then add it back. Use
  // Memory as "Network" placeholder? No, let's stick to the plan but do it
  // safely. We will just try one counter for now.
  // PdhAddEnglishCounter(netQuery, L"\\Network Interface(*)\\Bytes Total/sec",
  // NULL, &netTotal); (Wildcards in AddCounter produce a handle to the first or
  // requires special read)
}

void SystemMonitor::Update() {
  PDH_FMT_COUNTERVALUE counterVal;

  // CPU
  if (cpuQuery) {
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    cpuUsage = counterVal.doubleValue;
  }

  // Disk
  if (diskQuery) {
    PdhCollectQueryData(diskQuery);
    PdhGetFormattedCounterValue(diskTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    diskUsage = counterVal.doubleValue;
    // Clamp to 100 as disk time can exceed 100% physically (multiple
    // disks/queues)
    if (diskUsage > 100.0)
      diskUsage = 100.0;
  }

  // RAM (No PDH needed for basic usage)
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
  // dwMemoryLoad is physically used %
  ramUsage = (double)memInfo.dwMemoryLoad;

  // Network (stubbed for now to avoid wildcard complexity in first pass)
  networkBytesPerSec = 0.0;
}
