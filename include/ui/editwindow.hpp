#ifndef EDITWINDOW_HPP
#define EDITWINDOW_HPP

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;

#include "actions/editaction.hpp"
#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"
#include "config/songposition.hpp"
#include "resources/texture.hpp"

class AudioSystem;

struct EditWindow {
    EditWindow(bool open, int ID, int musicSourceIdx, std::string name, std::shared_ptr<SDL_Texture> artTexture, ChartInfo chartinfo, SongInfo songinfo) : 
        open(open), ID(ID), musicSourceIdx(musicSourceIdx), name(name), artTexture(artTexture), chartinfo(chartinfo), songinfo(songinfo), 
        currTopNotes(chartinfo.notes.keyFreqsSorted.size()) {
    }

    bool open { true };
    bool unsaved { true };
    bool initialSaved { false };
    bool focused { false };

    bool editingSomething = false;

    int ID;
    int musicSourceIdx;
    int lastSavedActionIndex = 0;

    std::string name;

    std::shared_ptr<SDL_Texture> artTexture;

    std::stack<std::shared_ptr<EditAction>> editActionsUndo;
    std::stack<std::shared_ptr<EditAction>> editActionsRedo;

    SongPosition songpos;

    ChartInfo chartinfo;
    SongInfo songinfo;

    int currTopNotes;

    void showChartData(AudioSystem * audioSystem);

    void showChartSections(AudioSystem * audioSystem);
    bool showAddSection() const;
    bool showEditSection() const;
    void showRemoveSection();
    void showSectionDataWindow(bool & newSection, bool newSectionEdit, bool initSectionData);
    void showChartSectionList(AudioSystem * audioSystem);

    void showChartStatistics();

    void showToolbar(AudioSystem * audioSystem, std::vector<bool> & keysPressed);
    void showMusicPosition(double musicLengthSecs) const;
    void showMusicControls(AudioSystem * audioSystem, std::vector<bool> & keysPressed);
    void showMusicPreview(AudioSystem * audioSystem, float musicLengthSecs);
    void showMusicPreviewSliders(float musicLengthSecs);
    void showMusicPreviewButton(AudioSystem * audioSystem);
    void showMusicOffset();

    void showMetadata();
    bool showSongConfig();
    bool showChartConfig();
};

#endif // EDITWINDOW_HPP
