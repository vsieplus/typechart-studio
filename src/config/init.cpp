#include "config/init.hpp"

namespace init {

bool initSDL() {
    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool initSDLImage() {
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        printf("Failed to initialize SDL Image: %s\n", SDL_GetError());

        return false;
    }

    return true;
}

SDL_Window * initWindow() {
    // Setup window
    auto window_flags = (SDL_WindowFlags) (SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    SDL_Window * window = SDL_CreateWindow(PROGRAM_NAME.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        constants::SCREEN_WIDTH, constants::SCREEN_WIDTH, window_flags);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    return window;
}

SDL_Renderer * initRenderer(SDL_Window * window) {
    // setup renderer
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        SDL_Log("Error creating SDL_Renderer!");
    }

    return renderer;
}

void initImGUI(SDL_Window * window, SDL_Renderer * renderer) {
    if(window && renderer) {
        // setup imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // setup platform, rendering backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer_Init(renderer);

        ImGuiIO & io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }
}

};
