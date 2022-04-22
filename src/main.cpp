// File: main.cpp
// Author: Ryan Sie
// Description: main method for typechart-studio

#include "editor.hpp"

bool initSDL() {
    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

int main(int agrc, char * argv[]) {
    initSDL();
    
    Editor editor;

    while(editor.isRunning()) {
        editor.loop();
    }

    editor.quit();

    return 0;
}