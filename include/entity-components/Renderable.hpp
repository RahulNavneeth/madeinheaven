#pragma once
#include <raylib.h>

enum class EntityType {
    REACTOR,
    WALL,
    ATTACKER,
    DEFENDER,
    PORTAL
};

class RenderableET {
public:
    Color color;
    EntityType type;
    float size;
    float height;

    RenderableET(Color col = BLUE, EntityType t = EntityType::WALL, 
                 float s = 2.0f, float h = 2.0f)
        : color(col), type(t), size(s), height(h) {}
};
