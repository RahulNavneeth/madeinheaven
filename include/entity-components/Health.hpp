#pragma once
#include <algorithm>

class HealthET {
public:
  float maxHealth;
  float currentHealth;

  HealthET(float health = 100.0f) : maxHealth(health), currentHealth(health) {}

  bool IsAlive() const { return currentHealth > 0; }
  void TakeDamage(float damage) {
    currentHealth = std::max(0.0f, currentHealth - damage);
  }
  void Heal(float amount) {
    currentHealth = std::min(maxHealth, currentHealth + amount);
  }
  float GetHealthPercentage() const {
    return (currentHealth / maxHealth) * 100.0f;
  }
};
