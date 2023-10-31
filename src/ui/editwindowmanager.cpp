#include "ui/editwindowmanager.hpp"

#include "config/constants.hpp"
#include "config/utils.hpp"
#include "ui/preferences.hpp"
#include "ui/windowsizes.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiFileDialog.h"

void EditWindowManager::initLastDirPaths() {
    lastOpenResourceDir = Preferences::Instance().getInputDir();
    lastChartOpenDir = Preferences::Instance().getSaveDir();
    lastChartSaveDir = Preferences::Instance().getSaveDir();
}

void EditWindowManager::startOpenChart() const {
    if(!ImGuiFileDialog::Instance()->IsOpened()) {
        ImGuiFileDialog::Instance()->OpenModal("openChart", "Open chart", constants::saveFileFilter.c_str(), lastChartOpenDir, "");
    }
}

std::string EditWindowManager::loadEditWindow(SDL_Renderer * renderer, AudioSystem * audioSystem, fs::path chartPath) {
    fs::path chartDir = chartPath.parent_path();

    // try to load songinfo, chart info
    fs::path songinfoPath { chartDir / fs::path(constants::SONGINFO_FILENAME) };
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

    int musicSourceIdx { audioSystem->loadMusic(songinfo.musicFilepath) };

    if(musicSourceIdx == -1) {
        popupFailedToLoadMusic = true;
        return "Failed to load music";
    }

    ChartInfo chartinfo;
    SongPosition songpos;
    if(!chartinfo.loadChart(chartPath, songpos)) {
        return "Failed to open chart";
    }

    auto [_, windowID] { getNextWindowNameAndID() };
    std::string windowName { chartinfo.savePath.filename().string() +  " (" + songinfo.getSongID() + ")" };
    auto artTexture { Texture::loadTexture(songinfo.coverartFilepath, renderer) };

    EditWindow newWindow { true, windowID, musicSourceIdx, windowName, artTexture, chartinfo, songinfo };

    newWindow.unsaved = false;
    newWindow.songpos = songpos;
    newWindow.initialSaved = true;
    newWindow.resetInfoDisplay = true;

    editWindows.push_back(newWindow);

    return "";
}

