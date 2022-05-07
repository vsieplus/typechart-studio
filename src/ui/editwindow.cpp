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

#include "ui/preferences.hpp"
#include "ui/editwindow.hpp"
#include "ui/windowsizes.hpp"

#include "systems/audiosystem.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

const std::map<int, std::string> ID_TO_KEYBOARDLAYOUT = {
    { 0 , "QWERTY" },
    { 1, "DVORAK" },
    { 2, "AZERTY" },
    { 3, "COLEMAK" }
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

static float UImusicPreviewStart = 0;
static float UImusicPreviewStop = 15;

void initLastDirPaths() {
    lastOpenResourceDir = Preferences::Instance().getInputDir();
    lastChartOpenDir = Preferences::Instance().getSaveDir();
    lastChartSaveDir = Preferences::Instance().getSaveDir();
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
        ImGuiFileDialog::Instance()->OpenModal("selectSonginfo", "Select songinfo.json", songinfoFileFilter, lastOpenResourceDir, "");
    }
    
    ImGui::SameLine();
    HelpMarker("Load song data from a pre-existing songinfo.json file");

    // songinfo dialog
    if(ImGuiFileDialog::Instance()->Display("selectSonginfo", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string songinfoPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string songinfoDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            loadSonginfo(songinfoPath, songinfoDir);

            lastOpenResourceDir = songinfoDir;
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

void showChartConfig() {
    ImGui::Text("Chart configuration");

    ImGui::InputText(ICON_FA_PENCIL " Typist", UItypist, 64);
    ImGui::Combo(ICON_FA_KEYBOARD " Keyboard", &UIkeyboardLayout, "QWERTY\0DVORAK\0AZERTY\0COLEMAK\0");
    ImGui::SameLine();
    HelpMarker("Choose the keyboard layout that this chart is\n"
                "intended to be played with. Charts will then be\n"
                "accordingly 'translated' to other keyboard layouts\n"
                "when loaded into Typing Tempo.");
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
    ChartInfo chartinfo = ChartInfo(UIlevel, UItypist, ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout));

    // attempt to load music
    if(!audioSystem->loadMusic(UImusicFilepath)) {
        ImGui::OpenPopup("failedToLoadMusic");
        popupFailedToLoadMusic = true;
        return;
    } else {
        popupFailedToLoadMusic = false;

        auto newWindowData = getNextWindowNameAndID();
        auto windowName = newWindowData.first;
        auto windowID = newWindowData.second;

        auto artTexture = Texture::loadTexture(UIcoverArtFilepath, renderer);

        EditWindowData newWindow = EditWindowData(true, windowID, windowName, artTexture, chartinfo, songinfo);

        // initial section from BPM
        float initialBpm = ::atof(UIbpmtext);
        BeatPos initialSectionStart = {0, 1, 0};
        newWindow.songpos.timeinfo.push_back(Timeinfo(initialSectionStart, nullptr, 4, initialBpm));
    
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

    if(!audioSystem->loadMusic(UImusicFilepath)) {
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

    EditWindowData newWindow = EditWindowData(true, windowID, windowName, artTexture, chartinfo, songinfo);
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
    iter = editWindows.erase(iter);

    audioSystem->stopMusic();
}

void saveCurrentChartFiles(EditWindowData & currWindow, std::string chartSavePath, std::string saveDir) {
    if(!currWindow.initialSaved && chartSavePath != currWindow.chartinfo.savePath) {
        currWindow.songinfo.saveSonginfo(saveDir);
    }
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

void tryCloseEditWindow(EditWindowData & currWindow, std::vector<EditWindowData>::iterator & iter, AudioSystem * audioSystem) {
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
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            currWindow.open = true;
        }

        ImGui::End();
    } else {
        closeWindow(currWindow, iter, audioSystem);
    }
}

std::pair<int, float> splitSecsbyMin(float seconds) {
    float minutes = seconds / 60;

    int fullMinutes = std::floor(minutes);
    float leftoverSecs = (minutes - fullMinutes) * 60.f;

    return std::make_pair(fullMinutes, leftoverSecs);
}

void updateAudioPosition(AudioSystem * audioSystem, SongPosition & songpos) {
    // udpate audio position
    if(audioSystem->isMusicPlaying()) {
        audioSystem->startMusic(songpos.absTime);
    } else {
        audioSystem->setMusicPosition(songpos.absTime);
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

void showEditWindowChartData(SDL_Texture * artTexture, AudioSystem * audioSystem, SongPosition & songpos, bool & unsaved) {
    ImGui::BeginChild("chartData", ImVec2(0, ImGui::GetContentRegionAvail().y * .35f), true);
    
    ImGui::Image(artTexture, ImVec2(ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y));

    ImGui::SameLine();
    ImGui::BeginChild("timedata");

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

    if(ImGui::Button(ICON_FA_PLUS)) {
        newSectionWindowOpen = true;
        newSectionWindowEdit = false;

        newSectionMeasure = currSection.beatpos.measure;
        newSectionBeatsplit = currSection.beatpos.beatsplit;
        newSectionSplit = currSection.beatpos.split;
        newSectionBeatsPerMeasure = currSection.beatsPerMeasure;
        newSectionBPM = currSection.bpm;
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

        newSectionBPM = std::max(0.f, newSectionBPM);
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
                Timeinfo newSection = Timeinfo(newBeatpos, prevSection, newSectionBeatsPerMeasure, newSectionBPM);
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
    if(ImGui::BeginListBox("##chartsections", ImVec2(ImGui::GetContentRegionAvail().x / 2.f, ImGui::GetContentRegionAvail().y))) {
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
                }

                songpos.setSongBeatPosition(currSection.absBeatStart + FLT_EPSILON);
                updateAudioPosition(audioSystem, songpos);
            }
        }

        ImGui::EndListBox();
    }
    
    ImGui::EndChild();

    ImGui::SameLine();

    // pane to edit the selected entity
    ImGui::BeginChild("selectedData", ImVec2(0, 0), true);
    ImGui::Text("Currently Selected: ");

    

    ImGui::EndChild();

    ImGui::EndChild();
}

// toolbar info / buttons
void showEditWindowToolbar(AudioSystem * audioSystem, float * previewStart, float * previewStop, SongPosition & songpos,
                           NoteSequence & notes, std::vector<bool> & keysPressed) {
    auto musicLengthSecs = audioSystem->getMusicLength();

    auto songAudioPos = splitSecsbyMin(songpos.absTime);
    auto songLength = splitSecsbyMin(musicLengthSecs);
    ImGui::Text("%02d:%05.2f/%02d:%05.2f", songAudioPos.first, songAudioPos.second, songLength.first, songLength.second);

    ImGui::SameLine();
    if((ImGui::Button(ICON_FA_PLAY "/" ICON_FA_PAUSE) || keysPressed[SDL_SCANCODE_SPACE]) && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId)) {
        if(songpos.paused) {
            audioSystem->resumeMusic();
            songpos.unpause();
        } else if(!songpos.started) {
            audioSystem->startMusic();
            songpos.start();
            audioSystem->setStopMusicEarly(false);
        } else {
            audioSystem->pauseMusic();
            songpos.pause();
        }
    }

    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Press [Space] to Play/Pause");

    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_STOP)) {
        audioSystem->stopMusic();
        songpos.stop();
        notes.resetPassed(songpos.absBeat, songpos.currSpb);
    }

    if(songpos.absTime > musicLengthSecs) {
        songpos.absTime = musicLengthSecs;
        songpos.started = false;
    }

    if(audioSystem->getStopMusicEarly() && songpos.absTime > audioSystem->getMusicStop()) {
        songpos.absTime = audioSystem->getMusicStop();
        songpos.started = false;
    }

    // allow user to play / set preview
    float sliderWidth = ImGui::GetContentRegionAvail().x * 0.25;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("##prevstart", previewStart, 0.f, musicLengthSecs, "%05.2f");
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
    ImGui::SliderFloat("##prevstop", previewStop, 0.f, musicLengthSecs, "%05.2f");
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
        audioSystem->stopMusic();
        audioSystem->startMusic(*previewStart);
        audioSystem->setMusicStop(*previewStop);

        audioSystem->setStopMusicEarly(true);
        
        if(!songpos.started)
            songpos.start();

        songpos.setSongTimePosition(*previewStart);

        if(songpos.paused)
            songpos.unpause();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2.f);
    ImGui::SliderInt("Offset (ms)", &songpos.offsetMS, 0, 1000);
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Offset from the first beat in milliseconds\n(Automatically +100ms when saved)\n(Ctrl + Click to enter)");
}

