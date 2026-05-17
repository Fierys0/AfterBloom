#include "globals.hpp"
#include "fumbo.hpp"

Font fontDefault{};
Sound mouseHover{};
Sound mouseClick{};
Texture2D globalBackground{};

void LoadGlobalAssets() {
  fontDefault      = Fumbo::Assets::LoadFont("assets/fonts/Montserrat-Bold.ttf", 64);
  mouseHover       = Fumbo::Assets::LoadSound("assets/audio/MouseHover.ogg");
  mouseClick       = Fumbo::Assets::LoadSound("assets/audio/MouseClick.ogg");
  globalBackground = Fumbo::Assets::LoadTexture("assets/images/MainBackground.jpg");
}

void UnloadGlobalAssets() {
  UnloadFont(fontDefault);
  UnloadSound(mouseHover);
  UnloadSound(mouseClick);
  UnloadTexture(globalBackground);
}
