#include "ui/timeline.hpp"

#include <format>

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

#include "ImGuiFileDialog.h"

namespace utils {

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

void updateAudioPosition(AudioSystem * audioSystem, const SongPosition & songpos, int musicSourceIdx) {
    // udpate audio position
    auto offsetAbsTime = static_cast<float>(songpos.absTime + (songpos.offsetMS / 1000.0));
    if(audioSystem->isMusicPlaying(musicSourceIdx)) {
        audioSystem->startMusic(musicSourceIdx, offsetAbsTime);
    } else {
        audioSystem->setMusicPosition(musicSourceIdx, offsetAbsTime);
    }
}

} // namespace utils

void Timeline::showContents(int musicSourceIdx, bool focused, bool & unsaved, AudioSystem * audioSystem,
    ChartInfo & chartinfo, SongPosition & songpos, std::vector<bool> & keysPressed)
{
    showBeatsplit();
    showCurrentBeat(musicSourceIdx, chartinfo, songpos, audioSystem);
    showBeatpos(songpos);
    showZoom(focused);
    showSequencer(focused, chartinfo, songpos);

    checkResetClicks();
    checkUpdatedBeat(focused, musicSourceIdx, chartinfo, songpos, audioSystem);

    std::string addItemPopup = std::format("add_item_{}", musicSourceIdx);

    // insert or update entity at the clicked beat
    prepUpdateEntity(focused, addItemPopup, songpos);
    setEntityType(focused, addItemPopup, songpos);

    checkEditActions(focused, keysPressed);

    if(ImGui::BeginPopup(addItemPopup.c_str())) {
        showAddItem(keysPressed);
        ImGui::EndPopup();
    }

    checkDeleteItem(focused, unsaved, chartinfo, songpos);
    checkUpdateNotes(focused, audioSystem, chartinfo, songpos);
    showHorizontalScroll(musicSourceIdx, chartinfo, songpos, audioSystem);
}

void Timeline::showBeatsplit() {
    ImGui::PushItemWidth(130);
    ImGui::Text("Beatsplit: ");
    ImGui::SameLine();
    ImGui::InputInt("##beatsplit", &currentBeatsplit, 1, 4);
    currentBeatsplit = std::max(1, currentBeatsplit);
    currentBeatsplitValue = 1.0 / currentBeatsplit;
}

void Timeline::showCurrentBeat(int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem) {
    ImGui::SameLine();
    ImGui::Text("Current beat: ");
    ImGui::SameLine();

    auto fullBeats = static_cast<int>(std::floor(songpos.absBeat));
    auto fullBeatSplits = static_cast<int>((songpos.absBeat - fullBeats) / currentBeatsplitValue + 0.5);
    double origNearBeat = fullBeats + (fullBeatSplits * (1.0 / currentBeatsplit));
    double prevTargetBeat = (songpos.absBeat == origNearBeat) ? origNearBeat - currentBeatsplitValue : origNearBeat;
    double nextTargetBeat = origNearBeat + (1.0 / currentBeatsplit);

    if(ImGui::InputDouble("##currbeat", &songpos.absBeat, currentBeatsplitValue, 2.0 / currentBeatsplit)) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();
            songpos.pauseCounter += static_cast<Uint64>((songpos.offsetMS / 1000.0) * static_cast<double>(SDL_GetPerformanceFrequency()));
        }

        // snap to the nearest beat split
        if(songpos.absBeat >= nextTargetBeat) {
            songpos.setSongBeatPosition(nextTargetBeat);
        } else if(songpos.absBeat <= prevTargetBeat) {
            songpos.setSongBeatPosition(prevTargetBeat);
        }

        if(songpos.absTime >= 0) {
            utils::updateAudioPosition(audioSystem, songpos, musicSourceIdx);
        } else {
            songpos.setSongBeatPosition(0);
        }

        chartinfo.notes.resetPassed(songpos.absBeat);
    }
}

void Timeline::showBeatpos(const SongPosition & songpos) {
    auto currBeatpos = utils::calculateBeatpos(songpos.absBeat, currentBeatsplit, songpos.timeinfo);
    ImGui::SameLine();
    ImGui::Text("[Pos]: [%d, %d, %d]", std::max(0, currBeatpos.measure), std::max(0, currBeatpos.measureSplit), std::max(0, currBeatpos.split));
}

