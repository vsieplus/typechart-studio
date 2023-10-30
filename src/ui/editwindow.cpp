#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <map>

#include <json.hpp>

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
#include "actions/shiftnote.hpp"

#include "config/constants.hpp"
#include "config/utils.hpp"

#include "ui/preferences.hpp"
#include "ui/editwindow.hpp"
#include "ui/windowsizes.hpp"

#include "systems/audiosystem.hpp"

using json = nlohmann::json;

namespace utils {

float getKeyFrequencies(void * data, int i) {
    auto * chartinfo = (ChartInfo *)data;

    if((unsigned int)i >= chartinfo->notes.keyFreqsSorted.size()) {
        return 0;
    } else {
        return (float)chartinfo->notes.getKeyItemData(i).second;
    }
}

const char * getKeyFrequencyLabels(void * data, int i) {
    auto * chartinfo = (ChartInfo *)data;

    if((unsigned int)i >= chartinfo->notes.keyFreqsSorted.size()) {
        return nullptr;
    } else {
        return chartinfo->notes.getKeyItemData(i).first.c_str();
    }
}

void updateAudioPosition(AudioSystem * audioSystem, const SongPosition & songpos, int musicSourceIdx) {
    // udpate audio position
    auto offsetAbsTime = static_cast<float>(songpos.absTime + (songpos.offsetMS / 1000.f));
    if(audioSystem->isMusicPlaying(musicSourceIdx)) {
        audioSystem->startMusic(musicSourceIdx, offsetAbsTime);
    } else {
        audioSystem->setMusicPosition(musicSourceIdx, offsetAbsTime);
    }
}

} // namespace utils


EditWindow::EditWindow(bool open, int ID, int musicSourceIdx, std::string_view name, std::shared_ptr<SDL_Texture> artTexture,
    const ChartInfo & chartinfo, const SongInfo & songinfo)
    : open(open)
    , ID(ID)
    , musicSourceIdx(musicSourceIdx)
    , currTopNotes((int)chartinfo.notes.keyFreqsSorted.size())
    , name(name)
    , artTexture(artTexture)
    , chartinfo(chartinfo)
    , songinfo(songinfo) {}

void EditWindow::saveCurrentChartFiles() {
    saveCurrentChartFiles(name, chartinfo.savePath, songinfo.saveDir);
}

void EditWindow::saveCurrentChartFiles(std::string_view chartSaveFilename, const fs::path & chartSavePath, const fs::path & saveDir) {
    songinfo.saveSongInfo(saveDir, initialSaved);
    chartinfo.saveChart(chartSavePath, songpos);

    initialSaved = true;
    unsaved = false;
    name = chartSaveFilename;

    lastSavedActionIndex = static_cast<int>(editActionsUndo.size());
}

void EditWindow::showContents(AudioSystem * audioSystem, std::vector<bool> & keysPressed) {
    showMetadata();

    ImGui::SameLine();
    showChartData(audioSystem);

    ImGui::Separator();
    showToolbar(audioSystem, keysPressed);

    ImGui::Separator();
    timeline.showContents(audioSystem, keysPressed);
}

void EditWindow::showMetadata() {
    // left side bar (child window) to show config info + selected entity info
    ImGui::BeginChild("configInfo", ImVec2(ImGui::GetContentRegionAvail().x * .3f, ImGui::GetContentRegionAvail().y * .35f), true);

    bool editingSongInfo { showSongConfig() };
    bool editingChartInfo { showChartConfig() };

    ImGui::EndChild();

    editingSomething = editingSongInfo || editingChartInfo;
}

