#define TOML_IMPLEMENTATION
#include "config_parser.hpp"

ConfigParser::ConfigParser(const std::string &config_filepath) : m_filepath(config_filepath)
{
    const auto data = toml::parse(m_filepath);
    m_window_config = parse_window(data);
    m_player_config = parse_player(data);
    m_enemy_config = parse_enemy(data);
    m_bullet_config = parse_bullet(data);
    m_score_config = parse_score(data);
    m_ability_config = parse_ability(data);
}

const WindowConfig &ConfigParser::get_window_config() const noexcept
{
    return m_window_config;
}

const PlayerConfig &ConfigParser::get_player_config() const noexcept
{
    return m_player_config;
}

const EnemyConfig &ConfigParser::get_enemy_config() const noexcept
{
    return m_enemy_config;
}

const BulletConfig &ConfigParser::get_bullet_config() const noexcept
{
    return m_bullet_config;
}

const ScoreConfig &ConfigParser::get_score_config() const noexcept
{
    return m_score_config;
}

const AbilityConfig &ConfigParser::get_ability_config() const noexcept
{
    return m_ability_config;
}

template <typename T>
[[nodiscard]] T ConfigParser::parse_section(const toml::value &data, const std::string &section_name)
{
    assert(data.contains(section_name));
    return toml::find<T>(data, section_name);
}

[[nodiscard]] WindowConfig ConfigParser::parse_window(const toml::value &data)
{
    return parse_section<WindowConfig>(data, "window");
}

[[nodiscard]] PlayerConfig ConfigParser::parse_player(const toml::value &data)
{
    return parse_section<PlayerConfig>(data, "player");
}

[[nodiscard]] EnemyConfig ConfigParser::parse_enemy(const toml::value &data)
{
    return parse_section<EnemyConfig>(data, "enemy");
}

[[nodiscard]] BulletConfig ConfigParser::parse_bullet(const toml::value &data)
{
    return parse_section<BulletConfig>(data, "bullet");
}

[[nodiscard]] ScoreConfig ConfigParser::parse_score(const toml::value &data)
{
    return parse_section<ScoreConfig>(data, "score");
}

[[nodiscard]] AbilityConfig ConfigParser::parse_ability(const toml::value &data)
{
    return parse_section<AbilityConfig>(data, "ability");
}
