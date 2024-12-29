#pragma once
#include "Particle.hpp"

class AttackParticle : public Particle {
private:
  Vector3 target;
  float progress;
  float speed;

public:
  AttackParticle(Vector3 start, Vector3 end, Color c, float spd = 4.0f)
      : Particle(start, c), target(end), progress(0.0f), speed(spd) {}

  void Update(float deltaTime) override {
    if (!active)
      return;

    progress += deltaTime * speed;

    if (progress >= 1.0f) {
      active = false;
      return;
    }

    position = Vector3Lerp(position, target, progress);
  }

  void Draw() override {
    if (!active)
      return;
    DrawSphere(position, 0.2f, color);
  }
};
