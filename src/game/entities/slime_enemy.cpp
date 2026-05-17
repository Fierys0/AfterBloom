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
  physicsObj->SetColor({80, 200, 80, 255});
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
}

void SlimeEnemy::Destroy() {
  if (physicsObj) {
    Fumbo::Graphic2D::Physics::Instance().RemoveObject(physicsObj);
    delete physicsObj;
    physicsObj = nullptr;
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

  // HP bar background (red) then green fill
  int barW = 40, barH = 5;
  int barX = (int)(pos.x - barW / 2);
  int barY = (int)(pos.y - 30);
  Fumbo::Graphic2D::DrawRectangle(barX, barY, barW, barH, RED);
  Fumbo::Graphic2D::DrawRectangle(barX, barY, (int)(((float)hp / (float)maxHealth) * barW), barH,
                GREEN);

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
    Fumbo::Graphic2D::DrawText(label, {pos.x - 15, pos.y - 48}, {}, 12, YELLOW);
  }
}
