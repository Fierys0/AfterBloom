#pragma once
#include "fumbo.hpp"
#include "ui_style.hpp"

// ── MinSlider ────────────────────────────────────────────────────────────────
// Custom slider that draws through Fumbo::Graphic2D (virtual-resolution safe).
// GetMousePosition() is mapped to virtual 1280x720 space before hit-testing.
struct MinSlider {
  float minVal = 0.f, maxVal = 1.f, value = 0.5f;
  bool dragging = false;

  MinSlider() = default;
  MinSlider(float mn, float mx, float v) : minVal(mn), maxVal(mx), value(v) {}

  // Returns virtual mouse position, accounting for letterbox offset + scale
  static Vector2 VirtualMouse() {
    float sw    = (float)GetScreenWidth();
    float sh    = (float)GetScreenHeight();
    float scale = std::min(sw / 1280.f, sh / 720.f);
    float ox    = (sw - 1280.f * scale) * 0.5f;
    float oy    = (sh - 720.f  * scale) * 0.5f;
    Vector2 m   = GetMousePosition();
    return {(m.x - ox) / scale, (m.y - oy) / scale};
  }

  // Call every frame; returns true if value changed
  bool Update(Rectangle b) {
    Vector2 m  = VirtualMouse();
    float  t   = Clamp((value - minVal) / (maxVal - minVal), 0.f, 1.f);
    float  kx  = b.x + t * b.width;
    // Click anywhere on the track to jump, or drag the knob
    Rectangle trackHit = {b.x - 4, b.y - 10, b.width + 8, b.height + 20};
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, trackHit))
      dragging = true;
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
      dragging = false;
    if (dragging) {
      float nt  = Clamp((m.x - b.x) / b.width, 0.f, 1.f);
      float nv  = minVal + nt * (maxVal - minVal);
      bool changed = (nv != value);
      value = nv;
      return changed;
    }
    return false;
  }

  // All drawing through Fumbo::Graphic2D → properly scaled
  void Draw(Rectangle b) {
    float t    = Clamp((value - minVal) / (maxVal - minVal), 0.f, 1.f);
    float fillW = t * b.width;

    // Track
    Fumbo::Graphic2D::DrawRectangleRounded(b, 1.f, 4, UI::LINE);
    // Fill
    if (fillW > 1.f)
      Fumbo::Graphic2D::DrawRectangleRounded({b.x, b.y, fillW, b.height}, 1.f, 4, UI::AB_ORANGE);
    // Knob
    constexpr float KW = 14.f, KH_EXTRA = 10.f;
    float kx = b.x + fillW - KW * 0.5f;
    float ky = b.y - KH_EXTRA * 0.5f;
    Fumbo::Graphic2D::DrawRectangleRounded(
        {kx, ky, KW, b.height + KH_EXTRA}, 0.5f, 6, UI::AB_YELLOW);
  }

  float GetValue() const { return value; }
  void  SetValue(float v) { value = Clamp(v, minVal, maxVal); }
};

// ── Settings ─────────────────────────────────────────────────────────────────
class Settings {
public:
  static Settings &Instance();

  bool Update(bool showTitleButton = false);
  void DrawClean();
  void DrawDirty();

private:
  Settings() = default;
  ~Settings() = default;
  Settings(const Settings &) = delete;
  Settings &operator=(const Settings &) = delete;

  Fumbo::UI::Button exitButton;
  Fumbo::UI::Button titleButton;
  Fumbo::UI::Button quitButton;

  MinSlider volumeSlider;
  MinSlider fpsSlider;

  Fumbo::UI::Button vsyncToggleButton;
  Fumbo::UI::Button godraysToggle;

  // State
  float masterVolume = 1.0f;
  bool  vsync        = false;
  int   fpsLimit     = 60;
  bool  godraysEnabled = false;

  bool closing              = false;
  bool loaded               = false;
  bool shouldShowTitleButton = false;
  Texture2D backgroundTexture{};

  bool Load();
};
