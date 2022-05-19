#ifndef EDITWINDOW_HPP
#define EDITWINDOW_HPP

#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

#include "actions/editaction.hpp"
#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"
#include "config/songposition.hpp"
#include "resources/texture.hpp"

class AudioSystem;

struct EditWindowData {
    EditWindowData(bool open, int ID, int musicSourceIdx, std::string name, std::shared_ptr<SDL_Texture> artTexture, ChartInfo chartinfo, SongInfo songinfo) : 
        open(open), ID(ID), musicSourceIdx(musicSourceIdx), name(name), artTexture(artTexture), chartinfo(chartinfo), songinfo(songinfo) {
    }

    bool open;
    bool unsaved = true;
    bool initialSaved = false;
    bool focused = false;

    int ID;
    int musicSourceIdx;
    std::string name;

    std::shared_ptr<SDL_Texture> artTexture;

    std::stack<std::shared_ptr<EditAction>> editActionsUndo;
    std::stack<std::shared_ptr<EditAction>> editActionsRedo;

    SongPosition songpos;

    ChartInfo chartinfo;
    SongInfo songinfo;
};

static std::queue<int> availableWindowIDs;
static std::vector<EditWindowData> editWindows;

static std::string lastOpenResourceDir;
static std::string lastChartOpenDir;
static std::string lastChartSaveDir;

void setCopy();
void setPaste();
void setCut();
void setUndo();
void setRedo();
void setFlip();

void initLastDirPaths();

void startOpenChart();
void loadEditWindow(SDL_Renderer * renderer, AudioSystem * audioSystem, std::string chartPath);
void showOpenChartWindow(SDL_Renderer * renderer, AudioSystem * audioSystem);

void startNewEditWindow();
void startSaveCurrentChart(bool saveAs = false);

BeatPos calculateBeatpos(float absBeat, int currentBeatsplit, const std::vector<Timeinfo> & timeinfo);

void showInitEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer);
void showEditWindows(AudioSystem * audioSystem, std::vector<bool> & keysPressed);

#endif // EDITWINDOW_HPP