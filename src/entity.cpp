#include "entity.hpp"

Entity::Entity(const std::string &tag, const size_t &id) noexcept : m_tag(tag), m_id(id)
{
}

[[nodiscard]] size_t Entity::id() const noexcept
{
    return m_id;
}

[[nodiscard]] bool Entity::is_alive() const noexcept
{
    return m_alive;
}

[[nodiscard]] const std::string &Entity::tag() const noexcept
{
    return m_tag;
}

void Entity::destroy() noexcept
{
    m_alive = false;
}