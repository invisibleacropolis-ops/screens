#pragma once

#include "ISettingsPanel.h"
#include <memory>
#include <vector>

// SettingsHost - Registry and dispatcher for settings panels
// Manages modular settings panels and delegates control operations
class SettingsHost {
public:
  SettingsHost() = default;
  ~SettingsHost() = default;

  // Register a panel with the host
  void RegisterPanel(std::unique_ptr<ISettingsPanel> panel) {
    m_panels.push_back(std::move(panel));
  }

  // Get number of registered panels
  size_t GetPanelCount() const { return m_panels.size(); }

  // Get panel by index
  ISettingsPanel *GetPanel(size_t index) {
    return index < m_panels.size() ? m_panels[index].get() : nullptr;
  }

  // Initialize all panels from config
  void InitAllFromConfig(HWND hDlg, const Config &cfg) {
    for (auto &panel : m_panels) {
      panel->InitFromConfig(hDlg, cfg);
    }
  }

  // Read all panels to config
  void ReadAllToConfig(HWND hDlg, Config &cfg) {
    for (auto &panel : m_panels) {
      panel->ReadToConfig(hDlg, cfg);
    }
  }

  // Handle slider scroll - dispatch to panels
  bool HandleSlider(HWND hDlg, int sliderId) {
    for (auto &panel : m_panels) {
      if (panel->HandleSlider(hDlg, sliderId)) {
        return true;
      }
    }
    return false;
  }

private:
  std::vector<std::unique_ptr<ISettingsPanel>> m_panels;
};
