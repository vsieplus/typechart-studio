#ifndef MENUBAR_HPP
#define MENUBAR_HPP

class ImFont;
class SDL_Renderer;

// all the imgui menubar functions
void showMenuBar(ImFont * menuFont, SDL_Renderer * renderer, AudioSystem * audioSystem);
void showFileMenu(SDL_Renderer * renderer, AudioSystem * audioSystem);
void showEditMenu();
void showOptionMenu();

#endif // MENUBAR_HPP