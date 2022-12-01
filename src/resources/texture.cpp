#include "resources/texture.hpp"

#include <string>
#include <SDL2/SDL_image.h>

namespace Texture {
    std::shared_ptr<SDL_Texture> loadTexture(std::string path, SDL_Renderer * renderer) {
        SDL_Surface * surface = IMG_Load(path.c_str());
        if(!surface) {
            return nullptr;
        }

        // Create SDL texture from the surface
        std::shared_ptr<SDL_Texture> newTexture (SDL_CreateTextureFromSurface(renderer, surface), SDL_DestroyTexture);
            
        if(!newTexture.get()) {
            return nullptr;
        }

        SDL_FreeSurface(surface);

        return newTexture;
    }
}
