#include "game.hpp"

// Randomizer
std::random_device Game::m_rd;
std::mt19937 Game::m_gen(Game::m_rd());

Game::Game(const std::string &config_filepath) : m_score_text(m_font), m_pause_text(m_font), m_cooldown_text(m_font)
{
    ConfigParser parser(config_filepath);
    m_window_config = parser.get_window_config();
    m_player_config = parser.get_player_config();
    m_enemy_config = parser.get_enemy_config();
    m_bullet_config = parser.get_bullet_config();
    m_score_config = parser.get_score_config();
    m_ability_config = parser.get_ability_config();

    init();
}

void Game::run() noexcept
{
    while (m_running)
    {
        m_entities.update();

        while (const std::optional event = m_window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                m_running = false;
            }

            system_user_input(event);
        }

        if (!m_paused)
        {
            system_enemy_spawner();
            system_movement();
            system_collision();
            system_lifespan();
            system_ability();
        }

        system_render();
    }

    m_window.close();
}

void Game::init() noexcept
{
    // Window config
    const sf::Vector2u sizes{m_window_config.width, m_window_config.height};
    const sf::Vector2f sizes_f = static_cast<sf::Vector2f>(sizes);
    m_window.create(sf::VideoMode(sizes), m_window_config.title);
    m_window.setMinimumSize(sizes);
    m_window.setMaximumSize(sizes);
    m_window.setFramerateLimit(m_window_config.framerate);
    m_window.setView(sf::View{{0.0f, 0.0f}, sizes_f});

    // Score config
    assert(m_font.openFromFile(m_score_config.font));
    m_score_text.setFont(m_font);
    m_score_text.setCharacterSize(m_score_config.size);
    m_score_text.setFillColor(array_to_color(m_score_config.color));
    m_score_text.setString(get_score_as_str());
    m_score_text.setPosition(-0.5f * sizes_f + sf::Vector2f{10.0f, 10.0f});

    // Pause text config
    m_pause_text.setFont(m_font);
    m_pause_text.setStyle(sf::Text::Bold);
    m_pause_text.setCharacterSize(64);
    m_pause_text.setFillColor(sf::Color::Red);
    m_pause_text.setString("PAUSE");
    m_pause_text.setOrigin(0.5f * m_pause_text.getLocalBounds().size);

    // Ability config
    m_cooldown_text.setFont(m_font);
    m_cooldown_text.setCharacterSize(m_score_config.size);
    m_cooldown_text.setFillColor(array_to_color(m_score_config.color));
    m_cooldown_text.setString(get_ability_as_str());
    const sf::Vector2f bounds = m_cooldown_text.getLocalBounds().size;
    const sf::Vector2f position = {0.5f * sizes_f.x - bounds.x - 10.0f, -0.5f * sizes_f.y + 10.0f};
    m_cooldown_text.setPosition(position);

    // Main loop config
    spawn_player();
}

void Game::system_movement() noexcept
{
    /* Player */
    static const float speed = m_player_config.speed;
    auto player = get_player();
    assert(player->has<CInput>() && player->has<CTransform>());
    auto &input = player->get<CInput>();
    auto &player_transform = player->get<CTransform>();

    if (input.up)
        player_transform.velocity.y -= speed;
    if (input.down)
        player_transform.velocity.y += speed;
    if (input.left)
        player_transform.velocity.x -= speed;
    if (input.right)
        player_transform.velocity.x += speed;
    if (input.shoot)
    {
        spawn_bullet(player_transform.pos);
        if (!m_using_ability)
            input.shoot = false;
    }

    // Normalize
    if (player_transform.velocity.lengthSquared() != 0.0f)
        player_transform.velocity = speed * player_transform.velocity.normalized();

    /* Update entities based on velocity */
    for (auto entity : m_entities.get_entities())
    {
        if (!entity->has<CTransform>())
            continue;

        auto &transform = entity->get<CTransform>();
        transform.pos += transform.velocity;
    }

    /* Resets player speed */
    player_transform.velocity = {0.0f, 0.0f};
}

