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
#include "config/notemaps.hpp"
#include "config/utils.hpp"

#include "ui/preferences.hpp"
#include "ui/editwindow.hpp"
#include "ui/windowsizes.hpp"

#include "systems/audiosystem.hpp"

using json = nlohmann::json;

const std::map<int, std::string> ID_TO_KEYBOARDLAYOUT = {
    { 0, "QWERTY" },
    { 1, "DVORAK" },
    { 2, "AZERTY" },
    { 3, "COLEMAK" }
};

const std::map<std::string, int> KEYBOARDLAYOUT_TO_ID = {
    { "QWERTY", 0 },
    { "DVORAK", 1 },
    { "AZERTY", 2 },
    { "COLEMAK", 3 }
};

const std::map<int, std::string> ID_TO_DIFFICULTY = {
    { 0, "easy" },
    { 1, "normal" },
    { 2, "hard" },
    { 3, "expert" },
    { 4, "unknown" }
};

const std::map<std::string, int> DIFFICULTY_TO_ID = {
    { "easy", 0 },
    { "normal", 1 },
    { "hard", 2 },
    { "expert", 3 },
    { "unknown", 4 }
};

static char UImusicFilename[128] = "";
static char UIcoverArtFilename[128] = "";


static std::string UImusicFilepath = "";
static std::string UIcoverArtFilepath = "";

static bool popupInvalidJSON = true;
static bool popupFailedToLoadMusic = false;

static float UImusicPreviewStart = 0;
static float UImusicPreviewStop = 15;

void updateAudioPosition(AudioSystem * audioSystem, SongPosition & songpos, int musicSourceIdx) {
    // udpate audio position
    auto offsetAbsTime = songpos.absTime + (songpos.offsetMS / 1000.f);
    if(audioSystem->isMusicPlaying(musicSourceIdx)) {
        audioSystem->startMusic(musicSourceIdx, offsetAbsTime);
    } else {
        audioSystem->setMusicPosition(musicSourceIdx, offsetAbsTime);
    }
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
    static char UItypist[64] = "";

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
            chartinfo.keyboardLayout = ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout);
            unsaved = true;
        }

        ImGui::SameLine();
        utils::HelpMarker("Choose the keyboard layout that this chart is\n"
                          "intended to be played with. Charts will then be\n"
                          "accordingly 'translated' to other keyboard layouts\n"
                          "when loaded into Typing Tempo.");

        if(ImGui::Combo(ICON_FA_CHESS_PAWN " Difficulty", &UIdifficulty, "easy\0normal\0hard\0expert\0unknown\0")) {
            chartinfo.difficulty = ID_TO_DIFFICULTY.at(UIdifficulty);
            unsaved = true;
        }

        if(ImGui::InputInt(ICON_FA_CHESS_ROOK " Level", &UIlevel)) {
            chartinfo.level = UIlevel;
            unsaved = true;
        }
    }

    return editingUItypist;
}

static float getKeyFrequencies(void * data, int i) {
    auto * chartinfo = (ChartInfo *)data;

    if((unsigned int)i >= chartinfo->notes.keyFreqsSorted.size()) {
        return 0;
    } else {
        return (float)chartinfo->notes.getKeyItemData(i).second;
    }
}

