#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "versionconfig.h"
#include "imgui.h"

#include <stdio.h>
#include <string>

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

        void quit();

        SDL_Window * window;
        SDL_Renderer * renderer;

        Uint64 lastTime;
        Uint64 currTime;

        ImVec4 clearColor = ImVec4(0.267f, 0.294f, 0.38f, 1.00f);

        bool running = true;
};

#endif // EDITOR_HPP