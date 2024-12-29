#pragma once
#include <algorithm>

class PortalET {
public:
  float cooldown;
  float currentCooldown;
  bool active;

  PortalET(float cd = 5.0f)
      : cooldown(cd), currentCooldown(0.0f), active(true) {}

  bool CanUse() const { return active && currentCooldown <= 0.0f; }
  void Activate() {
    if (CanUse()) {
      currentCooldown = cooldown;
    }
  }
  void Update(float deltaTime) {
    if (currentCooldown > 0) {
      currentCooldown = std::max(0.0f, currentCooldown - deltaTime);
    }
  }
};