bool EditWindow::showSongConfig() {
    static bool editingUItitle = false;
    static bool editingUIartist = false;
    static bool editingUIgenre = false;
    static bool editingUIbpmtext = false;

    static char UItitle[64] = "";
    static char UIartist[64] = "";
    static char UIgenre[64] = "";
    static char UIbpmtext[16] = "";

    if(ImGui::CollapsingHeader("Song config", ImGuiTreeNodeFlags_DefaultOpen)) {
        unsaved |= utils::showEditableText(ICON_FA_MUSIC " Title", UItitle, 64, editingUItitle, songinfo.title);
        unsaved |= utils::showEditableText(ICON_FA_MICROPHONE "Artist", UIartist, 64, editingUIartist, songinfo.artist);
        unsaved |= utils::showEditableText(ICON_FA_COMPACT_DISC "Genre", UIgenre, 64, editingUIgenre, songinfo.genre);
        unsaved |= utils::showEditableText(ICON_FA_HEART_PULSE "BPM", UIbpmtext, 16, editingUIbpmtext, songinfo.bpmtext);
    }

    return editingUItitle || editingUIartist || editingUIgenre || editingUIbpmtext;
}

bool EditWindow::showChartConfig() {
    static bool editingUItypist = false;
    static char UItypist[64] = "";
    static int UIlevel = 1;
    static int UIkeyboardLayout = 0;
    static int UIdifficulty = 0;

    if(ImGui::CollapsingHeader("Chart config", ImGuiTreeNodeFlags_DefaultOpen)) {
        unsaved |= utils::showEditableText(ICON_FA_PENCIL " Typist", UItypist, 64, editingUItypist, chartinfo.typist);

        if(ImGui::Combo(ICON_FA_KEYBOARD " Keyboard", &UIkeyboardLayout, "QWERTY\0DVORAK\0AZERTY\0COLEMAK\0")) {
            chartinfo.keyboardLayout = constants::ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout);
            unsaved = true;
        }

        ImGui::SameLine();
        utils::HelpMarker("Choose the keyboard layout that this chart is\n"
                          "intended to be played with. Charts will then be\n"
                          "accordingly 'translated' to other keyboard layouts\n"
                          "when loaded into Typing Tempo.");

        if(ImGui::Combo(ICON_FA_CHESS_PAWN " Difficulty", &UIdifficulty, "easy\0normal\0hard\0expert\0unknown\0")) {
            chartinfo.difficulty = constants::ID_TO_DIFFICULTY.at(UIdifficulty);
            unsaved = true;
        }

        if(ImGui::InputInt(ICON_FA_CHESS_ROOK " Level", &UIlevel)) {
            chartinfo.level = UIlevel;
            unsaved = true;
        }
    }

    return editingUItypist;
}

void EditWindow::showChartData(AudioSystem * audioSystem) {
    ImGui::BeginChild("chartData", ImVec2(0, ImGui::GetContentRegionAvail().y * .35f), true);

    ImGui::Image(artTexture.get(), ImVec2(ImGui::GetContentRegionAvail().y, ImGui::GetContentRegionAvail().y));

    ImGui::SameLine();
    showChartSections(audioSystem);

    ImGui::SameLine();
    showChartStatistics();

    ImGui::EndChild();
}

void EditWindow::showChartSections(AudioSystem * audioSystem) {
    ImGui::BeginChild("timedata", ImVec2(ImGui::GetContentRegionAvail().x * .5f, 0), true);
    ImGui::Text("Chart Sections");
    ImGui::SameLine();

    static bool newSection { false };
    static bool newSectionEdit { false };
    
    bool initSectionData { false };

    if(showAddSection()) {
        newSection = true;
        newSectionEdit = false;

        initSectionData = true;
    }

    ImGui::SameLine();
    showRemoveSection();

    ImGui::SameLine();
    if(showEditSection()) {
        newSection = true;
        newSectionEdit = true;

        initSectionData = true;
    }

    showSectionDataWindow(newSection, newSectionEdit, initSectionData);
    showChartSectionList(audioSystem);

    ImGui::EndChild();
}

bool EditWindow::showAddSection() const {
    if(ImGui::Button(ICON_FA_PLUS)) {
        return true;
    }

    return false;
}

