#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>

struct Particle {
  Vector3 position;
  bool active;
  Color color;

  Particle(Vector3 pos, Color c) : position(pos), active(true), color(c) {}

  virtual void Update(float deltaTime) = 0;
  virtual void Draw() = 0;
  virtual ~Particle() = default;
};

class ParticleSystem {
private:
  std::vector<std::unique_ptr<Particle>> particles;

public:
  void Update(float deltaTime) {
    for (auto &particle : particles) {
      particle->Update(deltaTime);
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const auto &p) { return !p->active; }),
                    particles.end());
  }

  void Draw() {
    for (auto &particle : particles) {
      particle->Draw();
    }
  }

  template <typename T, typename... Args> void AddParticle(Args &&...args) {
    particles.push_back(std::make_unique<T>(std::forward<Args>(args)...));
  }
};
