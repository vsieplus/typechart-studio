#ifndef EDITWINDOW_HPP
#define EDITWINDOW_HPP

#include <list>
#include <queue>
#include <string>

#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"

class AudioSystem;

struct EditWindowData {
    EditWindowData(bool open, int ID, std::string name, ChartInfo chartinfo, SongInfo songinfo) : 
        open(open), ID(ID), name(name), chartinfo(chartinfo), songinfo(songinfo) {}

    bool open;
    bool unsaved = true;

    int ID;
    std::string name;

    ChartInfo chartinfo;
    SongInfo songinfo;
};

static std::queue<int> availableWindowIDs;
static std::list<EditWindowData> editWindows;

void startNewEditWindow();

void showInitEditWindow(AudioSystem * audioSystem);
void showEditWindows(AudioSystem * audioSystem);

#endif // EDITWINDOW_HPP