void EditWindowManager::showOpenChartWindow(SDL_Renderer * renderer, AudioSystem * audioSystem) {
    bool popupFromOpenChart { false };

    if(ImGuiFileDialog::Instance()->Display("openChart", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if(ImGuiFileDialog::Instance()->IsOk()) {
            std::string chartPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string chartDir = ImGuiFileDialog::Instance()->GetCurrentPath();

            if(auto popupID = loadEditWindow(renderer, audioSystem, fs::path(chartPath)); popupID.size() > 0) {
                ImGui::OpenPopup(popupID.c_str());
                popupFromOpenChart = true;
            }

            lastChartOpenDir = chartDir;

            Preferences::Instance().addMostRecentFile(chartPath);
        }

        ImGuiFileDialog::Instance()->Close();
    }

    showNoSonginfoFound(popupFromOpenChart);
    showNoChartFound(popupFromOpenChart);
    showFailedToOpenSongInfo(popupFromOpenChart);
    showFailedToOpenChart(popupFromOpenChart);
}

void EditWindowManager::showNoSonginfoFound(bool popupFromOpenChart) const {
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
}

void EditWindowManager::showNoChartFound(bool popupFromOpenChart) const {
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
}

void EditWindowManager::showFailedToOpenSongInfo(bool popupFromOpenChart) const {
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
}

void EditWindowManager::showFailedToOpenChart(bool popupFromOpenChart) const {
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
    if(static_cast<int>(currentWindow) >= 0 && currentWindow < editWindows.size()) {
        auto & editWindow = editWindows.at(currentWindow);
        if(saveAs || editWindow.unsaved) {
            // save vs save as
            if(saveAs || !editWindow.initialSaved) {
                editWindow.initialSaved = false;
                ImGuiFileDialog::Instance()->OpenModal("saveChart", "Save current chart", constants::saveFileFilter.c_str(), Preferences::Instance().getSaveDir(),
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
    } else if(!editWindows.empty()) {
        windowID = static_cast<int>(editWindows.size());
        windowName += std::to_string(windowID);
    }

    return std::make_pair(windowName, windowID);
}

void EditWindowManager::createNewEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer) {
    // populate with current song, chart info
    SongInfo songinfo { UItitle, UIartist, UIgenre, UIbpmtext, UImusicFilename, UIcoverArtFilename,
        UImusicFilepath, UIcoverArtFilepath, UImusicPreviewStart, UImusicPreviewStop };
    ChartInfo chartinfo { UIlevel, UItypist, constants::ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout), constants::ID_TO_DIFFICULTY.at(UIdifficulty) };

    // attempt to load music
    int musicSourceIdx = audioSystem->loadMusic(UImusicFilepath);
    if(musicSourceIdx == -1) {
        ImGui::OpenPopup("Failed to load music");
        popupFailedToLoadMusic = true;
        return;
    } else {
        popupFailedToLoadMusic = false;

        auto [windowName, windowID] { getNextWindowNameAndID() };
        auto artTexture { Texture::loadTexture(UIcoverArtFilepath, renderer) };

        EditWindow newWindow { true, windowID, musicSourceIdx, windowName, artTexture, chartinfo, songinfo };
        newWindow.resetInfoDisplay = true;

        // initial section from BPM
        double initialBpm = ::atof(UIbpmtext);
        BeatPos initialSectionStart { 0, 1, 0 };
        newWindow.songpos.timeinfo.emplace_back(initialSectionStart, nullptr, 4, initialBpm, 0);
    
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
        if(checkWindowFocus(i, currWindow)) {
            currWindowSize = ImGui::GetWindowSize();
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

    showSaveChart(updatedName, sizeBeforeUpdate, currWindowSize);
    checkUndoRedo();
}

bool EditWindowManager::checkWindowFocus(unsigned int i, EditWindow & currWindow) {
    if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || (ImGuiFileDialog::Instance()->IsOpened() && currWindow.focused)) {
        currentWindow = i;
        currWindow.focused = true;
    } else {
        currWindow.focused = false;
    }

    return currWindow.focused;
}

void EditWindowManager::showSaveChart(bool & updatedName, ImVec2 & sizeBeforeUpdate, ImVec2 & currWindowSize) {
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
}

void EditWindowManager::checkUndoRedo() {
    if(activateUndo) {
        if(currentWindow < editWindows.size()) {
            auto & currEditWindow = editWindows.at(currentWindow);
            currEditWindow.undoLastAction();
        }

        activateUndo = false;
    }

    if(activateRedo) {
        if(currentWindow < editWindows.size()) {
            auto & currEditWindow = editWindows.at(currentWindow);
            currEditWindow.redoLastAction();
        }

        activateRedo = false;
    }
}

void EditWindowManager::showSongConfig() {
    // song config
    ImGui::Text(ICON_FA_CLIPBOARD " Song configuration");
    if(ImGui::Button("Load from existing...")) {
        ImGuiFileDialog::Instance()->OpenModal("selectSonginfo", "Select songinfo.json", constants::songinfoFileFilter.c_str(), lastChartOpenDir, "");
    }
    
    ImGui::SameLine();
    utils::HelpMarker("Load song data from a pre-existing songinfo.json file");

    // songinfo dialog
    if(ImGuiFileDialog::Instance()->Display("selectSonginfo", ImGuiWindowFlags_NoCollapse, constants::minFDSize, constants::maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string songinfoPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string songinfoDir = ImGuiFileDialog::Instance()->GetCurrentPath();

            if(SongInfo songinfo; songinfo.loadSongInfo(songinfoPath, songinfoDir)) {
                snprintf(UIartist, 64, "%s", songinfo.artist.c_str());
                snprintf(UItitle, 64, "%s", songinfo.title.c_str());
                snprintf(UIbpmtext, 16, "%s", songinfo.bpmtext.c_str());
                snprintf(UImusicFilename, 64, "%s", songinfo.musicFilename.c_str());
                UImusicFilepath = songinfo.musicFilepath;
                UImusicPreviewStart = songinfo.musicPreviewStart;
                UImusicPreviewStop = songinfo.musicPreviewStop;
            }

            lastChartOpenDir = songinfoDir;
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText(ICON_FA_MUSIC " Music", UImusicFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##music")) {
        ImGuiFileDialog::Instance()->OpenModal("selectMusicFile", "Select Music", constants::musicFileFilters.c_str(), lastOpenResourceDir, "");
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
        ImGuiFileDialog::Instance()->OpenModal("selectArt", "Select Art", constants::imageFileFilters.c_str(), lastOpenResourceDir, "");
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
    editWindows.at(currentWindow).timeline.activateCopy = copy;
}

void EditWindowManager::setPaste(bool paste) {
    editWindows.at(currentWindow).timeline.activatePaste = paste;
}

void EditWindowManager::setCut(bool cut) {
    editWindows.at(currentWindow).timeline.activateCut = cut;
}

void EditWindowManager::setFlip(bool flip) {
    editWindows.at(currentWindow).timeline.activateFlip = flip;
}

void EditWindowManager::setUndo(bool undo) {
    activateUndo = undo;
}

void EditWindowManager::setRedo(bool redo) {
    activateRedo = redo;
}
