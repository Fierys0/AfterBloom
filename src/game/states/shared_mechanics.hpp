#pragma once
#include "android_controls.hpp"
#include "fumbo.hpp"
#include "pause_action.hpp"
#include "pause_menu.hpp"
#include "template/platformer.hpp"

// Represents the shared mechanics logic (player, HUD)
class SharedMechanics : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override {}
  void DrawDirty() override;
  void DrawPlayerWorld(); // New: World-space rendering for player

  // Post-processing handling
  void BeginSceneRender();
  void EndSceneRenderAndPostProcess();

  // Called by individual Areas to inject the player into their Physics world
  void SetupForNewArea(Vector2 spawnPosition);

  Fumbo::Graphic2D::Object *GetPlayer() const { return player; }
  Fumbo::Platformer::PlatformerController *GetController() const {
    return controller;
  }

  // Player sprite data for external drawing
  Texture2D GetCurrentSheet() const { return currentSheet; }
  Rectangle GetCurrentSource() const { return currentSource; }
  Vector2 GetCurrentOffset() const;
  float spriteScale = 2.5f;

  // Player attack logic
  bool IsAttacking() const { return attackTimer > 0.0f; }
  Rectangle GetAttackHitbox() const;
  int GetFacingDirection() const {
    return controller ? controller->GetFacingDirection() : 1;
  }

  // Input state for NPCs/Interactables
  bool IsInteractPressed() const { return androidControls.IsInteract(); }

  void SetGameOver(bool over) { gameOver = over; }
  bool IsGameOver() const { return gameOver; }

  void SetInputsBlocked(bool blocked) { inputsBlocked = blocked; }
  bool AreInputsBlocked() const { return inputsBlocked; }

  void SetDebugEnabled(bool enabled) { showDebug = enabled; }
  bool IsDebugEnabled() const { return showDebug; }

  enum class RenderMode { NORMAL, MASK, GODRAYS };
  RenderMode renderMode = RenderMode::NORMAL;
  void SetRenderMode(RenderMode mode) { renderMode = mode; }

  // Player health
  void HurtPlayer(int dmg);
  bool IsInvincible() const { return playerInvincibleTimer > 0.0f; }
  int GetPlayerHealth() const { return playerHealth; }
  int GetPlayerMaxHealth() const { return playerMaxHealth; }

  void ResetStats() {
    gameOver = false;
    playerHealth = playerMaxHealth;
    playerInvincibleTimer = 0.0f;
  }

  // Pause / flow helpers for stages
  // True when the stage should skip its own Update logic this frame.
  bool ShouldSkipUpdate() const { return isPaused || gameOver; }
  bool IsPaused() const { return isPaused; }
  // True (once) when SharedMechanics has processed a RETRY action.
  // The stage should call its own Init() and then consume this flag.
  bool ConsumeResetStage() {
    if (!resetStage)
      return false;
    resetStage = false;
    return true;
  }

private:
  Fumbo::Graphic2D::Object *player = nullptr;
  Fumbo::Platformer::PlatformerController *controller = nullptr;

  bool isPaused = false;
  bool resetStage = false;    // set by RETRY; consumed by stage
  bool inputsBlocked = false; // set by RETRY; consumed by stage
  PauseMenu pauseMenu;
  AndroidControls androidControls; // stub on desktop, active on Android

  Fumbo::UI::Button btnRetry;
  Fumbo::UI::Button btnExit;

  float attackTimer = 0.0f;
  bool showDebug = false;
  bool gameOver = false;

  // Player health
  int playerHealth = 10;
  int playerMaxHealth = 10;
  float playerInvincibleTimer = 0.0f; // seconds of i-frames after a hit

  // Player sprite animation
  Texture2D sprIdle = {};
  Texture2D sprWalk = {};
  Texture2D sprRun = {};
  Texture2D sprJump = {};
  Texture2D sprPush = {};
  Texture2D sprAttack = {};
  Texture2D sprPotrait = {};
  Texture2D sprDeath = {};
  int sprFrame = 0;
  float sprTimer = 0.0f;
  float sprFps = 10.0f; // default; overridden per-state

  Texture2D currentSheet;
  Rectangle currentSource;
  Vector2 baseOffset = {0, -20};

  enum class AnimState { IDLE, WALK, RUN, JUMP, PUSH, ATTACK, DEATH };
  AnimState currentAnim = AnimState::IDLE;

  // Post-processing and Render Mode resources
  RenderTexture2D maskRenderTarget = {0};
  Shader maskShader = {0};
  RenderTexture2D blurRenderTarget = {0};
  Shader blurShader = {0};
  RenderTexture2D currentScreen = {0};

  int lastScreenWidth = 0;
  int lastScreenHeight = 0;
};
