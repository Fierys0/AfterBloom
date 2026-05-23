#pragma once
#include "../entities/interactable.hpp"
#include "../entities/slime_enemy.hpp"
#include "../states/shared_mechanics.hpp"
#include "fumbo.hpp"
#include "fumbo/physics.hpp"
#include "raylib.h"
#include "template/platformer.hpp"
#include <cstddef>
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
  Fumbo::Graphic2D::Object *ground = nullptr;
  Fumbo::Graphic2D::Object *platform1 = nullptr;
  Fumbo::Graphic2D::Object *platform2 = nullptr;
  Fumbo::Graphic2D::Object *platform3 = nullptr;
  Fumbo::Graphic2D::Object *invisWall = nullptr;
  Fumbo::Graphic2D::Object *groundHelper = nullptr;
  Fumbo::Graphic2D::Object *platform3Helper = nullptr;
  Fumbo::Graphic2D::Object *platform1Helper = nullptr;

  std::vector<Fumbo::Graphic2D::Object *> crates;

  // Multiple slimes
  std::vector<SlimeEnemy> slimes;

  // Interactables (door to Stage1)
  InteractableSystem interactables;

  // Attack hit gate: one damage event per swing, per slime
  bool attackHitRegistered = false;

  // --- Terrain and object sprites ---
  Texture2D tileTex{};
  Texture2D crateTex{};
  Texture2D undergroundTex{};

  // --- Non-solid decorations ---
  struct Decoration {
    Rectangle worldRect; // position + size in world space
    Texture2D tex;
  };
  std::vector<Decoration> decorations;
};
