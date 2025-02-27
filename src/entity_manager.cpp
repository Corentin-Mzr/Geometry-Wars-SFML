#include "entity_manager.hpp"

[[nodiscard]] std::shared_ptr<Entity> EntityManager::add_entity(const std::string &tag) noexcept
{
    const auto e = std::shared_ptr<Entity>(new Entity(tag, m_total_entities++));
    m_entities_to_add.push_back(e);
    return e;
}

[[nodiscard]] EntityVec &EntityManager::get_entities() noexcept
{
    return m_entities;
}

[[nodiscard]] EntityVec &EntityManager::get_entities(const std::string &tag)
{
    static EntityVec empty;
    if (m_entity_map.find(tag) != m_entity_map.end())
        return m_entity_map.at(tag);
    return empty;
}

void EntityManager::update() noexcept
{
    // Add entities from the queue in the main containers
    for (const auto e : m_entities_to_add)
    {
        m_entities.push_back(e);
        m_entity_map[e->tag()].push_back(e);
    }
    m_entities_to_add.clear();

    // Remove dead entities from m_entities
    remove_dead_entities(m_entities);

    // Iterate over entity map and remove dead entities from each vector
    for (auto &[tag, vec] : m_entity_map)
    {
        remove_dead_entities(vec);
    }
}

void EntityManager::remove_dead_entities(EntityVec &vec) noexcept
{
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [](const std::shared_ptr<Entity> &e)
                             { return !e->is_alive(); }),
              vec.end());
}