void Game::system_lifespan() noexcept
{
    for (auto entity : m_entities.get_entities())
    {
        if (!entity->has<CLifeSpan>())
            continue;

        auto &lifespan = entity->get<CLifeSpan>();
        lifespan.remaining--;
        if (lifespan.remaining <= 0)
            entity->destroy();
    }
}

void Game::system_user_input(const std::optional<sf::Event> &event) noexcept
{
    auto player = get_player();
    assert(player->has<CInput>() && player->has<CTransform>());
    auto &input = player->get<CInput>();
    const auto &transform = player->get<CTransform>();

    /* KEY PRESSED */
    if (const auto *key_pressed = event->getIf<sf::Event::KeyPressed>())
    {
        handle_key_pressed(key_pressed, input);
    }

    /* KEY RELEASED */
    if (const auto *key_released = event->getIf<sf::Event::KeyReleased>())
    {
        handle_key_released(key_released, input);
    }

    /* MOUSE PRESSED */
    if (const auto *mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>())
    {
        handle_mouse_button_pressed(mouse_pressed, input);
    }

    /* MOUSE RELEASED */
    if (const auto *mouse_released = event->getIf<sf::Event::MouseButtonReleased>())
    {
        handle_mouse_button_released(mouse_released, input);
    }
}

void Game::system_enemy_spawner() noexcept
{
    static const unsigned spawn_rate = m_enemy_config.spawn_rate;
    static unsigned frame_count = 0;

    if (frame_count == spawn_rate)
    {
        spawn_enemy();
        frame_count = 0;
    }
    else
    {
        frame_count++;
    }
}

void Game::system_collision() noexcept
{
    static const sf::View view = m_window.getView();
    static const sf::Vector2f center = view.getCenter();
    static const sf::Vector2f size = view.getSize();

    static const float xmin = center.x - 0.5f * size.x;
    static const float xmax = center.x + 0.5f * size.x;
    static const float ymin = center.y - 0.5f * size.y;
    static const float ymax = center.y + 0.5f * size.y;

    /* Wall collision */
    for (const auto e : m_entities.get_entities())
    {
        if (e->has<CCollision>() && e->has<CTransform>())
        {
            const auto &collision = e->get<CCollision>();
            auto &transform = e->get<CTransform>();

            // Left
            if (transform.pos.x - collision.radius < xmin)
            {
                transform.pos.x = xmin + collision.radius;
                transform.velocity.x *= -1;
            }

            // Right
            if (transform.pos.x + collision.radius > xmax)
            {
                transform.pos.x = xmax - collision.radius;
                transform.velocity.x *= -1;
            }

            // Up
            if (transform.pos.y - collision.radius < ymin)
            {
                transform.pos.y = ymin + collision.radius;
                transform.velocity.y *= -1;
            }

            // Down
            if (transform.pos.y + collision.radius > ymax)
            {
                transform.pos.y = ymax - collision.radius;
                transform.velocity.y *= -1;
            }
        }
    }

    /* Collision between bullets and enemies */
    auto &bullets = m_entities.get_entities("bullet");
    auto &enemies = m_entities.get_entities("enemy");

    for (auto bullet: bullets)
    {
        const auto &bullet_pos = bullet->get<CTransform>().pos;
        const auto &bullet_size = bullet->get<CCollision>().radius;

        for (auto enemy: enemies)
        {
            const auto &enemy_pos = enemy->get<CTransform>().pos;
            const auto &enemy_size = enemy->get<CCollision>().radius;

            /* Compute distance between bullet and enemy */
            const float distance_sq = (bullet_pos - enemy_pos).lengthSquared();
            const float radius_sum_sq = (bullet_size + enemy_size) * (bullet_size + enemy_size);

            /* Bullet and enemy collide */
            if (distance_sq <= radius_sum_sq)
            {
                bullet->destroy();
                enemy->destroy();

                /* Spawn children */
                if (!enemy->has<CLifeSpan>())
                    spawn_small_enemies(enemy);

                m_score += enemy->get<CScore>().score;
                m_highscore = std::fmax(m_highscore, m_score);

                break;
            }
        }
    }

    /* Collision between player and enemies */
    auto player = get_player();
    assert(player->has<CTransform>() && player->has<CCollision>());
    const auto &player_pos = player->get<CTransform>().pos;
    const auto &player_size = player->get<CCollision>().radius;

    for (auto it_enemy = enemies.begin(); it_enemy != enemies.end();)
    {
        auto &enemy = *it_enemy;
        if (!enemy->has<CTransform>() || !enemy->has<CCollision>())
            continue;

        const auto &enemy_pos = enemy->get<CTransform>().pos;
        const auto &enemy_size = enemy->get<CCollision>().radius;

        /* Compute distance between player and enemy */
        const float distance_sq = (player_pos - enemy_pos).lengthSquared();
        const float radius_sum_sq = (player_size + enemy_size) * (player_size + enemy_size);

        /* Player and enemy collide */
        if (distance_sq <= radius_sum_sq)
        {
            player->destroy();
            enemy->destroy();

            /* Spawn new player */
            spawn_player();
            break;
        }
        else
            it_enemy++;
    }
}

