#include "ui/editwindowmanager.hpp"

#include "config/constants.hpp"
#include "config/utils.hpp"
#include "ui/preferences.hpp"



#include "ImGuiFileDialog.h"

void EditWindowManager::initLastDirPaths() {
    lastOpenResourceDir = Preferences::Instance().getInputDir();
    lastChartOpenDir = Preferences::Instance().getSaveDir();
    lastChartSaveDir = Preferences::Instance().getSaveDir();
}

void EditWindowManager::startOpenChart() {
    if(!ImGuiFileDialog::Instance()->IsOpened()) {
        ImGuiFileDialog::Instance()->OpenModal("openChart", "Open chart", constants::saveFileFilter, lastChartOpenDir, "");
    }
}

std::string EditWindowManager::loadEditWindow(SDL_Renderer * renderer, AudioSystem * audioSystem, fs::path chartPath) {
    fs::path chartDir = chartPath.parent_path();

    // try to load songinfo, chart info
    fs::path songinfoPath = chartDir / fs::path(constants::SONGINFO_FILENAME);
    if(!fs::exists(songinfoPath)) {
        return "No songinfo.json found";
    }

    if(!fs::exists(chartPath)) {
        return "No Chart found";
    }

    SongInfo songinfo;
    if(!songinfo.loadSongInfo(songinfoPath, chartDir)) {
        return "Invalid json";
    }

    int musicSourceIdx = audioSystem->loadMusic(songinfo.musicFilepath);

    if(musicSourceIdx == -1) {
        popupFailedToLoadMusic = true;
        return "Failed to load music";
    }

    ChartInfo chartinfo;
    SongPosition songpos;
    if(!chartinfo.loadChart(chartPath, songpos)) {
        return "Failed to open chart";
    }

    auto newWindowData = getNextWindowNameAndID();
    auto windowID = newWindowData.second;
    std::string windowName = chartinfo.savePath.filename().string() +  " (" + songinfo.getSongID() + ")";

    auto artTexture = Texture::loadTexture(songinfo.coverartFilepath, renderer);

    EditWindow newWindow = EditWindow(true, windowID, musicSourceIdx, windowName, artTexture, chartinfo, songinfo);
    newWindow.unsaved = false;
    newWindow.songpos = songpos;
    newWindow.initialSaved = true;

    strcpy(newWindow.UItitle, UItitle);
    strcpy(newWindow.UIartist, UIartist);
    strcpy(newWindow.UIgenre, UIgenre);
    strcpy(newWindow.UIbpmtext, UIbpmtext);

    strcpy(newWindow.UItypist, chartinfo.typist.c_str());
    newWindow.UIlevel = chartinfo.level;
    newWindow.UIkeyboardLayout = KEYBOARDLAYOUT_TO_ID.at(chartinfo.keyboardLayout);
    newWindow.UIdifficulty = DIFFICULTY_TO_ID.at(chartinfo.difficulty);

    editWindows.push_back(newWindow);

    return "";
}