void Timeline::showZoom(bool focused) {
    ImGui::SameLine();
    ImGui::Text("Zoom " ICON_FA_MAGNIFYING_GLASS ": ");
    ImGui::SameLine();
    ImGui::InputDouble("##zoom", &zoom, constants::ZOOM_STEP, constants::ZOOM_STEP * 2, "%.2f");
    if(ImGui::IsItemHovered() && !ImGui::IsItemActive())
        ImGui::SetTooltip("Hold [Ctrl] while scrolling to zoom in/out");

    ImGui::PopItemWidth();

    // ctrl + scroll to adjust zoom
    if(const auto & io = ImGui::GetIO(); focused && !ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && io.KeyCtrl && !io.KeyShift) {
        int multiplier = io.MouseWheel > 0 ? 1 : -1;

        zoom += constants::ZOOM_STEP * multiplier;
    }

    zoom = ImMax(zoom, constants::ZOOM_STEP);
}

void Timeline::showSequencer(bool focused, ChartInfo & chartinfo, SongPosition & songpos) {
    rightClickedEntity = false;

    int beatsPerMeasure = songpos.timeinfo.size() > songpos.currentSection ? songpos.timeinfo.at(songpos.currentSection).beatsPerMeasure : 4;
    Sequencer(&(chartinfo.notes), zoom, currentBeatsplit, beatsPerMeasure, Preferences::Instance().isDarkTheme(), haveSelection, focused,
        nullptr, &updatedBeat, &leftClickedEntity, &leftClickReleased, &leftClickShift, &rightClickedEntity, &clickedBeat, &hoveredBeat,
        &clickedItemType, &releasedItemType, nullptr, &songpos.absBeat, ImSequencer::SEQUENCER_CHANGE_FRAME);
}

void Timeline::checkResetClicks() {
    if(ImGuiFileDialog::Instance()->IsOpened()) {
        leftClickedEntity = false;
        leftClickReleased = false;
        rightClickedEntity = false;
    }
}

void Timeline::checkUpdatedBeat(bool focused, int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem) {
    if(focused && updatedBeat) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();

            songpos.pauseCounter += static_cast<Uint64>((songpos.offsetMS / 1000.0) * static_cast<double>(SDL_GetPerformanceFrequency()));
        }
        songpos.setSongBeatPosition(songpos.absBeat);

        if(songpos.absTime >= 0) {
            utils::updateAudioPosition(audioSystem, songpos, musicSourceIdx);
        } else {
            songpos.setSongBeatPosition(0);
        }

        chartinfo.notes.resetPassed(songpos.absBeat);
    }
}

void Timeline::prepUpdateEntity(bool focused, std::string_view addItemPopup, const SongPosition & songpos) {
    if(focused && !ImGuiFileDialog::Instance()->IsOpened() && leftClickedEntity && !ImGui::IsPopupOpen(addItemPopup.c_str())) {
        bool hadSelection = haveSelection;
        if(haveSelection) {
            haveSelection = false;
        }

        if(!hadSelection || leftClickShift) {
            insertBeat = clickedBeat;
            insertItemType = clickedItemType;

            insertBeatpos = utils::calculateBeatpos(clickedBeat, currentBeatsplit, songpos.timeinfo);

            addItemFlags = ImGuiInputTextFlags_CharsUppercase;

            switch(static_cast<NoteSequenceItem::SequencerItemType>(insertItemType)) {
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
}

void Timeline::setEntityType(bool focused, std::string_view addItemPopup, const SongPosition & songpos) {
    if(focused && !ImGuiFileDialog::Instance()->IsOpened() && startedNote && leftClickReleased &&
        !ImGui::IsPopupOpen(addItemPopup.c_str()) && clickedBeat >= insertBeat)
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
            ImGui::OpenPopup(addItemPopup.c_str());
        }
        
        endBeat = clickedBeat;
        endBeatpos = utils::calculateBeatpos(endBeat, currentBeatsplit, songpos.timeinfo);

        leftClickReleased = false;
        leftClickShift = false;
        startedNote = false;
    }
}

void Timeline::checkEditActions(bool focused, bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos, std::vector<bool> & keysPressed) {
    const auto & io = ImGui::GetIO();

    // copy, cut, delete selection of notes
    if(focused && haveSelection) {
        if(activateCopy || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_c)])) {
            editCopy();
        } else if(activateCut || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_x)])) {
            editCut(unsaved, chartinfo);
        } else if(activateFlip || keysPressed[SDL_GetScancodeFromKey(SDLK_f)]) {
            editFlip(unsaved, chartinfo);
        }

        editShiftNotes(unsaved, chartinfo);

        if(keysPressed[SDL_SCANCODE_DELETE]) {
            editDelete();
        }

        if(keysPressed[SDL_SCANCODE_ESCAPE]) {
            haveSelection = false;
        }
    }

    if(focused && (activatePaste || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_v)]))) {
        editPaste(unsaved, chartinfo, songpos);
    }
}


