#include "settings.hpp"
#include "../states/title_state.hpp"
#include "../states/shared_mechanics.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"
#include "globals.hpp"

// Panel geometry (virtual 1280×720)
static constexpr float PX = 240.f, PY = 55.f, PW = 800.f, PH = 610.f;
static constexpr float LX = PX + 40.f;   // label column x
static constexpr float CX = PX + 360.f;  // control column x

Settings &Settings::Instance() {
  static Settings instance;
  return instance;
}

// Helper: draw an orange section label + golden divider line
static void DrawSection(const char *label, float y) {
  Fumbo::Graphic2D::DrawText(label, {LX, y}, fontDefault, 18, UI::AB_ORANGE);
  Fumbo::Graphic2D::DrawRectangle((int)LX, (int)(y + 24), (int)(PW - 80), 1, UI::LINE);
}

// Helper: make a flat textured-free button
static void MakeBtn(Fumbo::UI::Button &btn, const char *label) {
  btn = Fumbo::UI::Button();
  btn.SetButtonColor(UI::BTN_IDLE);
  btn.IdleColor(UI::BTN_IDLE);
  btn.HoveredColor(UI::BTN_HOVER);
  btn.SetButtonSound(mouseClick, mouseHover);
  btn.AddText(label, fontDefault, 22, UI::BTN_TEXT);
}

bool Settings::Update(bool showTitleButton) {
  if (!loaded) loaded = Load();
  shouldShowTitleButton = showTitleButton;

  // Keep debug toggle label in sync with F3 hotkey
  {
    auto ss = std::dynamic_pointer_cast<SharedMechanics>(
        Fumbo::Engine::Instance().GetSharedState());
    if (ss && ss->IsDebugEnabled() != debugEnabled) {
      debugEnabled = ss->IsDebugEnabled();
      debugToggle.AddText(debugEnabled ? "ON" : "OFF", fontDefault, 22, UI::BTN_TEXT);
    }
  }

  FadeManager &fader = Fumbo::Instance().GetFader();
  if (!closing && !fader.IsGroupActive(2))
    fader.AddFade(backgroundTexture, {0, 0}, {1280, 720}, 0.5f, true, 2);

  // Close (X) button
  exitButton.SetBounds(PX + PW - 48, PY + 8, 36, 36);
  if (exitButton.IsPressed()) { fader.ReverseGroup(2); return false; }

  // AUDIO — volume
  if (volumeSlider.Update({CX, PY + 148, 340, 14})) {
    masterVolume = volumeSlider.GetValue();
    Fumbo::Instance().GetAudioManager().SetMasterVolume(masterVolume);
  }

  // DISPLAY — vsync
  vsyncToggleButton.SetBounds(CX, PY + 218, 110, 38);
  if (vsyncToggleButton.IsPressed()) {
    vsync = !vsync;
    Fumbo::Instance().SetVSync(vsync);
    vsyncToggleButton.AddText(vsync ? "ON" : "OFF", fontDefault, 22, UI::BTN_TEXT);
  }

  // DISPLAY — FPS limit
  if (fpsSlider.Update({CX, PY + 288, 340, 14})) {
    fpsLimit = (int)fpsSlider.GetValue();
    Fumbo::Instance().LimitFPS(fpsLimit);
  }

  // SHADERS — godrays
  godraysToggle.SetBounds(CX, PY + 378, 110, 38);
  if (godraysToggle.IsPressed()) {
    godraysEnabled = !godraysEnabled;
    godraysToggle.AddText(godraysEnabled ? "ON" : "OFF", fontDefault, 22, UI::BTN_TEXT);
    auto ss = std::dynamic_pointer_cast<SharedMechanics>(
        Fumbo::Engine::Instance().GetSharedState());
    if (ss)
      ss->SetRenderMode(godraysEnabled ? SharedMechanics::RenderMode::GODRAYS
                                       : SharedMechanics::RenderMode::NORMAL);
  }

  // SHADERS — debug overlay
  debugToggle.SetBounds(CX, PY + 448, 110, 38);
  if (debugToggle.IsPressed()) {
    debugEnabled = !debugEnabled;
    debugToggle.AddText(debugEnabled ? "ON" : "OFF", fontDefault, 22, UI::BTN_TEXT);
    auto ss = std::dynamic_pointer_cast<SharedMechanics>(
        Fumbo::Engine::Instance().GetSharedState());
    if (ss) {
      ss->SetDebugEnabled(debugEnabled);
      Fumbo::Graphic2D::Physics::Instance().SetDebugDraw(debugEnabled);
    }
  }

  if (showTitleButton) {
    titleButton.SetBounds(LX,        PY + PH - 65, 160, 44);
    quitButton.SetBounds( LX + 180,  PY + PH - 65, 160, 44);
    if (titleButton.IsPressed()) {
      Fumbo::Instance().ChangeState(std::make_shared<TitleState>());
      fader.ReverseGroup(2);
      return false;
    }
    if (quitButton.IsPressed()) { Fumbo::Instance().Quit(); return false; }
  }
  return true;
}