bool EditWindow::showEditSection() const {
    if(ImGui::Button("Edit")) {
        return true;
    }

    return false;
}

void EditWindow::showRemoveSection() {
    // remove the selected section
    static bool invalidDeletion { false };
    if(ImGui::Button(ICON_FA_MINUS) && !songpos.timeinfo.empty()) {
        if(songpos.currentSection == 0) {
            invalidDeletion = true;
            ImGui::OpenPopup("Invalid deletion");
        } else {
            songpos.removeSection(songpos.currentSection);
            unsaved = true;
        }
    }

    if(invalidDeletion && ImGui::BeginPopup("Invalid deletion")) {
        ImGui::Text("Cannot delete the first section.");
        ImGui::EndPopup();
    }
}

void EditWindow::showChartSectionList(AudioSystem * audioSystem) {
    // display section info
    if(ImGui::BeginListBox("##chartsections", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
        for(unsigned int i = 0; i < songpos.timeinfo.size(); i++) {
            Timeinfo currSection = songpos.timeinfo.at(i);

            char sectionDesc[256];
            snprintf(sectionDesc, 256, "[%d,%d,%d] : BPM: %.1f, Beats / measure: %d", currSection.beatpos.measure, currSection.beatpos.measureSplit,
                currSection.beatpos.split, currSection.bpm, currSection.beatsPerMeasure);

            bool isSelected = i == songpos.currentSection;

            if(ImGui::Selectable(sectionDesc, &isSelected, ImGuiSelectableFlags_SelectOnClick)) {
                if(!songpos.started) {
                    songpos.start();
                    songpos.pause();

                    songpos.pauseCounter += static_cast<Uint64>((songpos.offsetMS / 1000.0) * SDL_GetPerformanceFrequency());
                }

                songpos.setSongBeatPosition(currSection.absBeatStart + FLT_EPSILON);
                chartinfo.notes.resetPassed(songpos.absBeat);
                utils::updateAudioPosition(audioSystem, songpos, musicSourceIdx);
            }
        }

        ImGui::EndListBox();
    }
}

void EditWindow::showSectionDataWindow(bool & newSection, bool newSectionEdit, bool initSectionData) {
    auto currSection = songpos.timeinfo.at(songpos.currentSection);

    static int newSectionMeasure { currSection.beatpos.measure };
    static int newSectionMeasureSplit { currSection.beatpos.measureSplit };
    static int newSectionSplit { currSection.beatpos.split };
    static int newSectionBeatsPerMeasure { currSection.beatsPerMeasure };
    static double newSectionBPM { currSection.bpm };
    static double newSectionInterpolateDuration { 0.0 };

    if(initSectionData) {
        newSectionMeasure = currSection.beatpos.measure;
        newSectionMeasureSplit = currSection.beatpos.measureSplit;
        newSectionSplit = currSection.beatpos.split;
        newSectionBeatsPerMeasure = currSection.beatsPerMeasure;
        newSectionBPM = currSection.bpm;
        newSectionInterpolateDuration = currSection.interpolateBeatDuration;
    }

    if(newSection) {
        auto windowEditTitle = newSectionEdit ? "Edit Section" : "New Section";
        ImGui::Begin(windowEditTitle, &newSection);

        static bool invalidInput { false };

        ImGui::Text("Section start");
        ImGui::SameLine();
        utils::HelpMarker("The absolute beat start is calculated as: measure + (split / beatsplit)\n"
                   "For example, entering [5, 8, 5] means starting on the 6th measure on the\n"
                   "6th eighth note (measure, split values are 0-indexed). Assuming 4 beats\n"
                   "per measure, for the previous sections, this would be equivalent to beat 26.5\n"
                   "These 'absolute' beats are based off of the song's initial bpm");

        ImGui::InputInt("Measure", &newSectionMeasure);
        ImGui::InputInt("MeasureSplit", &newSectionMeasureSplit);
        ImGui::InputInt("Split", &newSectionSplit);

        newSectionMeasure = std::max(0, newSectionMeasure);
        newSectionMeasureSplit = std::max(0, newSectionMeasureSplit);
        newSectionSplit = std::max(0, newSectionSplit);

        ImGui::Separator();

        ImGui::InputDouble("BPM", &newSectionBPM, 1, 5, "%.1f");
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_X "2")) {
            newSectionBPM *= 2;
        }
        ImGui::SameLine();
        if(ImGui::Button(ICON_FA_DIVIDE "2")) {
            newSectionBPM /= 2;
        }

        ImGui::InputInt("Beats Per Measure", &newSectionBeatsPerMeasure);
        ImGui::InputDouble("Beat Interpolation Duration", &newSectionInterpolateDuration, 0.25, 0.5, "%.2f");

        newSectionBPM = std::max(0.0, newSectionBPM);
        newSectionInterpolateDuration = std::max(0.0, newSectionInterpolateDuration);
        newSectionBeatsPerMeasure = std::max(0, newSectionBeatsPerMeasure);

        if(ImGui::Button("OK")) {
            BeatPos newBeatpos = { newSectionMeasure, newSectionMeasureSplit, newSectionSplit };

            invalidInput = newSectionEdit ?
                !songpos.editSection(songpos.currentSection, newSectionBeatsPerMeasure, newSectionBPM, newSectionInterpolateDuration, newBeatpos) :
                !songpos.addSection(newSectionBeatsPerMeasure, newSectionBPM, newSectionInterpolateDuration, newBeatpos);

            newSection = invalidInput;
            unsaved = !invalidInput;
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            newSection = false;
        }

        if(invalidInput && ImGui::BeginPopup("Invalid input")) {
            ImGui::Text("Section already exists at the specified beat position");
            ImGui::EndPopup();
        }

        ImGui::End();
    }
}

