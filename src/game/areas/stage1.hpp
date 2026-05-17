#pragma once
#include "fumbo.hpp"
#include "template/platformer.hpp"
#include "../states/shared_mechanics.hpp"
#include "../entities/interactable.hpp"
#include "../entities/npc.hpp"
#include "raylib.h"
#include <memory>
#include <vector>

// Stage1 — MC's house interior
class Stage1State : public IGameState {
public:
  ~Stage1State() override = default;
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;
  void DrawScene();

private:
  Texture2D bgTex{};
  Camera2D camera;

  // Physics objects
  Fumbo::Graphic2D::Object *ground  = nullptr;
  Fumbo::Graphic2D::Object *wallL   = nullptr;
  Fumbo::Graphic2D::Object *wallR   = nullptr;
  Fumbo::Graphic2D::Object *ceiling = nullptr;

  // Platform shelves / furniture (optional decoration)
  Fumbo::Graphic2D::Object *shelf1  = nullptr;
  Fumbo::Graphic2D::Object *shelf2  = nullptr;

  // Entities
  InteractableSystem interactables;
  NpcEntity          npc;

  // Attack gate
  bool attackHitRegistered = false;
};
