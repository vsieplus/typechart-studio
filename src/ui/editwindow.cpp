#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <float.h>
#include <fstream>
#include <map>

#include <nlohmann/json.hpp>

#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome6.h"

#include "actions/deleteitems.hpp"
#include "actions/deletenote.hpp"
#include "actions/editnote.hpp"
#include "actions/editskip.hpp"
#include "actions/flipnote.hpp"
#include "actions/insertitems.hpp"
#include "actions/placenote.hpp"
#include "actions/placestop.hpp"
#include "actions/placeskip.hpp"

#include "ui/preferences.hpp"
#include "ui/editwindow.hpp"
#include "ui/windowsizes.hpp"

#include "systems/audiosystem.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

const std::map<int, std::string> ID_TO_KEYBOARDLAYOUT = {
    { 0, "QWERTY" },
    { 1, "DVORAK" },
    { 2, "AZERTY" },
    { 3, "COLEMAK" }
};

const std::map<int, std::string> ID_TO_DIFFICULTY = {
    { 0, "easy" },
    { 1, "normal" },
    { 2, "hard" },
    { 3, "extreme" },
    { 4, "unknown" }
};

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static char UImusicFilename[128] = "";
static char UIcoverArtFilename[128] = "";

static char UItitle[64] = "";
static char UIartist[64] = "";
static char UIbpmtext[16] = "";

const char * saveFileFilter = "(*.type){.type}";
const char * songinfoFileFilter = "(*.json){.json}";
const char * imageFileFilters = "(*.jpg *.png){.jpg,.png}";
const char * musicFileFilters = "(*.flac *.mp3 *.ogg *.wav){.flac,.mp3,.ogg,.wav}";

static std::string UImusicFilepath = "";
static std::string UIcoverArtFilepath = "";
static std::string DEFAULT_WINDOW_NAME = "Untitled";

static bool popupIncomplete = true;
static bool popupInvalidJSON = true;
static bool popupFailedToLoadMusic = false;
static bool newEditStarted = false;

static bool activateCopy = false;
static bool activateCut = false;
static bool activatePaste = false;
static bool activateUndo = false;
static bool activateRedo = false;
static bool activateFlip = false;

static float UImusicPreviewStart = 0;
static float UImusicPreviewStop = 15;

inline static unsigned int currentWindow = 0;

void setCopy() {
    activateCopy = true;
}

void setPaste() {
    activatePaste = true;
}

void setCut() {
    activateCut = true;
}

void setUndo() {
    activateUndo = true;
}

void setRedo() {
    activateRedo = true;
}

void setFlip() {
    activateFlip = true;
}

void initLastDirPaths() {
    lastOpenResourceDir = Preferences::Instance().getInputDir();
    lastChartOpenDir = Preferences::Instance().getSaveDir();
    lastChartSaveDir = Preferences::Instance().getSaveDir();
}

void emptyActionStack(std::stack<std::shared_ptr<EditAction>> & stack) {
    while(!stack.empty())
        stack.pop();
}

bool loadSonginfo(std::string songinfoPath, std::string songinfoDir) {
    json songinfoJSON;

    try {
        std::ifstream in(songinfoPath);
        in >> songinfoJSON;
        popupInvalidJSON = false;
    } catch(...) {
        ImGui::OpenPopup("invalidJSON");
        popupInvalidJSON = true;
        return false;
    }

    int numFound = 0;

    if(songinfoJSON.contains("title")) {
        std::string title = songinfoJSON["title"];
        strcpy(UItitle, title.c_str());
        numFound++;
    }

    if(songinfoJSON.contains("artist")) {
        std::string artist = songinfoJSON["artist"];
        strcpy(UIartist, artist.c_str());
        numFound++;
    }

    if(songinfoJSON.contains("music")) {
        std::string music = songinfoJSON["music"];
        strcpy(UImusicFilename, music.c_str());
        numFound++;

        UImusicFilepath = (fs::path(songinfoDir) / fs::path(music)).string();
    }

    if(songinfoJSON.contains("coverart")) {
        std::string coverart = songinfoJSON["coverart"];
        strcpy(UIcoverArtFilename, coverart.c_str());
        numFound++;

        UIcoverArtFilepath = (fs::path(songinfoDir) / fs::path(coverart)).string();
    }

    if(songinfoJSON.contains("bpmtext")) {
        std::string bpmtext = songinfoJSON["bpmtext"];
        strcpy(UIbpmtext, bpmtext.c_str());
        numFound++;
    }

    if(songinfoJSON.contains("musicPreviewStart")) {
        UImusicPreviewStart = songinfoJSON["musicPreviewStart"];
        numFound++;
    }

    if(songinfoJSON.contains("musicPreviewStop")) {
        UImusicPreviewStop = songinfoJSON["musicPreviewStop"];
        numFound++;
    }

    if(numFound < 5) {
        ImGui::OpenPopup("incompleteSonginfo");
        popupIncomplete = true;
        return false;
    } else {
        popupIncomplete = false;
        return true;
    }
}

