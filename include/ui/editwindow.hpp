#ifndef EDITWINDOW_HPP
#define EDITWINDOW_HPP

#include <filesystem>
#include <memory>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"
#include "config/songposition.hpp"
#include "resources/texture.hpp"
#include "ui/timeline.hpp"

namespace fs = std::filesystem;

class AudioSystem;

struct EditWindow {
    EditWindow(bool open, int ID, int musicSourceIdx, std::string_view name, std::shared_ptr<SDL_Texture> artTexture,
        const ChartInfo & chartinfo, const SongInfo & songinfo);

    bool open { true };
    bool unsaved { true };
    bool initialSaved { false };
    bool focused { false };
    bool resetInfoDisplay { false };

    bool editingSomething { false };

    int ID { 0 };
    int musicSourceIdx { 0 };
    int lastSavedActionIndex { 0 };
    int currTopNotes { 0 };

    std::string name;

    std::shared_ptr<SDL_Texture> artTexture;

    ChartInfo chartinfo;
    SongInfo songinfo;
    SongPosition songpos;

    Timeline timeline;

    void saveCurrentChartFiles();
    void saveCurrentChartFiles(std::string_view chartSaveFilename, const fs::path & chartSavePath, const fs::path & saveDir);

    void undoLastAction();
    void redoLastAction();

    void showContents(AudioSystem * audioSystem, std::vector<bool> & keysPressed);

    void showMetadata();
    bool showSongConfig();
    bool showChartConfig();

    void showChartData(AudioSystem * audioSystem);

    void showChartSections(AudioSystem * audioSystem);
    bool showAddSection() const;
    bool showEditSection() const;
    void showRemoveSection();
    void showSectionDataWindow(bool & newSection, bool newSectionEdit, bool initSectionData);
    void showChartSectionList(AudioSystem * audioSystem);

    void showChartStatistics();

    void showToolbar(AudioSystem * audioSystem, std::vector<bool> & keysPressed);
    void showMusicPosition(float musicLengthSecs) const;
    void showMusicControls(AudioSystem * audioSystem, std::vector<bool> & keysPressed);
    void showMusicPreview(AudioSystem * audioSystem, float musicLengthSecs);
    void showMusicPreviewSliders(float musicLengthSecs);
    void showMusicPreviewButton(AudioSystem * audioSystem);
    void showMusicOffset();
};

#endif // EDITWINDOW_HPP
