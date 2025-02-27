#pragma once

#include <random>
#include <SFML/Graphics.hpp>

#include "entity_manager.hpp"
#include "config_parser.hpp"
#include "misc.hpp"

class Game
{
public:
    Game(const std::string &config_filepath);
    void run() noexcept;
    
private:
    sf::RenderWindow m_window;
    EntityManager m_entities;

    /* Config */
    WindowConfig m_window_config;
    PlayerConfig m_player_config;
    EnemyConfig m_enemy_config;
    BulletConfig m_bullet_config;
    ScoreConfig m_score_config;
    AbilityConfig m_ability_config;

    /* Score */
    sf::Font m_font;
    sf::Text m_score_text;
    int m_score = 0;
    int m_highscore = 0;

    /* Runtime */
    sf::Text m_pause_text;
    bool m_paused = false;
    bool m_running = true;

    /* Ability : berserk mode - unlimited shoot for X frames - player becomes red */
    int m_duration_remaining = 0;
    int m_cooldown_remaining = 0;
    bool m_using_ability = false;
    sf::Text m_cooldown_text; /* Ability : Available - or - Ability : T s (duration remaining + cooldown remaining)*/

    /* Enemy spawn */
    static std::random_device m_rd;
    static std::mt19937 m_gen;

    void init() noexcept;

    /* Systems */
    void system_movement() noexcept;
    void system_user_input(const std::optional<sf::Event> &event) noexcept;
    void system_enemy_spawner() noexcept;
    void system_collision() noexcept;
    void system_lifespan() noexcept;
    void system_render() noexcept;
    void system_ability() noexcept;

    /* Entity creation */
    void spawn_player() noexcept;
    void spawn_enemy() noexcept;
    void spawn_small_enemies(const std::shared_ptr<Entity> enemy) noexcept;
    void spawn_bullet(const sf::Vector2f &player_position) noexcept;


    /* Event handling */
    void handle_key_pressed(const sf::Event::KeyPressed *key_pressed, CInput &input) noexcept;
    void handle_key_released(const sf::Event::KeyReleased *key_released, CInput &input) noexcept;
    void handle_mouse_button_pressed(const sf::Event::MouseButtonPressed *mouse_pressed, CInput &input) noexcept;
    void handle_mouse_button_released(const sf::Event::MouseButtonReleased *mouse_released, CInput &input) noexcept;

    /* Helper funcs */
    std::shared_ptr<Entity> get_player() noexcept;
    std::string get_score_as_str() const noexcept;
    std::string get_ability_as_str() const noexcept;

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) noexcept = delete;
    Game &operator=(Game &&) noexcept = delete;

};