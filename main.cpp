#include "ECS.hpp"
#include "entity-components/Transform.hpp"
#include "raylib.h"
#include "raymath.h"
#include <unordered_map>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float GRID_SIZE = 2.0f;
const float ISOMETRIC_ANGLE = 30.0f * DEG2RAD;
const float CAMERA_DISTANCE = 35.0f;

const int gridSize = 17;
const float tileSize = 2.0f;
const float baseHeight = 0.5f;
const int wallWidth = 2;

BoundingBox GetBoundingBox(Vector3 position, float tileSize, float height) {
  Vector3 halfExtents = {tileSize / 2.0f, height / 2.0f, tileSize / 2.0f};
  Vector3 min = {position.x - halfExtents.x, position.y - halfExtents.y,
                 position.z - halfExtents.z};
  Vector3 max = {position.x + halfExtents.x, position.y + halfExtents.y,
                 position.z + halfExtents.z};
  return {min, max};
}

enum class SpawnState {
  NONE,
  SPAWN_ATTACKER,
  SPAWN_WALL,
  SELECTING_PORTAL_START,
  SELECTING_PORTAL_END
};

class Game {
private:
  Scene scene;
  Camera3D camera;
  float cameraAngle;
  std::unordered_map<Player, int> points;
  EntityId player1Reactor;
  EntityId player2Reactor;
  SpawnState currentState = SpawnState::NONE;
  Vector3 portalStartPos;
  std::vector<EntityId> selectedEntities;
  ParticleSystem particleSystem;

public:
  Game() : cameraAngle(-PI / 4) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Made in Heaven");
    SetTargetFPS(60);
    InitializeGrid();
    InitializeCamera();
    InitializeGame();
  }

  ~Game() { CloseWindow(); }

  void InitializeGrid() {
    for (int x = 0; x < gridSize; x++) {
      for (int z = 0; z < gridSize; z++) {
        EntityId entity = scene.NewEntity();
        float height = (z == gridSize / 2) ? 6.0f : 1.0f;
        TerrainType type =
            (z == gridSize / 2) ? TerrainType::DIRT : TerrainType::GRASS;

        int gridX = x - gridSize / 2;
        int gridZ = z - gridSize / 2;

        scene.AssignEntity<TransformET>(
            entity,
            TransformET({static_cast<float>(gridX * tileSize), height / 2.0f,
                         static_cast<float>(gridZ * tileSize)}));
        scene.AssignEntity<TileET>(entity, TileET(type, height));
      }
    }
  }

  EntityId CreateWall(Vector3 position, Player owner) {
    EntityId entity = scene.NewEntity();

    scene.AssignEntity<TransformET>(entity, position);
    scene.AssignEntity<RenderableET>(
        entity, owner == Player::PLAYER1 ? DARKGREEN : DARKPURPLE,
        EntityType::WALL, 2.0f, 2.0f);

    DefenderET defense;
    defense.defense = 5.0f;
    defense.blockChance = 0.3f;
    scene.AssignEntity<DefenderET>(entity, defense);

    scene.AssignEntity<HealthET>(entity, 75.0f);
    scene.AssignEntity<PlayerET>(entity, owner);

    return entity;
  }

  EntityId CreateAttacker(Vector3 position, Player owner) {
    EntityId entity = scene.NewEntity();

    scene.AssignEntity<TransformET>(entity, position);
    scene.AssignEntity<RenderableET>(entity,
                                     owner == Player::PLAYER1 ? BLUE : RED,
                                     EntityType::ATTACKER, 2.0f, 2.0f);

    AttackerET attacker;
    attacker.damage = 10.0f;
    attacker.range = 4.0f * 4;
    attacker.attackCooldown = 1.0f;
    attacker.currentCooldown = 0.0f;
    scene.AssignEntity<AttackerET>(entity, attacker);

    scene.AssignEntity<HealthET>(entity, 50.0f);
    scene.AssignEntity<PlayerET>(entity, owner);

    return entity;
  }

  Vector3 SnapToGrid(const Vector3 &position) {
    int x = round(position.x / tileSize);
    int z = round(position.z / tileSize);
    return {static_cast<float>(x * tileSize), position.y + 1.0f,
            static_cast<float>(z * tileSize)};
  }

  bool IsValidSpawnPosition(const Vector3 &position) {
    float halfGrid = (gridSize * tileSize) / 2.0f;
    return position.x >= -halfGrid && position.x <= halfGrid &&
           position.z >= -halfGrid && position.z <= halfGrid;
  }

  void HandleInput(EntityId hoveredEntity, const Vector3 &hitPosition) {
    if (IsKeyPressed(KEY_ONE))
      currentState = SpawnState::SPAWN_ATTACKER;
    if (IsKeyPressed(KEY_TWO))
      currentState = SpawnState::SELECTING_PORTAL_START;
    if (IsKeyPressed(KEY_THREE))
      currentState = SpawnState::SPAWN_WALL;
    if (IsKeyPressed(KEY_ESCAPE)) {
      currentState = SpawnState::NONE;
      selectedEntities.clear();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoveredEntity != -1) {
      Vector3 spawnPos = SnapToGrid(hitPosition);

      if (!IsValidSpawnPosition(spawnPos))
        return;

      switch (currentState) {
      case SpawnState::SPAWN_ATTACKER:
        if (points[GetCurrentPlayer()] >= 100) {
          CreateAttacker(spawnPos, GetCurrentPlayer());
          points[GetCurrentPlayer()] -= 100;
        }
        break;

      case SpawnState::SELECTING_PORTAL_START:
        if (points[GetCurrentPlayer()] >= 200) {
          portalStartPos = spawnPos;
          selectedEntities = GetEntitiesAtPosition(spawnPos);

          selectedEntities.erase(
              std::remove_if(selectedEntities.begin(), selectedEntities.end(),
                             [this](EntityId entity) {
                               auto playerComp =
                                   scene.GetComponent<PlayerET>(entity);
                               return !playerComp ||
                                      playerComp->player != GetCurrentPlayer();
                             }),
              selectedEntities.end());

          if (!selectedEntities.empty()) {
            currentState = SpawnState::SELECTING_PORTAL_END;
          }
        }
        break;

      case SpawnState::SELECTING_PORTAL_END:
        if (!selectedEntities.empty()) {
          for (auto entity : selectedEntities) {

            if (!scene.GetComponent<TileET>(entity)) {
              TeleportEntity(entity, spawnPos);
            }
          }
          points[GetCurrentPlayer()] -= 200;
          currentState = SpawnState::NONE;
          selectedEntities.clear();
        }
        break;
      case SpawnState::SPAWN_WALL:
        if (points[GetCurrentPlayer()] >= 150) {
          CreateWall(spawnPos, GetCurrentPlayer());
          points[GetCurrentPlayer()] -= 150;
        }
        break;

      default:
        break;
      }
    }
  }

  Player GetCurrentPlayer() {
    return (cameraAngle < 0) ? Player::PLAYER1 : Player::PLAYER2;
  }

  void RenderEntities() {
    for (auto entity : scene.GetAllEntities()) {
      auto transform = scene.GetComponent<TransformET>(entity);
      auto renderable = scene.GetComponent<RenderableET>(entity);
      auto health = scene.GetComponent<HealthET>(entity);

      if (transform && renderable) {
        Color color = renderable->color;

        if (health && health->currentHealth < health->maxHealth) {
          float healthPercent = health->currentHealth / health->maxHealth;
          color.a = static_cast<unsigned char>(255 * healthPercent);
        }

        DrawCube(transform->position, renderable->size, renderable->height,
                 renderable->size, color);
        DrawCubeWires(transform->position, renderable->size, renderable->height,
                      renderable->size, BLACK);

        if (health) {
          Vector3 healthBarPos = transform->position;
          healthBarPos.y += renderable->height;

          float healthPercent = health->currentHealth / health->maxHealth;
          float barWidth = 4.0f;
          float barHeight = 0.5f;
          int numSegments = 10;

          for (int i = 0; i < numSegments; i++) {
            float segmentWidth = barWidth / numSegments;
            DrawCube({healthBarPos.x - barWidth / 2 + i * segmentWidth,
                      healthBarPos.y, healthBarPos.z},
                     segmentWidth, barHeight, 0.1f, RED);
          }

          int filledSegments = static_cast<int>(numSegments * healthPercent);
          for (int i = 0; i < filledSegments; i++) {
            float segmentWidth = barWidth / numSegments;
            DrawCube({healthBarPos.x - barWidth / 2 + i * segmentWidth,
                      healthBarPos.y, healthBarPos.z},
                     segmentWidth, barHeight, 0.1f, GREEN);
          }

          std::string healthText =
              std::to_string(static_cast<int>(health->currentHealth)) + "/" +
              std::to_string(static_cast<int>(health->maxHealth));

          Vector2 screenPos = GetWorldToScreen(healthBarPos, camera);

          if (screenPos.x > 0 && screenPos.y > 0) {
            DrawText(healthText.c_str(), static_cast<int>(screenPos.x) - 20,
                     static_cast<int>(screenPos.y) - 10, 20, BLACK);
          }
        }
      }
    }
  }

  EntityId FindNearestTarget(const Vector3 &position, Player owner) {
    EntityId nearestTarget = -1;
    float minDistance = INFINITY;

    for (auto entity : scene.GetAllEntities()) {
      auto playerComp = scene.GetComponent<PlayerET>(entity);
      auto transform = scene.GetComponent<TransformET>(entity);
      auto health = scene.GetComponent<HealthET>(entity);

      if (playerComp && transform && health && playerComp->player != owner &&
          health->IsAlive() &&
          (scene.GetComponent<AttackerET>(entity) || entity == player1Reactor ||
           entity == player2Reactor)) {

        float distance = Vector3Distance(position, transform->position);
        if (distance < minDistance) {
          minDistance = distance;
          nearestTarget = entity;
        }
      }
    }
    return nearestTarget;
  }

  EntityId FindNearestEnemy(const Vector3 &position, Player owner) {
    EntityId nearest = -1;
    float minDistance = INFINITY;

    for (auto entity : scene.GetAllEntities()) {
      auto playerComp = scene.GetComponent<PlayerET>(entity);
      auto transform = scene.GetComponent<TransformET>(entity);
      auto health = scene.GetComponent<HealthET>(entity);

      if (playerComp && transform && health && playerComp->player != owner &&
          health->IsAlive()) {
        float distance = Vector3Distance(position, transform->position);
        if (distance < minDistance) {
          minDistance = distance;
          nearest = entity;
        }
      }
    }
    return nearest;
  }

  void InitializeCamera() {
    camera.position = {20.0f, 20.0f, 20.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
  }

  void InitializeGame() {
    points[Player::PLAYER1] = 1000;
    points[Player::PLAYER2] = 1000;

    Vector3 player1Pos = SnapToGrid({0.0f, 3.0f, -14.0f});
    Vector3 player2Pos = SnapToGrid({0.0f, 3.0f, 14.0f});

    player1Reactor = CreateReactor(player1Pos, Player::PLAYER1);
    player2Reactor = CreateReactor(player2Pos, Player::PLAYER2);
  }

  EntityId CreateReactor(Vector3 position, Player team) {
    EntityId entity = scene.NewEntity();

    scene.AssignEntity<TransformET>(entity, position);
    scene.AssignEntity<RenderableET>(entity,
                                     team == Player::PLAYER1 ? BLUE : RED,
                                     EntityType::REACTOR, 1.0f, 6.0f);
    scene.AssignEntity<HealthET>(entity, 100.0f);
    scene.AssignEntity<PlayerET>(entity, team);

    return entity;
  }

  EntityId CreatePortal(Vector3 position, Player team) {
    EntityId entity = scene.NewEntity();

    scene.AssignEntity<TransformET>(entity, position);
    scene.AssignEntity<RenderableET>(entity,
                                     team == Player::PLAYER1 ? BLUE : RED,
                                     EntityType::PORTAL, 2.0f, 0.5f);
    scene.AssignEntity<PortalET>(entity);
    scene.AssignEntity<PlayerET>(entity, team);

    return entity;
  }

  void Update() {
    Vector2 mousePosition = GetMousePosition();
    Ray ray = GetMouseRay(mousePosition, camera);

    RayCollision closestCollision = {0};
    closestCollision.distance = INFINITY;
    closestCollision.hit = false;
    EntityId hoveredEntity = -1;
    Vector3 hitPosition = {0};

    for (const EntityId &entity : scene.entities) {
      TransformET *transform = scene.GetComponent<TransformET>(entity);
      TileET *tile = scene.GetComponent<TileET>(entity);

      if (transform && tile) {
        BoundingBox box =
            GetBoundingBox(transform->position, tileSize, tile->height);
        RayCollision collision = GetRayCollisionBox(ray, box);

        if (collision.hit && collision.distance < closestCollision.distance) {
          closestCollision = collision;
          hoveredEntity = entity;
          hitPosition = collision.point;
        }
      }
    }

    HandleInput(hoveredEntity, hitPosition);
    particleSystem.Update(GetFrameTime());
    UpdateCamera();
    UpdateEntities();
    CheckWinCondition();
  }

  void UpdateCamera() {
    if (IsKeyDown(KEY_Q))
      cameraAngle -= 0.02f;
    if (IsKeyDown(KEY_E))
      cameraAngle += 0.02f;

    camera.position = {cosf(cameraAngle) * CAMERA_DISTANCE,
                       CAMERA_DISTANCE * sinf(ISOMETRIC_ANGLE),
                       sinf(cameraAngle) * CAMERA_DISTANCE};
  }

  void UpdateEntities() {
    for (auto entity : scene.GetEntitiesWithComponent<AttackerET>()) {
      auto attacker = scene.GetComponent<AttackerET>(entity);
      if (attacker && attacker->currentCooldown > 0) {
        attacker->currentCooldown -= GetFrameTime();
      }
    }

    particleSystem.Update(GetFrameTime());

    for (auto entity : scene.GetEntitiesWithComponent<AttackerET>()) {
      auto attacker = scene.GetComponent<AttackerET>(entity);
      auto transform = scene.GetComponent<TransformET>(entity);
      auto playerComp = scene.GetComponent<PlayerET>(entity);
      auto health = scene.GetComponent<HealthET>(entity);

      if (!attacker || !transform || !playerComp || !health ||
          !health->IsAlive()) {
        continue;
      }

      EntityId nearestTarget = -1;
      float minDistance = INFINITY;

      for (auto targetEntity : scene.GetAllEntities()) {
        auto targetPlayerComp = scene.GetComponent<PlayerET>(targetEntity);
        auto targetTransform = scene.GetComponent<TransformET>(targetEntity);
        auto targetHealth = scene.GetComponent<HealthET>(targetEntity);

        if (!targetPlayerComp || !targetTransform || !targetHealth ||
            !targetHealth->IsAlive() ||
            targetPlayerComp->player == playerComp->player) {
          continue;
        }

        if (!scene.GetComponent<AttackerET>(targetEntity) &&
            !scene.GetComponent<DefenderET>(targetEntity) &&
            targetEntity != player1Reactor && targetEntity != player2Reactor) {
          continue;
        }

        float distance =
            Vector3Distance(transform->position, targetTransform->position);
        if (distance < minDistance) {
          minDistance = distance;
          nearestTarget = targetEntity;
        }
      }

      if (nearestTarget != -1) {
        auto targetTransform = scene.GetComponent<TransformET>(nearestTarget);
        auto targetHealth = scene.GetComponent<HealthET>(nearestTarget);
        auto targetDefense = scene.GetComponent<DefenderET>(nearestTarget);

        float distance =
            Vector3Distance(transform->position, targetTransform->position);

        if (distance <= attacker->range && attacker->CanAttack()) {
          float finalDamage = attacker->damage;
          if (targetDefense) {
            finalDamage =
                targetDefense->CalculateDamageReduction(attacker->damage);
          }

          targetHealth->TakeDamage(finalDamage);
          attacker->Attack();

          Color particleColor;
          if (playerComp->player == Player::PLAYER1) {
            particleColor = Color{0, 120, 255, 255};
          } else {
            particleColor = Color{255, 60, 60, 255};
          }

          Vector3 startPos = transform->position;
          Vector3 endPos = targetTransform->position;
          startPos.y += 1.0f;
          endPos.y += 1.0f;

          float damageReductionFactor = finalDamage / attacker->damage;
          Color modifiedParticleColor = {
              static_cast<unsigned char>(particleColor.r *
                                         damageReductionFactor),
              static_cast<unsigned char>(particleColor.g *
                                         damageReductionFactor),
              particleColor.b, particleColor.a};

          particleSystem.AddParticle<AttackParticle>(
              startPos, endPos, modifiedParticleColor, 4.0f);

          if (!targetHealth->IsAlive()) {
            points[playerComp->player] += finalDamage > 0 ? 50 : 25;
          }
        }
      }
    }

    for (auto entity : scene.GetEntitiesWithComponent<HealthET>()) {
      auto health = scene.GetComponent<HealthET>(entity);
      if (health && !health->IsAlive()) {
        if (entity != player1Reactor && entity != player2Reactor) {
          scene.RemoveEntity(entity);
        }
      }
    }
  }

  void Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    DrawGrid(20, GRID_SIZE);

    Vector2 mousePosition = GetMousePosition();
    Ray ray = GetMouseRay(mousePosition, camera);

    RayCollision closestCollision = {0};
    closestCollision.distance = INFINITY;
    closestCollision.hit = false;
    EntityId hoveredEntity = -1;

    for (const EntityId &entity : scene.entities) {
      TransformET *transform = scene.GetComponent<TransformET>(entity);
      TileET *tile = scene.GetComponent<TileET>(entity);

      if (transform && tile) {
        BoundingBox box =
            GetBoundingBox(transform->position, tileSize, tile->height);
        RayCollision collision = GetRayCollisionBox(ray, box);

        if (collision.hit && collision.distance < closestCollision.distance &&
            tile->type != TerrainType::DIRT) {
          closestCollision = collision;
          hoveredEntity = entity;
        }
      }
    }

    for (const EntityId &entity : scene.entities) {
      TransformET *transform = scene.GetComponent<TransformET>(entity);
      TileET *tile = scene.GetComponent<TileET>(entity);

      if (transform && tile) {
        BoundingBox box =
            GetBoundingBox(transform->position, tileSize, tile->height);
        bool isHovered = (entity == hoveredEntity);
        Color baseColor = GetTerrainColor(tile->type, tile->height, isHovered);

        DrawCube(transform->position, tileSize, tile->height, tileSize,
                 baseColor);
        DrawCubeWires(transform->position, tileSize, tile->height, tileSize,
                      BLACK);
      }
    }

    RenderEntities();

    particleSystem.Draw();

    if (currentState == SpawnState::SELECTING_PORTAL_END &&
        !selectedEntities.empty()) {
      DrawLine3D(portalStartPos,
                 closestCollision.hit ? closestCollision.point : ray.position,
                 GetCurrentPlayer() == Player::PLAYER1 ? BLUE : RED);
    }

    EndMode3D();
    RenderUI();
    EndDrawing();
  }

  std::vector<EntityId> GetEntitiesAtPosition(const Vector3 &position) {
    std::vector<EntityId> entities;
    for (auto entity : scene.GetAllEntities()) {
      auto transform = scene.GetComponent<TransformET>(entity);
      if (transform) {
        Vector3 entityPos = transform->position;
        if (abs(entityPos.x - position.x) < tileSize / 2.0f &&
            abs(entityPos.z - position.z) < tileSize / 2.0f) {
          entities.push_back(entity);
        }
      }
    }
    return entities;
  }

  void TeleportEntity(EntityId entityId, const Vector3 &destination) {
    auto transform = scene.GetComponent<TransformET>(entityId);
    if (transform) {
      Vector3 newPos = destination;
      newPos.y = transform->position.y;
      transform->position = newPos;
    }
  }

  void RenderUI() {
    DrawText(TextFormat("Player 1 Points: %08i", points[Player::PLAYER1]), 10,
             10, 20, BLUE);
    DrawText(TextFormat("Player 2 Points: %08i", points[Player::PLAYER2]), 10,
             40, 20, RED);

    const char *stateText;
    switch (currentState) {
    case SpawnState::SPAWN_ATTACKER:
      stateText = "Attacker Spawn Mode (Cost: 100) - ESC to cancel";
      break;
    case SpawnState::SELECTING_PORTAL_START:
      stateText = "Select Portal Start Position (Cost: 200) - ESC to cancel";
      break;
    case SpawnState::SELECTING_PORTAL_END:
      stateText = "Select Portal End Position - ESC to cancel";
      break;
    case SpawnState::SPAWN_WALL:
      stateText = "Wall Spawn Mode (Cost: 150) - ESC to cancel";
      break;
    default:
      stateText = "Press 1 for Attacker, 2 for Portal, 3 for Wall";
      break;
    }
    DrawText(stateText, 10, 70, 20, DARKGRAY);
  }

  void CheckWinCondition() {
    auto reactor1 = scene.GetComponent<HealthET>(player1Reactor);
    auto reactor2 = scene.GetComponent<HealthET>(player2Reactor);

    if (reactor1 && reactor2) {
      if (!reactor1->IsAlive()) {
        DrawText("Player 2 Wins!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2,
                 40, RED);

      } else if (!reactor2->IsAlive()) {
        DrawText("Player 1 Wins!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2,
                 40, BLUE);
      }
    }
  }

  void EndGame() {
    while (!IsKeyPressed(KEY_SPACE)) {
      BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawText("Press SPACE to Exit", WINDOW_WIDTH / 2 - 150,
               WINDOW_HEIGHT / 2 + 50, 20, DARKGRAY);
      EndDrawing();
    }
    CloseWindow();
    exit(0);
  }
};

int main() {
  Game game;
  while (!WindowShouldClose()) {
    game.Update();
    game.Render();
  }

  return 0;
}
