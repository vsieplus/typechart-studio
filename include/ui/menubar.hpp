#ifndef MENUBAR_HPP
#define MENUBAR_HPP

class EditWindowManager;

namespace menubar {

// all the imgui menubar functions
void showMenuBar(ImFont * menuFont, SDL_Renderer * renderer, AudioSystem * audioSystem, EditWindowManager & editWindowManager);
std::string showFileMenu(SDL_Renderer * renderer, AudioSystem * audioSystem, EditWindowManager & editWindowManager);
void showEditMenu(EditWindowManager & editWindowManager);
void showOptionMenu();

}

#endif // MENUBAR_HPP
