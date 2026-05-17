#pragma once
#include "fumbo.hpp"
#include "raylib.h"
#include <cmath>
#include <functional>
#include <string>
#include <vector>

// ---------------------------------------------------------------
// InteractableSystem - manages a list of world-space interactable
// objects (doors, levers, NPCs, etc.) for any area state.
//
// Usage:
//   ia.Add({1400, 640}, 100.f, "Enter", doorObj, []{ ... });
//   ia.Update(playerPos);          // call each frame
//   ia.DrawWorld();                // inside BeginMode2D
//   ia.DrawHUD();                  // in DrawDirty (screen-space, Fumbo scaled)
//   ia.Cleanup();                  // before physics.Clear()
// ---------------------------------------------------------------
class InteractableSystem {
public:
  struct Entry {
    Vector2 pos;                        // world-space anchor
    float radius;                       // activation radius (pixels)
    std::string label;                  // verb shown on prompt, e.g. "Enter"
    Fumbo::Graphic2D::Object *obj;      // optional physics/visual object
    std::function<void()> onInteract;   // callback fired on KEY_E
  };

  // Add an interactable.  obj may be nullptr for invisible triggers.
  void Add(Vector2 pos, float radius, const std::string &label,
           Fumbo::Graphic2D::Object *obj,
           std::function<void()> callback);

  // Call every frame with the player's world position.
  // Handles KEY_E press and proximity detection.
  void Update(Vector2 playerPos, bool interactPressed = false);

  // Call inside BeginMode2D - draws "[E] Label" above each nearby object.
  void DrawWorld() const;

  // Call in DrawDirty (screen-space) - draws the pill prompt at bottom-center
  // using Fumbo::Graphic2D so it scales with the virtual resolution.
  void DrawHUD() const;

  // Remove all physics objects and clear the list.
  void Cleanup();

  bool IsPlayerNear() const { return playerNear; }

private:
  std::vector<Entry> entries;
  bool playerNear = false;   // updated each frame in Update()
  Vector2 lastPlayerPos = {0, 0};
};
