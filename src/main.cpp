#include "config/init.hpp"
#include "editor.hpp"

int main(int agrc, char * argv[]) {
    if(!init::initSDL() || !init::initSDLImage()) {
        return 1;
    }

    SDL_Window * window { init::initWindow() };
    SDL_Renderer * renderer { init::initRenderer(window) };

    if(!(window && renderer)) {
        return 1;
    }

    Editor editor{ window, renderer };
    while(editor.isRunning()) {
        editor.loop();
    }
    editor.quit();

    return 0;
}