void Timeline::editCopy() {
    copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    haveSelection = false;
    activateCopy = false;
}

void Timeline::editCut(bool & unsaved, ChartInfo & chartinfo) {
    copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

    if(!copiedItems.empty()) {
        auto delAction { std::make_shared<DeleteItemsAction>(insertItemType, insertItemTypeEnd, insertBeat, endBeat, copiedItems) };
        undoStack.push(delAction);
        utils::emptyActionStack(redoStack);

        unsaved = true;
    }

    haveSelection = false;
    activateCut = false;
}

void Timeline::editFlip(bool & unsaved, ChartInfo & chartinfo) {
    auto flipAction { std::make_shared<FlipNoteAction>(insertItemType, insertItemTypeEnd, insertBeat, endBeat, chartinfo.keyboardLayout) };
    undoStack.push(flipAction);
    utils::emptyActionStack(redoStack);

    chartinfo.notes.flipNotes(chartinfo.keyboardLayout, insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    unsaved = true;
    activateFlip = false;
}

void Timeline::editShiftNotes(bool & unsaved, ChartInfo & chartinfo, const std::vector<bool> & keysPressed) {
    // shift notes up, down, left, right
    ShiftNoteAction::ShiftDirection shiftDirection { ShiftNoteAction::ShiftDirection::ShiftNone };
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
        auto items { chartinfo.notes.shiftNotes(chartinfo.keyboardLayout, insertBeat, endBeat, insertItemType, insertItemTypeEnd, shiftDirection) };

        auto shiftAction { std::make_shared<ShiftNoteAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat, 
            chartinfo.keyboardLayout, shiftDirection, items) };
        undoStack.push(shiftAction);
        utils::emptyActionStack(redoStack);

        unsaved = true;
    }
}

void Timeline::editDelete(bool & unsaved, ChartInfo & chartinfo) {
    auto deletedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

    if(!deletedItems.empty()) {
        auto delAction = std::make_shared<DeleteItemsAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat, deletedItems);
        undoStack.push(delAction);
        utils::emptyActionStack(redoStack);

        unsaved = true;
    }

    haveSelection = false;
}

void Timeline::editPaste(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos) {
    if(!copiedItems.empty()) {
        double hoveredBeatEnd { hoveredBeat + (copiedItems.back()->beatEnd - copiedItems.front()->absBeat) };
        auto overwrittenItems { chartinfo.notes.getItems(hoveredBeat, hoveredBeatEnd, insertItemType, insertItemTypeEnd) };
        chartinfo.notes.insertItems(hoveredBeat, songpos.absBeat, insertItemType, insertItemTypeEnd, songpos.timeinfo, copiedItems);

        if(!overwrittenItems.empty()) {
            auto delAction = std::make_shared<DeleteItemsAction>(unsaved, insertItemType, insertItemTypeEnd, hoveredBeat, hoveredBeatEnd, overwrittenItems);
            undoStack.push(delAction);
        }

        auto insAction = std::make_shared<InsertItemsAction>(unsaved, insertItemType, insertItemTypeEnd, hoveredBeat, copiedItems, overwrittenItems);
        undoStack.push(insAction);
        utils::emptyActionStack(redoStack);

        unsaved = true;
    }

    activatePaste = false;
}

void Timeline::showAddItem(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos, std::vector <bool> & keysPressed) {
    static char addedItem[2];

    if(keysPressed[SDL_SCANCODE_ESCAPE]) {
        addedItem[0] = '\0';
        ImGui::CloseCurrentPopup();
    }

    switch(static_cast<NoteSequenceItem::SequencerItemType>(insertItemType)) {
        case NoteSequenceItem::SequencerItemType::TOP_NOTE:
        case NoteSequenceItem::SequencerItemType::MID_NOTE:
            showTopMidNote(unsaved, addedItem, chartinfo, songpos);
            break;
        case NoteSequenceItem::SequencerItemType::BOT_NOTE:
            showBotNote(unsaved, chartinfo, songpos);
            break;
        case NoteSequenceItem::SequencerItemType::SKIP:
            showSkip(unsaved, chartinfo, songpos);
            break;
        case NoteSequenceItem::SequencerItemType::STOP:
            showStop(unsaved, chartinfo, songpos);
            break;
    }
}


