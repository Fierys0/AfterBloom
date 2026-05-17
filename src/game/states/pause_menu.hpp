#pragma once
#include "fumbo.hpp"
#include "pause_action.hpp"

class PauseMenu {
public:
  PauseMenu() = default;

  // Call once after global assets are loaded
  void Init();

  // Returns the action chosen this frame (or NONE)
  PauseAction Update();

  // Draw the centered overlay (call inside DrawDirty)
  void Draw();

  bool IsSettingsOpen() const { return settingsOpen; }

private:
  bool initialized   = false;
  bool settingsOpen  = false;

  Fumbo::UI::Button btnResume;
  Fumbo::UI::Button btnRetry;
  Fumbo::UI::Button btnSettings;
  Fumbo::UI::Button btnQuit;

  // Panel geometry (centred at virtual 640×360)
  static constexpr float PANEL_W = 320.0f;
  static constexpr float PANEL_H = 320.0f;
  static constexpr float PANEL_X = (1280.0f - PANEL_W) / 2.0f;
  static constexpr float PANEL_Y = (720.0f  - PANEL_H) / 2.0f;
  static constexpr float BTN_W   = 220.0f;
  static constexpr float BTN_H   = 50.0f;
  static constexpr float BTN_X   = PANEL_X + (PANEL_W - BTN_W) / 2.0f;
  static constexpr float BTN_GAP = 16.0f;
  // y positions for each button
  static constexpr float BTN_Y0  = PANEL_Y + 70.0f;
};
