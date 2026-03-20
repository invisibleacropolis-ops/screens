#include "SystemMonitor.h"
#include "Logger.h"
#include <cwchar>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "pdh.lib")

SystemMonitor::SystemMonitor() {
  cpuQuery = NULL;
  cpuTotal = NULL;
  ctxtTotal = NULL;
  intrTotal = NULL;
  sysCallTotal = NULL;
  memQuery = NULL;
  pageFaultTotal = NULL;
  diskQuery = NULL;
  diskTotal = NULL;
  diskReadTotal = NULL;
  diskWriteTotal = NULL;
  sysQuery = NULL;
  processCounter = NULL;
  threadCounter = NULL;
  handleCounter = NULL;
  netQuery = NULL;
}

SystemMonitor::~SystemMonitor() {
  if (cpuQuery)
    PdhCloseQuery(cpuQuery);
  if (memQuery)
    PdhCloseQuery(memQuery);
  if (diskQuery)
    PdhCloseQuery(diskQuery);
  if (sysQuery)
    PdhCloseQuery(sysQuery);
  if (netQuery)
    PdhCloseQuery(netQuery);
}

void SystemMonitor::Initialize() {
  Logger::LogS("SystemMonitor::Initialize() called");

  // --- CPU Query ---
  if (PdhOpenQueryW(NULL, 0, &cpuQuery) == ERROR_SUCCESS) {
    Logger::LogS("CPU Query Open Success");
    if (PdhAddEnglishCounterW(cpuQuery,
                              L"\\Processor(_Total)\\% Processor Time", 0,
                              &cpuTotal) != ERROR_SUCCESS)
      Logger::LogS("Failed: CPU Total");
    if (PdhAddEnglishCounterW(cpuQuery, L"\\System\\Context Switches/sec", 0,
                              &ctxtTotal) != ERROR_SUCCESS)
      Logger::LogS("Failed: Context Switches");
    if (PdhAddEnglishCounterW(cpuQuery, L"\\Processor(_Total)\\Interrupts/sec",
                              0, &intrTotal) != ERROR_SUCCESS)
      Logger::LogS("Failed: Interrupts");
    if (PdhAddEnglishCounterW(cpuQuery, L"\\System\\System Calls/sec", 0,
                              &sysCallTotal) != ERROR_SUCCESS)
      Logger::LogS("Failed: Sys Calls");
    PdhCollectQueryData(cpuQuery);
  } else {
    Logger::LogS("CPU Query Open FAILED");
  }

  // --- Memory Query ---
  if (PdhOpenQueryW(NULL, 0, &memQuery) == ERROR_SUCCESS) {
    Logger::LogS("Mem Query Open Success");
    if (PdhAddEnglishCounterW(memQuery, L"\\Memory\\Page Faults/sec", 0,
                              &pageFaultTotal) != ERROR_SUCCESS)
      Logger::LogS("Failed: Page Faults");
    PdhCollectQueryData(memQuery);
  } else {
    Logger::LogS("Mem Query Open FAILED");
  }

  // --- Disk Query ---
  if (PdhOpenQueryW(NULL, 0, &diskQuery) == ERROR_SUCCESS) {
    Logger::LogS("Disk Query Open Success");
    PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\% Disk Time", 0,
                          &diskTotal);
    PdhAddEnglishCounterW(diskQuery,
                          L"\\PhysicalDisk(_Total)\\Disk Read Bytes/sec", 0,
                          &diskReadTotal);
    PdhAddEnglishCounterW(diskQuery,
                          L"\\PhysicalDisk(_Total)\\Disk Write Bytes/sec", 0,
                          &diskWriteTotal);
    PdhCollectQueryData(diskQuery);
  } else {
    Logger::LogS("Disk Query Open FAILED");
  }

  // --- System Object Query ---
  if (PdhOpenQueryW(NULL, 0, &sysQuery) == ERROR_SUCCESS) {
    Logger::LogS("Sys Query Open Success");
    PdhAddEnglishCounterW(sysQuery, L"\\System\\Processes", 0, &processCounter);
    PdhAddEnglishCounterW(sysQuery, L"\\System\\Threads", 0, &threadCounter);
    PdhAddEnglishCounterW(sysQuery, L"\\Process(_Total)\\Handle Count", 0,
                          &handleCounter);
    PdhCollectQueryData(sysQuery);
  } else {
    Logger::LogS("Sys Query Open FAILED");
  }

  // --- Network Query ---
  PdhOpenQueryW(NULL, 0, &netQuery);
  DWORD bufferSize = 0;
  PDH_STATUS netStatus = PdhExpandWildCardPathW(
      NULL, L"\\Network Interface(*)\\Bytes Total/sec", NULL, &bufferSize, 0);
  if (netStatus == PDH_MORE_DATA && bufferSize > 0) {
    std::vector<wchar_t> buffer(bufferSize);
    netStatus =
        PdhExpandWildCardPathW(NULL, L"\\Network Interface(*)\\Bytes Total/sec",
                               buffer.data(), &bufferSize, 0);
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
    Logger::LogS("Network Counters Found: " +
                 std::to_string(netCounters.size()));
  } else {
    Logger::LogS("No Network Counters Found");
  }
}

