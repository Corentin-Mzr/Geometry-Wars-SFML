#include <iostream>

#include "config_parser.hpp"
#include "misc.hpp"
#include "game.hpp"

int main()
{
    try
    {
        Game game("../resources/config.toml");
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}