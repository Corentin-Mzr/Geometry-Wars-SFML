#pragma once

#include <string>
#include <array>

#include <SFML/Graphics.hpp>

#include "toml.hpp"

struct WindowConfig
{
    unsigned width = 0;
    unsigned height = 0;
    std::string title = "";
    unsigned framerate = 0;
    std::array<uint8_t, 4> color = {0, 0, 0, 255};
};
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(WindowConfig, width, height, title, framerate, color);

struct PlayerConfig
{
    unsigned sides = 0;
    float size = 0.0f;
    float speed = 0.0f;
    float rotation = 0.0f;
    std::array<uint8_t, 4> color = {0, 0, 0, 255};
};
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(PlayerConfig, sides, size, speed, rotation, color);

struct EnemyConfig
{
    unsigned sides_min = 0;
    unsigned sides_max = 0;
    float speed_min = 0.0f;
    float speed_max = 0.0f;
    float rotation = 0.0f;
    float size = 0.0f;
    float child_size = 0.0f;
    unsigned child_lifespan = 0;
    unsigned spawn_rate = 0;
};
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(EnemyConfig, sides_min, sides_max, speed_min, speed_max, rotation, size, child_size, child_lifespan, spawn_rate)

struct BulletConfig
{
    float speed = 0.0f;
    float radius = 0.0f;
    unsigned lifespan = 0;
    std::array<uint8_t, 4> color = {0, 0, 0, 255};
};
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(BulletConfig, speed, radius, lifespan, color)

struct ScoreConfig
{
    std::string font = "";
    unsigned size = 0;
    std::array<uint8_t, 4> color = {0, 0, 0, 255};
};
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(ScoreConfig, font, size, color)

struct AbilityConfig
{
    int duration = 0;
    int cooldown = 0;
    std::array<uint8_t, 4> color = {0, 0, 0, 255};
};
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(AbilityConfig, duration, cooldown, color)