void Game::system_render() noexcept
{
    static const sf::Color bg_color = array_to_color(m_window_config.color);
    m_window.clear(bg_color);

    for (const auto e : m_entities.get_entities())
    {
        if (e->has<CShape>() && e->has<CTransform>())
        {
            auto &shape = e->get<CShape>().circle;
            const auto &transform = e->get<CTransform>();
            shape.setPosition(transform.pos);
            shape.setRotation(shape.getRotation() + sf::degrees(transform.angle));

            /* Bullets */
            if (e->has<CLifeSpan>())
            {
                auto &lifespan = e->get<CLifeSpan>();
                auto color = shape.getFillColor();
                color.a = 255 * static_cast<float>(lifespan.remaining) / static_cast<float>(lifespan.lifespan);
                shape.setFillColor(color);
            }

            m_window.draw(shape);
        }
    }

    /* Draw score */
    m_score_text.setString(get_score_as_str());
    m_window.draw(m_score_text);

    /* Draw ability cooldown */
    m_cooldown_text.setString(get_ability_as_str());
    m_window.draw(m_cooldown_text);

    /* Draw pause if pausing */
    if (m_paused)
        m_window.draw(m_pause_text);

    m_window.display();
}

void Game::system_ability() noexcept
{
    /*
    Berserk mode
    When right click is pressed:
    - Unlimited shoot for X frames
    - Player becomes red, fading into normal color
    - Cooldown for Y frames
    */

    static sf::Color ability_color = array_to_color(m_ability_config.color);
    static sf::Color player_color = array_to_color(m_player_config.color);

    /* Ability is in cooldown */
    if (m_cooldown_remaining > 0 && !m_using_ability)
    {
        m_cooldown_remaining--;
    }

    /* Check if player is not already using ability */
    auto player = get_player();
    assert(player->has<CShape>() && player->has<CInput>());
    auto &input = player->get<CInput>();
    auto &shape = player->get<CShape>().circle;
    if (input.ability && !m_using_ability && m_cooldown_remaining == 0)
    {
        m_using_ability = true;
        m_cooldown_remaining = m_ability_config.cooldown;
    }

    /* Ability in use */
    if (m_using_ability)
    {
        m_duration_remaining--;
        const float t = static_cast<float>(m_duration_remaining) / static_cast<float>(m_ability_config.duration);
        const uint8_t red = static_cast<uint8_t>(ability_color.r * t + player_color.r * (1.0f - t));
        const uint8_t green = static_cast<uint8_t>(ability_color.g * t + player_color.g * (1.0f - t));
        const uint8_t blue = static_cast<uint8_t>(ability_color.b * t + player_color.b * (1.0f - t));
        shape.setFillColor({red, green, blue, player_color.a});
    }

    /* Check the time remaining to use the ability */
    if (m_duration_remaining == 0)
    {
        m_using_ability = false;
        m_duration_remaining = m_ability_config.duration;
        shape.setFillColor(player_color);
    }
}

