#ifndef EDITWINDOWMANAGER_HPP
#define EDITWINDOWMANAGER_HPP

#include <filesystem>
#include <queue>
#include <string>

#include "ui/editwindow.hpp"

namespace fs = std::filesystem;

class EditWindowManager {
public:
    EditWindowManager() = default;

    void initLastDirPaths();

    void startOpenChart();
    std::string loadEditWindow(SDL_Renderer * renderer, AudioSystem * audioSystem, fs::path chartPath);
    void showOpenChartWindow(SDL_Renderer * renderer, AudioSystem * audioSystem);

    void startNewEditWindow();
    void startSaveCurrentChart(bool saveAs = false);

    void showInitEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer);
    void showEditWindows(AudioSystem * audioSystem, std::vector<bool> & keysPressed);
    void createNewEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer);\

    void closeWindow(const EditWindow & currWindow, std::vector<EditWindow>::iterator & iter, AudioSystem * audioSystem);
    bool tryCloseEditWindow(EditWindow & currWindow, std::vector<EditWindow>::iterator & iter, AudioSystem * audioSystem);

    void setCopy(bool copy);
    void setPaste(bool paste);
    void setCut(bool cut);
    void setFlip(bool flip);
    void setUndo(bool undo);
    void setRedo(bool redo);
private:
    std::pair<std::string, int> getNextWindowNameAndID();

    void showSongConfig();
    void showChartConfig();

    unsigned int currentWindow { 0 };

    bool newEditStarted { false };

    bool activateUndo { false };
    bool activateRedo { false };

    std::queue<int> availableWindowIDs;
    std::vector<EditWindow> editWindows;

    std::string lastOpenResourceDir;
    std::string lastChartOpenDir;
    std::string lastChartSaveDir;
};

#endif // EDITWINDOWMANAGER_HPP