void EditWindow::showChartStatistics() {
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
    ImGui::SliderInt("##topNotes", &currTopNotes, 0, (int)chartinfo.notes.keyFreqsSorted.size());
    
    float maxFreq = utils::getKeyFrequencies((void*)&chartinfo, 0);
    ImGui::PlotHistogram("##keyFreqs", utils::getKeyFrequencies, utils::getKeyFrequencyLabels, (void*)&chartinfo, currTopNotes, 0, nullptr, 0, maxFreq,
        ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
    ImGui::EndChild();
}

void EditWindow::showToolbar(AudioSystem * audioSystem, std::vector<bool> & keysPressed) {
    auto musicLengthSecs = audioSystem->getMusicLength(musicSourceIdx);
    showMusicPosition(musicLengthSecs);

    ImGui::SameLine();
    showMusicControls(audioSystem, keysPressed);

    if(songpos.absTime > musicLengthSecs) {
        songpos.absTime = musicLengthSecs;
        songpos.started = false;
    }

    if(audioSystem->getStopMusicEarly(musicSourceIdx) && songpos.absTime > audioSystem->getMusicStop(musicSourceIdx)) {
        songpos.absTime = audioSystem->getMusicStop(musicSourceIdx);
        songpos.started = false;
    }

    ImGui::SameLine();
    showMusicPreview(audioSystem, musicLengthSecs);

    ImGui::SameLine();
    showMusicOffset();
}

void EditWindow::showMusicPosition(double musicLengthSecs) const {
    auto [audioPosMin, audioPosSec] { utils::splitSecsbyMin(songpos.absTime) };
    auto [songLengthMin, songLengthSec] { utils::splitSecsbyMin(musicLengthSecs) };
    audioPosMin = std::max(0, audioPosMin);
    audioPosSec = audioPosMin < 0 ? 0 : audioPosSec;
    ImGui::Text("%02d:%05.2f/%02d:%05.2f", audioPosMin, audioPosSec, songLengthMin, songLengthSec);
}

void EditWindow::showMusicControls(AudioSystem * audioSystem, std::vector<bool> & keysPressed) {
    if((focused && (ImGui::Button(ICON_FA_PLAY "/" ICON_FA_PAUSE) || (!editingSomething && keysPressed[SDL_SCANCODE_SPACE]))) 
        && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
    {
        if(songpos.paused) {
            audioSystem->resumeMusic(musicSourceIdx);
            songpos.unpause();
        } else if(!songpos.started) {
            audioSystem->startMusic(musicSourceIdx);
            songpos.start();
            audioSystem->setStopMusicEarly(musicSourceIdx, false);
        } else {
            audioSystem->pauseMusic(musicSourceIdx);
            songpos.pause();
        }
    }

    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Press [Space] to Play/Pause");

    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_STOP)) {
        audioSystem->stopMusic(musicSourceIdx);
        songpos.stop();
        chartinfo.notes.resetPassed(songpos.absBeat);
    }
}