void EditWindowManager::showOpenChartWindow(SDL_Renderer * renderer, AudioSystem * audioSystem) {
    bool popupFromOpenChart = false;

    if(ImGuiFileDialog::Instance()->Display("openChart", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string chartPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string chartDir = ImGuiFileDialog::Instance()->GetCurrentPath();

            auto popupID = loadEditWindow(renderer, audioSystem, fs::path(chartPath));
            if(popupID.size() > 0) {
                ImGui::OpenPopup(popupID.c_str());
                popupFromOpenChart = true;
            }

            lastChartOpenDir = chartDir;

            Preferences::Instance().addMostRecentFile(chartPath);
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if(ImGui::BeginPopupModal("No Songinfo found")) {
        ImGui::Text("No songinfo.json found in the selected chart's directory");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            if(popupFromOpenChart) {
                startOpenChart();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("No Chart found")) {
        ImGui::Text("The selected chart file could not be located");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            if(popupFromOpenChart) {
                startOpenChart();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("failedToOpenSonginfo")) {
        ImGui::Text("Failed to load selected songinfo.json file");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            if(popupFromOpenChart) {
                startOpenChart();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("Failed to open Chart")) {
        ImGui::Text("Failed to load selected chart file");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            if(popupFromOpenChart) {
                startOpenChart();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static char UImusicFilename[128] = "";
static char UIcoverArtFilename[128] = "";


static std::string UImusicFilepath = "";
static std::string UIcoverArtFilepath = "";

static bool popupInvalidJSON = true;
static bool popupFailedToLoadMusic = false;

static float UImusicPreviewStart = 0;
static float UImusicPreviewStop = 15;

void EditWindowManager::startNewEditWindow() {
    if(!newEditStarted) {
        newEditStarted = true;

        // reset config;
        UIlevel = 1;
        UImusicFilename[0] = '\0';
        UIcoverArtFilename[0] = '\0';
        UItitle[0] = '\0';
        UIartist[0] = '\0';
        UIgenre[0] = '\0';
        UIbpmtext[0] = '\0';
        
        UImusicFilepath = "";
        UIcoverArtFilepath = "";

        UImusicPreviewStart = 0;
        UImusicPreviewStop = 15;
    }
}

void EditWindowManager::startSaveCurrentChart(bool saveAs) {
    if(currentWindow >= 0 && currentWindow < editWindows.size()) {
        auto & editWindow = editWindows.at(currentWindow);
        if(saveAs || editWindow.unsaved) {
            // save vs save as
            if(saveAs || !editWindow.initialSaved) {
                editWindow.initialSaved = false;
                ImGuiFileDialog::Instance()->OpenModal("saveChart", "Save current chart", constants::saveFileFilter, Preferences::Instance().getSaveDir(),
                    "Untitled.type", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
            } else {
                editWindow.saveCurrentChartFiles();
            }
        }
    }
}

void EditWindowManager::showInitEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer) {
    if(newEditStarted) {
        ImGui::SetNextWindowSize(constants::newEditWindowSize);
        ImGui::Begin("New Chart", &newEditStarted, ImGuiWindowFlags_NoResize);

        showSongConfig();
        ImGui::Separator();
        showChartConfig();

        if(ImGui::Button("Create")) {
            createNewEditWindow(audioSystem, renderer);
        }

        ImGui::End();
    }

    if(ImGui::BeginPopupModal("Failed to load music", &popupFailedToLoadMusic)) {
        ImGui::Text("Unable to load the selected music");
        ImGui::EndPopup();
    }
}

std::pair<std::string, int> EditWindowManager::getNextWindowNameAndID() {
    std::string windowName = constants::DEFAULT_WINDOW_NAME;
    int windowID = 0;

    if(!availableWindowIDs.empty()) {
        windowID = availableWindowIDs.back();
        availableWindowIDs.pop();

        if(windowID > 0)
            windowName += std::to_string(windowID);
    } else if(editWindows.size() > 0) {
        windowID = editWindows.size();
        windowName += std::to_string(windowID);
    }

    return std::make_pair(windowName, windowID);
}

void EditWindowManager::createNewEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer) {
    // populate with current song, chart info
    SongInfo songinfo = SongInfo(UItitle, UIartist, UIgenre, UIbpmtext, UImusicFilename, UIcoverArtFilename,
        UImusicFilepath, UIcoverArtFilepath, UImusicPreviewStart, UImusicPreviewStop);
    ChartInfo chartinfo = ChartInfo(UIlevel, UItypist, ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout), ID_TO_DIFFICULTY.at(UIdifficulty));

    // attempt to load music
    int musicSourceIdx = audioSystem->loadMusic(UImusicFilepath);
    if(musicSourceIdx == -1) {
        ImGui::OpenPopup("Failed to load music");
        popupFailedToLoadMusic = true;
        return;
    } else {
        popupFailedToLoadMusic = false;

        auto newWindowData = getNextWindowNameAndID();
        auto windowName = newWindowData.first;
        auto windowID = newWindowData.second;

        auto artTexture = Texture::loadTexture(UIcoverArtFilepath, renderer);

        EditWindow newWindow = EditWindow(true, windowID, musicSourceIdx, windowName, artTexture, chartinfo, songinfo);
        strcpy(newWindow.UItitle, UItitle);
        strcpy(newWindow.UIartist, UIartist);
        strcpy(newWindow.UIgenre, UIgenre);
        strcpy(newWindow.UIbpmtext, UIbpmtext);

        strcpy(newWindow.UItypist, UItypist);
        newWindow.UIlevel = UIlevel;
        newWindow.UIkeyboardLayout = UIkeyboardLayout;
        newWindow.UIdifficulty = UIdifficulty;

        // initial section from BPM
        float initialBpm = ::atof(UIbpmtext);
        BeatPos initialSectionStart = {0, 1, 0};
        newWindow.songpos.timeinfo.push_back(Timeinfo(initialSectionStart, nullptr, 4, initialBpm, 0));
    
        editWindows.push_back(newWindow);

        newEditStarted = false;
    }
}

bool EditWindowManager::tryCloseEditWindow(EditWindow & currWindow, std::vector<EditWindow>::iterator & iter, AudioSystem * audioSystem) {
    bool closed = false;

    if(currWindow.unsaved) {
        char msg[128];
        snprintf(msg, 128, "Unsaved work! [%s]", currWindow.name.c_str());
        ImGui::Begin(msg);

        ImGui::Text("Save before closing?");
        if(ImGui::Button("Yes")) {
            startSaveCurrentChart();
        }

        ImGui::SameLine();
        if(ImGui::Button("No")) {
            closeWindow(currWindow, iter, audioSystem);
            closed = true;
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            currWindow.open = true;
        }

        ImGui::End();
    } else {
        closeWindow(currWindow, iter, audioSystem);
        closed = true;
    }

    return closed;
}

void EditWindowManager::closeWindow(const EditWindow & currWindow, std::vector<EditWindow>::iterator & iter, AudioSystem * audioSystem) {
    availableWindowIDs.push(currWindow.ID);

    audioSystem->stopMusic(currWindow.musicSourceIdx);
    audioSystem->deactivateMusicSource(currWindow.musicSourceIdx);

    iter = editWindows.erase(iter);
}

void EditWindowManager::showEditWindows(AudioSystem * audioSystem, std::vector<bool> & keysPressed) {
    static bool updatedName = false;
    static ImVec2 sizeBeforeUpdate { constants::editWindowSize };
    static ImVec2 currWindowSize { 0, 0 };

    unsigned int i = 0;
    for(auto iter = editWindows.begin(); iter != editWindows.end();) {
        auto & currWindow = *iter;
        currWindow.songpos.update();

        ImGuiWindowFlags windowFlags = 0;
        if(currWindow.unsaved)  windowFlags |= ImGuiWindowFlags_UnsavedDocument;
        if(!currWindow.open)    windowFlags |= ImGuiWindowFlags_NoInputs;

        if(updatedName) {
            updatedName = false;
            ImGui::SetNextWindowSize(sizeBeforeUpdate);
        }

        ImGui::Begin(currWindow.name.c_str(), &(currWindow.open), windowFlags);

        if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
            (ImGuiFileDialog::Instance()->IsOpened() && currWindow.focused)) {
            currentWindow = i;
            currWindowSize = ImGui::GetWindowSize();
            currWindow.focused = true;
        } else {
            currWindow.focused = false;
        }

        currWindow.showContents(audioSystem, keysPressed);

        ImGui::End();

        bool closedCurrwindow = false;
        if(!currWindow.open) {
            closedCurrwindow = tryCloseEditWindow(currWindow, iter, audioSystem);
        }

        if(!closedCurrwindow) {
            i++;
            iter++;
        }
    }

    if(ImGuiFileDialog::Instance()->Display("saveChart", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if(ImGuiFileDialog::Instance()->IsOk()) {
            std::string chartSavePath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string chartSaveFilename = ImGuiFileDialog::Instance()->GetCurrentFileName();
            std::string saveDir = ImGuiFileDialog::Instance()->GetCurrentPath();

            EditWindow & currEditWindow = editWindows.at(currentWindow);
            if(currEditWindow.name != chartSaveFilename) {
                updatedName = true;
                sizeBeforeUpdate = currWindowSize;
            }

            currEditWindow.saveCurrentChartFiles(chartSaveFilename, fs::path(chartSavePath), fs::path(saveDir));

            Preferences::Instance().addMostRecentFile(chartSavePath);
            lastChartSaveDir = saveDir;

            ImGuiIO& io = ImGui::GetIO();
            io.MouseClicked[0] = false;
            io.MouseClicked[1] = false;
            io.MouseClicked[2] = false;
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if(activateUndo) {
        if(currentWindow < editWindows.size() && !editWindows.at(currentWindow).editActionsUndo.empty()) {
            auto & currEditWindow = editWindows.at(currentWindow);

            auto action = currEditWindow.editActionsUndo.top();

            action->undoAction(&currEditWindow);

            currEditWindow.editActionsRedo.push(action);
            currEditWindow.editActionsUndo.pop();

            currEditWindow.unsaved = (int)currEditWindow.editActionsUndo.size() != currEditWindow.lastSavedActionIndex;
        }

        activateUndo = false;
    }

    if(activateRedo) {
        if(currentWindow < editWindows.size() && !editWindows.at(currentWindow).editActionsRedo.empty()) {
            auto & currEditWindow = editWindows.at(currentWindow);

            auto action = currEditWindow.editActionsRedo.top();
            action->redoAction(&currEditWindow);

            currEditWindow.editActionsUndo.push(action);
            currEditWindow.editActionsRedo.pop();
            currEditWindow.unsaved = true;
        }

        activateRedo = false;
    }
}

void EditWindowManager::showSongConfig() {
    // song config
    ImGui::Text(ICON_FA_CLIPBOARD " Song configuration");
    if(ImGui::Button("Load from existing...")) {
        ImGuiFileDialog::Instance()->OpenModal("selectSonginfo", "Select songinfo.json", constants::songinfoFileFilter, lastChartOpenDir, "");
    }
    
    ImGui::SameLine();
    utils::HelpMarker("Load song data from a pre-existing songinfo.json file");

    // songinfo dialog
    if(ImGuiFileDialog::Instance()->Display("selectSonginfo", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string songinfoPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string songinfoDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            //loadSonginfo(songinfoPath, songinfoDir);

            lastChartOpenDir = songinfoDir;
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText(ICON_FA_MUSIC " Music", UImusicFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##music")) {
        ImGuiFileDialog::Instance()->OpenModal("selectMusicFile", "Select Music", constants::musicFileFilters, lastOpenResourceDir, "");
    }

    // music file dialog
    if(ImGuiFileDialog::Instance()->Display("selectMusicFile", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            UImusicFilepath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            strcpy(UImusicFilename, fileName.c_str());

            lastOpenResourceDir = ImGuiFileDialog::Instance()->GetCurrentPath();
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText(ICON_FA_PHOTO_FILM " Art", UIcoverArtFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##art")) {
        ImGuiFileDialog::Instance()->OpenModal("selectArt", "Select Art", constants::imageFileFilters, lastOpenResourceDir, "");
    }

    // art file dialog
    if(ImGuiFileDialog::Instance()->Display("selectArt", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            UIcoverArtFilepath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            strcpy(UIcoverArtFilename, fileName.c_str());

            lastOpenResourceDir = ImGuiFileDialog::Instance()->GetCurrentPath();
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText(ICON_FA_HEADING " Title", UItitle, 64);
    ImGui::InputText(ICON_FA_MICROPHONE " Artist", UIartist, 64);
    ImGui::InputText(ICON_FA_COMPACT_DISC " Genre", UIgenre, 64);
    ImGui::InputText(ICON_FA_HEART_PULSE " BPM", UIbpmtext, 64);
    ImGui::SameLine();
    utils::HelpMarker("If the song has BPM changes, enter the initial BPM.\nYou will be able to add BPM changes later.");
}

void EditWindowManager::showChartConfig() {
    ImGui::Text("Chart configuration");

    ImGui::InputText(ICON_FA_PENCIL " Typist", UItypist, 64);
    ImGui::Combo(ICON_FA_KEYBOARD " Keyboard", &UIkeyboardLayout, "QWERTY\0DVORAK\0AZERTY\0COLEMAK\0");
    ImGui::SameLine();
    utils::HelpMarker("Choose the keyboard layout that this chart is\n"
               "intended to be played with. Charts will then be\n"
               "accordingly 'translated' to other keyboard layouts\n"
               "when loaded into Typing Tempo.");
    ImGui::Combo(ICON_FA_CHESS_PAWN " Difficulty", &UIdifficulty, "easy\0normal\0hard\0expert\0unknown\0");
    ImGui::InputInt(ICON_FA_CHESS_ROOK " Level", &UIlevel);
}

void EditWindowManager::setCopy(bool copy) {
    activateCopy = copy;
}

void EditWindowManager::setPaste(bool paste) {
    activatePaste = paste;
}

void EditWindowManager::setCut(bool cut) {
    activateCut = cut;
}

void EditWindowManager::setUndo(bool undo) {
    activateUndo = undo;
}

void EditWindowManager::setRedo(bool redo) {
    activateRedo = redo;
}

void EditWindowManager::setFlip(bool flip) {
    activateFlip = flip;
}
