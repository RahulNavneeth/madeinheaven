#pragma once
#include "EntityComponent.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

using EntityId = std::size_t;

class Scene {
private:
  EntityId nextEntityId = 0;
  std::unordered_map<EntityId,
                     std::unordered_map<std::type_index, std::shared_ptr<void>>>
      components;

public:
  std::vector<EntityId> entities;
  EntityId NewEntity() {
    EntityId id = nextEntityId++;
    entities.push_back(id);
    return id;
  }

  template <typename T> T *GetComponent(EntityId entity) {
    auto entityIt = components.find(entity);
    if (entityIt == components.end())
      return nullptr;

    auto componentIt = entityIt->second.find(std::type_index(typeid(T)));
    if (componentIt == entityIt->second.end())
      return nullptr;

    return static_cast<T *>(componentIt->second.get());
  }

  template <typename T, typename... Args>
  void AssignEntity(EntityId entity, Args &&...args) {
    auto component = std::make_shared<T>(std::forward<Args>(args)...);
    components[entity][std::type_index(typeid(T))] = component;
  }

  template <typename T> std::vector<EntityId> GetEntitiesWithComponent() {
    std::vector<EntityId> result;
    for (const auto &entity : entities) {
      if (GetComponent<T>(entity) != nullptr) {
        result.push_back(entity);
      }
    }
    return result;
  }

  void RemoveEntity(EntityId entity) {
    components.erase(entity);
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end()) {
      entities.erase(it);
    }
  }

  const std::vector<EntityId> &GetAllEntities() const { return entities; }
};
