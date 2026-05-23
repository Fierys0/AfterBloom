#include "stage0.hpp"
#include "../core/globals.hpp"
#include "../core/settings.hpp"
#include "../states/title_state.hpp"
#include "event1.hpp"
#include "fumbo.hpp"
#include "fumbo/physics.hpp"
#include "raylib.h"
#include "stage1.hpp"
#include "template/platformer.hpp"

void Stage0State::Init() {

  // Setup camera
  camera = {0};
  camera.zoom = 1.0f;
  camera.offset = {0, 0};
  camera.target = {0, 0};

  attackHitRegistered = false;

  // Load background
  bgTex = Fumbo::Assets::LoadTexture("assets/images/orig.png");

  // Setup physics
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  physics.Clear();
  physics.SetGravity({0, 980.0f});
  physics.SetFixedTimeStep(60.0f);

  // Platforms
  ground = Fumbo::Platformer::CreatePlatform({0, 690}, {7000, 96}, DARKGRAY);
  platform1 = Fumbo::Platformer::CreatePlatform({1800, 500}, {400, 70}, GREEN);
  platform2 = Fumbo::Platformer::CreatePlatform({2300, 400}, {600, 130}, GREEN);
  platform3 = Fumbo::Platformer::CreatePlatform({5000, 400}, {3000, 96}, GREEN);
  invisWall = Fumbo::Platformer::CreatePlatform({-800, 0}, {100, 10000}, BLANK);
  groundHelper =
      Fumbo::Platformer::CreatePlatform({0, 1735}, {11000, 2000}, DARKGRAY);
  platform3Helper =
      Fumbo::Platformer::CreatePlatform({5000, 745}, {3000, 600}, GREEN);
  platform1Helper = Fumbo::Platformer::CreatePlatform({2300, 500}, {600, 70});

  // Player via shared mechanics
  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());
  if (sharedState) {
    sharedState->SetupForNewArea({100, 600});
    Fumbo::Engine::Instance().PauseSharedState(false);
  }

  // Pushable crates
  auto crate = new Fumbo::Graphic2D::Object();
  crate->SetPosition({2400, 300});
  crate->SetRectangle(150, 150);
  crate->SetMass(1);
  crate->SetRestitution(0.0f);
  crate->SetFriction(0.2);
  Color colors[] = {BROWN, ORANGE, YELLOW};
  crate->SetColor(BROWN);
  physics.AddObject(crate);
  crates.push_back(crate);

  struct SlimeConfig {
    Vector2 pos;
    int hp;
    float patrolSpeed;
    float chaseSpeed;
    float chaseRange;
  };

  const SlimeConfig configs[] = {
      {{800, 600}, 30, 70.0f, 120.0f, 300.0f},
      {{1300, 600}, 20, 100.0f, 140.0f, 250.0f},
      {{1800, 600}, 30, 60.0f, 110.0f, 350.0f},
      {{2300, 600}, 40, 80.0f, 130.0f, 280.0f},
  };

  slimes.clear();
  for (const auto &cfg : configs) {
    SlimeEnemy s;
    s.maxHealth = cfg.hp;
    s.damagePerHit = 10;
    s.onDeath = nullptr;
    s.patrolSpeed = cfg.patrolSpeed;
    s.chaseSpeed = cfg.chaseSpeed;
    s.chaseRange = cfg.chaseRange;
    s.Spawn(cfg.pos);
    slimes.push_back(std::move(s));
  }

  // ---- Door - enters Nao's House (Stage1) ----
  auto doorObj = new Fumbo::Graphic2D::Object();
  doorObj->SetPosition({100, 625});
  doorObj->SetRectangle(30, 80);
  doorObj->SetColor({139, 90, 43, 255}); // wood brown
  doorObj->SetBodyType(Fumbo::Graphic2D::BodyType::Static);
  doorObj->SetCollidable(false); // walk-through
  physics.AddObject(doorObj);

  interactables.Add(
      doorObj->GetPosition(), 100.0f, "Enter House", doorObj,
      []() { Fumbo::Instance().ChangeState(std::make_shared<Stage1State>()); });

  // --- Load terrain and object sprites ---
  tileTex = Fumbo::Assets::LoadTexture("assets/sprite/Terrain/tile27.png");
  crateTex = Fumbo::Assets::LoadTexture("assets/sprite/Terrain/box.png");
  undergroundTex =
      Fumbo::Assets::LoadTexture("assets/sprite/Terrain/tile52.png");

  const float groundTop = 650; // world y of ground surface
  decorations.clear();

  // Pre-load decoration textures we want to use
  Texture2D texTree1 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/trees1_1.png");
  Texture2D texTree2 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/trees1_3.png");
  Texture2D texTree3 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/trees2_1.png");
  Texture2D texGrass1 =
      Fumbo::Assets::LoadTexture("assets/sprite/Terrain/Decoration/grass1.png");
  Texture2D texGrass2 =
      Fumbo::Assets::LoadTexture("assets/sprite/Terrain/Decoration/grass2.png");
  Texture2D texGrass3 =
      Fumbo::Assets::LoadTexture("assets/sprite/Terrain/Decoration/grass3.png");
  Texture2D texRock1 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/rocks1_1.png");
  Texture2D texRock2 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/rocks2_1.png");
  Texture2D texBush1 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/pink_bush1.png");
  Texture2D texBush2 = Fumbo::Assets::LoadTexture(
      "assets/sprite/Terrain/Decoration/pink_bush2.png");

  auto addDec = [&](float x, float groundSurface, float w, float h,
                    Texture2D t) {
    decorations.push_back({{x - w * 0.5f, groundSurface - h, w, h}, t});
  };

  // Trees
  addDec(150, groundTop, 60, 120, texTree1);
  addDec(450, groundTop, 60, 120, texTree3);
  addDec(650, groundTop, 60, 120, texTree2);
  addDec(1050, groundTop, 60, 120, texTree1);
  addDec(1600, groundTop, 60, 120, texTree3);
  addDec(1900, groundTop, 60, 120, texTree2);
  addDec(2050, groundTop, 60, 120, texTree1);

  // Grass clusters
  addDec(200, groundTop, 32, 24, texGrass1);
  addDec(350, groundTop, 32, 24, texGrass2);
  addDec(560, groundTop, 32, 24, texGrass3);
  addDec(900, groundTop, 32, 24, texGrass1);
  addDec(1150, groundTop, 32, 24, texGrass2);
  addDec(1700, groundTop, 32, 24, texGrass3);

  // Rocks
  addDec(280, groundTop, 40, 30, texRock1);
  addDec(750, groundTop, 40, 30, texRock2);
  addDec(1200, groundTop, 40, 30, texRock1);

  // Bushes
  addDec(500, groundTop, 48, 36, texBush1);
  addDec(1000, groundTop, 48, 36, texBush2);
  addDec(1750, groundTop, 48, 36, texBush1);
}

