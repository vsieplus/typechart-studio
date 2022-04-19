#ifndef MENUBAR_HPP
#define MENUBAR_HPP

class ImFont;

// all the imgui menubar functions
void showMenuBar(ImFont * menuFont, ImFont * submenuFont);
void showFileMenu(ImFont * submenuFont);
void showEditMenu(ImFont * submenuFont);

#endif // MENUBAR_HPP