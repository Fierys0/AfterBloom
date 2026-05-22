#include "event1.hpp"
#include "../core/globals.hpp"
#include "../core/ui_style.hpp"
#include "../states/title_state.hpp"
#include "fumbo.hpp"
#include <cstdio>
#include <string>
#include <vector>

std::vector<VNLine> BuildEvent1Script() {
  // Each entry: { speaker, text, cgPath, musicId }
  // cgPath / musicId are only set when they need to CHANGE from the previous.
  return {
      {"Narrator", "End of Demo.", "", ""},
      {"Narrator", "Thanks for playing!", "", ""},
      {"Narrator", "Returning to title screen.", "", ""},

  };
}

void Event1::Init() {
  // Choose script
  if (customScript_.empty()) {
    defaultScript_ = BuildEvent1Script();
    activeScript_ = &defaultScript_;
  } else {
    activeScript_ = &customScript_;
  }

  // Initial CG - white placeholder until a cgPath is specified
  bgTex = Fumbo::Utils::ColorToTexture(WHITE);
  currentCGPath.clear();

  testFont =
      Fumbo::Assets::LoadFont("assets/fonts/Montserrat-ExtraBold.ttf", 24);

  vn = std::make_unique<Fumbo::UI::VisualNovel>("", 30.0f);

  Fumbo::UI::MessageBoxStyle boxStyle;
  boxStyle.backgroundColor = Color{UI::PANEL.r, UI::PANEL.g, UI::PANEL.b, 220};
  boxStyle.borderColor = UI::LINE;
  boxStyle.borderThickness = 2.0f;
  boxStyle.borderRounding = 0.0f; // flat for full width
  boxStyle.paddingTop = 20.0f;
  boxStyle.paddingBottom = 20.0f;
  boxStyle.paddingLeft = 40.0f;
  boxStyle.paddingRight = 40.0f;
  boxStyle.enableShadow = true;
  boxStyle.shadowOffset = {0.0f, -2.0f}; // shadow above
  boxStyle.shadowColor = Color{0, 0, 0, 50};
  vn->SetMessageBoxStyle(boxStyle);

  // Fill the screen width
  vn->SetMessageBoxBounds(Rectangle{0, 520, 1280, 200});
  vn->EnableMessageBox(true);

  Fumbo::UI::TextStyle textStyle;
  textStyle.alignment = Fumbo::UI::TextAlign::LEFT;
  textStyle.lineHeightMultiplier = 1.2f;
  textStyle.enableShadow =
      false; // Disable shadow for better readability on light theme
  vn->SetTextStyle(textStyle);

  // Skip button (top-right)
  skipButton = Fumbo::UI::Button();
  skipButton.SetButtonColor(UI::BTN_IDLE);
  skipButton.IdleColor(UI::BTN_IDLE);
  skipButton.HoveredColor(UI::BTN_HOVER);
  skipButton.SetButtonSound(mouseClick, mouseHover);
  skipButton.AddText("SKIP >>", fontDefault, 22, UI::BTN_TEXT);
  skipButton.SetBounds(1110.0f, 10.0f, 160.0f, 42.0f);

  currentScene = 0;
  waitingForInput = false;
  sceneTimer = 0.0f;
  isFadingOut = false;
  fadeOutTimer = 0.0f;

  SetupScene(0);
}

void Event1::Cleanup() {
  UnloadTexture(bgTex);
  UnloadFont(testFont);
  Fumbo::Audio::AudioManager::Instance().StopMusic(0);
}

void Event1::SetupScene(int sceneNum) {
  currentScene = sceneNum;
  waitingForInput = false;

  if (!activeScript_ || sceneNum >= (int)activeScript_->size()) {
    isFadingOut = true;
    fadeOutTimer = 0.0f;
    return;
  }

  const VNLine &line = (*activeScript_)[sceneNum];

  // --- CG swap ---
  if (!line.cgPath.empty() && line.cgPath != currentCGPath) {
    UnloadTexture(bgTex);
    bgTex = Fumbo::Assets::LoadTexture(line.cgPath);
    currentCGPath = line.cgPath;
  }

  // --- Music swap ---
  if (!line.musicId.empty() && line.musicId != currentMusicId) {
    Fumbo::Audio::AudioManager::Instance().StopMusicFade(0, 0.5f);
    Fumbo::Audio::AudioManager::Instance().PlayMusic(line.musicId, 0, true);
    currentMusicId = line.musicId;
  }

  // Speaker colour
  Color speakerColor = UI::AB_ORANGE;
  if (line.speaker == "Nao")
    speakerColor = UI::AB_YELLOW;
  else if (line.speaker == "Nakayama" || line.speaker == "???")
    speakerColor = Color{100, 160, 60, 255}; // Muted green

  vn->SetSpeaker(line.speaker == "Narrator" ? "" : line.speaker, speakerColor);
  vn->SetText(line.text);
}

void Event1::AdvanceScene() {
  if (!isFadingOut) {
    SetupScene(currentScene + 1);
  }
}

void Event1::SkipAll() {
  if (!isFadingOut) {
    isFadingOut = true;
    fadeOutTimer = 0.0f;
  }
}

void Event1::Update() {
  float deltaTime = GetFrameTime();

  if (isFadingOut) {
    fadeOutTimer += deltaTime;
    if (fadeOutTimer >= 1.5f) {
      Fumbo::Instance().ChangeState(std::make_shared<TitleState>());
    }
    return;
  }

  // Skip button (entire VN)
  if (skipButton.IsPressed()) {
    SkipAll();
    return;
  }

  vn->Update(deltaTime);

  // Advance / skip current line
  if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (vn->IsComplete()) {
      AdvanceScene();
    } else {
      vn->Skip();
    }
  }
}

void Event1::DrawClean() {}

void Event1::DrawDirty() {
  Fumbo::Graphic2D::DrawTexturePro(
      bgTex, Rectangle{0, 0, (float)bgTex.width, (float)bgTex.height},
      Rectangle{0, 0, 1280.0f, 720.0f}, Vector2{0, 0}, 0.0f, WHITE);

  if (isFadingOut) {
    Fumbo::Graphic2D::DrawRectangle(0, 0, 1280, 720,
                                    Fade(BLACK, fadeOutTimer / 1.5f));
  } else {
    vn->DrawComplete(testFont, 26, 1.0f, UI::TEXT);
    skipButton.Draw();
  }
}