void Settings::DrawClean() {}

void Settings::DrawDirty() {
  Fumbo::Instance().GetFader().Draw(2);

  // ── Cream panel ─────────────────────────────────────
  Fumbo::Graphic2D::DrawRectangleRounded({PX, PY, PW, PH}, 0.03f, 4, UI::PANEL);
  Fumbo::Graphic2D::DrawRectangle((int)PX, (int)PY, (int)PW, (int)UI::ACCENT_H, UI::AB_YELLOW);
  Fumbo::Graphic2D::DrawRectangleRoundedLines({PX, PY, PW, PH}, 0.03f, 4, UI::LINE);

  // Title + close button
  Fumbo::Graphic2D::DrawText("SETTINGS", {LX, PY + 14}, fontDefault, 26, UI::AB_ORANGE);
  exitButton.Draw();

  // ── AUDIO ────────────────────────────────────────────
  DrawSection("AUDIO", PY + 110);
  Fumbo::Graphic2D::DrawText("Master Volume", {LX, PY + 140}, fontDefault, 22, UI::TEXT);
  volumeSlider.Draw({CX, PY + 148, 340, 14});
  Fumbo::Graphic2D::DrawText(
      TextFormat("%d%%", (int)(masterVolume * 100)),
      {CX + 356, PY + 140}, fontDefault, 22, UI::TEXT);

  // ── DISPLAY ──────────────────────────────────────────
  DrawSection("DISPLAY", PY + 196);
  Fumbo::Graphic2D::DrawText("VSync",     {LX, PY + 224}, fontDefault, 22, UI::TEXT);
  vsyncToggleButton.Draw();
  Fumbo::Graphic2D::DrawText("FPS Limit", {LX, PY + 284}, fontDefault, 22, UI::TEXT);
  fpsSlider.Draw({CX, PY + 288, 340, 14});
  Fumbo::Graphic2D::DrawText(
      TextFormat("%d", fpsLimit),
      {CX + 356, PY + 284}, fontDefault, 22, UI::TEXT);

  // ── SHADERS ──────────────────────────────────────────
  DrawSection("SHADERS", PY + 348);
  Fumbo::Graphic2D::DrawText("God Rays",      {LX, PY + 376}, fontDefault, 22, UI::TEXT);
  godraysToggle.Draw();
  Fumbo::Graphic2D::DrawText("Debug Overlay", {LX, PY + 446}, fontDefault, 22, UI::TEXT);
  debugToggle.Draw();

  if (shouldShowTitleButton) {
    titleButton.Draw();
    quitButton.Draw();
  }
}

bool Settings::Load() {
  masterVolume = Fumbo::Instance().GetAudioManager().GetMasterVolume();
  vsync        = IsWindowState(FLAG_VSYNC_HINT);
  fpsLimit     = GetFPS();
  if (fpsLimit == 0) fpsLimit = 60;

  // Background overlay for the fader — warm cream, generated without buttonTex
  backgroundTexture = Fumbo::Utils::ColorToTexture({255, 244, 210, 200});

  MakeBtn(exitButton,        "X");
  MakeBtn(titleButton,       "TO TITLE");
  MakeBtn(quitButton,        "EXIT GAME");
  MakeBtn(vsyncToggleButton, vsync          ? "ON" : "OFF");
  MakeBtn(godraysToggle,     godraysEnabled ? "ON" : "OFF");

  // Sync debug state from SharedMechanics on first load
  auto ss = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());
  if (ss) debugEnabled = ss->IsDebugEnabled();
  MakeBtn(debugToggle, debugEnabled ? "ON" : "OFF");

  // MinSlider — no SliderConfig needed, colours baked into MinSlider::Draw()
  volumeSlider = MinSlider(0.f, 1.f, masterVolume);
  fpsSlider    = MinSlider(30.f, 240.f, (float)fpsLimit);

  return true;
}