const std::unordered_map<int, std::string> FUNCTION_KEY_COMBO_ITEMS = {
    {0, "L" ICON_FA_ARROW_UP },
    {1, "R" ICON_FA_ARROW_UP },
    {2, ICON_FA_ARROW_UP },
    {3, ICON_FA_ARROW_LEFT_LONG },
    {4, "_" },
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
        if (absBeat < time.absBeatStart || i == timeinfo.size() - 1) {
            int currBeatsPerMeasure = (i == timeinfo.size() - 1) ? time.beatsPerMeasure : prevBeatsPerMeasure;

            float leftoverMeasures = (absBeat - prevSectionAbsBeat) / currBeatsPerMeasure;
            int leftoverMeasuresFull = std::floor(leftoverMeasures);
            float leftoverBeats = (leftoverMeasures - leftoverMeasuresFull) * currBeatsPerMeasure;
            int leftoverBeatsplits = (int)(leftoverBeats * currentBeatsplit);

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

void showEditWindowTimeline(AudioSystem * audioSystem, ChartInfo & chartinfo, SongPosition & songpos, bool & unsaved, std::vector<bool> & keysPressed) {
    // let's create the sequencer
    static int currentBeatsplit = 4;
    static int clickedItemType = 0;
    static bool updatedBeat = false;
    static float clickedBeat = 0.f;
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
    float fullBeatSplits = std::floor((songpos.absBeat - fullBeats) / currentBeatsplitValue);
    float origNearBeat = fullBeats + (fullBeatSplits * (1.f / currentBeatsplit));
    float prevTargetBeat = (songpos.absBeat == origNearBeat) ? origNearBeat - currentBeatsplitValue : origNearBeat;
    float nextTargetBeat = origNearBeat + (1.f / currentBeatsplit);

    if(ImGui::InputFloat("##currbeat", &songpos.absBeat, currentBeatsplitValue, 2.f / currentBeatsplit)) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();
        }

        songpos.setSongBeatPosition(songpos.absBeat);

        // snap to the nearest beat split
        if(songpos.absBeat > nextTargetBeat) {
            songpos.setSongBeatPosition(nextTargetBeat);
        } else if(songpos.absBeat < prevTargetBeat) {
            songpos.setSongBeatPosition(prevTargetBeat);
        }

        if(songpos.absTime >= 0) {
            updateAudioPosition(audioSystem, songpos);
        } else {
            songpos.setSongBeatPosition(0);
        }

        chartinfo.notes.resetPassed(songpos.absBeat, songpos.currSpb);
    }

    static float zoomStep = 0.25f;

    ImGui::SameLine();
    ImGui::Text("Zoom " ICON_FA_MAGNIFYING_GLASS ": ");
    ImGui::SameLine();
    ImGui::InputFloat("##zoom", &timelineZoom, zoomStep, zoomStep * 2, "%.2f");
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Hold [Ctrl] while scrolling to zoom in/out");

    ImGui::PopItemWidth();

    // ctrl + scroll to adjust zoom
    auto & io = ImGui::GetIO();
    if(!ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && io.KeyCtrl && !io.KeyShift) {
        int multiplier = io.MouseWheel > 0 ? 1 : -1;

        timelineZoom += zoomStep * multiplier;
    }

    if(timelineZoom < zoomStep * 2) {
        timelineZoom = zoomStep * 2;
    }

    static bool leftClickedEntity = false;
    static bool leftClickReleased = false;
    bool rightClickedEntity = false;

    int beatsPerMeasure = songpos.timeinfo.size() > songpos.currentSection ? songpos.timeinfo.at(songpos.currentSection).beatsPerMeasure : 4;
    Sequencer(&(chartinfo.notes), timelineZoom, currentBeatsplit, beatsPerMeasure, nullptr, &updatedBeat, &leftClickedEntity, &leftClickReleased,
              &rightClickedEntity, &clickedBeat, &clickedItemType, nullptr, &songpos.absBeat, ImSequencer::SEQUENCER_CHANGE_FRAME);

    if(updatedBeat) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();
        }
        songpos.setSongBeatPosition(songpos.absBeat);

        if(songpos.absTime >= 0) {
            updateAudioPosition(audioSystem, songpos);
        } else {
            songpos.setSongBeatPosition(0);
        }

        chartinfo.notes.resetPassed(songpos.absBeat, songpos.currSpb);
    }

    // insert or update entity at the clicked beat
    static char addedItem[2];
    static float insertBeat;
    static BeatPos insertBeatpos;
    static int insertItemType;
    static bool startedNote = false;
    static ImGuiInputTextFlags addItemFlags = 0;
    static ImGuiInputTextCallbackData addItemCallbackData;
    if(!ImGuiFileDialog::Instance()->IsOpened() && leftClickedEntity && !ImGui::IsPopupOpen("add_item")) {
        insertBeat = clickedBeat;
        insertItemType = clickedItemType;

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

    static float endBeat;
    static BeatPos endBeatpos;
    if(!ImGuiFileDialog::Instance()->IsOpened() && startedNote && leftClickReleased && !ImGui::IsPopupOpen("add_item") && clickedBeat >= insertBeat) {
        ImGui::OpenPopup("add_item");
        endBeat = clickedBeat;

        endBeatpos = calculateBeatpos(endBeat, currentBeatsplit, songpos.timeinfo); 

        leftClickReleased = false;
        startedNote = false;
    }

    if(ImGui::BeginPopup("add_item")) {
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

                        if(chartinfo.notes.containsItemAt(insertBeat, insertItemType)) {
                            chartinfo.notes.editItem(insertBeat, insertItemType, keyText);
                        } else {
                            chartinfo.notes.addItem(insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, insertItemType, keyText);
                        }

                        addedItem[0] = '\0';
                        ImGui::CloseCurrentPopup();
                        unsaved = true;
                    }
                }
                break;
            case SequencerItemType::BOT_NOTE:
                static int selectedFuncKey = 0;
                ImGui::SetNextItemWidth(64);

                if(ImGui::BeginCombo("##addfunction_key", FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey).c_str())) {
                    for(auto & [keyIdx, keyTxt] : FUNCTION_KEY_COMBO_ITEMS) {
                        bool keySelected = false;
                        if(ImGui::Selectable(keyTxt.c_str(), &keySelected))
                            selectedFuncKey = keyIdx;

                        if(keySelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                if(keysPressed[SDL_SCANCODE_RETURN] || keysPressed[SDL_SCANCODE_RETURN2]) {
                    std::string keyText = FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey);                
                        if(chartinfo.notes.containsItemAt(insertBeat, insertItemType)) {
                            chartinfo.notes.editItem(insertBeat, insertItemType, keyText);
                        } else {
                            chartinfo.notes.addItem(insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, insertItemType, keyText);
                        }

                        selectedFuncKey = 0;
                        ImGui::CloseCurrentPopup();
                        unsaved = true;
                }
                break;
            case SequencerItemType::SKIP:
                break;
            case SequencerItemType::STOP:
                break;
        }
    
        ImGui::EndPopup();
    }

    if(!ImGuiFileDialog::Instance()->IsOpened() && rightClickedEntity && chartinfo.notes.containsItemAt(clickedBeat, clickedItemType)) {
        chartinfo.notes.deleteItem(clickedBeat, clickedItemType);
    }

    chartinfo.notes.update(songpos.absBeat, songpos.currSpb, audioSystem);

    // sideways scroll
    if(!ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && !io.KeyCtrl) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();
        }

        bool decrease = io.MouseWheel > 0;
        int beatsplitChange = std::floor(io.MouseWheel);
        if(beatsplitChange == 0)
            beatsplitChange = 1;

        int fullBeats = std::floor(songpos.absBeat);
        float fullBeatSplits = std::floor((songpos.absBeat - fullBeats) / currentBeatsplitValue);
        float origNearBeat = fullBeats + (fullBeatSplits * (1.f / currentBeatsplit));

        float targetBeat;
        if(decrease && songpos.absBeat > origNearBeat) {
            targetBeat = origNearBeat - (beatsplitChange - 1) * currentBeatsplitValue;
        } else {
            targetBeat = origNearBeat - beatsplitChange * currentBeatsplitValue;
        }

        // scroll up, decrease beat, scroll down increase beat
        songpos.setSongBeatPosition(songpos.absBeat - (beatsplitChange * currentBeatsplitValue));

        // clamp to nearest split
        if((decrease && songpos.absBeat < targetBeat) || (!decrease && songpos.absBeat > targetBeat)) {
            songpos.setSongBeatPosition(targetBeat);
        }

        if(songpos.absTime >= 0) {
            updateAudioPosition(audioSystem, songpos);
        } else {
            songpos.setSongBeatPosition(FLT_EPSILON);
        }

        chartinfo.notes.resetPassed(songpos.absBeat, songpos.currSpb);
    }
}

void showEditWindows(AudioSystem * audioSystem, std::vector<bool> & keysPressed) {
    static bool updatedName = false;
    static ImVec2 sizeBeforeUpdate;

    unsigned int i = 0;
    for(auto iter = editWindows.begin(); iter != editWindows.end(); iter++) {
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
        }

        showEditWindowMetadata(currWindow);
        ImGui::SameLine();
        showEditWindowChartData(currWindow.artTexture.get(), audioSystem, currWindow.songpos, currWindow.unsaved);

        ImGui::Separator();
        showEditWindowToolbar(audioSystem, &(currWindow.songinfo.musicPreviewStart), &(currWindow.songinfo.musicPreviewStop), currWindow.songpos, 
                              currWindow.chartinfo.notes, keysPressed);
        ImGui::Separator();

        showEditWindowTimeline(audioSystem, currWindow.chartinfo, currWindow.songpos, currWindow.unsaved, keysPressed);

        ImGui::End();

        if(!currWindow.open) {
            tryCloseEditWindow(currWindow, iter, audioSystem);
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

        i++;
    }
}