static const char * getKeyFrequencyLabels(void * data, int i) {
    auto * chartinfo = (ChartInfo *)data;

    if((unsigned int)i >= chartinfo->notes.keyFreqsSorted.size()) {
        return nullptr;
    } else {
        return chartinfo->notes.getKeyItemData(i).first.c_str();
    }
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
    static bool invalidDeletion = false;
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

                    songpos.pauseCounter += (Uint64)((songpos.offsetMS / 1000.0) * SDL_GetPerformanceFrequency());
                }

                songpos.setSongBeatPosition(currSection.absBeatStart + FLT_EPSILON);
                chartinfo.notes.resetPassed(songpos.absBeat);
                updateAudioPosition(audioSystem, songpos, musicSourceIdx);
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

        static bool invalidInput = false;

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
    
    float maxFreq = getKeyFrequencies((void*)&chartinfo, 0);
    ImGui::PlotHistogram("##keyFreqs", getKeyFrequencies, getKeyFrequencyLabels, (void*)&chartinfo, currTopNotes, 0, nullptr, 0, maxFreq,
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

const std::unordered_map<int, std::string> FUNCTION_KEY_COMBO_ITEMS = {
    //{0, "L" ICON_FA_ARROW_UP },
    //{1, "R" ICON_FA_ARROW_UP },
    //{2, ICON_FA_ARROW_UP },
    //{3, ICON_FA_ARROW_LEFT_LONG },
    { 0, "_" },
};

const std::unordered_map<int, SDL_Scancode> FUNCTION_KEY_COMBO_SCANCODES = {
    //{0, SDL_SCANCODE_LSHIFT },
    //{1, SDL_SCANCODE_RSHIFT },
    //{2, SDL_SCANCODE_CAPSLOCK },
    //{3, SDL_SCANCODE_RETURN },
    { 0, SDL_SCANCODE_SPACE },
};

int filterInputMiddleKey(ImGuiInputTextCallbackData * data) {
    std::string keyboardLayout;
    if(data->UserData) {
        keyboardLayout = static_cast<char*>(data->UserData);
    }

    auto c = data->EventChar;
    bool validChar = notemaps::MIDDLE_ROW_KEYS.find(keyboardLayout) != notemaps::MIDDLE_ROW_KEYS.end() &&
                     notemaps::MIDDLE_ROW_KEYS.at(keyboardLayout).find(c) != notemaps::MIDDLE_ROW_KEYS.at(keyboardLayout).end();

    return validChar ? 0 : 1;
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
        if(songpos.absBeat >= nextTargetBeat) {
            songpos.setSongBeatPosition(nextTargetBeat);
        } else if(songpos.absBeat <= prevTargetBeat) {
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

    auto currBeatpos = utils::calculateBeatpos(songpos.absBeat, currentBeatsplit, songpos.timeinfo);
    ImGui::SameLine();
    ImGui::Text("[Pos]: [%d, %d, %d]", std::max(0, currBeatpos.measure), std::max(0, currBeatpos.measureSplit), std::max(0, currBeatpos.split));

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

    timelineZoom = ImMax(timelineZoom, zoomStep);

    static bool leftClickedEntity = false;
    static bool leftClickReleased = false;
    static bool leftClickShift = false;
    static bool haveSelection = false;
    bool rightClickedEntity = false;

    int beatsPerMeasure = songpos.timeinfo.size() > songpos.currentSection ? songpos.timeinfo.at(songpos.currentSection).beatsPerMeasure : 4;
    Sequencer(&(chartinfo.notes), timelineZoom, currentBeatsplit, beatsPerMeasure, Preferences::Instance().isDarkTheme(), haveSelection, windowFocused,
              nullptr, &updatedBeat, &leftClickedEntity, &leftClickReleased, &leftClickShift, &rightClickedEntity, &clickedBeat, &hoveredBeat,
              &clickedItemType, &releasedItemType, nullptr, &songpos.absBeat, ImSequencer::SEQUENCER_CHANGE_FRAME);

    if(ImGuiFileDialog::Instance()->IsOpened()) {
        leftClickedEntity = false;
        leftClickReleased = false;
        rightClickedEntity = false;
    }

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
        bool hadSelection = haveSelection;
        if(haveSelection) {
            haveSelection = false;
        }

        if(!hadSelection || leftClickShift) {
            insertBeat = clickedBeat;
            insertItemType = clickedItemType;

            insertBeatpos = utils::calculateBeatpos(clickedBeat, currentBeatsplit, songpos.timeinfo);

            addItemFlags = ImGuiInputTextFlags_CharsUppercase;

            switch (insertItemType) {
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                    addItemFlags |= ImGuiInputTextFlags_CharsDecimal;
                    break;
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                    addItemFlags |= ImGuiInputTextFlags_CallbackCharFilter;
                    break;
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                    addItemFlags = 0;
                    break;
                default:
                    break;
            }
            startedNote = true;
        }

        leftClickedEntity = false;
    }

    static int insertItemTypeEnd = 0;
    static float endBeat;
    static BeatPos endBeatpos;
    if(windowFocused && !ImGuiFileDialog::Instance()->IsOpened() && startedNote && leftClickReleased &&
        !ImGui::IsPopupOpen(addItemPopup) && clickedBeat >= insertBeat)
    {
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
        endBeatpos = utils::calculateBeatpos(endBeat, currentBeatsplit, songpos.timeinfo); 

        leftClickReleased = false;
        leftClickShift = false;
        startedNote = false;
    }

    // copy, cut, delete selection of notes
    static std::list<std::shared_ptr<NoteSequenceItem>> copiedItems;
    if(windowFocused && haveSelection) {
        if(activateCopy || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_c)])) {
            copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            haveSelection = false;
            activateCopy = false;
        } else if(activateCut || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_x)])) {
            copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

            if(!copiedItems.empty()) {
                auto delAction = std::make_shared<DeleteItemsAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat, copiedItems);
                editActionsUndo.push(delAction);
                utils::emptyActionStack(editActionsRedo);

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
            utils::emptyActionStack(editActionsRedo);

            chartinfo.notes.flipNotes(chartinfo.keyboardLayout, insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            unsaved = true;
            activateFlip = false;
        }

        // shift notes up, down, left, right
        ShiftNoteAction::ShiftDirection shiftDirection = ShiftNoteAction::ShiftDirection::ShiftNone;
        if(keysPressed[SDL_GetScancodeFromKey(SDLK_UP)]) {
            shiftDirection = ShiftNoteAction::ShiftDirection::ShiftUp;
        } else if(keysPressed[SDL_GetScancodeFromKey(SDLK_DOWN)]) {
            shiftDirection = ShiftNoteAction::ShiftDirection::ShiftDown;
        } else if(keysPressed[SDL_GetScancodeFromKey(SDLK_LEFT)]) {
            shiftDirection = ShiftNoteAction::ShiftDirection::ShiftLeft;
        } else if(keysPressed[SDL_GetScancodeFromKey(SDLK_RIGHT)]) {
            shiftDirection = ShiftNoteAction::ShiftDirection::ShiftRight;
        }

        if(shiftDirection != ShiftNoteAction::ShiftDirection::ShiftNone) {
            auto items = chartinfo.notes.shiftNotes(chartinfo.keyboardLayout, insertBeat, endBeat, insertItemType, insertItemTypeEnd, shiftDirection);

            auto shiftAction = std::make_shared<ShiftNoteAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat,
                chartinfo.keyboardLayout, shiftDirection, items);
            editActionsUndo.push(shiftAction);
            utils::emptyActionStack(editActionsRedo);

            unsaved = true;
        }

        if(keysPressed[SDL_SCANCODE_DELETE]) {
            auto deletedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
            chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

            if(!deletedItems.empty()) {
                auto delAction = std::make_shared<DeleteItemsAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat, deletedItems);
                editActionsUndo.push(delAction);
                utils::emptyActionStack(editActionsRedo);

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
            auto hoveredBeatEnd = hoveredBeat + (copiedItems.back()->beatEnd - copiedItems.front()->absBeat);
            auto overwrittenItems = chartinfo.notes.getItems(hoveredBeat, hoveredBeatEnd, insertItemType, insertItemTypeEnd);
            chartinfo.notes.insertItems(hoveredBeat, songpos.absBeat, insertItemType, insertItemTypeEnd, songpos.timeinfo, copiedItems);

            if(!overwrittenItems.empty()) {
                auto delAction = std::make_shared<DeleteItemsAction>(unsaved, insertItemType, insertItemTypeEnd, hoveredBeat, hoveredBeatEnd, overwrittenItems);
                editActionsUndo.push(delAction);
            }

            auto insAction = std::make_shared<InsertItemsAction>(unsaved, insertItemType, insertItemTypeEnd, hoveredBeat, copiedItems, overwrittenItems);
            editActionsUndo.push(insAction);
            utils::emptyActionStack(editActionsRedo);

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
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
                ImGui::SetNextItemWidth(32);
                if(!ImGui::IsAnyItemActive() && !ImGuiFileDialog::Instance()->IsOpened() && !ImGui::IsMouseClicked(0))
                    ImGui::SetKeyboardFocusHere(0);

                if(ImGui::InputText("##addnote_text", addedItem, 2, addItemFlags, filterInputMiddleKey, (void *)chartinfo.keyboardLayout.c_str())) {
                    if(addedItem[0] != '\0') {
                        std::string keyText(addedItem);

                        std::shared_ptr<EditAction> currAction;
                        auto foundItem = chartinfo.notes.containsItemAt(insertBeat, insertItemType);

                        if(foundItem.get()) {
                            currAction = std::make_shared<EditNoteAction>(unsaved, insertBeat, (NoteSequenceItem::SequencerItemType)insertItemType, foundItem->displayText, keyText);
                            chartinfo.notes.editNote(insertBeat, insertItemType, keyText);
                        } else {
                            chartinfo.notes.addNote(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos,
                                (NoteSequenceItem::SequencerItemType)insertItemType, keyText);
                            currAction = std::make_shared<PlaceNoteAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (NoteSequenceItem::SequencerItemType)insertItemType, keyText);
                        }

                        editActionsUndo.push(currAction);
                        utils::emptyActionStack(editActionsRedo);

                        addedItem[0] = '\0';
                        ImGui::CloseCurrentPopup();
                        unsaved = true;
                    }
                }
                break;
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
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

                insertKey = true;

                // for(auto & [keyIdx, keyText] : FUNCTION_KEY_COMBO_ITEMS) {
                //     if(keysPressed[FUNCTION_KEY_COMBO_SCANCODES.at(keyIdx)]) {
                //         selectedFuncKey = keyIdx;
                //         insertKey = true;
                //         break;
                //     }
                // }

                if(insertKey) {
                    std::string keyText = FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey);

                    std::shared_ptr<EditAction> currAction;
                    auto foundItem = chartinfo.notes.containsItemAt(insertBeat, insertItemType);
                    if(foundItem.get()) {
                        currAction = std::make_shared<EditNoteAction>(unsaved, insertBeat, (NoteSequenceItem::SequencerItemType)insertItemType, foundItem->displayText, keyText);
                        chartinfo.notes.editNote(insertBeat, insertItemType, keyText);
                    } else {
                        chartinfo.notes.addNote(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (NoteSequenceItem::SequencerItemType)insertItemType, keyText);
                        currAction = std::make_shared<PlaceNoteAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (NoteSequenceItem::SequencerItemType)insertItemType, keyText);
                    }

                    editActionsUndo.push(currAction);
                    utils::emptyActionStack(editActionsRedo);

                    selectedFuncKey = 0;
                    ImGui::CloseCurrentPopup();
                    unsaved = true;
                    insertKey = false;
                    leftClickReleased = false;
                }
                break;
            case NoteSequenceItem::SequencerItemType::SKIP:
                {
                    if(!ImGui::IsAnyItemActive() && !ImGuiFileDialog::Instance()->IsOpened() && !ImGui::IsMouseClicked(0))
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
                        utils::emptyActionStack(editActionsRedo);

                        ImGui::CloseCurrentPopup();
                        unsaved = true;

                        leftClickReleased = false;
                    }
                }
                break;
            case NoteSequenceItem::SequencerItemType::STOP:
                chartinfo.notes.addStop(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);

                auto putAction = std::make_shared<PlaceStopAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);
                editActionsUndo.push(putAction);
                utils::emptyActionStack(editActionsRedo);

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
            utils::emptyActionStack(editActionsRedo);

            unsaved = true;

            if(itemToDelete->getItemType() == NoteSequenceItem::SequencerItemType::SKIP) {
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

        float scrollAmt = io.MouseWheel;
        bool decrease = io.MouseWheel < 0;
        scrollAmt *= (2.f / timelineZoom);
        int beatsplitChange = std::lround(scrollAmt);
        if(beatsplitChange == 0)
            beatsplitChange = decrease ? -1 : 1;

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
