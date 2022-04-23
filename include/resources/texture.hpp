#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <memory>
#include <SDL2/SDL.h>

namespace Texture {
    std::shared_ptr<SDL_Texture> loadTexture(std::string path, SDL_Renderer * renderer, SDL_Window * window);
};


#endif // TEXTURE_HPP