#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

#include <SDL2/SDL.h>

namespace Texture {
    std::shared_ptr<SDL_Texture> loadTexture(fs::path path, SDL_Renderer * renderer);
};


#endif // TEXTURE_HPP