void Game::spawn_player() noexcept
{
    /* Player creation */
    auto player = m_entities.add_entity("player");
    player->add<CShape>(m_player_config.size, m_player_config.sides, array_to_color(m_player_config.color));
    player->add<CCollision>(m_player_config.size);
    player->add<CTransform>(sf::Vector2f{0.0f, 0.0f}, sf::Vector2f{0.0f, 0.0f}, m_player_config.rotation);
    player->add<CInput>();

    /* Resets data for score and ability */
    m_score = 0;
    m_cooldown_remaining = 0;
    m_duration_remaining = m_ability_config.duration;
    m_using_ability = false;
}

void Game::spawn_enemy() noexcept
{
    static const sf::View view = m_window.getView();
    static const sf::Vector2f center = view.getCenter();
    static const sf::Vector2f size = view.getSize();

    static const float xmin = center.x - 0.5f * size.x;
    static const float xmax = center.x + 0.5f * size.x;
    static const float ymin = center.y - 0.5f * size.y;
    static const float ymax = center.y + 0.5f * size.y;

    const auto player = get_player();
    assert(player->has<CTransform>() && player->has<CCollision>());
    const sf::Vector2f player_pos = player->get<CTransform>().pos;
    const float player_radius = player->get<CCollision>().radius;

    /* Random position */
    static std::uniform_real_distribution<float> x(xmin, xmax);
    static std::uniform_real_distribution<float> y(ymin, ymax);
    sf::Vector2f pos = {x(m_gen), y(m_gen)};

    /* Check if not overlapping player */
    while (float dist_to_player = (pos - player_pos).lengthSquared() < 12.0f * player_radius * player_radius)
    {
        pos = {x(m_gen), y(m_gen)};
    }

    /* Random velocity */
    static std::uniform_real_distribution<float> vx(m_enemy_config.speed_min, m_enemy_config.speed_max);
    static std::uniform_real_distribution<float> vy(m_enemy_config.speed_min, m_enemy_config.speed_max);
    const sf::Vector2f vel = sf::Vector2f{vx(m_gen), vy(m_gen)}.normalized();

    /* Random color */
    static std::uniform_int_distribution<uint8_t> r(0, 255);
    static std::uniform_int_distribution<uint8_t> g(0, 255);
    static std::uniform_int_distribution<uint8_t> b(0, 255);
    const sf::Color color = {r(m_gen), g(m_gen), b(m_gen)};

    /* Random number of sides */
    static std::uniform_int_distribution<unsigned> sides(m_enemy_config.sides_min, m_enemy_config.sides_max);
    const unsigned n = sides(m_gen);

    /* Enemy creation */
    auto enemy = m_entities.add_entity("enemy");
    enemy->add<CShape>(m_enemy_config.size, n, color);
    enemy->add<CCollision>(m_enemy_config.size);
    enemy->add<CTransform>(pos, vel, m_enemy_config.rotation);
    enemy->add<CScore>(100.0f * n);
}

void Game::spawn_small_enemies(const std::shared_ptr<Entity> enemy) noexcept
{
    assert(enemy->has<CShape>() && enemy->has<CTransform>());
    const auto &parent_shape = enemy->get<CShape>().circle;
    const float &parent_velocity = enemy->get<CTransform>().velocity.length();

    /* Children data */
    const size_t n = parent_shape.getPointCount();
    const sf::Vector2f position = parent_shape.getPosition();
    static const float size = m_enemy_config.child_size;
    static const float lifespan = m_enemy_config.child_lifespan;

    /* Create N small enemies */
    for (size_t i = 0; i < n; ++i)
    {
        /* Compute velocity */
        const float angle = (i * 360.0 / n) * M_PI / 180.0f;
        const sf::Vector2f velocity = sf::Vector2f{cosf(angle), sinf(angle)} * parent_velocity;

        auto enemy = m_entities.add_entity("enemy");
        enemy->add<CShape>(size, n, parent_shape.getFillColor());
        enemy->add<CCollision>(size);
        enemy->add<CTransform>(position, velocity, m_enemy_config.rotation);
        enemy->add<CLifeSpan>(lifespan);
        enemy->add<CScore>(200.0f * n);
    }
}

