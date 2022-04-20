#ifndef MENUBAR_HPP
#define MENUBAR_HPP

class ImFont;

// all the imgui menubar functions
void showMenuBar(ImFont * menuFont);
void showFileMenu();
void showEditMenu();
void showOptionMenu();
void showHelpMenu();

#endif // MENUBAR_HPP