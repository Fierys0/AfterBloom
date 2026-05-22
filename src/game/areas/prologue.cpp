#include "prologue.hpp"
#include "../core/globals.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"
#include "stage0.hpp"
#include <cstdio>
#include <string>
#include <vector>

std::vector<VNLine> BuildDefaultPrologueScript() {
  // Each entry: { speaker, text, cgPath, musicId }
  // cgPath / musicId are only set when they need to CHANGE from the previous.
  return {
      {"Narrator", "I simply watches as the flowers blooms.", "", ""},
      {"Narrator", "In that moment, i felt sad nor pain.", "", ""},
      {"Narrator", "As i watches those beautiful flowers dancing in the wind.",
       "", ""},
      {"Narrator", "Washing down the view im seeing.", "", ""},
      {"Narrator",
       "Amidst the cloying scent of those flowers, a coffin was carried by the "
       "hands of people.",
       "", ""},
      {"Narrator",
       "My father's coffin is set within the beautiful flowers, is simply "
       "hideous.",
       "", ""},
      {"Narrator",
       "I always wonder, what the world look like inside that wooden box?", "",
       ""},
      {"Narrator", "The wind blows.", "", ""},
      {"Narrator", "Flowers petals flutter.", "", ""},
      {"Narrator", "Haa... The world is simply beautiful.", "", ""},
      {"Narrator", "Standing in graveyard, no tears fall from my eyes.", "",
       ""},
      {"Narrator", "And I can find no emotion resembling sadness.", "", ""},
      {"Narrator", "I can find no sadness in the landscape before me.", "", ""},
      {"Narrator",
       "What about the people around me? Are they sad after all? Do they felt "
       "heart broken?",
       "", ""},
      {"Narrator",
       "A crack runs through my heart for just a moment. The memory of me and "
       "my father together.",
       "", ""},
      {"Narrator",
       "The death of a man who was a great warrior for my kingdom ended with "
       "an ordinary funeral.",
       "", ""},
      {"Narrator",
       "Perhaps thinking that way was just childish delusion, a way to enact "
       "the most insulting revenge as possible against my father",
       "", ""},
      {"Narrator", "Truthfully, i don't know.", "", ""},
      {"Narrator", "but-", "", ""},
      {"Narrator",
       "Either way, it was the day where a person that i shared blood "
       "connection with vanished from this world.",
       "", ""},
      {"Narrator", "That was certain.", "", ""},
      // CG changes here (first dialogue with visual)
      {"Nao", "It's over now doesn't it?", "assets/images/prologue_1.png", ""},
      {"???", "It must be rough huh...", "", ""},
      {"Nao", "Hmmm? Not really.", "", ""},
      {"???", "Nao...", "", ""},
      {"Narrator",
       "With sorrowful expression, a face i have seen for years peers into "
       "mine",
       "", ""},
      {"Narrator", "His name is Nakayama.", "", ""},
      {"Narrator", "He's the last friend i have.", "", ""},
      {"Narrator", "Atleast the closest to me.", "", ""},
      {"Nao", "Don't look at me like that.", "", ""},
      {"Nao", "It makes me sick.", "", ""},
      {"Narrator", "I look up in the sky as if i were looking at something.",
       "", ""},
      {"Nakayama", "What's wrong Nao?", "", ""},
      {"Nakayama", "Want me to hug?", "", ""},
      {"Nao", "Ugh.. what was that! Gross.", "", ""},
      {"Narrator",
       "An April afternoon, with a slight chill lingering in the wind.", "",
       ""},
      {"Narrator", "I could see the sky so clearly.", "", ""},
      {"Narrator", "The wind blew.", "", ""},
      {"Narrator", "Scattering petals.", "", ""},
      {"Narrator", "Flowers blooms.", "", ""},
      {"Narrator", "And then,", "", ""},
      {"Narrator", "Cloud drifted by.", "", ""},
      {"Narrator", "Slowly but surely...", "", ""},
      {"Narrator", "Cloud drifted across the vast expanse of the sky.", "", ""},
      {"Nao", "It's not something important.", "", ""},
      {"Nao", "Don't worry about it.", "", ""},
      {"Nakayama", "Nao...", "", ""},
      {"Nao", "Don't look at me like that.", "", ""},
      {"Nao", "You know i'm not good with sentimental stuff.", "", ""},
      {"Nakayama", "I know.", "", ""},
      {"Nao", "Then stop worrying about me..", "", ""},
      {"Nakayama", "...", "", ""},
  };
}

void PrologueState::Init() {
  // Choose script
  if (customScript_.empty()) {
    defaultScript_ = BuildDefaultPrologueScript();
    activeScript_ = &defaultScript_;
  } else {
    activeScript_ = &customScript_;
  }

  // Initial CG - white placeholder until a cgPath is specified
  bgTex = Fumbo::Utils::ColorToTexture(WHITE);
  currentCGPath.clear();

  // Default music
  Fumbo::Audio::AudioManager::Instance().LoadAudio(
      "sakuuta", "./assets/audio/sakuuta_0.ogg",
      Fumbo::Audio::AudioType::MUSIC);
  Fumbo::Audio::AudioManager::Instance().PlayMusic("sakuuta", 0, true);
  currentMusicId = "sakuuta";

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

void PrologueState::Cleanup() {
  UnloadTexture(bgTex);
  UnloadFont(testFont);
  Fumbo::Audio::AudioManager::Instance().StopMusic(0);
}

void PrologueState::SetupScene(int sceneNum) {
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

void PrologueState::AdvanceScene() {
  if (!isFadingOut) {
    SetupScene(currentScene + 1);
  }
}

void PrologueState::SkipAll() {
  if (!isFadingOut) {
    isFadingOut = true;
    fadeOutTimer = 0.0f;
  }
}

void PrologueState::Update() {
  float deltaTime = GetFrameTime();

  if (isFadingOut) {
    fadeOutTimer += deltaTime;
    if (fadeOutTimer >= 1.5f) {
      Fumbo::Instance().ChangeState(std::make_shared<Stage0State>());
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

void PrologueState::DrawClean() {}

void PrologueState::DrawDirty() {
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
