#include "stage1.hpp"
#include "stage0.hpp"
#include "../core/globals.hpp"
#include "../core/settings.hpp"
#include "../states/title_state.hpp"
#include "fumbo.hpp"

void Stage1State::Init() {

  // ---- Camera ----
  camera        = {0};
  camera.zoom   = 1.0f;
  camera.offset = {0, 0};
  camera.target = {0, 0};

  attackHitRegistered = false;

  // ---- Background (plain warm-beige for a cozy room) ----
  bgTex = Fumbo::Utils::ColorToTexture({220, 200, 170, 255});

  // ---- Physics ----
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  physics.Clear();
  physics.SetGravity({0, 980.0f});
  physics.SetFixedTimeStep(60.0f);

  // ---- Room geometry ----
  // Ground / floor
  ground  = Fumbo::Platformer::CreatePlatform({0, 680}, {3000, 40},
                                              {140, 110, 80, 255});
  // Left wall
  wallL   = Fumbo::Platformer::CreatePlatform({-40, 0}, {40, 720},
                                              {160, 130, 100, 255});
  // Right wall
  wallR   = Fumbo::Platformer::CreatePlatform({3000, 0}, {40, 720},
                                             {160, 130, 100, 255});
  // Ceiling
  ceiling = Fumbo::Platformer::CreatePlatform({0, -40}, {3000, 40},
                                              {160, 130, 100, 255});

  // Shelves / furniture as platforms
  shelf1  = Fumbo::Platformer::CreatePlatform({300, 480}, {200, 20},
                                              {120, 80, 40, 255});
  shelf2  = Fumbo::Platformer::CreatePlatform({700, 380}, {200, 20},
                                              {120, 80, 40, 255});

  // ---- Player via SharedMechanics ----
  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());
  if (sharedState) {
    // Spawn just inside the door on the right side of the room entrance
    sharedState->SetupForNewArea({200, 600});
    Fumbo::Engine::Instance().PauseSharedState(false);
  }

  // ---- Door - exits back to Stage0 ----
  // Placed on the left side of the room (the "front door")
  auto doorObj = new Fumbo::Graphic2D::Object();
  doorObj->SetPosition({80, 635});
  doorObj->SetRectangle(30, 80);
  doorObj->SetColor({100, 60, 20, 255}); // dark wood
  doorObj->SetBodyType(Fumbo::Graphic2D::BodyType::Static);
  doorObj->SetCollidable(false); // walk-through trigger
  physics.AddObject(doorObj);

  interactables.Add({80, 650}, 100.0f, "Exit", doorObj, []() {
    Fumbo::Instance().ChangeState(std::make_shared<Stage0State>());
  });

  // ---- NPC - Kaede (friend / neighbor) ----
  npc.label   = "Kaede";
  npc.radius  = 90.0f;
  npc.color   = {200, 140, 100, 255};
  npc.dialogue = {
    "Oh, Nao! You're finally home.",
    "I brought some rice from next door. Hope that's okay.",
    "You've been pushing yourself too hard lately...",
    "Just remember — it's okay to rest sometimes.",
    "Anyway! The food's on the table. Eat up!"
  };
  npc.Spawn({600, 628}, {28, 56});
}

void Stage1State::Cleanup() {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  // NPC
  npc.Destroy();

  // Interactables
  interactables.Cleanup();

  // Room geometry
  auto cleanObj = [&](Fumbo::Graphic2D::Object *&obj) {
    if (obj) {
      physics.RemoveObject(obj);
      delete obj;
      obj = nullptr;
    }
  };
  cleanObj(ground);
  cleanObj(wallL);
  cleanObj(wallR);
  cleanObj(ceiling);
  cleanObj(shelf1);
  cleanObj(shelf2);

  UnloadTexture(bgTex);
}

void Stage1State::Update() {
  auto &physics   = Fumbo::Graphic2D::Physics::Instance();
  float deltaTime = GetFrameTime();

  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());
  auto player = sharedState ? sharedState->GetPlayer() : nullptr;

  if (IsKeyPressed(KEY_R))
    Init();

  // SharedMechanics handles pause / RETRY / QUIT / game-over.
  if (sharedState && sharedState->ConsumeResetStage()) {
    Init();
    return;
  }

  if (sharedState && sharedState->ShouldSkipUpdate())
    return;

  // No enemies in Stage1 - skip attack gate

  // Physics
  physics.Update(deltaTime);

  if (player) {
    bool interactPressed = sharedState && sharedState->IsInteractPressed();
    // NPC update (handles proximity + dialogue input)
    npc.Update(player->GetPosition(), interactPressed);

    if (sharedState) {
        sharedState->SetInputsBlocked(npc.IsTalking());
    }

    // Interactables (door)
    interactables.Update(player->GetPosition(), interactPressed);
  }

  // Camera follow
  if (player) {
    Vector2 pos = player->GetPosition();
    Fumbo::Utils::Camera2DFollow(&camera, {pos.x - 20, pos.y - 20, 40, 40},
                                 200, 0, 25.0f);

    // Fall-kill (shouldn't happen with ceiling/walls but just in case)
    if (pos.y >= 3000 && sharedState)
      sharedState->SetGameOver(true);
  }
}

void Stage1State::DrawClean() {
  Fumbo::Graphic2D::DrawBackground(bgTex);
}

void Stage1State::DrawScene() {
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

  // NPC world-space prompt
  npc.DrawWorld();

  // Interactable world-space prompts
  interactables.DrawWorld();

  EndMode2D();
}

void Stage1State::DrawDirty() {
  auto sharedState = std::dynamic_pointer_cast<SharedMechanics>(
      Fumbo::Engine::Instance().GetSharedState());

  if (sharedState)
    sharedState->BeginSceneRender();
  DrawScene();
  if (sharedState)
    sharedState->EndSceneRenderAndPostProcess();

  // NPC dialogue textbox
  npc.DrawHUD();

  // Door prompt
  interactables.DrawHUD();

  // Room label
  Fumbo::Graphic2D::DrawText("~ Nao's House ~", {560, 10}, {}, 22,
                              Color{200, 170, 120, 200});

  // Keybind hint
  Fumbo::Graphic2D::DrawText(
      "A/D: Move | SPACE: Jump | K: Attack | E: Interact | F1: Mode | F3: Debug",
      {10, 700}, {}, 18, LIGHTGRAY);
}
