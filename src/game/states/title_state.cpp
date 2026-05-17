#include "title_state.hpp"
#include "../core/globals.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"
#include "../areas/prologue.hpp"
#include <raylib.h>

TitleState::TitleState() {}

static void MakeBtn(Fumbo::UI::Button &btn, const char *label,
                    float x, float y, float w = UI::BTN_W, float h = UI::BTN_H) {
  btn = Fumbo::UI::Button();
  btn.SetButtonColor(UI::BTN_IDLE);
  btn.IdleColor(UI::BTN_IDLE);
  btn.HoveredColor(UI::BTN_HOVER);
  btn.SetButtonSound(mouseClick, mouseHover);
  btn.AddText(label, fontDefault, 24, UI::BTN_TEXT);
  btn.SetBounds(x, y, w, h);
}

void TitleState::Init() {
  Fumbo::Instance().PauseSharedState(true);
  titleTex = Fumbo::Assets::LoadTexture("assets/images/title.png");

  // Buttons stacked on the LEFT side
  constexpr float BX = 60.0f;
  MakeBtn(btnNewGame,  "NEW GAME",  BX, 300);
  MakeBtn(btnLoadGame, "LOAD GAME", BX, 366);
  MakeBtn(btnSettings, "SETTINGS",  BX, 432);
  MakeBtn(btnExit,     "EXIT",      BX, 498);

  buildversion     = buildinfo.GetBuildString();
  settingsMenuOpen = false;
}

void TitleState::Cleanup() { UnloadTexture(titleTex); }

void TitleState::Update() {
  if (settingsMenuOpen) {
    bool closed = !Settings::Instance().Update(false);
    if (closed) {
      settingsMenuOpen = false;
      Fumbo::Instance().InvalidateCleanLayer();
    }
  } else {
    if (btnNewGame.IsPressed())
      Fumbo::Instance().ChangeState(std::make_shared<PrologueState>());
    if (btnSettings.IsPressed()) {
      settingsMenuOpen = true;
      Fumbo::Instance().InvalidateCleanLayer();
    }
    if (btnExit.IsPressed())
      Fumbo::Instance().Quit();
  }
}

void TitleState::DrawClean() {
  // Original global background image
  Fumbo::Graphic2D::DrawBackground(globalBackground);

  // Title image — right side, same position as original
  Fumbo::Graphic2D::DrawTexture(
      titleTex, {320, -50},
      {(float)titleTex.width / 3.0f, (float)titleTex.height / 3.0f},
      0.0f, WHITE);

  // Top & bottom accent lines over the background
  Fumbo::Graphic2D::DrawRectangle(0,   0, 1280, (int)UI::ACCENT_H, UI::AB_YELLOW);
  Fumbo::Graphic2D::DrawRectangle(0, 717, 1280, (int)UI::ACCENT_H, UI::AB_ORANGE);

  if (settingsMenuOpen) {
    Settings::Instance().DrawClean();
    return;
  }

  // Build version — bottom-left
  Fumbo::Graphic2D::DrawText(buildversion, {14, 703}, fontDefault, 13, UI::TEXT_DIM);
}

void TitleState::DrawDirty() {
  if (settingsMenuOpen) {
    Settings::Instance().DrawDirty();
    Fumbo::Instance().GetFader().DrawExcept(2);
    return;
  }

  btnNewGame.Draw();
  btnLoadGame.Draw();
  btnSettings.Draw();
  btnExit.Draw();

  Fumbo::Instance().GetFader().Draw();
}
