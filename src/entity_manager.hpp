#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cassert>

#include "entity.hpp"

using EntityVec = std::vector<std::shared_ptr<Entity>>;
using EntityMap = std::unordered_map<std::string, EntityVec>;

class EntityManager
{

public:
    EntityManager() noexcept = default;
    EntityManager(const EntityManager &) noexcept = default;
    EntityManager &operator=(const EntityManager &) noexcept = default;
    EntityManager(EntityManager &&) noexcept = default;
    EntityManager &operator=(EntityManager &&) noexcept = default;

    [[nodiscard]] std::shared_ptr<Entity> add_entity(const std::string &tag) noexcept;
    [[nodiscard]] EntityVec &get_entities() noexcept;
    [[nodiscard]] EntityVec &get_entities(const std::string &tag) noexcept;
    void update() noexcept;

private:
    EntityVec m_entities;
    EntityVec m_entities_to_add;
    EntityMap m_entity_map;
    size_t m_total_entities = 0;

    void remove_dead_entities(EntityVec &vec) noexcept;
};