void Game::spawn_bullet(const sf::Vector2f &player_position) noexcept
{
    /* Bullet data */
    const sf::Vector2f bullet_direction = (m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)) - player_position).normalized();
    const sf::Vector2f bullet_velocity = m_bullet_config.speed * bullet_direction;

    /* Bullet creation */
    auto bullet = m_entities.add_entity("bullet");
    bullet->add<CShape>(m_bullet_config.radius, 36, array_to_color(m_bullet_config.color));
    bullet->add<CCollision>(m_bullet_config.radius);
    bullet->add<CTransform>(player_position, bullet_velocity, 0.0f);
    bullet->add<CLifeSpan>(m_bullet_config.lifespan);
}

void Game::handle_key_pressed(const sf::Event::KeyPressed *key_pressed, CInput &input) noexcept
{
    // Up
    if (key_pressed->scancode == sf::Keyboard::Scancode::W)
        input.up = true;

    // Left
    if (key_pressed->scancode == sf::Keyboard::Scancode::A)
        input.left = true;

    // Down
    if (key_pressed->scancode == sf::Keyboard::Scancode::S)
        input.down = true;

    // Right
    if (key_pressed->scancode == sf::Keyboard::Scancode::D)
        input.right = true;

    // Pause - Unpause
    if (key_pressed->scancode == sf::Keyboard::Scancode::P)
        m_paused = !m_paused;

    // Quit
    if (key_pressed->scancode == sf::Keyboard::Scancode::Escape)
        m_running = false;
}

void Game::handle_key_released(const sf::Event::KeyReleased *key_released, CInput &input) noexcept
{
    // Up
    if (key_released->scancode == sf::Keyboard::Scancode::W)
        input.up = false;

    // Left
    if (key_released->scancode == sf::Keyboard::Scancode::A)
        input.left = false;

    // Down
    if (key_released->scancode == sf::Keyboard::Scancode::S)
        input.down = false;

    // Right
    if (key_released->scancode == sf::Keyboard::Scancode::D)
        input.right = false;
}

void Game::handle_mouse_button_pressed(const sf::Event::MouseButtonPressed *mouse_pressed, CInput &input) noexcept
{
    if (mouse_pressed->button == sf::Mouse::Button::Left)
        input.shoot = true;
    if (mouse_pressed->button == sf::Mouse::Button::Right)
        input.ability = true;
}

void Game::handle_mouse_button_released(const sf::Event::MouseButtonReleased *mouse_released, CInput &input) noexcept
{
    if (mouse_released->button == sf::Mouse::Button::Left)
        input.shoot = false;
    if (mouse_released->button == sf::Mouse::Button::Right)
        input.ability = false;
}

std::shared_ptr<Entity> Game::get_player() noexcept
{
    auto &players = m_entities.get_entities("player");
    assert(players.size() == 1);
    return players.front();
}

std::string Game::get_score_as_str() const noexcept
{
    return "Score: " + std::to_string(m_score) + "\nHighscore: " + std::to_string(m_highscore);
}

std::string Game::get_ability_as_str() const noexcept
{
    if (m_using_ability)
    {
        return "Ability\nIn Use";
    }

    if (m_cooldown_remaining > 0)
    {
        float t = static_cast<float>(m_cooldown_remaining) / static_cast<float>(m_window_config.framerate);
        return "Ability\nIn " + float_to_string(t, 1) + "s";
    }
    return "Ability\nOK";
}
