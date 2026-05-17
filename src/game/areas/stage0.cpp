#include "stage0.hpp"
#include "stage1.hpp"
#include "../core/globals.hpp"
#include "../core/settings.hpp"
#include "../states/title_state.hpp"
#include "fumbo.hpp"

void Stage0State::Init() {

  // Setup camera
  camera = {0};
  camera.zoom = 1.0f;
  camera.offset = {0, 0};
  camera.target = {0, 0};

  attackHitRegistered = false;

  // Load background
  bgTex = Fumbo::Assets::LoadTexture("");

  // Setup physics
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  physics.Clear();
  physics.SetGravity({0, 980.0f});
  physics.SetFixedTimeStep(60.0f);

  // Platforms
  ground    = Fumbo::Platformer::CreatePlatform({0, 680}, {9000, 80}, DARKGRAY);
  platform1 = Fumbo::Platformer::CreatePlatform({1000, 500}, {200, 20}, GREEN);
  platform2 = Fumbo::Platformer::CreatePlatform({640, 400}, {200, 20}, GREEN);

  // Player via shared mechanics
  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());
  if (sharedState) {
    sharedState->SetupForNewArea({100, 600});
    Fumbo::Engine::Instance().PauseSharedState(false);
  }

  // Pushable crates
  for (int i = 0; i < 3; i++) {
    auto crate = new Fumbo::Graphic2D::Object();
    crate->SetPosition({300.0f + i * 200.0f, 420.0f - i * 100.0f});
    crate->SetRectangle(40 + i * 10, 40 + i * 10);
    crate->SetMass(5.0f + i * 2.0f);
    crate->SetRestitution(0.0f);
    crate->SetFriction(0.5f);
    Color colors[] = {BROWN, ORANGE, YELLOW};
    crate->SetColor(colors[i]);
    physics.AddObject(crate);
    crates.push_back(crate);
  }

  // ---- Multiple Slimes ----
  // Each slime has slightly different stats / positions to make it interesting
  struct SlimeConfig {
    Vector2 pos;
    int     hp;
    float   patrolSpeed;
    float   chaseSpeed;
    float   chaseRange;
  };

  const SlimeConfig configs[] = {
    {{800,  600}, 30, 70.0f,  120.0f, 300.0f},
    {{1300, 600}, 20, 100.0f, 140.0f, 250.0f},
    {{1800, 600}, 30, 60.0f,  110.0f, 350.0f},
    {{2300, 600}, 40, 80.0f,  130.0f, 280.0f},
  };

  slimes.clear();
  for (const auto &cfg : configs) {
    SlimeEnemy s;
    s.maxHealth    = cfg.hp;
    s.damagePerHit = 10;
    s.onDeath      = nullptr;
    s.patrolSpeed  = cfg.patrolSpeed;
    s.chaseSpeed   = cfg.chaseSpeed;
    s.chaseRange   = cfg.chaseRange;
    s.Spawn(cfg.pos);
    slimes.push_back(std::move(s));
  }

  // ---- Door - enters Nao's House (Stage1) ----
  auto doorObj = new Fumbo::Graphic2D::Object();
  doorObj->SetPosition({1400, 625});
  doorObj->SetRectangle(30, 80);
  doorObj->SetColor({139, 90, 43, 255}); // wood brown
  doorObj->SetBodyType(Fumbo::Graphic2D::BodyType::Static);
  doorObj->SetCollidable(false); // walk-through
  physics.AddObject(doorObj);

  interactables.Add({1400, 640}, 100.0f, "Enter House", doorObj, []() {
    Fumbo::Instance().ChangeState(std::make_shared<Stage1State>());
  });
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
      if (s.IsAlive() &&
          CheckCollisionRecs(sharedState->GetAttackHitbox(),
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
    Fumbo::Utils::Camera2DFollow(&camera, {pos.x - 20, pos.y - 20, 40, 40},
                                 200, 0, 25.0f);

    // Fall-out kill plane
    if (pos.y >= 3000 && sharedState)
      sharedState->SetGameOver(true);
  }
}

void Stage0State::DrawClean() { Fumbo::Graphic2D::DrawBackground(bgTex); }

void Stage0State::DrawScene() {
  BeginMode2D(camera);

  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  for (const auto &obj : physics.GetObjects())
    obj->Render();

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
