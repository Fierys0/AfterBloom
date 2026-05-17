#pragma once

// Android on-screen touch controls - header-only, inline implementation.
// PLATFORM_ANDROID: real buttons with textures and touch hit-testing.
// Other platforms:  stub class - all methods are no-ops / return false.

#ifdef PLATFORM_ANDROID

#include "../core/globals.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"
#include "raylib.h"
#include <cmath>

class AndroidControls {
public:
  // Lifecycle
  void Init() {
    arrowTex = Fumbo::Assets::LoadTexture("assets/ui/Arrow.png");
    swordTex = Fumbo::Assets::LoadTexture("assets/ui/Sword.png");
    pauseTex = Fumbo::Assets::LoadTexture("assets/ui/Pause.png");

    // Layout (virtual 1280x720)
    // Bottom-left: [Left] [Right]
    float ly = 720.f - MARGIN - BTN;
    btnLeft = {MARGIN, ly, BTN, BTN};
    btnRight = {MARGIN + BTN + GAP, ly, BTN, BTN};

    // Bottom-right: [JUMP] [ATTACK]
    float ry = 720.f - MARGIN - BTN;
    float rx = 1280.f - MARGIN - BTN * 2 - GAP;
    btnJump = {rx, ry, BTN, BTN};
    btnAttack = {rx + BTN + GAP, ry, BTN, BTN};

    // Top-right: [PAUSE]
    btnPause = {1280.f - MARGIN - PBTN, MARGIN, PBTN, PBTN};

    // Above Jump: [INTERACT]
    btnInteract = {rx, ry - BTN - GAP, BTN, BTN};
  }

  void Unload() {
    UnloadTexture(arrowTex);
    UnloadTexture(swordTex);
    UnloadTexture(pauseTex);
  }

  // Update (call every frame)
  void Update() {
    leftHeld = Hits(btnLeft);
    rightHeld = Hits(btnRight);

    // Edge-detect: only true on the first frame of contact
    static bool pJump = false, pAtk = false, pPause = false, pInteract = false;
    bool cJump = Hits(btnJump);
    bool cAtk = Hits(btnAttack);
    bool cPause = Hits(btnPause);

    bool cInteract = Hits(btnInteract);

    jumpPressed = cJump && !pJump;
    attackPressed = cAtk && !pAtk;
    pausePressed = cPause && !pPause;
    interactPressed = cInteract && !pInteract;

    pJump = cJump;
    pAtk = cAtk;
    pPause = cPause;
    pInteract = cInteract;
  }

  // Draw
  void Draw() {
    // Define a background color to make buttons less boring, using UI style
    Color btnBg = Color{UI::BTN_IDLE.r, UI::BTN_IDLE.g, UI::BTN_IDLE.b, 160};

    // [Left] arrow.png is RIGHT-pointing -> mirror horizontally (flipH = true)
    DrawBtn(btnLeft, arrowTex, leftHeld, true, 0.f, btnBg);

    // [Right] arrow.png as-is
    DrawBtn(btnRight, arrowTex, rightHeld, false, 0.f, btnBg);

    // [Up] arrow.png rotated -90 degrees (CCW) -> right becomes up
    DrawBtn(btnJump, arrowTex, Hits(btnJump), false, -90.f, btnBg);

    // [Attack] sword icon, no flip/rotation needed
    DrawBtn(btnAttack, swordTex, Hits(btnAttack), false, 0.f, btnBg);

    // [Interact] empty button, we will draw text over it
    DrawBtn(btnInteract, Texture2D{0}, Hits(btnInteract), false, 0.f, btnBg);
    Fumbo::Graphic2D::DrawText(
        "E", {btnInteract.x + BTN * 0.5f - 10, btnInteract.y + BTN * 0.5f - 16},
        fontDefault, 32, UI::TEXT);

    // [Pause] pause icon - smaller button, top-right
    DrawPauseBtn();
  }

  // State getters
  bool IsLeft() const { return leftHeld; }
  bool IsRight() const { return rightHeld; }
  bool IsJump() const { return jumpPressed; }
  bool IsAttack() const { return attackPressed; }
  bool IsPause() const { return pausePressed; }
  bool IsInteract() const { return interactPressed; }

private:
  Texture2D arrowTex{}, swordTex{}, pauseTex{};

  Rectangle btnLeft{}, btnRight{}, btnJump{}, btnAttack{}, btnPause{},
      btnInteract{};

