#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "versionconfig.h"
#include "imgui.h"

#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>

class Editor {
    public:
        Editor();
        
        void loop();

        bool isRunning() const;
    private:
        void handleEvents();
        void update();
        void render();

        void handleShortcutEvents();
        void updateShortcuts();

        void setWindowIcon();

        void initFonts();
        void initKeys();

        void quit();

        SDL_Window * window;
        SDL_Renderer * renderer;

        ImVec4 clearColor = ImVec4(0.267f, 0.294f, 0.38f, 1.00f);

        ImFont * menuFont = nullptr;

        bool running = true;

        // key states
        std::vector<bool> keysPressed;
        std::vector<bool> keysHeld;

        std::unordered_map<SDL_Keycode, bool> shortcutsActivated = {
            { SDLK_c, false },
            { SDLK_x, false },
            { SDLK_v, false },
            { SDLK_n, false },
            { SDLK_o, false },
            { SDLK_s, false },
            { SDLK_z, false },
            { SDLK_y, false },
            { SDLK_p, false }
        };
};

#endif // EDITOR_HPP