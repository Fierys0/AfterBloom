#pragma once
#include "fumbo.hpp"
#include "raylib.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>

// ---------------------------------------------------------------
// NpcEntity - a reusable, stationary NPC that shows a multi-line
// dialogue textbox when the player presses KEY_E nearby.
//
// Usage (in any area state):
//   npc.dialogue = {"Hello!", "How are you?", "Goodbye."};
//   npc.label    = "Kaede";
//   npc.Spawn({500, 620}, {30, 60});
//
//   // each frame:
//   npc.Update(playerPos);          // proximity + input
//   npc.DrawWorld();                // inside BeginMode2D
//   npc.DrawHUD();                  // in DrawDirty (screen-space)
//
//   npc.Destroy();                  // before physics.Clear()
// ---------------------------------------------------------------
class NpcEntity {
public:
  // --- Configuration (set before Spawn) ---
  std::string              label    = "NPC";     // name shown in textbox
  std::vector<std::string> dialogue;             // lines to cycle through
  float                    radius   = 90.0f;     // interaction radius (px)
  Color                    color    = {180, 120, 60, 255}; // body colour

  // --- Lifecycle ---
  void Spawn(Vector2 position, Vector2 size = {28, 56});
  void Destroy();
  bool IsAlive() const { return physicsObj != nullptr; }

  // --- Per-frame ---
  void Update(Vector2 playerPos, bool interactPressed = false);         // call every frame
  void DrawWorld() const;                 // inside BeginMode2D
  void DrawHUD()   const;                 // DrawDirty (screen-space)

  Fumbo::Graphic2D::Object *GetObject() const { return physicsObj; }

  bool IsTalking() const { return talking; }

private:
  Fumbo::Graphic2D::Object *physicsObj = nullptr;
  Vector2 spawnPos = {0, 0};

  bool talking        = false;
  bool playerNear     = false;
  int  dialogueIndex  = 0;

  std::unique_ptr<Fumbo::UI::VisualNovel> vn;
  Font npcFont{};

  void StartLine(int idx);
};
