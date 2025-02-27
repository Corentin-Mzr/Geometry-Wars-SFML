#pragma once

#include <SFML/Graphics.hpp>

struct Component
{
    bool exists = false;
};

struct CTransform : public Component
{
    sf::Vector2f pos = {0.0f, 0.0f};
    sf::Vector2f velocity = {0.0f, 0.0f};
    float angle = 0.0f;

    CTransform() noexcept = default;
    CTransform(const sf::Vector2f &p, const sf::Vector2f &v, float a) noexcept : pos(p),
                                                                                 velocity(v),
                                                                                 angle(a)
    {
    }
};

struct CLifeSpan : public Component
{
    int lifespan = 0;
    int remaining = 0;

    CLifeSpan() noexcept = default;
    CLifeSpan(int total_lifespan) : lifespan(total_lifespan), remaining(total_lifespan)
    {
        assert(lifespan >= 0 && remaining >= 0);
    }
};

struct CInput : public Component
{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
    bool ability = false;

    CInput() noexcept = default;
};

struct CCollision : public Component
{
    float radius = 0.0f;

    CCollision() noexcept = default;
    CCollision(float r) : radius(r)
    {
        assert(radius >= 0.0f);
    }
};

struct CScore : public Component
{
    int score = 0;
    CScore() noexcept = default;
    CScore(int s) noexcept : score(s)
    {
    }
};

struct CShape : public Component
{
    sf::CircleShape circle;

    CShape() noexcept = default;
    CShape(float radius, size_t points, const sf::Color &color) noexcept : circle(radius)
    {
        circle.setPointCount(points);
        circle.setFillColor(color);
        circle.setOrigin({radius, radius});
    }
};