void SystemMonitor::Update() {
  PDH_FMT_COUNTERVALUE counterVal;

  // --- CPU & System Activity ---
  if (cpuQuery) {
    PdhCollectQueryData(cpuQuery);

    if (PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawCpuUsage = counterVal.doubleValue;

    if (PdhGetFormattedCounterValue(ctxtTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawContextSwitches = counterVal.doubleValue;

    if (PdhGetFormattedCounterValue(intrTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawInterrupts = counterVal.doubleValue;

    if (PdhGetFormattedCounterValue(sysCallTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawSystemCalls = counterVal.doubleValue;
  }

  // --- Memory Activity ---
  if (memQuery) {
    PdhCollectQueryData(memQuery);
    if (PdhGetFormattedCounterValue(pageFaultTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawPageFaults = counterVal.doubleValue;
  }

  // --- Disk ---
  if (diskQuery) {
    PdhCollectQueryData(diskQuery);

    if (PdhGetFormattedCounterValue(diskTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS) {
      rawDiskUsage = counterVal.doubleValue;
      if (rawDiskUsage > 100.0)
        rawDiskUsage = 100.0;
    }

    if (PdhGetFormattedCounterValue(diskReadTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawReadBytes = counterVal.doubleValue;

    if (PdhGetFormattedCounterValue(diskWriteTotal, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawWriteBytes = counterVal.doubleValue;
  }

  // --- System Objects ---
  if (sysQuery) {
    PdhCollectQueryData(sysQuery);
    if (PdhGetFormattedCounterValue(processCounter, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawProcessCount = counterVal.doubleValue;
    if (PdhGetFormattedCounterValue(threadCounter, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawThreadCount = counterVal.doubleValue;
    if (PdhGetFormattedCounterValue(handleCounter, PDH_FMT_DOUBLE, NULL,
                                    &counterVal) == ERROR_SUCCESS)
      rawHandleCount = counterVal.doubleValue;
  }

  // RAM (Physical %)
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
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

    contextSwitches = rawContextSwitches;
    interrupts = rawInterrupts;
    systemCalls = rawSystemCalls;
    pageFaults = rawPageFaults;
    processCount = rawProcessCount;
    threadCount = rawThreadCount;
    handleCount = rawHandleCount;
    readBytes = rawReadBytes;
    writeBytes = rawWriteBytes;

    smoothingInitialized = true;
  } else {
    // Smoother alpha for visuals
    double alpha = smoothingAlpha;
    cpuUsage = alpha * rawCpuUsage + (1.0 - alpha) * cpuUsage;
    ramUsage = alpha * rawRamUsage + (1.0 - alpha) * ramUsage;
    diskUsage = alpha * rawDiskUsage + (1.0 - alpha) * diskUsage;
    networkBytesPerSec =
        alpha * rawNetworkBytesPerSec + (1.0 - alpha) * networkBytesPerSec;

    // For spiky metrics (Disk/Net/Interrupts), use a faster alpha?
    double fastAlpha = 0.9; // Spiky!

    contextSwitches =
        fastAlpha * rawContextSwitches + (1.0 - fastAlpha) * contextSwitches;
    interrupts = fastAlpha * rawInterrupts + (1.0 - fastAlpha) * interrupts;
    systemCalls = fastAlpha * rawSystemCalls + (1.0 - fastAlpha) * systemCalls;
    pageFaults = fastAlpha * rawPageFaults + (1.0 - fastAlpha) * pageFaults;

    // Process/Thread counts are stable, keep smooth
    processCount = alpha * rawProcessCount + (1.0 - alpha) * processCount;
    threadCount = alpha * rawThreadCount + (1.0 - alpha) * threadCount;
    handleCount = alpha * rawHandleCount + (1.0 - alpha) * handleCount;

    readBytes = fastAlpha * rawReadBytes + (1.0 - fastAlpha) * readBytes;
    writeBytes = fastAlpha * rawWriteBytes + (1.0 - fastAlpha) * writeBytes;
  }

  static int logSkip = 0;
  if (logSkip++ > 30) { // faster logging (30 frames = 0.5s)
    logSkip = 0;
    std::string msg = "Spectrum: ";
    for (int i = 0; i < 10; ++i) {
      // Print with 4 decimal places to see micro-movements
      char buf[32];
      snprintf(buf, 32, "%.4f", GetSpectrumBand(i));
      msg += std::string(buf) + " | ";
    }
    Logger::LogS(msg);
  }
}

// Helper: Normalize metrics to 0.0-1.0 range
// Index 0-9 corresponds to "Frequency Bands"
float SystemMonitor::GetSpectrumBand(int index) const {
  double val = 0.0;
  double maxVal = 1.0; // Normalization factor

  switch (index) {
  case 0: // Bass: Process Count
    val = processCount;
    maxVal = 400.0; // Increased
    break;
  case 1: // Low Mids: Thread Count
    val = threadCount;
    maxVal = 6000.0; // Increased
    break;
  case 2: // Mids: Handle Count
    val = handleCount;
    maxVal = 300000.0; // Increased to 300k
    break;
  case 3: // High Mids: CPU Usage (The fundamental)
    val = cpuUsage;
    maxVal = 100.0;
    break;
  case 4: // Highs: Context Switches (Jittery)
    val = contextSwitches;
    maxVal = 60000.0; // Increased significantly
    break;
  case 5: // Presence: System Calls
    val = systemCalls;
    maxVal = 400000.0; // Increased significantly
    break;
  case 6: // Brilliance: Interrupts
    val = interrupts;
    maxVal = 50000.0; // Increased significantly
    break;
  case 7: // Air: Page Faults (Spiky)
    val = pageFaults;
    maxVal = 5000.0;
    break;
  case 8: // Disk Read (Periodic Thump)
    val = readBytes;
    maxVal = 50000000.0; // 50MB/s
    break;
  case 9: // Disk Write (Periodic Thump)
    val = writeBytes;
    maxVal = 20000000.0; // 20MB/s
    break;
  default:
    return 0.0f;
  }

  // Debug logging removed for cleanliness after tuning
  return (std::min)(1.0f, (std::max)(0.0f, static_cast<float>(val / maxVal)));
}