  bool leftHeld = false, rightHeld = false;
  bool jumpPressed = false, attackPressed = false, pausePressed = false,
       interactPressed = false;

  static constexpr float BTN = 150.f;  // movement/action button size
  static constexpr float PBTN = 100.f; // pause button size
  static constexpr float MARGIN = 24.f;
  static constexpr float GAP = 40.f;

  // Helpers

  // Screen coords -> virtual 1280x720 (letterbox-aware)
  static Vector2 ToVirtual(Vector2 s) {
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    float sc = std::fmin(sw / 1280.f, sh / 720.f);
    return {(s.x - (sw - 1280.f * sc) * .5f) / sc,
            (s.y - (sh - 720.f * sc) * .5f) / sc};
  }

  // True if any touch point is inside rect r (virtual coords)
  static bool Hits(Rectangle r) {
    for (int i = 0; i < GetTouchPointCount(); i++)
      if (CheckCollisionPointRec(ToVirtual(GetTouchPosition(i)), r))
        return true;
    return false;
  }

  // Draw a square button with an icon, optional horizontal flip + rotation.
  //   flipH     - mirrors the source rect (arrow.png right->left)
  //   rotateDeg - clockwise degrees (DrawTexturePro convention);
  //               use -90 to rotate the right-arrow upward
  static void DrawBtn(Rectangle r, Texture2D tex, bool held, bool flipH,
                      float rotateDeg, Color overrideBg = {0, 0, 0, 0}) {
    // Background
    Color baseBg = (overrideBg.a == 0) ? Color{255, 244, 210, 160} : overrideBg;
    Color bg = held ? Color{255, 107, 0, 215} : baseBg;
    Color bdr = held ? UI::AB_YELLOW : UI::LINE;
    Fumbo::Graphic2D::DrawRectangleRounded(r, 0.25f, 6, bg);
    Fumbo::Graphic2D::DrawRectangleRoundedLines(r, 0.25f, 6, bdr);

    if (tex.id == 0)
      return;

    // Dynamically scale the icon so it occupies 70% of the button's smallest
    // dimension
    float maxDim = std::max((float)tex.width, (float)tex.height);
    float scale = (std::min(r.width, r.height) * 0.70f) / maxDim;

    float iw = tex.width * scale;
    float ih = tex.height * scale;
    float cx = r.x + r.width * 0.5f;
    float cy = r.y + r.height * 0.5f;

    // Negative source width = horizontal flip
    Rectangle src = {0, 0, flipH ? -(float)tex.width : (float)tex.width,
                     (float)tex.height};
    // Destination centred on button; origin also centred for rotation pivot
    Rectangle dst = {cx, cy, iw, ih};
    Fumbo::Graphic2D::DrawTexturePro(tex, src, dst, {iw * 0.5f, ih * 0.5f},
                                     rotateDeg, WHITE);
  }

  // Dedicated draw for the smaller pause button (top-right, semi-transparent)
  void DrawPauseBtn() const {
    bool held = Hits(btnPause);
    Color bg = held ? Color{255, 107, 0, 215} : Color{255, 244, 210, 140};
    Color bdr = held ? UI::AB_YELLOW : UI::LINE;
    Fumbo::Graphic2D::DrawRectangleRounded(btnPause, 0.3f, 6, bg);
    Fumbo::Graphic2D::DrawRectangleRoundedLines(btnPause, 0.3f, 6, bdr);

    if (pauseTex.id == 0)
      return;

    float maxDim = std::max((float)pauseTex.width, (float)pauseTex.height);
    float scale = (std::min(btnPause.width, btnPause.height) * 0.70f) / maxDim;

    float iw = pauseTex.width * scale;
    float ih = pauseTex.height * scale;
    float cx = btnPause.x + btnPause.width * 0.5f;
    float cy = btnPause.y + btnPause.height * 0.5f;
    Fumbo::Graphic2D::DrawTexturePro(
        pauseTex, {0, 0, (float)pauseTex.width, (float)pauseTex.height},
        {cx, cy, iw, ih}, {iw * 0.5f, ih * 0.5f}, 0.f, WHITE);
  }
};

// Stub for non-Android platforms
#else

class AndroidControls {
public:
  void Init() {}
  void Unload() {}
  void Update() {}
  void Draw() {}
  bool IsLeft() const { return false; }
  bool IsRight() const { return false; }
  bool IsJump() const { return false; }
  bool IsAttack() const { return false; }
  bool IsPause() const { return false; }
  bool IsInteract() const { return false; }
};

#endif // PLATFORM_ANDROID
