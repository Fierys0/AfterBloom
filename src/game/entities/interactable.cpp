#include "interactable.hpp"
#include "raylib.h"

// ---------------------------------------------------------------
// Add
// ---------------------------------------------------------------

void InteractableSystem::Add(Vector2 pos, float radius,
                              const std::string &label,
                              Fumbo::Graphic2D::Object *obj,
                              std::function<void()> callback) {
  entries.push_back({pos, radius, label, obj, std::move(callback)});
}

// ---------------------------------------------------------------
// Update  (call every frame)
// ---------------------------------------------------------------

void InteractableSystem::Update(Vector2 playerPos, bool interactPressed) {
  lastPlayerPos = playerPos;
  playerNear    = false;

  for (auto &e : entries) {
    float dx   = playerPos.x - e.pos.x;
    float dy   = playerPos.y - e.pos.y;
    bool  near = (sqrtf(dx * dx + dy * dy) <= e.radius);

    if (near) {
      playerNear = true;

      if ((IsKeyPressed(KEY_E) || interactPressed) && e.onInteract) {
        e.onInteract();
        return; // fire at most one interactable per frame
      }
    }
  }
}

// ---------------------------------------------------------------
// DrawWorld  (inside BeginMode2D)
// ---------------------------------------------------------------

void InteractableSystem::DrawWorld() const {
  for (const auto &e : entries) {
    float dx   = lastPlayerPos.x - e.pos.x;
    float dy   = lastPlayerPos.y - e.pos.y;
    bool  near = (sqrtf(dx * dx + dy * dy) <= e.radius);

    if (near) {
      // World-space label - raw DrawText because we're inside a camera transform
      DrawText(TextFormat("[E] %s", e.label.c_str()),
               (int)e.pos.x - 20, (int)e.pos.y - 70, 18, WHITE);
    }
  }
}

// ---------------------------------------------------------------
// DrawHUD  (screen-space, Fumbo::Graphic2D for virtual-res scaling)
// ---------------------------------------------------------------

void InteractableSystem::DrawHUD() const {
  if (!playerNear)
    return;

  // Dark pill at bottom-centre of the virtual 1280x720 canvas
  Fumbo::Graphic2D::DrawRectangle(500, 620, 280, 40, Fade(BLACK, 0.7f));
  Fumbo::Graphic2D::DrawRectangleLines(500, 620, 280, 40, WHITE);
  Fumbo::Graphic2D::DrawText("[E] Interact", {520, 630}, {}, 22, WHITE);
}

// ---------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------

void InteractableSystem::Cleanup() {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  for (auto &e : entries) {
    if (e.obj) {
      physics.RemoveObject(e.obj);
      delete e.obj;
      e.obj = nullptr;
    }
  }
  entries.clear();
  playerNear = false;
}
