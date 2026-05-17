#include "raylib.h"
#include <memory>
#include <string>

struct WeaponBlueprint {
  int id;
  int type;
  float baseDamage;
  float attackDurration;
  float cooldown;
  Vector2 hbsize;
  Texture2D sprite = {};
};

class Weapon {
public:
  Weapon(WeaponBlueprint data) : weaponStats(data) {}

  void Update(float deltaTime) {
    if (weaponStats.cooldown > 0)
      weaponStats.cooldown -= deltaTime;
    if (weaponStats.cooldown <= 0) {
      weaponStats.cooldown = 0;
      active = false;
    }
  }

  bool Attack() {
    if (weaponStats.cooldown <= 0) {
      weaponStats.cooldown = cooldownDuration;
      return true;
    } else {
      return false;
    }
  }

  float GetDamage() const { return weaponStats.baseDamage; }

private:
  WeaponBlueprint weaponStats;
  float cooldownDuration = weaponStats.cooldown;
  bool active;
};

class Player {
public:
  void EquipWeapon(std::shared_ptr<Weapon> newWeapon) {
    this->activeWeapon = newWeapon;
  }

  Weapon *GetActiveWeapon() { return activeWeapon.get(); }

private:
  std::string name = "Hero";
  std::shared_ptr<Weapon> activeWeapon = nullptr;
};
