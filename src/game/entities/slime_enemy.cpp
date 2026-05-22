#include "slime_enemy.hpp"
#include "../states/shared_mechanics.hpp"
#include <cmath>
#include <cstdlib>

void SlimeEnemy::Spawn(Vector2 position) {
  Destroy();

  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  physicsObj = new Fumbo::Graphic2D::Object();
  physicsObj->SetPosition(position);
  physicsObj->SetRectangle(36, 28);
  physicsObj->SetColor({0, 0, 0, 0}); // transparent — sprite drawn separately
  physicsObj->SetMass(2.0f);
  physicsObj->SetGravityScale(1.0f);
  physicsObj->SetRestitution(0.1f);
  physics.AddObject(physicsObj);

  health = maxHealth;
  state = State::PATROL;
  stateTimer = 0.0f;
  patrolDir = 1.0f;
  patrolFlipTimer = 2.0f;
  contactCooldown = 0.0f;
  justKilled = false;

  // Load sprite sheets
  sprIdle = Fumbo::Assets::LoadTexture("assets/sprite/Enemy/Slime/Idle.png");
  sprWalk = Fumbo::Assets::LoadTexture("assets/sprite/Enemy/Slime/Walk.png");
  sprRun = Fumbo::Assets::LoadTexture("assets/sprite/Enemy/Slime/Run.png");
  sprJump = Fumbo::Assets::LoadTexture("assets/sprite/Enemy/Slime/Jump.png");

  animFrame = 0;
  animTimer = 0.0f;
  facingRight = true;
}

void SlimeEnemy::Destroy() {
  if (physicsObj) {
    Fumbo::Graphic2D::Physics::Instance().RemoveObject(physicsObj);
    delete physicsObj;
    physicsObj = nullptr;
  }

  // Unload sprite sheets if loaded
  if (sprIdle.id != 0) {
    UnloadTexture(sprIdle);
    sprIdle = {};
  }
  if (sprWalk.id != 0) {
    UnloadTexture(sprWalk);
    sprWalk = {};
  }
  if (sprRun.id != 0) {
    UnloadTexture(sprRun);
    sprRun = {};
  }
  if (sprJump.id != 0) {
    UnloadTexture(sprJump);
    sprJump = {};
  }
}

void SlimeEnemy::Update(float deltaTime, Vector2 playerPos,
                        Fumbo::Graphic2D::Object *playerObj,
                        SharedMechanics *shared) {
  justKilled = false;

  if (!physicsObj)
    return;

  // Tick contact cooldown
  if (contactCooldown > 0.0f)
    contactCooldown -= deltaTime;

  Vector2 pos = physicsObj->GetPosition();
  Vector2 vel = physicsObj->GetVelocity();
  bool grounded = fabsf(vel.y) < 60.0f;

  float dx = playerPos.x - pos.x;
  float dist = fabsf(dx);
  float dirToPlayer = (dx >= 0.0f) ? 1.0f : -1.0f;

  // ---- State machine ----
  switch (state) {

  case State::PATROL: {
    patrolFlipTimer -= deltaTime;
    if (patrolFlipTimer <= 0.0f) {
      patrolDir = -patrolDir;
      patrolFlipTimer = 2.0f + (float)(rand() % 100) / 50.0f;
    }
    if (grounded)
      physicsObj->SetVelocity({patrolDir * patrolSpeed, vel.y});

    if (dist < chaseRange)
      state = State::CHASE;
    break;
  }

  case State::CHASE: {
    if (grounded)
      physicsObj->SetVelocity({dirToPlayer * chaseSpeed, vel.y});

    if (dist < jumpRange && grounded)
      state = State::JUMP_ATTACK;

    if (dist > chaseRange + 50.0f) // hysteresis band
      state = State::PATROL;
    break;
  }

  case State::JUMP_ATTACK: {
    physicsObj->SetVelocity({dirToPlayer * jumpSpeedH, jumpSpeedV});
    state = State::COOLDOWN;
    stateTimer = cooldownTime;
    break;
  }

  case State::COOLDOWN: {
    stateTimer -= deltaTime;
    if (stateTimer <= 0.0f)
      state = (dist < chaseRange) ? State::CHASE : State::PATROL;
    break;
  }
  }
  if (playerObj && physicsObj->IsCollidingWith(playerObj) &&
      contactCooldown <= 0.0f) {
    Vector2 playerP = playerObj->GetPosition();

    if (shared)
      shared->HurtPlayer(1);

    // Push player away from slime
    float pushDir = (playerP.x >= pos.x) ? 1.0f : -1.0f;
    playerObj->SetVelocity({pushDir * 250.0f, -200.0f});

    contactCooldown = 0.5f;
  }

  // Re-fetch velocity for animation (state machine may have changed it)
  vel = physicsObj->GetVelocity();
  float speed = fabsf(vel.x);

  // Update facing direction based on horizontal velocity
  if (vel.x > 5.0f)
    facingRight = true;
  else if (vel.x < -5.0f)
    facingRight = false;

  // Pick animation sheet based on state
  Texture2D *sheet = &sprIdle;
  int totalFrames = 4;
  float fps = 8.0f;

  switch (state) {
  case State::JUMP_ATTACK:
    sheet = &sprJump;
    totalFrames = (sprJump.id != 0 && sprJump.height > 0)
                      ? sprJump.width / sprJump.height
                      : 4;
    fps = 10.0f;
    break;
  case State::CHASE:
    sheet = &sprRun;
    totalFrames = (sprRun.id != 0 && sprRun.height > 0)
                      ? sprRun.width / sprRun.height
                      : 4;
    fps = 12.0f;
    break;
  case State::PATROL:
    if (speed > 10.0f) {
      sheet = &sprWalk;
      totalFrames = (sprWalk.id != 0 && sprWalk.height > 0)
                        ? sprWalk.width / sprWalk.height
                        : 4;
      fps = 8.0f;
    } else {
      sheet = &sprIdle;
      totalFrames = (sprIdle.id != 0 && sprIdle.height > 0)
                        ? sprIdle.width / sprIdle.height
                        : 4;
      fps = 6.0f;
    }
    break;
  default:
    sheet = &sprIdle;
    totalFrames = (sprIdle.id != 0 && sprIdle.height > 0)
                      ? sprIdle.width / sprIdle.height
                      : 4;
    fps = 6.0f;
    break;
  }

  // Advance animation frame
  if (totalFrames > 0) {
    animTimer += deltaTime;
    if (animTimer >= 1.0f / fps) {
      animTimer -= 1.0f / fps;
      animFrame = (animFrame + 1) % totalFrames;
    }
  }

  // ---- Death check ----
  if (health <= 0 && physicsObj) {
    justKilled = true;
    Destroy();
    if (onDeath)
      onDeath();
  }
}

