#include "shared_mechanics.hpp"
#include "../core/globals.hpp"
#include "../core/ui_style.hpp"
#include "fumbo.hpp"
#include "pause_menu.hpp"
#include "title_state.hpp"
#include "raylib.h"

void SharedMechanics::Init() {
  attackTimer = 0.0f;
  playerHealth = playerMaxHealth;
  playerInvincibleTimer = 0.0f;

  // Load player sprite sheets
  sprIdle =
      Fumbo::Assets::LoadTexture("assets/sprite/Player/Player Idle 48x48.png");
  sprWalk =
      Fumbo::Assets::LoadTexture("assets/sprite/Player/PlayerWalk 48x48.png");
  sprRun =
      Fumbo::Assets::LoadTexture("assets/sprite/Player/player run 48x48.png");
  sprJump = Fumbo::Assets::LoadTexture(
      "assets/sprite/Player/player new jump 48x48.png");
  sprPush =
      Fumbo::Assets::LoadTexture("assets/sprite/Player/player push 48x48.png");
  sprAttack = Fumbo::Assets::LoadTexture(
      "assets/sprite/Player/Player Sword Stab 96x48.png");
  sprDeath =
      Fumbo::Assets::LoadTexture("assets/sprite/Player/Player Death 64x64.png");
  sprFrame = 0;
  sprTimer = 0.0f;
  showDebug = true;
  gameOver = false;
  Fumbo::Graphic2D::Physics::Instance().SetDebugDraw(showDebug);

  sprPotrait = Fumbo::Assets::LoadTexture("");

  // Initialize Android on-screen controls (no-op stub on desktop)
  androidControls.Init();

  // Initialize Game Over Buttons
  btnRetry = Fumbo::UI::Button();
  btnRetry.SetButtonColor(UI::BTN_IDLE);
  btnRetry.IdleColor(UI::BTN_IDLE);
  btnRetry.HoveredColor(UI::BTN_HOVER);
  btnRetry.AddText("RETRY", fontDefault, 24, UI::BTN_TEXT);
  btnRetry.SetBounds(500, 360, 280, 50);

  btnExit = Fumbo::UI::Button();
  btnExit.SetButtonColor(UI::BTN_IDLE);
  btnExit.IdleColor(UI::BTN_IDLE);
  btnExit.HoveredColor(UI::BTN_HOVER);
  btnExit.AddText("EXIT", fontDefault, 24, UI::BTN_TEXT);
  btnExit.SetBounds(500, 420, 280, 50);

  // Initialize render texture for scene
  currentScreen = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  lastScreenWidth = GetScreenWidth();
  lastScreenHeight = GetScreenHeight();
}

void SharedMechanics::SetupForNewArea(Vector2 spawnPosition) {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  // Create player if it doesn't exist
  if (!player) {
    player = Fumbo::Platformer::CreateCharacter(spawnPosition, {40, 40},
                                                {0, 0, 0, 0});
    controller = Fumbo::Platformer::CreateController(player, 350.0f, 600.0f);
  } else {
    // Just move existing player and re-add to physics
    // (Assuming the area just called physics.Clear())
    player->SetPosition(spawnPosition);
    player->SetVelocity({0, 0});
    physics.AddObject(player);
  }
}

void SharedMechanics::Cleanup() {
  if (controller) {
    delete controller;
    controller = nullptr;
  }
  if (player) {
    Fumbo::Graphic2D::Physics::Instance().RemoveObject(player);
    delete player;
    player = nullptr;
  }
  UnloadTexture(sprIdle);
  UnloadTexture(sprWalk);
  UnloadTexture(sprRun);
  UnloadTexture(sprJump);
  UnloadTexture(sprJump);
  UnloadTexture(sprPush);
  UnloadTexture(sprAttack);
  UnloadTexture(sprDeath);

  androidControls.Unload();

  if (currentScreen.id != 0) {
    UnloadRenderTexture(currentScreen);
    currentScreen = {0};
  }
  if (maskRenderTarget.id != 0) {
    UnloadRenderTexture(maskRenderTarget);
    maskRenderTarget = {0};
  }
  if (maskShader.id != 0) {
    UnloadShader(maskShader);
    maskShader = {0};
  }
  if (blurRenderTarget.id != 0) {
    UnloadRenderTexture(blurRenderTarget);
    blurRenderTarget = {0};
  }
  if (blurShader.id != 0) {
    UnloadShader(blurShader);
    blurShader = {0};
  }
}

