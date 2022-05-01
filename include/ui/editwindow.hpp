#ifndef EDITWINDOW_HPP
#define EDITWINDOW_HPP

#include <queue>
#include <string>
#include <vector>

#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"
#include "config/songposition.hpp"
#include "resources/texture.hpp"

class AudioSystem;

struct EditWindowData {
    EditWindowData(bool open, int ID, std::string name, std::shared_ptr<SDL_Texture> artTexture, ChartInfo chartinfo, SongInfo songinfo) : 
        open(open), ID(ID), name(name), artTexture(artTexture), chartinfo(chartinfo), songinfo(songinfo) {
    }

    bool open;
    bool unsaved = true;
    bool initialSaved = false;

    int ID;
    std::string name;

    std::shared_ptr<SDL_Texture> artTexture;

    SongPosition songpos;

    ChartInfo chartinfo;
    SongInfo songinfo;
};

static std::queue<int> availableWindowIDs;
static std::vector<EditWindowData> editWindows;

static unsigned int currentWindow = 0;

void startNewEditWindow();
void startSaveCurrentChart(bool saveAs = false);

void showInitEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer);
void showEditWindows(AudioSystem * audioSystem, std::vector<bool> & keysPressed);

#endif // EDITWINDOW_HPP