void EditWindow::showMusicPreview(AudioSystem * audioSystem, float musicLengthSecs) {
    showMusicPreviewSliders(musicLengthSecs);

    ImGui::SameLine();
    showMusicPreviewButton(audioSystem);
}

void EditWindow::showMusicPreviewSliders(float musicLengthSecs) {
    // allow user to play / set preview
    float sliderWidth { ImGui::GetContentRegionAvail().x * 0.25f };

    ImGui::SetNextItemWidth(sliderWidth);
    if(ImGui::SliderFloat("##prevstart", &songinfo.musicPreviewStart, 0.f, musicLengthSecs, "%05.2f")) {
        unsaved = true;
    }

    if(ImGui::IsItemDeactivatedAfterEdit()) {
        // update max prev stop if needed, clamp
        songinfo.musicPreviewStart = std::min(songinfo.musicPreviewStart, songinfo.musicPreviewStop);
        songinfo.musicPreviewStart = std::max(0.f, songinfo.musicPreviewStart);
        songinfo.musicPreviewStart = std::min(songinfo.musicPreviewStart, musicLengthSecs);
    }

    if(ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        ImGui::SetTooltip("Music preview start");
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(sliderWidth);
    if(ImGui::SliderFloat("##prevstop", &songinfo.musicPreviewStop, 0.f, musicLengthSecs, "%05.2f")) {
        unsaved = true;
    }

    if(ImGui::IsItemDeactivatedAfterEdit()) {
        // update min prevStart, clamp
        songinfo.musicPreviewStop = std::max(songinfo.musicPreviewStop, songinfo.musicPreviewStart);
        songinfo.musicPreviewStop = std::max(0.f, songinfo.musicPreviewStop);
        songinfo.musicPreviewStop = std::min(songinfo.musicPreviewStop, musicLengthSecs);
    }

    if(ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        ImGui::SetTooltip("Music preview stop");
    }
}

void EditWindow::showMusicPreviewButton(AudioSystem * audioSystem) {
    if(ImGui::Button("Preview " ICON_FA_TRAILER)) {
        audioSystem->stopMusic(musicSourceIdx);
        audioSystem->startMusic(musicSourceIdx, songinfo.musicPreviewStart);
        audioSystem->setMusicStop(musicSourceIdx, songinfo.musicPreviewStop);

        audioSystem->setStopMusicEarly(musicSourceIdx, true);

        if(!songpos.started)
            songpos.start();

        songpos.setSongTimePosition(songinfo.musicPreviewStart);

        if(songpos.paused)
            songpos.unpause();
    }
}

void EditWindow::showMusicOffset() {
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2.f);
    if(ImGui::SliderInt("Offset (ms)", &songpos.offsetMS, -1000, 1000)) {
        songinfo.offsetMS = songpos.offsetMS;
        unsaved = true;
    }

    if(ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        ImGui::SetTooltip("Offset from the first beat in milliseconds\n(Ctrl + Click to enter)");
    }
}
