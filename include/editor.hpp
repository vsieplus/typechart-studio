#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "versionconfig.h"
#include "imgui.h"

#include <stdio.h>
#include <string>
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
};

#endif // EDITOR_HPP