void SharedMechanics::Update() {
  float deltaTime = GetFrameTime();

  // Window resize handling for post-processing
  int currentWidth = GetScreenWidth();
  int currentHeight = GetScreenHeight();
  if (currentWidth != lastScreenWidth || currentHeight != lastScreenHeight) {
    if (currentScreen.id != 0)
      UnloadRenderTexture(currentScreen);
    currentScreen = LoadRenderTexture(currentWidth, currentHeight);

    if (maskRenderTarget.id != 0) {
      UnloadRenderTexture(maskRenderTarget);
      maskRenderTarget = {0};
    }
    if (blurRenderTarget.id != 0) {
      UnloadRenderTexture(blurRenderTarget);
      blurRenderTarget = {0};
    }

    lastScreenWidth = currentWidth;
    lastScreenHeight = currentHeight;
  }

  // Render mode cycle (now handled by Settings, keep F1 as debug shortcut)
  if (IsKeyPressed(KEY_F1))
    renderMode =
        static_cast<RenderMode>((static_cast<int>(renderMode) + 1) % 3);

  androidControls.Update();

  // --- Pause toggle (ESC on desktop, pause button on Android) ---
  if ((IsKeyPressed(KEY_ESCAPE) || androidControls.IsPause()) && !gameOver) {
    isPaused = !isPaused;
  }

  // While paused, update the pause menu and skip all game logic
  if (isPaused) {
    PauseAction action = pauseMenu.Update();
    if (action == PauseAction::RESUME) {
      isPaused = false;
    } else if (action == PauseAction::RETRY) {
      isPaused = false;
      ResetStats();
      resetStage = true;  // signal active stage to call Init()
    } else if (action == PauseAction::QUIT) {
      isPaused = false;
      Fumbo::Instance().ChangeState(std::make_shared<TitleState>());
    }
    return;
  }

  // Game-over: R restarts stage, handled here so stages don't duplicate it
  if (gameOver) {
    if (IsKeyPressed(KEY_R) || btnRetry.IsPressed()) {
      ResetStats();
      resetStage = true;
    } else if (IsKeyPressed(KEY_ESCAPE) || btnExit.IsPressed()) {
      Fumbo::Instance().ChangeState(std::make_shared<TitleState>());
    }
    return;
  }

  // Handle Input - android overlay takes priority on PLATFORM_ANDROID,
  // keyboard always available as fallback on all platforms.
  if (controller && player && !gameOver) {
    if (inputsBlocked) {
      controller->Update(false, false, false);
    } else {
      bool goLeft   = androidControls.IsLeft()  || IsKeyDown(KEY_A)     || IsKeyDown(KEY_LEFT);
      bool goRight  = androidControls.IsRight() || IsKeyDown(KEY_D)     || IsKeyDown(KEY_RIGHT);
      bool doJump   = androidControls.IsJump()  || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W);
      bool doAttack = androidControls.IsAttack()|| IsKeyPressed(KEY_K);

      controller->Update(goLeft, goRight, doJump);

      if (doAttack && attackTimer <= 0.0f) {
        attackTimer = 0.5f;
      }
    }

    if (IsKeyPressed(KEY_F3)) {
      showDebug = !showDebug;
      Fumbo::Graphic2D::Physics::Instance().SetDebugDraw(showDebug);
    }
  } else if (controller && player && gameOver) {
    // Disable inputs and zero velocity when dead
    controller->Update(false, false, false);
    Vector2 vel = player->GetVelocity();
    player->SetVelocity({0, vel.y});
  }

  if (attackTimer > 0) {
    attackTimer -= deltaTime;
  }

  // Tick down invincibility frames
  if (playerInvincibleTimer > 0.0f) {
    playerInvincibleTimer -= deltaTime;
    if (playerInvincibleTimer < 0.0f)
      playerInvincibleTimer = 0.0f;
  }

  // Advance sprite animation
  if (player) {
    float speedX = fabsf(player->GetVelocity().x);
    float speedY = player->GetVelocity().y;
    bool grounded = controller ? controller->IsGrounded() : true;

    // Sticky air state: stay in JUMP state until grounded AND velocity is near
    // zero
    bool inAir = (currentAnim == AnimState::JUMP);
    if (inAir) {
      if (grounded && fabsf(speedY) < 1.0f)
        inAir = false;
    } else {
      if (!grounded || fabsf(speedY) > 5.0f)
        inAir = true;
    }

    bool isMovingLeft = !inputsBlocked && (androidControls.IsLeft() || IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
    bool isMovingRight = !inputsBlocked && (androidControls.IsRight() || IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
    int facing = GetFacingDirection();

    // Check for pushing: holding a move key against a dynamic object
    bool isPushing = false;
    if (grounded &&
        ((isMovingLeft && facing == -1) || (isMovingRight && facing == 1))) {
      Vector2 origin = player->GetPosition();
      Vector2 dir = {(float)facing, 0};

      // Raycast from center outwards, check all hits to ignore player
      auto hits =
          Fumbo::Graphic2D::Physics::Instance().RaycastAll(origin, dir, 35.0f);
      for (const auto &hit : hits) {
        if (hit.hit && hit.object && hit.object != player) {
          if (hit.object->GetBodyType() ==
              Fumbo::Graphic2D::BodyType::Dynamic) {
            isPushing = true;
          }
          // Stop at the first thing we hit that isn't us
          break;
        }
      }
    }

    Texture2D *sheet = &sprIdle;
    int frameCount = 10;
    sprFps = 8.0f;
    AnimState nextAnim = AnimState::IDLE;

    if (gameOver) {
      sheet = &sprDeath;
      frameCount = 10;
      sprFps = 10.0f;
      nextAnim = AnimState::DEATH;
    } else if (attackTimer > 0) {
      sheet = &sprAttack;
      frameCount = 7;
      sprFps = 14.0f;
      nextAnim = AnimState::ATTACK;
    } else if (inAir) {
      sheet = &sprJump;
      frameCount = 6;
      sprFps = 10.0f;
      nextAnim = AnimState::JUMP;
    } else if (isPushing) {
      sheet = &sprPush;
      frameCount = 10;
      sprFps = 8.0f;
      nextAnim = AnimState::PUSH;
    } else if (speedX > 250.0f) {
      sheet = &sprRun;
      frameCount = 8;
      sprFps = 12.0f;
      nextAnim = AnimState::RUN;
    } else if (speedX > 10.0f) {
      sheet = &sprWalk;
      frameCount = 8;
      sprFps = 10.0f;
      nextAnim = AnimState::WALK;
    } else {
      sheet = &sprIdle;
      frameCount = 10;
      sprFps = 8.0f;
      nextAnim = AnimState::IDLE;
    }

    // Reset animation if the state changed
    if (nextAnim != currentAnim) {
      sprFrame = 0;
      sprTimer = 0.0f;
      currentAnim = nextAnim;
    }

    sprTimer += deltaTime;
    if (sprTimer >= 1.0f / sprFps) {
      sprTimer -= 1.0f / sprFps;

      if (currentAnim == AnimState::JUMP || currentAnim == AnimState::ATTACK ||
          currentAnim == AnimState::DEATH) {
        // Stop at last frame for jump/attack/death
        if (sprFrame < frameCount - 1)
          sprFrame++;
        else
          sprFrame = frameCount - 1;
      } else {
        // Loop normally for others
        sprFrame = (sprFrame + 1) % frameCount;
      }
    }

    currentSheet = *sheet;
    float fw = sheet->width / (float)frameCount;
    float fh = (float)sheet->height;

    currentSource = {fw * (sprFrame % frameCount), 0, (facing == -1) ? -fw : fw,
                     fh};
  }
}

Vector2 SharedMechanics::GetCurrentOffset() const {
  Vector2 offset = baseOffset;
  if (currentAnim == AnimState::ATTACK) {
    int facing = GetFacingDirection();
    offset.x -= facing;
  }
  return offset;
}

void SharedMechanics::DrawPlayerWorld() {
  if (!player)
    return;

  // Draw player sprite in world space
  Fumbo::Utils::DrawWorldSprite(player, currentSheet, currentSource,
                                GetCurrentOffset(), spriteScale, WHITE);

  // Draw player-specific debug info
  if (showDebug) {
    int facing = GetFacingDirection();
    Vector2 playerPos = player->GetPosition();
    Fumbo::Graphic2D::DrawLineEx({playerPos.x, playerPos.y},
                                 {playerPos.x + facing * 30.0f, playerPos.y},
                                 3.0f, BLUE);

    if (IsAttacking()) {
      Rectangle hitBox = GetAttackHitbox();
      Fumbo::Graphic2D::DrawRectangleRec(hitBox, {255, 0, 0, 150});
    }
  }
}

Rectangle SharedMechanics::GetAttackHitbox() const {
  if (!player || !controller)
    return {0, 0, 0, 0};
  int facing = GetFacingDirection();
  Vector2 playerPos = player->GetPosition();
  return {playerPos.x + (facing == 1 ? 20 : -100), playerPos.y - 40, 80, 60};
}

void SharedMechanics::HurtPlayer(int dmg) {
  if (playerInvincibleTimer > 0.0f)
    return; // still invincible
  playerHealth -= dmg;
  playerInvincibleTimer = 0.6f; // 0.6 s of i-frames
  if (playerHealth <= 0) {
    playerHealth = 0;
    gameOver = true;
  }
}

void SharedMechanics::DrawDirty() {

  // Debug overlay
  if (player && showDebug) {
    Vector2 vel = player->GetVelocity();
    Vector2 pos = player->GetPosition();
    Fumbo::Graphic2D::DrawText(TextFormat("vel (%.0f, %.0f)", vel.x, vel.y),
                               {1050, 15}, {}, 18, UI::TEXT_DIM);
    Fumbo::Graphic2D::DrawText(TextFormat("pos (%.0f, %.0f)", pos.x, pos.y),
                               {1050, 38}, {}, 18, UI::TEXT_DIM);
  }
  if (showDebug)
    Fumbo::Instance().DrawFPS(1050, 60);

  // Minimal HUD - top-left
  // Light cream semi-transparent backing strip
  Fumbo::Graphic2D::DrawRectangle(0, 0, 260, 60, {255, 242, 200, 210});
  // Left yellow accent bar
  Fumbo::Graphic2D::DrawRectangle(0, 0, (int)UI::ACCENT_H, 60, UI::AB_YELLOW);

  // Player name (dark text)
  Fumbo::Graphic2D::DrawText("NAO", {14, 8}, fontDefault, 22, UI::TEXT);

  // Health pips (12x12 squares, gap 5px)
  constexpr float PIP = 12.0f, GAP = 5.0f;
  bool flicker = (playerInvincibleTimer > 0.0f) &&
                 (((int)(playerInvincibleTimer * 10)) % 2 == 0);
  for (int i = 0; i < playerMaxHealth; i++) {
    float px = 14.0f + i * (PIP + GAP);
    Color filled = flicker ? Fade(UI::AB_YELLOW, 0.3f) : UI::AB_YELLOW;
    Color empty = {210, 185, 120, 180};
    Color c = (i < playerHealth) ? filled : empty;
    Fumbo::Graphic2D::DrawRectangle((int)px, 36, (int)PIP, (int)PIP, c);
    if (i >= playerHealth)
      Fumbo::Graphic2D::DrawRectangleLines((int)px, 36, (int)PIP, (int)PIP,
                                           UI::LINE);
  }

  // Orange bottom accent on HUD strip
  Fumbo::Graphic2D::DrawRectangle(0, 58, 260, 2, UI::AB_ORANGE);

  // Pause menu overlay
  if (isPaused) {
    pauseMenu.Draw();
  }

  // Android on-screen controls (stub does nothing on desktop)
  if (!isPaused && !gameOver) {
    androidControls.Draw();
  }

  // Game-over overlay
  if (gameOver) {
    Fumbo::Graphic2D::DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.7f));
    Fumbo::Graphic2D::DrawText("GAME OVER!", {440, 260}, {}, 60, RED);
#ifdef PLATFORM_ANDROID
    btnRetry.Draw();
    btnExit.Draw();
#else
    Fumbo::Graphic2D::DrawText("Press R to Retry", {500, 380}, {}, 25, WHITE);
    Fumbo::Graphic2D::DrawText("Press ESC to exit", {500, 415}, {}, 20, LIGHTGRAY);
#endif
  }
}

