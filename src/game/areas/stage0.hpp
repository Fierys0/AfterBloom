#pragma once
#include "fumbo.hpp"
#include "template/platformer.hpp"
#include "../states/shared_mechanics.hpp"
#include "../entities/slime_enemy.hpp"
#include "../entities/interactable.hpp"
#include "raylib.h"
#include <memory>
#include <vector>

class Stage0State : public IGameState {
public:
  ~Stage0State() override = default;
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
  Fumbo::Graphic2D::Object *ground    = nullptr;
  Fumbo::Graphic2D::Object *platform1 = nullptr;
  Fumbo::Graphic2D::Object *platform2 = nullptr;

  std::vector<Fumbo::Graphic2D::Object *> crates;

  // Multiple slimes
  std::vector<SlimeEnemy> slimes;

  // Interactables (door to Stage1)
  InteractableSystem interactables;

  // Attack hit gate: one damage event per swing, per slime
  bool attackHitRegistered = false;
};
