#ifndef INIT_HPP
#define INIT_HPP

#include <string>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "ui/windowsizes.hpp"

namespace init {

const std::string PROGRAM_NAME = "Typechart Studio";

bool initSDL();
bool initSDLImage();
SDL_Window * initWindow();
SDL_Renderer * initRenderer(SDL_Window * window);
void initImGUI(SDL_Window * window, SDL_Renderer * renderer);

};

#endif // INIT_HPP
