#pragma once
#include "../core/buildinfo.hpp"
#include "../core/settings.hpp"
#include "fumbo.hpp"
#include <string>

class TitleState : public IGameState {
public:
  TitleState();
  ~TitleState() override = default;

  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  BuildInfo buildinfo;

  Texture2D titleTex{};
  bool settingsMenuOpen = false;
  std::string buildversion;

  Fumbo::UI::Button btnNewGame;
  Fumbo::UI::Button btnLoadGame;
  Fumbo::UI::Button btnSettings;
  Fumbo::UI::Button btnExit;
};
