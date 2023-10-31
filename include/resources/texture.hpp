#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <memory>
#include <filesystem>

#include <SDL2/SDL.h>

namespace fs = std::filesystem;

namespace Texture {
    std::shared_ptr<SDL_Texture> loadTexture(const fs::path & path, SDL_Renderer * renderer);
};


#endif // TEXTURE_HPP
