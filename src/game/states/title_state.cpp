#include "title_state.hpp"
#include "../areas/prologue.hpp"
#include "../core/globals.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"
#include <raylib.h>

TitleState::TitleState() {}

static void MakeBtn(Fumbo::UI::Button &btn, const char *label, float x, float y,
                    float w = UI::BTN_W, float h = UI::BTN_H) {
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

  // P4 Style Menu on the right
  constexpr float MENU_X = 850.0f;
  constexpr float MENU_W = 350.0f;
  constexpr float START_Y = 460.0f;
  constexpr float SPACING = 65.0f;

  MakeBtn(btnLoadGame, "CONTINUE", MENU_X, START_Y, MENU_W, 50);
  MakeBtn(btnNewGame, "NEW GAME", MENU_X, START_Y + SPACING, MENU_W, 50);
  MakeBtn(btnSettings, "CONFIG", MENU_X, START_Y + SPACING * 2, MENU_W, 50);
  MakeBtn(btnExit, "EXIT", MENU_X, START_Y + SPACING * 3, MENU_W, 50);

  buildversion = buildinfo.GetBuildString();
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
  // Solid P4 Yellow background (no bgTex cutout)
  Color p4y = {254, 235, 44, 255};
  Fumbo::Graphic2D::DrawRectangle(0, 0, 1280, 720, p4y);

  // Title image (Logo) — left side
  Fumbo::Graphic2D::DrawTexture(
      titleTex, {50, 50},
      {(float)titleTex.width / 2.5f, (float)titleTex.height / 2.5f}, 0.0f,
      WHITE);

  // Top & bottom accent lines over the background
  Fumbo::Graphic2D::DrawRectangle(0, 0, 1280, (int)UI::ACCENT_H, UI::AB_YELLOW);
  Fumbo::Graphic2D::DrawRectangle(0, 717, 1280, (int)UI::ACCENT_H,
                                  UI::AB_ORANGE);

  if (settingsMenuOpen) {
    Settings::Instance().DrawClean();
    return;
  }

  // Build version — bottom-left
  Fumbo::Graphic2D::DrawText(buildversion, {14, 703}, fontDefault, 13,
                             UI::TEXT_DIM);
}

void TitleState::DrawDirty() {
  if (settingsMenuOpen) {
    Settings::Instance().DrawDirty();
    Fumbo::Instance().GetFader().DrawExcept(2);
    return;
  }

  // Draw the standard UI buttons
  btnNewGame.Draw();
  btnLoadGame.Draw();
  btnSettings.Draw();
  btnExit.Draw();

  Fumbo::Instance().GetFader().Draw();
}