void showSongConfig() {
    // song config
    ImGui::Text(ICON_FA_CLIPBOARD " Song configuration");
    if(ImGui::Button("Load from existing...")) {
        ImGuiFileDialog::Instance()->OpenModal("selectSonginfo", "Select songinfo.json", songinfoFileFilter, lastChartOpenDir, "");
    }
    
    ImGui::SameLine();
    HelpMarker("Load song data from a pre-existing songinfo.json file");

    // songinfo dialog
    if(ImGuiFileDialog::Instance()->Display("selectSonginfo", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string songinfoPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string songinfoDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            loadSonginfo(songinfoPath, songinfoDir);

            lastChartOpenDir = songinfoDir;
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText(ICON_FA_MUSIC " Music", UImusicFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##music")) {
        ImGuiFileDialog::Instance()->OpenModal("selectMusicFile", "Select Music", musicFileFilters, lastOpenResourceDir, "");
    }

    // music file dialog
    if(ImGuiFileDialog::Instance()->Display("selectMusicFile", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
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
        ImGuiFileDialog::Instance()->OpenModal("selectArt", "Select Art", imageFileFilters, lastOpenResourceDir, "");
    }

    // art file dialog
    if(ImGuiFileDialog::Instance()->Display("selectArt", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
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
    ImGui::InputText(ICON_FA_HEART_PULSE " BPM", UIbpmtext, 64);
    ImGui::SameLine();
    HelpMarker("If the song has BPM changes, enter the initial BPM");
    
}

static char UItypist[64] = "";
static int UIlevel = 1;
static int UIkeyboardLayout = 0;
static int UIdifficulty = 0;

void showChartConfig() {
    ImGui::Text("Chart configuration");

    ImGui::InputText(ICON_FA_PENCIL " Typist", UItypist, 64);
    ImGui::Combo(ICON_FA_KEYBOARD " Keyboard", &UIkeyboardLayout, "QWERTY\0DVORAK\0AZERTY\0COLEMAK\0");
    ImGui::SameLine();
    HelpMarker("Choose the keyboard layout that this chart is\n"
                "intended to be played with. Charts will then be\n"
                "accordingly 'translated' to other keyboard layouts\n"
                "when loaded into Typing Tempo.");
    ImGui::Combo(ICON_FA_CHESS_PAWN " Difficulty", &UIdifficulty, "easy\0normal\0hard\0expert\0unknown\0");
    ImGui::InputInt(ICON_FA_CHESS_ROOK " Level", &UIlevel);
}

void startOpenChart() {
    if(!ImGuiFileDialog::Instance()->IsOpened()) {
        ImGuiFileDialog::Instance()->OpenModal("openChart", "Open chart", saveFileFilter, lastChartOpenDir, "");
    }
}

void startNewEditWindow() {
    if(!newEditStarted) {
        newEditStarted = true;

        // reset config;
        UIlevel = 1;
        UImusicFilename[0] = '\0';
        UIcoverArtFilename[0] = '\0';
        UItitle[0] = '\0';
        UIartist[0] = '\0';
        UIbpmtext[0] = '\0';
        
        UImusicFilepath = "";
        UIcoverArtFilepath = "";

        UImusicPreviewStart = 0;
        UImusicPreviewStop = 15;
    }
}

std::pair<std::string, int> getNextWindowNameAndID() {
    std::string windowName = DEFAULT_WINDOW_NAME;
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

void createNewEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer) {
    // populate with current song, chart info
    SongInfo songinfo = SongInfo(UItitle, UIartist, UIbpmtext, UImusicFilename, UIcoverArtFilename, 
                                 UImusicFilepath, UIcoverArtFilepath, UImusicPreviewStart, UImusicPreviewStop);
    ChartInfo chartinfo = ChartInfo(UIlevel, UItypist, ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout), ID_TO_DIFFICULTY.at(UIdifficulty));

    // attempt to load music
    int musicSourceIdx = audioSystem->loadMusic(UImusicFilepath);
    if(musicSourceIdx == -1) {
        ImGui::OpenPopup("failedToLoadMusic");
        popupFailedToLoadMusic = true;
        return;
    } else {
        popupFailedToLoadMusic = false;

        auto newWindowData = getNextWindowNameAndID();
        auto windowName = newWindowData.first;
        auto windowID = newWindowData.second;

        auto artTexture = Texture::loadTexture(UIcoverArtFilepath, renderer);

        EditWindowData newWindow = EditWindowData(true, windowID, musicSourceIdx, windowName, artTexture, chartinfo, songinfo);

        // initial section from BPM
        float initialBpm = ::atof(UIbpmtext);
        BeatPos initialSectionStart = {0, 1, 0};
        newWindow.songpos.timeinfo.push_back(Timeinfo(initialSectionStart, nullptr, 4, initialBpm, 0));
    
        editWindows.push_back(newWindow);

        newEditStarted = false;
    }
}

void loadEditWindow(SDL_Renderer * renderer, AudioSystem * audioSystem, std::string chartPath) {
    fs::path chartDir = fs::path(chartPath).parent_path();

    // try to load songinfo, chart info
    std::string songinfoPath = (chartDir / fs::path("songinfo.json")).string();
    if(!fs::exists(songinfoPath)) {
        ImGui::OpenPopup("noSonginfoFound");
        return;
    }
    
    ChartInfo chartinfo;
    SongPosition songpos;

    if(!loadSonginfo(songinfoPath, chartDir.string())) {
        return;
    }

    SongInfo songinfo = SongInfo(UItitle, UIartist, UIbpmtext, UImusicFilename, UIcoverArtFilename, 
                                 UImusicFilepath, UIcoverArtFilepath, UImusicPreviewStart, UImusicPreviewStop);
    songinfo.saveDir = chartDir.string();

    int musicSourceIdx = audioSystem->loadMusic(UImusicFilepath);

    if(musicSourceIdx == -1) {
        ImGui::OpenPopup("failedToLoadMusic");
        popupFailedToLoadMusic = true;
        return;
    }

    if(!chartinfo.loadChart(chartPath, songpos)) {
        ImGui::OpenPopup("failedToOpenChart");
        return;
    }

    auto newWindowData = getNextWindowNameAndID();
    auto windowID = newWindowData.second;
    std::string windowName = fs::path(chartinfo.savePath).filename().string();

    auto artTexture = Texture::loadTexture(songinfo.coverartFilepath, renderer);

    EditWindowData newWindow = EditWindowData(true, windowID, musicSourceIdx, windowName, artTexture, chartinfo, songinfo);
    newWindow.unsaved = false;
    newWindow.songpos = songpos;
    newWindow.initialSaved = true;
    editWindows.push_back(newWindow);
}

void showOpenChartWindow(SDL_Renderer * renderer, AudioSystem * audioSystem) {
    if(ImGuiFileDialog::Instance()->Display("openChart", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string chartPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string chartDir = ImGuiFileDialog::Instance()->GetCurrentPath();

            loadEditWindow(renderer, audioSystem, chartPath);

            lastChartOpenDir = chartDir;

            Preferences::Instance().addMostRecentFile(chartPath);
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if(ImGui::BeginPopupModal("noSonginfoFound")) {
        ImGui::Text("No songinfo.json found in the selected chart's directory");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            startOpenChart();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("failedToOpenSonginfo")) {
        ImGui::Text("Failed to load selected songinfo.json file");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            startOpenChart();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopupModal("failedToOpenChart")) {
        ImGui::Text("Failed to load selected chart file");
        ImGui::SameLine();
        if(ImGui::Button("OK")) {
            startOpenChart();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void showInitEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer) {
    if(newEditStarted) {
        ImGui::SetNextWindowSize(newEditWindowSize);
        ImGui::Begin("New Chart", &newEditStarted, ImGuiWindowFlags_NoResize);

        showSongConfig();
        ImGui::Separator();
        showChartConfig();

        if(ImGui::Button("Create")) {
            createNewEditWindow(audioSystem, renderer);
        }

        ImGui::End();
    }

    if(ImGui::BeginPopupModal("incompleteSonginfo", &popupIncomplete)) {
        ImGui::Text("The chosen song configuration file was incomplete");
        ImGui::EndPopup();
    }
    if(ImGui::BeginPopupModal("invalidJSON", &popupInvalidJSON)) {
        ImGui::Text("Unable to read the selected JSON file");
        ImGui::EndPopup();
    }
    if(ImGui::BeginPopupModal("failedToLoadMusic", &popupFailedToLoadMusic)) {
        ImGui::Text("Unable to load the selected music");
        ImGui::EndPopup();
    }
}

void closeWindow(EditWindowData & currWindow, std::vector<EditWindowData>::iterator & iter, AudioSystem * audioSystem) {
    availableWindowIDs.push(currWindow.ID);

    audioSystem->stopMusic(currWindow.musicSourceIdx);
    audioSystem->deactivateMusicSource(currWindow.musicSourceIdx);

    iter = editWindows.erase(iter);
}

void saveCurrentChartFiles(EditWindowData & currWindow, std::string chartSavePath, std::string saveDir) {
    currWindow.songinfo.saveSonginfo(saveDir, currWindow.initialSaved);
    currWindow.chartinfo.saveChart(chartSavePath, currWindow.songpos);
    
    currWindow.initialSaved = true;
    currWindow.unsaved = false;
}

void startSaveCurrentChart(bool saveAs) {
    if(currentWindow >= 0 && currentWindow < editWindows.size()) {
        auto & editWindow = editWindows.at(currentWindow);
        if(editWindow.unsaved) {
            // save vs save as
            if(saveAs || !editWindow.initialSaved) {
                editWindow.initialSaved = false;
                ImGuiFileDialog::Instance()->OpenModal("saveChart", "Save current chart", saveFileFilter, Preferences::Instance().getSaveDir(), "Untitled.type",
                                                        1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
            } else {
                saveCurrentChartFiles(editWindow, editWindow.chartinfo.savePath, editWindow.songinfo.saveDir);
            }
        }
    }

}

bool tryCloseEditWindow(EditWindowData & currWindow, std::vector<EditWindowData>::iterator & iter, AudioSystem * audioSystem) {
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

std::pair<int, float> splitSecsbyMin(float seconds) {
    float minutes = seconds / 60;

    int fullMinutes = std::floor(minutes);
    float leftoverSecs = (minutes - fullMinutes) * 60.f;

    return std::make_pair(fullMinutes, leftoverSecs);
}

void updateAudioPosition(AudioSystem * audioSystem, SongPosition & songpos, int musicSourceIdx) {
    // udpate audio position
    if(audioSystem->isMusicPlaying(musicSourceIdx)) {
        audioSystem->startMusic(musicSourceIdx, songpos.absTime + (songpos.offsetMS / 1000.f));
    } else {
        audioSystem->setMusicPosition(musicSourceIdx, songpos.absTime  + (songpos.offsetMS / 1000.f));
    }
}

void showEditWindowMetadata(EditWindowData & currWindow) {
    // left side bar (child window) to show config info + selected entity info
    ImGui::BeginChild("configInfo", ImVec2(ImGui::GetContentRegionAvail().x * .3f, ImGui::GetContentRegionAvail().y * .35f), true);

    if(ImGui::CollapsingHeader("Song config", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Title: %s", currWindow.songinfo.title.c_str());
        ImGui::Text("Artist: %s", currWindow.songinfo.artist.c_str());
        ImGui::Text("BPM: %s", currWindow.songinfo.bpmtext.c_str());
    }

    if(ImGui::CollapsingHeader("Chart config", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Typist: %s", currWindow.chartinfo.typist.c_str());
        ImGui::Text("Keyboard: %s", currWindow.chartinfo.keyboardLayout.c_str());
        ImGui::Text("Level: %s", std::to_string(currWindow.chartinfo.level).c_str());
    }

    ImGui::EndChild();
}

void removeSection(SongPosition & songpos) {
    auto currSectionBeatpos = songpos.timeinfo.at(songpos.currentSection).beatpos;

    Timeinfo * prevSection = nullptr;

    // update following section(s) time start after adding new section
    for(auto iter = songpos.timeinfo.begin(); iter != songpos.timeinfo.end(); iter++) {
        auto & section = *iter;

        if(section.beatpos == currSectionBeatpos) {
            iter = songpos.timeinfo.erase(iter);
            if(iter != songpos.timeinfo.begin())
                iter--;

            prevSection = &(*iter);
        } else if(currSectionBeatpos < section.beatpos) {
            section.absTimeStart = section.calculateTimeStart(prevSection);
            prevSection = &section;
        }
    }

    songpos.setSongBeatPosition(songpos.absBeat);
}


static float getKeyFrequencies(void * data, int i) {
    ChartInfo * chartinfo = (ChartInfo *)data;

    if((unsigned int)i >= chartinfo->notes.keyFreqsSorted.size()) {
        return 0;
    } else {
        auto & keyData = chartinfo->notes.getKeyItemData(i);
        return keyData.second;
    }
}

static const char * getKeyFrequencyLabels(void * data, int i) {
    ChartInfo * chartinfo = (ChartInfo *)data;

    if((unsigned int)i >= chartinfo->notes.keyFreqsSorted.size()) {
        return nullptr;
    } else {
        auto & keyData = chartinfo->notes.getKeyItemData(i);
        auto & keyText = keyData.first;
    
        return keyText.c_str();
    }
}

void showEditWindowChartData(SDL_Texture * artTexture, AudioSystem * audioSystem, int * currTopNotes, 
        ChartInfo & chartinfo, SongPosition & songpos, bool & unsaved, int musicSourceIdx) {
    ImGui::BeginChild("chartData", ImVec2(0, ImGui::GetContentRegionAvail().y * .35f), true);
    
    ImGui::Image(artTexture, ImVec2(ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y));

    ImGui::SameLine();
    ImGui::BeginChild("timedata", ImVec2(ImGui::GetContentRegionAvail().x * .5f, 0), true);

    ImGui::Text("Chart Sections");
    ImGui::SameLine();
    
    // add a section
    static bool newSectionWindowOpen = false;
    static bool newSectionWindowEdit = false;

    auto currSection = songpos.timeinfo.at(songpos.currentSection);

    static int newSectionMeasure = currSection.beatpos.measure;
    static int newSectionBeatsplit = currSection.beatpos.beatsplit;
    static int newSectionSplit = currSection.beatpos.split;
    static int newSectionBeatsPerMeasure = currSection.beatsPerMeasure;
    static float newSectionBPM = currSection.bpm;
    static float newSectionInterpolateDuration = 0;

    if(ImGui::Button(ICON_FA_PLUS)) {
        newSectionWindowOpen = true;
        newSectionWindowEdit = false;

        newSectionMeasure = currSection.beatpos.measure;
        newSectionBeatsplit = currSection.beatpos.beatsplit;
        newSectionSplit = currSection.beatpos.split;
        newSectionBeatsPerMeasure = currSection.beatsPerMeasure;
        newSectionBPM = currSection.bpm;
        newSectionInterpolateDuration = 0;
    }

    ImGui::SameLine();
    // remove the selected section
    if(ImGui::Button(ICON_FA_MINUS) && songpos.timeinfo.size() > 1) {
        removeSection(songpos);
    }

    ImGui::SameLine();
    if(ImGui::Button("Edit")) {
        newSectionWindowOpen = true;
        newSectionWindowEdit = true;
        
        newSectionMeasure = currSection.beatpos.measure;
        newSectionBeatsplit = currSection.beatpos.beatsplit;
        newSectionSplit = currSection.beatpos.split;
        newSectionBeatsPerMeasure = currSection.beatsPerMeasure;
        newSectionBPM = currSection.bpm;
        newSectionInterpolateDuration = currSection.interpolateBeatDuration;
    }

    if(newSectionWindowOpen) {
        ImGui::Begin("New Section", &newSectionWindowOpen);

        static bool invalidInput = false;

        ImGui::Text("Section start");
        ImGui::SameLine();
        HelpMarker("The absolute beat start is calculated as: measure + (split / beatsplit)\n"
                   "For example, entering [5, 8, 5] means starting on the 6th measure on the\n"
                   "6th eighth note (measure, split values are 0-indexed). Assuming 4 beats\n"
                   "per measure, for the previous sections, this would be equivalent to beat 26.5\n"
                   "These 'absolute' beats are based off of the song's initial bpm");

        ImGui::InputInt("Measure", &newSectionMeasure);
        ImGui::InputInt("Beatsplit", &newSectionBeatsplit);
        ImGui::InputInt("Split", &newSectionSplit);

        newSectionMeasure = std::max(0, newSectionMeasure);
        newSectionBeatsplit = std::max(0, newSectionBeatsplit);
        newSectionSplit = std::max(0, newSectionSplit);

        ImGui::Separator();

        ImGui::InputFloat("BPM", &newSectionBPM, 1, 5, "%.1f");
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_X "2")) {
            newSectionBPM *= 2;
        }
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_DIVIDE "2")) {
            newSectionBPM /= 2;
        }

        ImGui::InputInt("Beats Per Measure", &newSectionBeatsPerMeasure);
        ImGui::InputFloat("Beat Interpolation Duration", &newSectionInterpolateDuration, 0.25, 0.5, "%.2f");

        newSectionBPM = std::max(0.f, newSectionBPM);
        newSectionInterpolateDuration = std::max(0.f, newSectionInterpolateDuration);
        newSectionBeatsPerMeasure = std::max(0, newSectionBeatsPerMeasure);

        if(ImGui::Button("OK")) {
            BeatPos newBeatpos = { newSectionMeasure, newSectionBeatsplit, newSectionSplit };

            Timeinfo * prevSection = nullptr;

            for(auto & section : songpos.timeinfo) {
                if(newBeatpos == section.beatpos && newSectionWindowEdit) {
                    prevSection = &section;
                } else if(section.beatpos < newBeatpos || section.beatpos == songpos.timeinfo.back().beatpos) {
                    prevSection = &section;
                }
                
                if(newBeatpos == section.beatpos) {
                    if(!newSectionWindowEdit) {
                        invalidInput = true;
                        ImGui::OpenPopup("Invalid input");
                        prevSection = nullptr;
                    }
                    break;
                }
            }

            if(prevSection) {
                Timeinfo newSection = Timeinfo(newBeatpos, prevSection, newSectionBeatsPerMeasure, newSectionBPM, newSectionInterpolateDuration);
                if(newSectionWindowEdit) {
                    *prevSection = newSection;
                } else {
                    songpos.timeinfo.push_back(newSection);
                }

                std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());
                prevSection = &(songpos.timeinfo.front());

                // update following section(s) time start after adding new section
                for(auto & section : songpos.timeinfo) {
                    if(*(prevSection) < section) {
                        section.absTimeStart = section.calculateTimeStart(prevSection);
                        prevSection = &section;
                    }
                }
                
                newSectionWindowOpen = false;
                songpos.setSongBeatPosition(songpos.absBeat);
                unsaved = true;
            }
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            newSectionWindowOpen = false;
        }

        if(invalidInput && ImGui::BeginPopup("Invalid input")) {
            ImGui::Text("Section already exists at the specified beat position");
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    // display section info
    if(ImGui::BeginListBox("##chartsections", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
        for(unsigned int i = 0; i < songpos.timeinfo.size(); i++) {
            Timeinfo currSection = songpos.timeinfo.at(i);

            char sectionDesc[256];
            snprintf(sectionDesc, 256, "[%d,%d,%d] : BPM: %.1f, Beats / measure: %d", currSection.beatpos.measure, currSection.beatpos.beatsplit, currSection.beatpos.split,
                     currSection.bpm, currSection.beatsPerMeasure);

            bool isSelected = i == songpos.currentSection;

            if(ImGui::Selectable(sectionDesc, &isSelected, ImGuiSelectableFlags_SelectOnClick)) {
                if(!songpos.started) {
                    songpos.start();
                    songpos.pause();

                    songpos.pauseCounter += (songpos.offsetMS / 1000.f) * SDL_GetPerformanceFrequency();
                }

                songpos.setSongBeatPosition(currSection.absBeatStart + FLT_EPSILON);
                chartinfo.notes.resetPassed(songpos.absBeat);
                updateAudioPosition(audioSystem, songpos, musicSourceIdx);
            }
        }

        ImGui::EndListBox();
    }
    
    ImGui::EndChild();

    ImGui::SameLine();

    // chart / note statistics
    ImGui::BeginChild("chartStatistics", ImVec2(0, 0), true);
    ImGui::Text("Total Notes: %d", chartinfo.notes.GetItemCount());
    ImGui::Separator();

    ImGui::Text("Key Distribution by Lane");
    ImGui::Text("Top: %d | Middle: %d | Bottom: %d", chartinfo.notes.numTopNotes, chartinfo.notes.numMidNotes, chartinfo.notes.numBotNotes);
    ImGui::Separator();

    ImGui::Text("Most frequent Keys");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(128.f);
    ImGui::SliderInt("##topNotes", currTopNotes, 0, chartinfo.notes.keyFreqsSorted.size());
    
    int maxFreq = getKeyFrequencies((void*)&chartinfo, 0);
    ImGui::PlotHistogram("##keyFreqs", getKeyFrequencies, getKeyFrequencyLabels, (void*)&chartinfo, *currTopNotes, 0, NULL, 0, maxFreq,
                         ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));

    ImGui::EndChild();

    ImGui::EndChild();
}

// toolbar info / buttons
void showEditWindowToolbar(AudioSystem * audioSystem, float * previewStart, float * previewStop, SongPosition & songpos,
                           NoteSequence & notes, std::vector<bool> & keysPressed, EditWindowData & currWindow) {
    auto musicLengthSecs = audioSystem->getMusicLength(currWindow.musicSourceIdx);

    auto songAudioPos = splitSecsbyMin(songpos.absTime);
    auto songLength = splitSecsbyMin(musicLengthSecs);
    auto songAudioPosMin = std::max(0, songAudioPos.first);
    auto songAudioPosSecs = songAudioPos.first < 0 ? 0 : songAudioPos.second;
    ImGui::Text("%02d:%05.2f/%02d:%05.2f", songAudioPosMin, songAudioPosSecs, songLength.first, songLength.second);

    ImGui::SameLine();
    if((currWindow.focused && (ImGui::Button(ICON_FA_PLAY "/" ICON_FA_PAUSE) || keysPressed[SDL_SCANCODE_SPACE])) && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId)) {
        if(songpos.paused) {
            audioSystem->resumeMusic(currWindow.musicSourceIdx);
            songpos.unpause();
        } else if(!songpos.started) {
            audioSystem->startMusic(currWindow.musicSourceIdx);
            songpos.start();
            audioSystem->setStopMusicEarly(currWindow.musicSourceIdx, false);
        } else {
            audioSystem->pauseMusic(currWindow.musicSourceIdx);
            songpos.pause();
        }
    }

    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Press [Space] to Play/Pause");

    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_STOP)) {
        audioSystem->stopMusic(currWindow.musicSourceIdx);
        songpos.stop();
        notes.resetPassed(songpos.absBeat);
    }

    if(songpos.absTime > musicLengthSecs) {
        songpos.absTime = musicLengthSecs;
        songpos.started = false;
    }

    if(audioSystem->getStopMusicEarly(currWindow.musicSourceIdx) && songpos.absTime > audioSystem->getMusicStop(currWindow.musicSourceIdx)) {
        songpos.absTime = audioSystem->getMusicStop(currWindow.musicSourceIdx);
        songpos.started = false;
    }

    // allow user to play / set preview
    float sliderWidth = ImGui::GetContentRegionAvail().x * 0.25;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(sliderWidth);
    if(ImGui::SliderFloat("##prevstart", previewStart, 0.f, musicLengthSecs, "%05.2f")) {
        currWindow.unsaved = true;
    }
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Music preview start");

    // clamp to max prevStop, min 0
    if(*previewStart > *previewStop) {
        *previewStart = *previewStop;
    }

    if(*previewStart < 0) {
        *previewStart = 0;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(sliderWidth);
    if(ImGui::SliderFloat("##prevstop", previewStop, 0.f, musicLengthSecs, "%05.2f")) {
        currWindow.unsaved = true;
    }
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Music preview stop");

    // clamp to min prevStart, max song length
    if(*previewStop < *previewStart) {
        *previewStop = *previewStart;
    }
    if(*previewStop > musicLengthSecs) {
        *previewStop = musicLengthSecs;
    }

    ImGui::SameLine();
    if(ImGui::Button("Preview " ICON_FA_TRAILER)) {
        audioSystem->stopMusic(currWindow.musicSourceIdx);
        audioSystem->startMusic(currWindow.musicSourceIdx, *previewStart);
        audioSystem->setMusicStop(currWindow.musicSourceIdx, *previewStop);

        audioSystem->setStopMusicEarly(currWindow.musicSourceIdx, true);

        if(!songpos.started)
            songpos.start();

        songpos.setSongTimePosition(*previewStart);

        if(songpos.paused)
            songpos.unpause();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2.f);
    if(ImGui::SliderInt("Offset (ms)", &songpos.offsetMS, -1000, 1000)) {
        currWindow.unsaved = true;
    }
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Offset from the first beat in milliseconds\n(Ctrl + Click to enter)");
}

const std::unordered_map<int, std::string> FUNCTION_KEY_COMBO_ITEMS = {
    {0, "L" ICON_FA_ARROW_UP },
    {1, "R" ICON_FA_ARROW_UP },
    {2, ICON_FA_ARROW_UP },
    {3, ICON_FA_ARROW_LEFT_LONG },
    {4, "_" },
};

const std::unordered_map<int, SDL_Scancode> FUNCTION_KEY_COMBO_SCANCODES = {
    {0, SDL_SCANCODE_LSHIFT },
    {1, SDL_SCANCODE_RSHIFT },
    {2, SDL_SCANCODE_CAPSLOCK },
    {3, SDL_SCANCODE_RETURN },
    {4, SDL_SCANCODE_SPACE },
};

int filterInputMiddleKey(ImGuiInputTextCallbackData * data) {
    std::string keyboardLayout;
    if(data->UserData) {
        keyboardLayout = static_cast<char*>(data->UserData);
    }

    auto c = data->EventChar;
    bool validChar = MIDDLE_ROW_KEYS.find(keyboardLayout) != MIDDLE_ROW_KEYS.end() &&
                     MIDDLE_ROW_KEYS.at(keyboardLayout).find(c) != MIDDLE_ROW_KEYS.at(keyboardLayout).end();

    return validChar ? 0 : 1;
}

BeatPos calculateBeatpos(float absBeat, int currentBeatsplit, const std::vector<Timeinfo> & timeinfo) {
    int measure = 0;
    int measureSplit = 0;
    int split = 0;

    int prevBeatsPerMeasure = 0;
    float prevSectionAbsBeat = 0.f;

    unsigned int i = 0;
    for(const auto & time : timeinfo) {
        // track current measure
        if(absBeat >= time.absBeatStart && i > 0) {
            float prevSectionMeasures = (time.absBeatStart - prevSectionAbsBeat) / prevBeatsPerMeasure;
            int prevSectionMeasuresFull = std::floor(prevSectionMeasures);

            measure += prevSectionMeasuresFull;
        }

        // calculate the leftover beats
        bool isLastSection = i == timeinfo.size() - 1;
        bool beatInPrevSection = absBeat < time.absBeatStart; 
        if (beatInPrevSection || isLastSection) {
            int currBeatsPerMeasure = beatInPrevSection ? prevBeatsPerMeasure : time.beatsPerMeasure;
            float currAbsBeat = beatInPrevSection ? prevSectionAbsBeat : time.absBeatStart;

            float leftoverMeasures = (absBeat - currAbsBeat) / currBeatsPerMeasure;
            int leftoverMeasuresFull = std::floor(leftoverMeasures);
            float leftoverBeats = (leftoverMeasures - leftoverMeasuresFull) * currBeatsPerMeasure;
            int leftoverBeatsplits = (int)(leftoverBeats * currentBeatsplit + 0.5);

            measure += leftoverMeasuresFull;
            measureSplit = currentBeatsplit * currBeatsPerMeasure;
            split = leftoverBeatsplits;
            break;            
        }

        prevSectionAbsBeat = time.absBeatStart;
        prevBeatsPerMeasure = time.beatsPerMeasure;
        i++;
    }

    return (BeatPos){measure, measureSplit, split};
}

void showEditWindowTimeline(AudioSystem * audioSystem, ChartInfo & chartinfo, SongPosition & songpos, bool & unsaved, std::vector<bool> & keysPressed,
    std::stack<std::shared_ptr<EditAction>> & editActionsUndo, std::stack<std::shared_ptr<EditAction>> & editActionsRedo,
    int musicSourceIdx, bool windowFocused)
{
    // let's create the sequencer
    static int currentBeatsplit = 4;
    static int clickedItemType = 0;
    static int releasedItemType = 0;
    static bool updatedBeat = false;
    static float clickedBeat = 0.f;
    static float hoveredBeat = 0.f;
    static float timelineZoom = 2.f;

    ImGui::PushItemWidth(130);
    ImGui::Text("Beatsplit: ");
    ImGui::SameLine();
    ImGui::InputInt("##beatsplit", &currentBeatsplit, 1, 4);
    currentBeatsplit = std::max(1, currentBeatsplit);
    float currentBeatsplitValue = 1.f / currentBeatsplit;
    
    ImGui::SameLine();
    ImGui::Text("Current beat: ");
    ImGui::SameLine();

    int fullBeats = std::floor(songpos.absBeat);
    int fullBeatSplits = (int)((songpos.absBeat - fullBeats) / currentBeatsplitValue + 0.5);
    float origNearBeat = fullBeats + (fullBeatSplits * (1.f / currentBeatsplit));
    float prevTargetBeat = (songpos.absBeat == origNearBeat) ? origNearBeat - currentBeatsplitValue : origNearBeat;
    float nextTargetBeat = origNearBeat + (1.f / currentBeatsplit);

    if(ImGui::InputFloat("##currbeat", &songpos.absBeat, currentBeatsplitValue, 2.f / currentBeatsplit)) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();
            songpos.pauseCounter += (songpos.offsetMS / 1000.f) * SDL_GetPerformanceFrequency();
        }

        // snap to the nearest beat split
        if(songpos.absBeat > nextTargetBeat) {
            songpos.setSongBeatPosition(nextTargetBeat);
        } else if(songpos.absBeat < prevTargetBeat) {
            songpos.setSongBeatPosition(prevTargetBeat);
        }

        if(songpos.absTime >= 0) {
            updateAudioPosition(audioSystem, songpos, musicSourceIdx);
        } else {
            songpos.setSongBeatPosition(0);
        }

        chartinfo.notes.resetPassed(songpos.absBeat);
    }

    static float zoomStep = 0.25f;

    auto currBeatpos = calculateBeatpos(songpos.absBeat, currentBeatsplit, songpos.timeinfo);
    ImGui::SameLine();
    ImGui::Text("[Pos]: [%d, %d, %d]", std::max(0, currBeatpos.measure), std::max(0, currBeatpos.beatsplit), std::max(0, currBeatpos.split));

    ImGui::SameLine();
    ImGui::Text("Zoom " ICON_FA_MAGNIFYING_GLASS ": ");
    ImGui::SameLine();
    ImGui::InputFloat("##zoom", &timelineZoom, zoomStep, zoomStep * 2, "%.2f");
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Hold [Ctrl] while scrolling to zoom in/out");

    ImGui::PopItemWidth();

    // ctrl + scroll to adjust zoom
    auto & io = ImGui::GetIO();
    if(windowFocused && !ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && io.KeyCtrl && !io.KeyShift) {
        int multiplier = io.MouseWheel > 0 ? 1 : -1;

        timelineZoom += zoomStep * multiplier;
    }

    if(windowFocused && timelineZoom < zoomStep * 2) {
        timelineZoom = zoomStep * 2;
    }

    static bool leftClickedEntity = false;
    static bool leftClickReleased = false;
    static bool leftClickShift = false;
    static bool haveSelection = false;
    bool rightClickedEntity = false;

    int beatsPerMeasure = songpos.timeinfo.size() > songpos.currentSection ? songpos.timeinfo.at(songpos.currentSection).beatsPerMeasure : 4;
    Sequencer(&(chartinfo.notes), timelineZoom, currentBeatsplit, beatsPerMeasure, Preferences::Instance().isDarkTheme(), haveSelection, windowFocused,
              nullptr, &updatedBeat, &leftClickedEntity, &leftClickReleased, &leftClickShift, &rightClickedEntity, &clickedBeat, &hoveredBeat,
              &clickedItemType, &releasedItemType, nullptr, &songpos.absBeat, ImSequencer::SEQUENCER_CHANGE_FRAME);

    if(windowFocused && updatedBeat) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();

            songpos.pauseCounter += (songpos.offsetMS / 1000.f) * SDL_GetPerformanceFrequency();
        }
        songpos.setSongBeatPosition(songpos.absBeat);

        if(songpos.absTime >= 0) {
            updateAudioPosition(audioSystem, songpos, musicSourceIdx);
        } else {
            songpos.setSongBeatPosition(0);
        }

        chartinfo.notes.resetPassed(songpos.absBeat);
    }

    char addItemPopup[16];
    snprintf(addItemPopup, 16, "add_item_%d", musicSourceIdx);

    // insert or update entity at the clicked beat
    static char addedItem[2];
    static float insertBeat;
    static BeatPos insertBeatpos;
    static int insertItemType = 0;
    static bool startedNote = false;
    static ImGuiInputTextFlags addItemFlags = 0;
    static ImGuiInputTextCallbackData addItemCallbackData;
    if(windowFocused && !ImGuiFileDialog::Instance()->IsOpened() && leftClickedEntity && !ImGui::IsPopupOpen(addItemPopup)) {
        insertBeat = clickedBeat;
        insertItemType = clickedItemType;
        haveSelection = false;

        insertBeatpos = calculateBeatpos(clickedBeat, currentBeatsplit, songpos.timeinfo);
        
        addItemFlags = ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue;

        switch (insertItemType) {
            case SequencerItemType::TOP_NOTE:
                addItemFlags |= ImGuiInputTextFlags_CharsDecimal;
                break;                
            case SequencerItemType::MID_NOTE:
                addItemFlags |= ImGuiInputTextFlags_CallbackCharFilter;
                break;
            case SequencerItemType::BOT_NOTE:
                addItemFlags = 0;
                break;
            default:
                break;
        }

        leftClickedEntity = false;
        startedNote = true;
    }

    static int insertItemTypeEnd = 0;
    static float endBeat;
    static BeatPos endBeatpos;
    if(windowFocused && !ImGuiFileDialog::Instance()->IsOpened() && startedNote && leftClickReleased &&
       !ImGui::IsPopupOpen(addItemPopup) && clickedBeat >= insertBeat) {
        if(leftClickShift) {
            haveSelection = true;

            if(releasedItemType < insertItemType) {
                auto tmp = insertItemType;
                insertItemType = releasedItemType;
                insertItemTypeEnd = tmp;
            } else {
                insertItemTypeEnd = releasedItemType;
            }
        } else {
            ImGui::OpenPopup(addItemPopup);
        }
        
        endBeat = clickedBeat;
        endBeatpos = calculateBeatpos(endBeat, currentBeatsplit, songpos.timeinfo); 

        leftClickReleased = false;
        leftClickShift = false;
        startedNote = false;
    }

    // copy, cut, delete selection of notes
    static std::vector<std::shared_ptr<NoteSequenceItem>> copiedItems;
    if(windowFocused && haveSelection) {
        if(activateCopy || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_c)])) {
            copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            haveSelection = false;
            activateCopy = false;
        } else if(activateCut || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_x)])) {
            copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

            if(!copiedItems.empty()) {
                auto delAction = std::make_shared<DeleteItemsAction>(unsaved, currentBeatsplit, insertItemType, insertItemTypeEnd, insertBeat, endBeat, copiedItems);
                editActionsUndo.push(delAction);
                emptyActionStack(editActionsRedo);

                unsaved = true;
            }

            haveSelection = false;
            activateCut = false;
        }

        // flip selected notes
        if(activateFlip || keysPressed[SDL_GetScancodeFromKey(SDLK_f)]) {
            auto flipAction = std::make_shared<FlipNoteAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat,
                chartinfo.keyboardLayout);
            editActionsUndo.push(flipAction);
            emptyActionStack(editActionsRedo);

            chartinfo.notes.flipNotes(chartinfo.keyboardLayout, insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            unsaved = true;
            activateFlip = false;
        }

        if(keysPressed[SDL_SCANCODE_DELETE]) {
            auto deletedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

            if(!deletedItems.empty()) {
                auto delAction = std::make_shared<DeleteItemsAction>(unsaved, currentBeatsplit, insertItemType, insertItemTypeEnd, insertBeat, endBeat, deletedItems);
                editActionsUndo.push(delAction);
                emptyActionStack(editActionsRedo);

                unsaved = true;
            }

            haveSelection = false;
        }

        if(keysPressed[SDL_SCANCODE_ESCAPE]) {
            haveSelection = false;
        }
    }

    if(windowFocused && (activatePaste || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_v)]))) {
        if(!copiedItems.empty()) {
            auto overwrittenItems = chartinfo.notes.getItems(hoveredBeat, hoveredBeat + (copiedItems.back()->absBeat - copiedItems.front()->absBeat), insertItemType, insertItemTypeEnd);
            chartinfo.notes.insertItems(hoveredBeat, songpos.absBeat, currentBeatsplit, insertItemType, insertItemTypeEnd, songpos.timeinfo, copiedItems);

            auto insAction = std::make_shared<InsertItemsAction>(unsaved, currentBeatsplit, insertItemType, insertItemTypeEnd, hoveredBeat, copiedItems, overwrittenItems);
            editActionsUndo.push(insAction);
            emptyActionStack(editActionsRedo);

            unsaved = true;
        }

        activatePaste = false;
    }

    if(ImGui::BeginPopup(addItemPopup)) {
        if(keysPressed[SDL_SCANCODE_ESCAPE]) {
            addedItem[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        switch(insertItemType) {
            case SequencerItemType::TOP_NOTE:
            case SequencerItemType::MID_NOTE:
                ImGui::SetNextItemWidth(32);
                if(!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
                    ImGui::SetKeyboardFocusHere(0);

                if(ImGui::InputText("##addnote_text", addedItem, 2, addItemFlags, filterInputMiddleKey, (void *)chartinfo.keyboardLayout.c_str())) {
                    if(addedItem[0] != '\0') {
                        std::string keyText(addedItem);

                        std::shared_ptr<EditAction> currAction;
                        auto foundItem = chartinfo.notes.containsItemAt(insertBeat, insertItemType);

                        if(foundItem.get()) {
                            currAction = std::make_shared<EditNoteAction>(unsaved, insertBeat, (SequencerItemType)insertItemType, foundItem->displayText, keyText);
                            chartinfo.notes.editNote(insertBeat, insertItemType, keyText);
                        } else {
                            chartinfo.notes.addNote(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos,
                                (SequencerItemType)insertItemType, keyText);
                            currAction = std::make_shared<PlaceNoteAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (SequencerItemType)insertItemType, keyText);
                        }

                        editActionsUndo.push(currAction);
                        emptyActionStack(editActionsRedo);

                        addedItem[0] = '\0';
                        ImGui::CloseCurrentPopup();
                        unsaved = true;
                    }
                }
                break;
            case SequencerItemType::BOT_NOTE:
                static int selectedFuncKey = 0;
                static bool insertKey = false;
                ImGui::SetNextItemWidth(64);

                if(ImGui::BeginCombo("##addfunction_key", FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey).c_str())) {
                    for(auto & [keyIdx, keyTxt] : FUNCTION_KEY_COMBO_ITEMS) {
                        bool keySelected = false;
                        if(ImGui::Selectable(keyTxt.c_str(), &keySelected)) {
                            selectedFuncKey = keyIdx;
                            insertKey = true;
                            break;
                        }

                        if(keySelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                for(auto & [keyIdx, keyText] : FUNCTION_KEY_COMBO_ITEMS) {
                    if(keysPressed[FUNCTION_KEY_COMBO_SCANCODES.at(keyIdx)]) {                        
                        selectedFuncKey = keyIdx;
                        insertKey = true;
                        break;
                    }
                }

                if(insertKey) {
                    std::string keyText = FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey);

                    std::shared_ptr<EditAction> currAction;
                    auto foundItem = chartinfo.notes.containsItemAt(insertBeat, insertItemType);
                    if(foundItem.get()) {
                        currAction = std::make_shared<EditNoteAction>(unsaved, insertBeat, (SequencerItemType)insertItemType, foundItem->displayText, keyText);
                        chartinfo.notes.editNote(insertBeat, insertItemType, keyText);
                    } else {
                        chartinfo.notes.addNote(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (SequencerItemType)insertItemType, keyText);
                        currAction = std::make_shared<PlaceNoteAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (SequencerItemType)insertItemType, keyText);
                    }

                    editActionsUndo.push(currAction);
                    emptyActionStack(editActionsRedo);

                    selectedFuncKey = 0;
                    ImGui::CloseCurrentPopup();
                    unsaved = true;
                    insertKey = false;
                    leftClickReleased = false;
                }
                break;
            case SequencerItemType::SKIP:
                {
                    if(!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
                        ImGui::SetKeyboardFocusHere(0);

                    ImGui::SetNextItemWidth(128);

                    static float skipBeats = 0;
                    ImGui::InputFloat("Skip beats", &skipBeats, currentBeatsplitValue / 2.f, currentBeatsplitValue / 2.f);
                    ImGui::SameLine();
                    HelpMarker("How many beats will the skip be displayed for?\n- 0 -> the skip is instant\n"
                            "- skip_duration / 2 -> the skip will take half the time as without the skip\n"
                            "- skip_duration -> will look the same as no skip");

                    if(skipBeats < 0.f)
                        skipBeats = 0;

                    auto currItem = chartinfo.notes.containsItemAt(insertBeat, insertItemType);
                    if(currItem) {
                        auto currSkip = std::dynamic_pointer_cast<Skip>(currItem);
                        if(skipBeats > currSkip->beatDuration)
                            skipBeats = currSkip->beatDuration;
                    } else {
                        if(skipBeats > endBeat - insertBeat)
                            skipBeats = endBeat - insertBeat;
                    }

                    if(keysPressed[SDL_SCANCODE_RETURN] || keysPressed[SDL_SCANCODE_KP_ENTER]) {
                        std::shared_ptr<EditAction> currAction;
                        if(currItem.get()) {
                            auto currSkip = std::dynamic_pointer_cast<Skip>(currItem);
                            currAction = std::make_shared<EditSkipAction>(unsaved, insertBeat, currSkip->skipTime, skipBeats);
                            chartinfo.notes.editSkip(insertBeat, skipBeats);
                        } else {
                            auto newSkip = chartinfo.notes.addSkip(insertBeat, songpos.absBeat, skipBeats, endBeat - insertBeat, insertBeatpos, endBeatpos);
                            currAction = std::make_shared<PlaceSkipAction>(unsaved, insertBeat, skipBeats, endBeat - insertBeat, insertBeatpos, endBeatpos);
                        
                            songpos.addSkip(newSkip);
                        }

                        editActionsUndo.push(currAction);
                        emptyActionStack(editActionsRedo);

                        ImGui::CloseCurrentPopup();
                        unsaved = true;

                        leftClickReleased = false;
                    }
                }
                break;
            case SequencerItemType::STOP:
                chartinfo.notes.addStop(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);

                auto putAction = std::make_shared<PlaceStopAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);
                editActionsUndo.push(putAction);
                emptyActionStack(editActionsRedo);

                ImGui::CloseCurrentPopup();
                unsaved = true;

                break;
        }
    
        ImGui::EndPopup();
    }

    if(windowFocused && !ImGuiFileDialog::Instance()->IsOpened() && rightClickedEntity) {
        auto itemToDelete = chartinfo.notes.containsItemAt(clickedBeat, clickedItemType);
        if(itemToDelete.get()) {
            chartinfo.notes.deleteItem(clickedBeat, clickedItemType);
            auto deleteAction = std::make_shared<DeleteNoteAction>(unsaved, itemToDelete->absBeat, itemToDelete->beatEnd - itemToDelete->absBeat,
                                                                   itemToDelete->beatpos, itemToDelete->endBeatpos, itemToDelete->getItemType(), itemToDelete->displayText);
            editActionsUndo.push(deleteAction);
            emptyActionStack(editActionsRedo);

            unsaved = true;

            if(itemToDelete->getItemType() == SequencerItemType::SKIP) {
                songpos.removeSkip(clickedBeat);
            }
        }
    }

    if(windowFocused)
        chartinfo.notes.update(songpos.absBeat, audioSystem, Preferences::Instance().isNotesoundEnabled());

    // sideways scroll
    if(windowFocused && !ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && !io.KeyCtrl) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();

            // decrease pause counter manually by offset, since skipping
            songpos.pauseCounter += (songpos.offsetMS / 1000.f) * SDL_GetPerformanceFrequency();
        }

        bool decrease = io.MouseWheel > 0;
        io.MouseWheel *= (2.f / timelineZoom);
        int beatsplitChange = std::lround(io.MouseWheel);
        if(beatsplitChange == 0)
            beatsplitChange = decrease ? 1 : -1;

        int fullBeats = std::floor(songpos.absBeat);
        int fullBeatSplits = (int)((songpos.absBeat - fullBeats) / currentBeatsplitValue + 0.5);
        float origNearBeat = fullBeats + (fullBeatSplits * currentBeatsplitValue);

        float targetBeat;
        if(decrease && songpos.absBeat > origNearBeat) {
            targetBeat = origNearBeat - (beatsplitChange - 1) * currentBeatsplitValue;
        } else {
            targetBeat = origNearBeat - beatsplitChange * currentBeatsplitValue;
        }

        if(decrease || (!decrease && songpos.absTime < audioSystem->getMusicLength(musicSourceIdx))) {
            // scroll up, decrease beat, scroll down increase beat

            //songpos.setSongBeatPosition(songpos.absBeat - (beatsplitChange * currentBeatsplitValue));
            // clamp to nearest split
            songpos.setSongBeatPosition(targetBeat);
        }

        if(songpos.absTime >= 0) {
            updateAudioPosition(audioSystem, songpos, musicSourceIdx);
        } else {
            audioSystem->stopMusic(musicSourceIdx);
            songpos.stop();
        }

        chartinfo.notes.resetPassed(songpos.absBeat);
    }
}

