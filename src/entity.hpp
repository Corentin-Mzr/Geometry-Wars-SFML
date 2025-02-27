#pragma once

#include <tuple>
#include <string>

#include "components.hpp"

using ComponentTuple = std::tuple<CTransform, CLifeSpan, CInput, CCollision, CScore, CShape>;

class Entity
{
    friend class EntityManager;

public:
    template <typename T, typename... Args>
    void add(Args &&...args);

    template <typename T>
    [[nodiscard]] T &get() noexcept;

    template <typename T>
    [[nodiscard]] const T &get() const noexcept;

    template <typename T>
    [[nodiscard]] bool has() const noexcept;

    template <typename T>
    void remove() const noexcept;

    [[nodiscard]] size_t id() const noexcept;
    [[nodiscard]] bool is_alive() const noexcept;
    [[nodiscard]] const std::string &tag() const noexcept;

    void destroy() noexcept;

private:
    ComponentTuple m_components;
    bool m_alive = true;
    std::string m_tag = "default";
    size_t m_id = 0;

    Entity() noexcept = default;
    Entity(const std::string &tag, const size_t &id) noexcept;
    Entity(const Entity &) = delete;
    Entity &operator=(const Entity &) = delete;
    Entity(Entity &&) noexcept = delete;
    Entity &operator=(Entity &&) noexcept = delete;
};

/* TEMPLATE FUNCTIONS HERE */

template <typename T, typename... Args>
void Entity::add(Args &&...args)
{
    auto &component = get<T>();
    component = T(std::forward<Args>(args)...);
    component.exists = true;
    std::get<T>(m_components) = component;
}

template <typename T>
[[nodiscard]] T &Entity::get() noexcept
{
    return std::get<T>(m_components);
}

template <typename T>
[[nodiscard]] const T &Entity::get() const noexcept
{
    return std::get<T>(m_components);
}

template <typename T>
[[nodiscard]] bool Entity::has() const noexcept
{
    return get<T>().exists;
}

template <typename T>
void Entity::remove() const noexcept
{
    get<T>() = T();
}