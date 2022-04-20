#ifndef EDITWINDOW_HPP
#define EDITWINDOW_HPP

#include <list>
#include <queue>
#include <string>

struct EditWindowData {
    EditWindowData(bool open, int ID, std::string name) : 
        open(open), ID(ID), name(name) {}

    bool open;
    bool unsaved = true;

    int ID;
    std::string name;
};

static std::queue<int> availableWindowIDs;
static std::list<EditWindowData> editWindows;

void createNewEditWindow();

void showInitEditWindow();
void showEditWindows();

#endif // EDITWINDOW_HPP