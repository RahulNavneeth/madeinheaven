#pragma once
#include "raylib.h"

enum class TerrainType { GRASS, WATER, STONE, DIRT, SAND };

struct TileET {
  TerrainType type;
  float height;
  bool isHighlighted;

  TileET(TerrainType t = TerrainType::GRASS, float h = 0.0f, bool hgl = 0)
      : type(t), height(h), isHighlighted(hgl) {}
};

unsigned char ClampColor(int value) {
  if (value > 255)
    return 255;
  if (value < 0)
    return 0;
  return (unsigned char)value;
}

Color GetTerrainColor(TerrainType type, float height,
                      bool isHighlighted = false) {
  Color baseColor;
  switch (type) {
  case TerrainType::GRASS:
    baseColor = Color{76, ClampColor(187 + (int)(height * 30)), 23, 255};
    break;
  case TerrainType::WATER:
    baseColor = Color{28, 113, ClampColor(216 + (int)(height * 20)), 255};
    break;
  case TerrainType::STONE: {
    unsigned char stoneValue = ClampColor(145 + (int)(height * 20));
    baseColor = Color{stoneValue, stoneValue, stoneValue, 255};
    break;
  }
  case TerrainType::DIRT:
    baseColor = Color{ClampColor(161 + (int)(height * 20)),
                      ClampColor(116 + (int)(height * 20)), 56, 255};
    break;
  case TerrainType::SAND:
    baseColor = Color{ClampColor(238 + (int)(height * 10)),
                      ClampColor(213 + (int)(height * 10)), 183, 255};
    break;
  default:
    baseColor = GREEN;
    break;
  }

  if (isHighlighted) {
    return Color{ClampColor(baseColor.r + 50), ClampColor(baseColor.g + 50),
                 ClampColor(baseColor.b + 50), 255};
  }

  return baseColor;
}
