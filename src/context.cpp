#include <iostream>

#include "context.hpp"

namespace kovra {
Context::Context(SDL_Window *window) : instance{window} {
    std::cout << "Context::Context()" << std::endl;
}
} // namespace kovra
