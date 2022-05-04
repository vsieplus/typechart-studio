#ifndef MENUBAR_HPP
#define MENUBAR_HPP

class ImFont;
class SDL_Renderer;

// all the imgui menubar functions
void showMenuBar(ImFont * menuFont, SDL_Renderer * renderer);
void showFileMenu(SDL_Renderer * renderer);
void showEditMenu();
void showOptionMenu();
void showHelpMenu();

#endif // MENUBAR_HPP