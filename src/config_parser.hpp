#pragma once

#include <string>
#include <cassert>

#include "toml.hpp"
#include "config_structs.hpp"

class ConfigParser
{
public:
    ConfigParser(const std::string &config_filepath);

    const WindowConfig &get_window_config() const noexcept;
    const PlayerConfig &get_player_config() const noexcept;
    const EnemyConfig &get_enemy_config() const noexcept;
    const BulletConfig &get_bullet_config() const noexcept;
    const ScoreConfig &get_score_config() const noexcept;
    const AbilityConfig &get_ability_config() const noexcept;

private:
    std::string m_filepath;

    WindowConfig m_window_config;
    PlayerConfig m_player_config;
    EnemyConfig m_enemy_config;
    BulletConfig m_bullet_config;
    ScoreConfig m_score_config;
    AbilityConfig m_ability_config;

    template <typename T>
    [[nodiscard]] static T parse_section(const toml::value &data, const std::string &section_name);

    [[nodiscard]] static WindowConfig parse_window(const toml::value &data);
    [[nodiscard]] static PlayerConfig parse_player(const toml::value &data);
    [[nodiscard]] static EnemyConfig parse_enemy(const toml::value &data);
    [[nodiscard]] static BulletConfig parse_bullet(const toml::value &data);
    [[nodiscard]] static ScoreConfig parse_score(const toml::value &data);
    [[nodiscard]] static AbilityConfig parse_ability(const toml::value &data);
};
