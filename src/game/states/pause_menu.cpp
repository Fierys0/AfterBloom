#include "pause_menu.hpp"
#include "../core/globals.hpp"
#include "../core/settings.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"

static constexpr float PW = 300.0f, PH = 330.0f;
static constexpr float PX = (1280.0f - PW) / 2.0f;
static constexpr float PY = (720.0f  - PH) / 2.0f;
static constexpr float BW = 220.0f, BH = 48.0f;
static constexpr float BX = PX + (PW - BW) / 2.0f;
static constexpr float BY0 = PY + 88.0f;
static constexpr float BGAP = BH + 12.0f;

void PauseMenu::Init() {
  if (initialized) return;
  initialized = true;

  auto makeBtn = [&](Fumbo::UI::Button &btn, const char *label, float yOff) {
    btn = Fumbo::UI::Button();
    btn.SetButtonColor(UI::BTN_IDLE);
    btn.IdleColor(UI::BTN_IDLE);
    btn.HoveredColor(UI::BTN_HOVER);
    btn.SetButtonSound(mouseClick, mouseHover);
    btn.AddText(label, fontDefault, 22, UI::BTN_TEXT);
    btn.SetBounds(BX, BY0 + yOff, BW, BH);
  };

  makeBtn(btnResume,   "RESUME",   0.0f);
  makeBtn(btnRetry,    "RETRY",    BGAP);
  makeBtn(btnSettings, "SETTINGS", BGAP * 2);
  makeBtn(btnQuit,     "QUIT",     BGAP * 3);
}

PauseAction PauseMenu::Update() {
  if (!initialized) Init();

  if (settingsOpen) {
    bool still = Settings::Instance().Update(true);
    if (!still) { settingsOpen = false; Fumbo::Instance().InvalidateCleanLayer(); }
    return PauseAction::NONE;
  }

  btnResume.SetBounds(BX,  BY0,            BW, BH);
  btnRetry.SetBounds(BX,   BY0 + BGAP,     BW, BH);
  btnSettings.SetBounds(BX, BY0 + BGAP*2,  BW, BH);
  btnQuit.SetBounds(BX,    BY0 + BGAP*3,   BW, BH);

  if (btnResume.IsPressed())   return PauseAction::RESUME;
  if (btnRetry.IsPressed())    return PauseAction::RETRY;
  if (btnSettings.IsPressed()) { settingsOpen = true; return PauseAction::SETTINGS; }
  if (btnQuit.IsPressed())     return PauseAction::QUIT;

  return PauseAction::NONE;
}

void PauseMenu::Draw() {
  if (!initialized) return;

  if (settingsOpen) {
    Settings::Instance().DrawClean();
    Settings::Instance().DrawDirty();
    return;
  }

  // Light frosted scrim
  Fumbo::Graphic2D::DrawRectangle(0, 0, 1280, 720, {255, 240, 190, 100});

  // Cream panel
  Fumbo::Graphic2D::DrawRectangleRounded({PX, PY, PW, PH}, 0.04f, 4, UI::PANEL);
  // Yellow top accent bar
  Fumbo::Graphic2D::DrawRectangle((int)PX, (int)PY, (int)PW, (int)UI::ACCENT_H, UI::AB_YELLOW);
  // Warm border
  Fumbo::Graphic2D::DrawRectangleRoundedLines({PX, PY, PW, PH}, 0.04f, 4, UI::LINE);

  // "PAUSED" title in orange
  Fumbo::Graphic2D::DrawText("PAUSED",
      {PX + (PW - 120.0f) / 2.0f, PY + 22}, fontDefault, 30, UI::AB_ORANGE);

  // Thin divider
  Fumbo::Graphic2D::DrawRectangle((int)(PX + 20), (int)(PY + 68), (int)(PW - 40), 1, UI::LINE);

  btnResume.Draw();
  btnRetry.Draw();
  btnSettings.Draw();
  btnQuit.Draw();
}