void showEditWindows(AudioSystem * audioSystem, std::vector<bool> & keysPressed) {
    static bool updatedName = false;
    static ImVec2 sizeBeforeUpdate;

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
        ImVec2 currWindowSize = ImGui::GetWindowSize();

        if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            currentWindow = i;
            currWindow.focused = true;
        } else {
            currWindow.focused = false;
        }

        showEditWindowMetadata(currWindow);
        ImGui::SameLine();
        showEditWindowChartData(currWindow.artTexture.get(), audioSystem, &currWindow.currTopNotes, currWindow.chartinfo, currWindow.songpos,
            currWindow.unsaved, currWindow.musicSourceIdx);

        ImGui::Separator();
        showEditWindowToolbar(audioSystem, &(currWindow.songinfo.musicPreviewStart), &(currWindow.songinfo.musicPreviewStop), currWindow.songpos, 
                              currWindow.chartinfo.notes, keysPressed, currWindow);
        ImGui::Separator();

        showEditWindowTimeline(audioSystem, currWindow.chartinfo, currWindow.songpos, currWindow.unsaved, keysPressed,
                               currWindow.editActionsUndo, currWindow.editActionsRedo, currWindow.musicSourceIdx, currWindow.focused);

        ImGui::End();

        bool closedCurrwindow = false;
        if(!currWindow.open) {
            closedCurrwindow = tryCloseEditWindow(currWindow, iter, audioSystem);
        }

        if(ImGuiFileDialog::Instance()->Display("saveChart", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string chartSavePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string chartSaveFilename = ImGuiFileDialog::Instance()->GetCurrentFileName();
                std::string saveDir = ImGuiFileDialog::Instance()->GetCurrentPath();

                saveCurrentChartFiles(currWindow, chartSavePath, saveDir);
                currWindow.name = chartSaveFilename;

                updatedName = true;
                sizeBeforeUpdate = currWindowSize;

                Preferences::Instance().addMostRecentFile(chartSavePath);
                lastChartSaveDir = saveDir;
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if(!closedCurrwindow) {
            i++;
            iter++;
        }
    }

    if(activateUndo) {
        if(currentWindow < editWindows.size() && !editWindows.at(currentWindow).editActionsUndo.empty()) {
            auto action = editWindows.at(currentWindow).editActionsUndo.top();
            action->undoAction(&editWindows.at(currentWindow));

            editWindows.at(currentWindow).editActionsRedo.push(action);
            editWindows.at(currentWindow).editActionsUndo.pop();
        }

        activateUndo = false;
    }

    if(activateRedo) {
        if(currentWindow < editWindows.size() && !editWindows.at(currentWindow).editActionsRedo.empty()) {
            auto action = editWindows.at(currentWindow).editActionsRedo.top();
            action->redoAction(&editWindows.at(currentWindow));

            editWindows.at(currentWindow).editActionsUndo.push(action);
            editWindows.at(currentWindow).editActionsRedo.pop();
            editWindows.at(currentWindow).unsaved = true;
        }

        activateRedo = false;
    }
}
