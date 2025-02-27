#include <iostream>

#include "config_parser.hpp"
#include "misc.hpp"
#include "game.hpp"

int main()
{
    Game game("../resources/config.toml");
    game.run();

    return 0;
}