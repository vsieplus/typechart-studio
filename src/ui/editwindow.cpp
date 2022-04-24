#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>

#include <nlohmann/json.hpp>

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "ui/preferences.hpp"
#include "ui/editwindow.hpp"
#include "ui/windowsizes.hpp"

#include "systems/audiosystem.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

static bool newEditStarted = false;

std::string DEFAULT_WINDOW_NAME = "Untitled";

const std::map<int, std::string> ID_TO_KEYBOARDLAYOUT = {
    { 0 , "QWERTY" },
    { 1, "DVORAK" },
    { 2, "AZERTY" }
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

const char * songinfoFileFilter = "(*.json){.json}";
const char * imageFileFilters = "(*.jpg *.png){.jpg,.png}";
const char * musicFileFilters = "(*.flac *.mp3 *.ogg *.wav){.flac,.mp3,.ogg,.wav}";

static std::string UImusicFilepath = "";
static std::string UIcoverArtFilepath = "";

static bool popupIncomplete = true;
static bool popupInvalidJSON = true;
static bool popupFailedToLoadMusic = false;

static float UImusicPreviewStart = 0;
static float UImusicPreviewStop = 15;

void loadSonginfo(std::string songinfoPath, std::string songinfoDir) {
    json songinfoJSON;

    try {
        std::ifstream in(songinfoPath);
        in >> songinfoJSON;
        popupInvalidJSON = false;
    } catch(...) {
        ImGui::OpenPopup("invalidJSON");
        popupInvalidJSON = true;
        return;
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
    } else {
        popupIncomplete = false;
    }
}

void showSongConfig() {
    // song config
    ImGui::Text("Song configuration");
    if(ImGui::Button("Load from existing...")) {
        ImGuiFileDialog::Instance()->OpenDialog("selectSonginfo", "Select songinfo.json", songinfoFileFilter, 
                                                Preferences::Instance().getInputDir());
    }
    
    ImGui::SameLine();
    HelpMarker("Load song data from a pre-existing songinfo.json file");

    // songinfo dialog
    if(ImGuiFileDialog::Instance()->Display("selectSonginfo", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string songinfoPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string songinfoDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            loadSonginfo(songinfoPath, songinfoDir);
        }

        ImGuiFileDialog::Instance()->Close();
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

    ImGui::InputText("Music", UImusicFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##music")) {
        ImGuiFileDialog::Instance()->OpenDialog("selectMusicFile", "Select Music", musicFileFilters,
                                                Preferences::Instance().getInputDir());
    }

    // music file dialog
    if(ImGuiFileDialog::Instance()->Display("selectMusicFile", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            UImusicFilepath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            strcpy(UImusicFilename, fileName.c_str());
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText("Art", UIcoverArtFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##art")) {
        ImGuiFileDialog::Instance()->OpenDialog("selectArt", "Select Art", imageFileFilters,
                                                Preferences::Instance().getInputDir());
    }

    // art file dialog
    if(ImGuiFileDialog::Instance()->Display("selectArt", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            UIcoverArtFilepath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            strcpy(UIcoverArtFilename, fileName.c_str());
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText("Title", UItitle, 64);
    ImGui::InputText("Artist", UIartist, 64);
    ImGui::InputText("BPM", UIbpmtext, 64);
    ImGui::SameLine();
    HelpMarker("This value is only used as the 'displayed' BPM");
    
}

static char UItypist[64] = "";
static int UIlevel = 1;
static int UIkeyboardLayout = 0;

void showChartConfig() {
    ImGui::Text("Chart configuration");

    ImGui::InputText("Typist", UItypist, 64);
    ImGui::Combo("Keyboard", &UIkeyboardLayout, "QWERTY\0DVORAK\0AZERTY\0\0");
    ImGui::SameLine();
    HelpMarker("Choose the keyboard layout that this chart is\n"
                "intended to be played with. Charts will then be\n"
                "accordingly 'translated' to other keyboard layouts\n"
                "when loaded into Typing Tempo.");
    ImGui::InputInt("Level", &UIlevel);
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

void createNewEditWindow(AudioSystem * audioSystem, SDL_Renderer * renderer) {
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

    // populate with current song, chart info
    SongInfo songInfo = SongInfo(UItitle, UIartist, UIbpmtext, UImusicFilename, UIcoverArtFilename, 
                                 UImusicFilepath, UIcoverArtFilepath, UImusicPreviewStart, UImusicPreviewStop);
    ChartInfo chartInfo = ChartInfo(UIlevel, UItypist, ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout));

    // attempt to load music
    if(!audioSystem->loadMusic(UImusicFilepath)) {
        ImGui::OpenPopup("failedToLoadMusic");
        popupFailedToLoadMusic = true;
        return;
    } else {
        popupFailedToLoadMusic = false;

        auto artTexture = Texture::loadTexture(UIcoverArtFilepath, renderer);

        EditWindowData newWindow = EditWindowData(true, windowID, windowName, artTexture, chartInfo, songInfo);
        editWindows.push_back(newWindow);
    
        newEditStarted = false;
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
}

void closeWindow(EditWindowData & currWindow, std::list<EditWindowData>::iterator & iter) {
    availableWindowIDs.push(currWindow.ID);
    iter = editWindows.erase(iter);
}

void tryCloseEditWindow(EditWindowData & currWindow, std::list<EditWindowData>::iterator & iter) {
    if(currWindow.unsaved) {
        char msg[128];
        snprintf(msg, 128, "Unsaved work! [%s]", currWindow.name.c_str());
        ImGui::Begin(msg);

        ImGui::Text("Save before closing?");
        if(ImGui::Button("Yes")) {
            // save chart
        }

        ImGui::SameLine();
        if(ImGui::Button("No")) {
            closeWindow(currWindow, iter);
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            currWindow.open = true;
        }

        ImGui::End();
    } else {
        closeWindow(currWindow, iter);
    }
}

std::pair<int, float> splitSecsbyMin(float seconds) {
    float minutes = seconds / 60;

    int fullMinutes = std::floor(minutes);
    float leftoverSecs = (minutes - fullMinutes) * 60.f;

    return std::make_pair(fullMinutes, leftoverSecs);
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

void showEditWindowChartData(EditWindowData & currWindow) {
    ImGui::BeginChild("chartData", ImVec2(0, ImGui::GetContentRegionAvail().y * .35f), true);
    
    ImGui::Image(currWindow.artTexture.get(), ImVec2(ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y));

    ImGui::SameLine();
    ImGui::BeginChild("timeAndNoteData");

    ImGui::Text("Chart Sections");
    
    ImGui::EndChild();
    ImGui::EndChild();
}

// toolbar info / buttons
void showEditWindowToolbar(AudioSystem * audioSystem, float * previewStart, float * previewStop) {
    auto musicLengthSecs = audioSystem->getMusicLength();

    auto songAudioPos = splitSecsbyMin(audioSystem->getSongPosition());
    auto songLength = splitSecsbyMin(musicLengthSecs);
    ImGui::Text("%02d:%05.2f/%02d:%05.2f", songAudioPos.first, songAudioPos.second, songLength.first, songLength.second);

    ImGui::SameLine();
    if(ImGui::Button("Play")) {
        if(audioSystem->isMusicPaused()) {
            audioSystem->resumeMusic();
        } else {
            audioSystem->startMusic();
        }

        audioSystem->setStopMusicEarly(false);
    }

    ImGui::SameLine();
    if(ImGui::Button("Pause")) {
        audioSystem->pauseMusic();
    }

    ImGui::SameLine();
    if(ImGui::Button("Stop")) {
        audioSystem->stopMusic();
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
    if(ImGui::Button("Preview")) {
        audioSystem->stopMusic();
        audioSystem->startMusic(*previewStart);
        audioSystem->setMusicStop(*previewStop);

        audioSystem->setStopMusicEarly(true);
    }

}

void showEditWindowTimeline(ChartInfo & chartinfo, SongPosition & songpos) {
    // let's create the sequencer
    static int selectedEntry = -1;
    static int currentBeatsplit = 8;
    static bool expanded = true;
    static float timelineZoom = 1.f;

    ImGui::PushItemWidth(130);
    ImGui::Text("Beatsplit: ");
    ImGui::SameLine();
    ImGui::InputInt("##beatsplit", &currentBeatsplit, 1, 4);
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
        // snap to the nearest beat split
        if(songpos.absBeat > nextTargetBeat) {
            songpos.absBeat = nextTargetBeat;
        } else if(songpos.absBeat < prevTargetBeat) {
            songpos.absBeat = prevTargetBeat;
        }
    }

    ImGui::SameLine();
    ImGui::Text("Zoom: ");
    ImGui::SameLine();
    ImGui::InputFloat("##zoom", &timelineZoom, 0.25, 0.5, "%.2f");
    ImGui::PopItemWidth();

    int beatsPerMeasure = (int)songpos.timeinfo.size() > songpos.currentSection ? songpos.timeinfo.at(songpos.currentSection).beatsPerMeasure : 4;
    Sequencer(&(chartinfo.notes), timelineZoom, currentBeatsplit, beatsPerMeasure, 
              &expanded, &selectedEntry, &songpos.absBeat, ImSequencer::SEQUENCER_CHANGE_FRAME);
    // add a UI to edit that particular item
    if (selectedEntry != -1) {
        const NoteSequence::NoteSequenceItem &item = chartinfo.notes.myItems[selectedEntry];
        ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
        // switch (type) ....
    }

    // sideways scroll
    auto & io = ImGui::GetIO();
    if(io.MouseWheel != 0.f && io.KeyShift) {
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
        songpos.absBeat -= beatsplitChange * currentBeatsplitValue;

        // clamp to nearest split
        if((decrease && songpos.absBeat < targetBeat) || (!decrease && songpos.absBeat > targetBeat)) {
            songpos.absBeat = targetBeat;
        }
    }
}

void showEditWindows(AudioSystem * audioSystem) {
    for(auto iter = editWindows.begin(); iter != editWindows.end(); iter++) {
        auto & currWindow = *iter;

        ImGuiWindowFlags windowFlags = 0;
        if(currWindow.unsaved)  windowFlags |= ImGuiWindowFlags_UnsavedDocument;
        if(!currWindow.open)    windowFlags |= ImGuiWindowFlags_NoInputs;

        ImGui::Begin(currWindow.name.c_str(), &(currWindow.open), windowFlags);

        showEditWindowMetadata(currWindow);
        ImGui::SameLine();
        showEditWindowChartData(currWindow);

        ImGui::Separator();
        showEditWindowToolbar(audioSystem, &(currWindow.songinfo.musicPreviewStart), &(currWindow.songinfo.musicPreviewStop));
        ImGui::Separator();

        showEditWindowTimeline(currWindow.chartinfo, currWindow.songpos);

        ImGui::End();

        if(!currWindow.open) {
            tryCloseEditWindow(currWindow, iter);
        }
    }
}