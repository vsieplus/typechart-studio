#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "imgui.h"

#include "ui/editwindowmanager.hpp"
#include "systems/audiosystem.hpp"

#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class Editor {
    public:
        Editor(SDL_Window * window, SDL_Renderer * renderer);
        
        void loop();
        void quit();

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
        void initAudio();

        SDL_Window * window{ nullptr };
        SDL_Renderer * renderer{ nullptr };

        ImVec4 clearColor { 0.0784f, 0.0784f, 0.0784f, 1.00f };

        ImFont * menuFont { nullptr };

        bool running { true };

        // key states
        std::vector<bool> keysPressed;
        std::vector<bool> keysHeld;

        std::unordered_map<SDL_Keycode, bool> shortcutsActivated {
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

        EditWindowManager editWindowManager{};
        AudioSystem audioSystem{};
};

#endif // EDITOR_HPP
