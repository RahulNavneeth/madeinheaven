#pragma once
#include <raylib.h>

class TransformET {
public:
  Vector3 position;
  Vector3 rotation;
  Vector3 scale;

  TransformET(Vector3 pos = {0, 0, 0}, Vector3 rot = {0, 0, 0},
              Vector3 scl = {1, 1, 1})
      : position(pos), rotation(rot), scale(scl) {}
};