void SharedMechanics::BeginSceneRender() {
  BeginTextureMode(currentScreen);
  ClearBackground(BLANK);
}

void SharedMechanics::EndSceneRenderAndPostProcess() {
  EndTextureMode();

  // --- Post-processing composite ---
  if (renderMode == RenderMode::NORMAL) {
    DrawTexturePro(currentScreen.texture,
                   {0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0.0f, WHITE);

  } else if (renderMode == RenderMode::MASK) {
    Texture2D mask = Fumbo::Shaders::DrawMask(currentScreen, maskRenderTarget,
                                              maskShader, WHITE, BLACK);
    DrawTexturePro(mask, {0, 0, (float)mask.width, -(float)mask.height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0.0f, WHITE);

  } else if (renderMode == RenderMode::GODRAYS) {
    Texture2D backlight = Fumbo::Shaders::DrawMask(
        currentScreen, maskRenderTarget, maskShader, BLANK, BLACK);

    Texture2D mask =
        Fumbo::Shaders::DrawMask(currentScreen, maskRenderTarget, maskShader,
                                 {255, 255, 255, 120}, BLACK);
    Texture2D godRays = Fumbo::Shaders::ApplyRadialBlur(
        mask, blurRenderTarget, blurShader, 0.5f, {100, 600});

    DrawTexturePro(currentScreen.texture,
                   {0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0.0f, WHITE);

    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(godRays, {0, 0, (float)godRays.width, (float)godRays.height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0.0f, WHITE);
    EndBlendMode();
    DrawTexturePro(backlight,
                   {0, 0, (float)backlight.width, -(float)backlight.height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0.0f, {255, 255, 255, 50});
  }
}
