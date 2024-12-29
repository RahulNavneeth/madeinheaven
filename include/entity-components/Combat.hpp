#pragma once
#include "AttackParticle.hpp"
#include <algorithm>
#include <raylib.h>

class AttackerET {
public:
  float damage;
  float range;
  float attackCooldown;
  float currentCooldown;

  AttackerET(float dmg = 10.0f, float rng = 4.0f, float cd = 1.0f)
      : damage(dmg), range(rng), attackCooldown(cd), currentCooldown(0.0f) {}

  bool CanAttack() const { return currentCooldown <= 0.0f; }

  void Update(float deltaTime) {
    if (currentCooldown > 0) {
      currentCooldown = std::max(0.0f, currentCooldown - deltaTime);
    }
  }

  void Attack() {
    if (CanAttack()) {
      currentCooldown = attackCooldown;
    }
  }
};

class DefenderET {
public:
  float defense;
  float blockChance;

  DefenderET(float def = 5.0f, float block = 0.2f)
      : defense(def), blockChance(block) {}

  float CalculateDamageReduction(float incomingDamage) const {
    if (GetRandomValue(0, 100) < blockChance * 100) {
      return 0.0f;
    }
    return std::max(0.0f, incomingDamage - defense);
  }
};