void Stage0State::Cleanup() {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  // Crates
  for (auto &crate : crates) {
    physics.RemoveObject(crate);
    delete crate;
  }
  crates.clear();

  // All slimes
  for (auto &s : slimes)
    s.Destroy();
  slimes.clear();

  // Interactables (removes their physics objects too)
  interactables.Cleanup();

  // Platforms
  if (ground) {
    physics.RemoveObject(ground);
    delete ground;
    ground = nullptr;
  }
  if (platform1) {
    physics.RemoveObject(platform1);
    delete platform1;
    platform1 = nullptr;
  }
  if (platform2) {
    physics.RemoveObject(platform2);
    delete platform2;
    platform2 = nullptr;
  }

  // Render textures
  UnloadTexture(bgTex);

  // Terrain + crate textures
  if (tileTex.id != 0) {
    UnloadTexture(tileTex);
    UnloadTexture(undergroundTex);
  }
  if (crateTex.id != 0)
    UnloadTexture(crateTex);

  // Decoration textures (each stored per-decoration)
  for (auto &d : decorations)
    if (d.tex.id != 0)
      UnloadTexture(d.tex);
  decorations.clear();
}

void Stage0State::Update() {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  float deltaTime = GetFrameTime();

  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());
  auto player = sharedState ? sharedState->GetPlayer() : nullptr;

  if (IsKeyPressed(KEY_R))
    Init();

  // SharedMechanics handles pause / RETRY / QUIT / game-over.
  // When it signals a stage reset (RETRY or R-key), re-init this stage.
  if (sharedState && sharedState->ConsumeResetStage()) {
    Init();
    return;
  }

  // Skip stage logic while paused or game-over (SharedMechanics returns early
  // for those frames too, so input is already suppressed).
  if (sharedState && sharedState->ShouldSkipUpdate())
    return;

  // Player attack - reset hit gate when swing ends
  if (!sharedState->IsAttacking())
    attackHitRegistered = false;

  // Check attack hitbox against every living slime
  if (sharedState->IsAttacking() && !attackHitRegistered) {
    for (auto &s : slimes) {
      if (s.IsAlive() && CheckCollisionRecs(sharedState->GetAttackHitbox(),
                                            s.GetObject()->GetAABB())) {
        attackHitRegistered = true;
        s.TakeHit(10, (float)sharedState->GetFacingDirection() * 150.0f);
        break; // one slime hit per swing (prevents multi-hit in one frame)
      }
    }
  }

  // Physics step
  physics.Update(deltaTime);

  // Update all slimes
  for (auto &s : slimes)
    if (player)
      s.Update(deltaTime, player->GetPosition(), player, sharedState.get());

  // Interactable proximity + KEY_E
  if (player) {
    bool interactPressed = sharedState && sharedState->IsInteractPressed();
    interactables.Update(player->GetPosition(), interactPressed);
  }

  // Camera
  if (player) {
    Vector2 pos = player->GetPosition();
    Fumbo::Utils::Camera2DFollow(&camera, {pos.x - 20, pos.y - 20, 40, 40}, 200,
                                 0, 25.0f);

    // Fall-out kill plane
    if (pos.y >= 3000 && sharedState)
      sharedState->SetGameOver(true);
  }

  if (player && player->GetPosition().x >= 5200) {
    Fumbo::Instance().PauseSharedState(true);
    Fumbo::Instance().ChangeState(std::make_shared<Event1>());
  }
}