void SlimeEnemy::TakeHit(int dmg, float knockbackX) {
  if (!physicsObj)
    return;

  health -= dmg;

  if (health <= 0) {
    health = 0;
    // Actual destruction happens at end of Update, or caller can check
    // IsAlive()
    return;
  }

  // Knock back + stun
  Vector2 vel = physicsObj->GetVelocity();
  physicsObj->SetVelocity({knockbackX, -200.0f});
  state = State::COOLDOWN;
  stateTimer = stunTime;
}

void SlimeEnemy::DrawWorld(bool showDebug) const {
  if (!physicsObj)
    return;

  Vector2 pos = physicsObj->GetPosition();
  int hp = health;

  // --- Draw animated sprite ---
  // Frame is square (128×128). totalFrames = sheet_width / sheet_height.
  const Texture2D *sheet = nullptr;
  int totalFrames = 4;

  switch (state) {
  case State::JUMP_ATTACK:
    if (sprJump.id != 0) {
      sheet = &sprJump;
      totalFrames = sprJump.width / sprJump.height;
    }
    break;
  case State::CHASE:
    if (sprRun.id != 0) {
      sheet = &sprRun;
      totalFrames = sprRun.width / sprRun.height;
    }
    break;
  case State::PATROL:
    if (fabsf(physicsObj->GetVelocity().x) > 10.0f && sprWalk.id != 0) {
      sheet = &sprWalk;
      totalFrames = sprWalk.width / sprWalk.height;
    } else if (sprIdle.id != 0) {
      sheet = &sprIdle;
      totalFrames = sprIdle.width / sprIdle.height;
    }
    break;
  default:
    if (sprIdle.id != 0) {
      sheet = &sprIdle;
      totalFrames = sprIdle.width / sprIdle.height;
    }
    break;
  }

  if (sheet && sheet->id != 0 && totalFrames > 0) {
    int frame = animFrame % totalFrames;
    // Frame is square: width == height of the sheet
    int frameW = sheet->height;
    int frameH = sheet->height;

    float srcX = (float)(frame * frameW);
    // Flip by negating source width
    float srcW = facingRight ? (float)frameW : -(float)frameW;
    if (!facingRight)
      srcX += (float)frameW; // shift start for flipped frame

    Rectangle source = {srcX, 0, srcW, (float)frameH};
    const float scale = 1;
    Fumbo::Utils::DrawWorldSprite(physicsObj, *sheet, source, {0, -48}, scale);
  }

  // HP bar background (red) then green fill
  int barW = 40, barH = 5;
  int barX = (int)(pos.x - barW / 2);
  int barY = (int)(pos.y - 42);
  Fumbo::Graphic2D::DrawRectangle(barX, barY, barW, barH, RED);
  Fumbo::Graphic2D::DrawRectangle(
      barX, barY, (int)(((float)hp / (float)maxHealth) * barW), barH, GREEN);

  // Debug state label
  if (showDebug) {
    const char *label = "PATROL";
    switch (state) {
    case State::CHASE:
      label = "CHASE";
      break;
    case State::JUMP_ATTACK:
      label = "JUMP";
      break;
    case State::COOLDOWN:
      label = "COOL";
      break;
    default:
      break;
    }
    Fumbo::Graphic2D::DrawText(label, {pos.x - 15, pos.y - 58}, {}, 12, YELLOW);
  }
}
