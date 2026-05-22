#pragma once
#include "fumbo.hpp"
#include "template/platformer.hpp"
#include <functional>

// ---------------------------------------------------------------
// SlimeEnemy - a reusable slime entity with its own AI state machine.
//
// Usage (in any area state):
//   slime.Spawn({800, 600});      // creates physics object + registers health
//   slime.Update(deltaTime, playerPos, sharedState);
//   slime.DrawWorld();            // call inside BeginMode2D
//   slime.Destroy();              // cleanup before physics.Clear()
// ---------------------------------------------------------------
class SlimeEnemy {
public:
  // --- Configuration (set before Spawn) ---
  int   maxHealth       = 30;    // total HP
  int   damagePerHit    = 10;    // damage dealt to player per contact
  float patrolSpeed     = 70.0f; // px/s while patrolling
  float chaseSpeed      = 120.0f;
  float jumpSpeedH      = 160.0f; // horizontal component of jump
  float jumpSpeedV      = -280.0f;// vertical component (negative = upward)
  float chaseRange      = 300.0f; // pixels before chasing
  float jumpRange       = 70.0f;  // pixels before jump-attacking
  float cooldownTime    = 1.5f;   // seconds after jump before next action
  float stunTime        = 0.5f;   // seconds of stun after being hit

  // Optional callback: fired when the slime dies
  std::function<void()> onDeath;

  // --- Lifecycle ---
  void Spawn(Vector2 position);
  void Destroy();
  bool IsAlive() const { return physicsObj != nullptr; }

  // --- Per-frame ---
  // playerPos: world position of the player
  // sharedState: used to call HurtPlayer on contact
  void Update(float deltaTime, Vector2 playerPos,
              Fumbo::Graphic2D::Object *playerObj,
              class SharedMechanics *shared);

  // Call inside BeginMode2D (world-space)
  void DrawWorld(bool showDebug = false) const;

  // --- Combat ---
  void TakeHit(int dmg, float knockbackX); // called by Stage when player attacks
  bool WasJustKilled() const { return justKilled; } // true for one frame after death

  Fumbo::Graphic2D::Object *GetObject() const { return physicsObj; }
  int GetHealth() const { return health; }

private:
  Fumbo::Graphic2D::Object *physicsObj = nullptr;
  int   health = 30;
  bool  justKilled = false;

  enum class State { PATROL, CHASE, JUMP_ATTACK, COOLDOWN };
  State state = State::PATROL;

  float stateTimer      = 0.0f;
  float patrolDir       = 1.0f;
  float patrolFlipTimer = 2.0f;

  // Contact damage cooldown (mirrors player i-frames so it fires once per jump)
  float contactCooldown = 0.0f;

  // --- Sprite animation ---
  Texture2D sprIdle = {};
  Texture2D sprWalk = {};
  Texture2D sprRun  = {};
  Texture2D sprJump = {};

  int   animFrame     = 0;
  float animTimer     = 0.0f;
  bool  facingRight   = true;  // flips sprite horizontally
};