void Timeline::showTopMidNote(bool & unsaved, char * addedItem, ChartInfo & chartinfo, const SongPosition & songpos) {
    ImGui::SetNextItemWidth(32);
    if(!ImGui::IsAnyItemActive() && !ImGuiFileDialog::Instance()->IsOpened() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    if(ImGui::InputText("##addnote_text", addedItem, 2, addItemFlags, utils::filterInputMiddleKey, (void *)chartinfo.keyboardLayout.c_str())) {
        if(addedItem[0] != '\0') {
            std::string keyText{ addedItem };
            auto itemType { static_cast<NoteSequenceItem::SequencerItemType>(insertItemType) };

            std::shared_ptr<EditAction> currAction { nullptr };
            auto foundItem { chartinfo.notes.containsItemAt(insertBeat, insertItemType) };

            if(foundItem) {
                currAction = std::make_shared<EditNoteAction>(insertBeat, itemType, foundItem->displayText, keyText);
                chartinfo.notes.editNote(insertBeat, insertItemType, keyText);
            } else {
                auto beatDuration = endBeat - insertBeat;
                chartinfo.notes.addNote(insertBeat, songpos.absBeat, beatDuration, insertBeatpos, endBeatpos, itemType, keyText);
                currAction = std::make_shared<PlaceNoteAction>(insertBeat, beatDuration, insertBeatpos, endBeatpos, itemType, keyText);
            }

            undoStack.push(currAction);
            utils::emptyActionStack(redoStack);

            addedItem[0] = '\0';
            ImGui::CloseCurrentPopup();
            unsaved = true;
        }
    }
}

void Timeline::showBotNote(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos) {
    static int selectedFuncKey { 0 };
    ImGui::SetNextItemWidth(64);

    if(ImGui::BeginCombo("##addfunction_key", constants::FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey).c_str())) {
        for(const auto & [keyIdx, keyTxt] : constants::FUNCTION_KEY_COMBO_ITEMS) {
            bool keySelected = false;
            if(ImGui::Selectable(keyTxt.c_str(), &keySelected)) {
                selectedFuncKey = keyIdx;
                break;
            }

            if(keySelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    std::string keyText { constants::FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey) };

    std::shared_ptr<EditAction> currAction;
    auto itemType = static_cast<NoteSequenceItem::SequencerItemType>(insertItemType);
    auto foundItem = chartinfo.notes.containsItemAt(insertBeat, itemType);

    if(foundItem.get()) {
        currAction = std::make_shared<EditNoteAction>(insertBeat, itemType, foundItem->displayText, keyText);
        chartinfo.notes.editNote(insertBeat, itemType, keyText);
    } else {
        auto beatDuration { endBeat - insertBeat };
        chartinfo.notes.addNote(insertBeat, songpos.absBeat, beatDuration, insertBeatpos, endBeatpos, itemType, keyText);
        currAction = std::make_shared<PlaceNoteAction>(insertBeat, beatDuration, insertBeatpos, endBeatpos, itemType, keyText);
    }

    undoStack.push(currAction);
    utils::emptyActionStack(redoStack);

    ImGui::CloseCurrentPopup();

    selectedFuncKey = 0;
    unsaved = true;
    leftClickReleased = false;
}

void Timeline::showSkip(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos) {
    if(!ImGui::IsAnyItemActive() && !ImGuiFileDialog::Instance()->IsOpened() && !ImGui::IsMouseClicked(0)) {
        ImGui::SetKeyboardFocusHere(0);
    }

    ImGui::SetNextItemWidth(128);

    static double skipBeats { 0.0 };
    ImGui::InputDouble("Skip beats", &skipBeats, currentBeatsplitValue / 2.0, currentBeatsplitValue / 2.0);
    ImGui::SameLine();
    HelpMarker("How many beats will the skip be displayed for?\n- 0 -> the skip is instant\n"
            "- skip_duration / 2 -> the skip will take half the time as without the skip\n"
            "- skip_duration -> will look the same as no skip");

    if(skipBeats < 0.0) {
        skipBeats = 0.0;
    }

    auto currItem { chartinfo.notes.containsItemAt(insertBeat, insertItemType) };
    if(currItem) {
        auto currSkip { std::dynamic_pointer_cast<Skip>(currItem) };
        if(skipBeats > currSkip->beatDuration) {
            skipBeats = currSkip->beatDuration;
        }
    } else {
        if(skipBeats > endBeat - insertBeat) {
            skipBeats = endBeat - insertBeat;
        }
    }

    if(keysPressed[SDL_SCANCODE_RETURN] || keysPressed[SDL_SCANCODE_KP_ENTER]) {
        std::shared_ptr<EditAction> currAction { nullptr };
        if(currItem) {
            auto currSkip { std::dynamic_pointer_cast<Skip>(currItem) };
            currAction = std::make_shared<EditSkipAction>(insertBeat, currSkip->skipTime, skipBeats);
            chartinfo.notes.editSkip(insertBeat, skipBeats);
        } else {
            auto newSkip { chartinfo.notes.addSkip(insertBeat, songpos.absBeat, skipBeats, endBeat - insertBeat, insertBeatpos, endBeatpos) };
            currAction = std::make_shared<PlaceSkipAction>(insertBeat, skipBeats, endBeat - insertBeat, insertBeatpos, endBeatpos);

            songpos.addSkip(newSkip);
        }

        undoStack.push(currAction);
        utils::emptyActionStack(redoStack);

        ImGui::CloseCurrentPopup();
        unsaved = true;

        leftClickReleased = false;
    }
}

void Timeline::showStop(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos) {
    chartinfo.notes.addStop(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);

    auto putAction { std::make_shared<PlaceStopAction>(insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos) };
    undoStack.push(putAction);
    utils::emptyActionStack(redoStack);

    ImGui::CloseCurrentPopup();
    unsaved = true;
}

void Timeline::checkDeleteItem(bool focused, bool & unsaved, ChartInfo & chartinfo, SongPosition & songpos) {
    // check to delete item
    if(focused && !ImGuiFileDialog::Instance()->IsOpened() && rightClickedEntity) {
        auto itemToDelete { chartinfo.notes.containsItemAt(clickedBeat, clickedItemType) };
        if(itemToDelete.get()) {
            chartinfo.notes.deleteItem(clickedBeat, clickedItemType);
            auto deleteAction { std::make_shared<DeleteNoteAction>(itemToDelete->absBeat, itemToDelete->beatEnd - itemToDelete->absBeat,
                itemToDelete->beatpos, itemToDelete->endBeatpos, itemToDelete->itemType, itemToDelete->displayText) };
            undoStack.push(deleteAction);
            utils::emptyActionStack(redoStack);

            unsaved = true;

            if(itemToDelete->itemType == NoteSequenceItem::SequencerItemType::SKIP) {
                songpos.removeSkip(clickedBeat);
            }
        }
    }
}

void Timeline::checkUpdateNotes(bool focused, AudioSystem * audioSystem, ChartInfo & chartinfo, const SongPosition & songpos) {
    // update notes
    if(focused) {
        chartinfo.notes.update(songpos.absBeat, audioSystem, Preferences::Instance().isNotesoundEnabled());
    }
}

void Timeline::showHorizontalScroll(int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem) {
    const auto & io = ImGui::GetIO();

    // sideways scroll
    if(focused && !ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && !io.KeyCtrl) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();

            // decrease pause counter manually by offset, since skipping
            songpos.pauseCounter += (Uint64)((songpos.offsetMS / 1000.0) * static_cast<double>(SDL_GetPerformanceFrequency()));
        }

        double scrollAmt = io.MouseWheel;
        bool decrease = io.MouseWheel < 0;
        scrollAmt *= (2.0 / zoom);
        auto beatsplitChange = std::lround(scrollAmt);
        if(beatsplitChange == 0)
            beatsplitChange = decrease ? -1 : 1;

        auto fullBeats = static_cast<int>(std::floor(songpos.absBeat));
        auto fullBeatSplits = static_cast<int>((songpos.absBeat - fullBeats) / currentBeatsplitValue + 0.5);
        double origNearBeat = fullBeats + (fullBeatSplits * currentBeatsplitValue);

        double targetBeat;
        if(decrease && songpos.absBeat > origNearBeat) {
            targetBeat = origNearBeat - static_cast<double>(beatsplitChange - 1) * currentBeatsplitValue;
        } else {
            targetBeat = origNearBeat - static_cast<double>(beatsplitChange) * currentBeatsplitValue;
        }

        if(decrease || (!decrease && songpos.absTime < audioSystem->getMusicLength(musicSourceIdx))) {
            // scroll up, decrease beat, scroll down increase beat
            songpos.setSongBeatPosition(targetBeat);
        }

        if(songpos.absTime >= 0) {
            utils::updateAudioPosition(audioSystem, songpos, musicSourceIdx);
        } else {
            audioSystem->stopMusic(musicSourceIdx);
            songpos.stop();
        }

        chartinfo.notes.resetPassed(songpos.absBeat);
    }
}

int Timeline::getUndoStackSize() const {
    return static_cast<int>(undoStack.size());
}

int Timeline::getRedoStackSize() const {
    return static_cast<int>(redoStack.size());
}

void Timeline::setCopy(bool copy) {
    activateCopy = copy;
}

void Timeline::setPaste(bool paste) {
    activatePaste = paste;
}

void Timeline::setCut(bool cut) {
    activateCut = cut;
}

void Timeline::setFlip(bool flip) {
    activateFlip = flip;
}