void Stage0State::DrawClean() { Fumbo::Graphic2D::DrawBackground(bgTex); }

void Stage0State::DrawScene() {
  BeginMode2D(camera);

  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  // Non-solid decorations (trees, grass, rocks, bushes)
  for (const auto &d : decorations) {
    if (d.tex.id == 0)
      continue;
    Rectangle src = {0, 0, (float)d.tex.width, (float)d.tex.height};
    Fumbo::Utils::DrawWorldSpriteAt(d.worldRect, d.tex, src);
  }

  for (const auto &obj : physics.GetObjects())
    obj->Render();

  if (tileTex.id != 0) {
    const float tileRepeat = 96;
    Fumbo::Utils::DrawWorldSpriteTiled(platform3Helper, undergroundTex,
                                       tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_XY);
    Fumbo::Utils::DrawWorldSpriteTiled(groundHelper, undergroundTex, tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_XY);
    Fumbo::Utils::DrawWorldSpriteTiled(platform1Helper, undergroundTex,
                                       tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_X);
    Fumbo::Utils::DrawWorldSpriteTiled(platform3, tileTex, tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_X);
    Fumbo::Utils::DrawWorldSpriteTiled(ground, tileTex, tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_X);
    Fumbo::Utils::DrawWorldSpriteTiled(platform1, tileTex, tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_X);
    Fumbo::Utils::DrawWorldSpriteTiled(platform2, tileTex, tileRepeat,
                                       Fumbo::Utils::TileMode::TILE_X);
  }

  // Crate sprites
  if (crateTex.id != 0) {
    for (const auto &crate : crates) {
      if (!crate)
        continue;
      Rectangle aabb = crate->GetAABB();
      Rectangle src = {0, 0, (float)crateTex.width, (float)crateTex.height};
      Fumbo::Utils::DrawWorldSpriteAt(aabb, crateTex, src);
    }
  }

  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());

  if (sharedState)
    sharedState->DrawPlayerWorld();

  if (sharedState && sharedState->IsDebugEnabled())
    physics.DrawDebug();

  // Draw all slimes (HP bars + debug labels)
  bool dbg = sharedState && sharedState->IsDebugEnabled();
  for (const auto &s : slimes)
    s.DrawWorld(dbg);

  // Interactable world-space "[E] Label" prompts
  interactables.DrawWorld();

  EndMode2D();
}

void Stage0State::DrawDirty() {
  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());

  if (sharedState)
    sharedState->BeginSceneRender();
  DrawScene();
  if (sharedState)
    sharedState->EndSceneRenderAndPostProcess();

  interactables.DrawHUD();

  // Keybind hint
  Fumbo::Graphic2D::DrawText("A/D: Move | SPACE: Jump | K: Attack | E: "
                             "Interact | F1: Mode | F3: Debug",
                             {10, 700}, {}, 18, LIGHTGRAY);
}
