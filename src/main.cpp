#include <iostream>
#include "application/builder.hpp"

using namespace game;

int main(int argc, char *argv[])
{   
    std::ios_base::sync_with_stdio(false);
    try {
        auto app = applicationBuilder().build();
		app.run();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}