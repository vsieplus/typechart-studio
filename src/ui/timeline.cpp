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

void Timeline::editCopy() {
    copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    haveSelection = false;
    activateCopy = false;
}

void Timeline::editCut(ChartInfo & chartinfo) {
    copiedItems = chartinfo.notes.getItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    chartinfo.notes.deleteItems(insertBeat, endBeat, insertItemType, insertItemTypeEnd);

    if(!copiedItems.empty()) {
        auto delAction { std::make_shared<DeleteItemsAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat, copiedItems) };
        undoStack.push(delAction);
        utils::emptyActionStack(redoStack);

        unsaved = true;
    }

    haveSelection = false;
    activateCut = false;
}

void Timeline::editFlip(ChartInfo & chartinfo) {
    auto flipAction { std::make_shared<FlipNoteAction>(unsaved, insertItemType, insertItemTypeEnd, insertBeat, endBeat, chartinfo.keyboardLayout) };
    undoStack.push(flipAction);
    utils::emptyActionStack(redoStack);

    chartinfo.notes.flipNotes(chartinfo.keyboardLayout, insertBeat, endBeat, insertItemType, insertItemTypeEnd);
    unsaved = true;
    activateFlip = false;
}

void Timeline::checkEditActions(bool focused, std::vector<bool> & keysPressed) {
    const auto & io = ImGui::GetIO();

    // copy, cut, delete selection of notes
    if(focused && haveSelection) {
        if(activateCopy || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_c)])) {
            editCopy();
        } else if(activateCut || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_x)])) {
            editCut();
        } else if(activateFlip || keysPressed[SDL_GetScancodeFromKey(SDLK_f)]) {
            editFlip();
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
            undoStack.push(shiftAction);
            utils::emptyActionStack(redoStack);

            unsaved = true;
        }

        if(keysPressed[SDL_SCANCODE_DELETE]) {
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

        if(keysPressed[SDL_SCANCODE_ESCAPE]) {
            haveSelection = false;
        }
    }
}

void Timeline::showContents(int musicSourceIdx, bool focused, AudioSystem * audioSystem, ChartInfo & chartinfo, SongPosition & songpos,
    std::vector<bool> & keysPressed)
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

    if(focused && (activatePaste || (io.KeyCtrl && keysPressed[SDL_GetScancodeFromKey(SDLK_v)]))) {
        if(!copiedItems.empty()) {
            auto hoveredBeatEnd = hoveredBeat + (copiedItems.back()->beatEnd - copiedItems.front()->absBeat);
            auto overwrittenItems = chartinfo.notes.getItems(hoveredBeat, hoveredBeatEnd, insertItemType, insertItemTypeEnd);
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

    if(ImGui::BeginPopup(addItemPopup.c_str())) {
        showAddItem(keysPressed);
        ImGui::EndPopup();
    }

    checkDeleteItem(focused);

    // update notes
    if(focused) {
        chartinfo.notes.update(songpos.absBeat, audioSystem, Preferences::Instance().isNotesoundEnabled());
    }

    showHorizontalScroll(audioSystem);
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

void Timeline::showAddItem(std::vector <bool> & keysPressed) {
    static char addedItem[2];

    if(keysPressed[SDL_SCANCODE_ESCAPE]) {
        addedItem[0] = '\0';
        ImGui::CloseCurrentPopup();
    }

    switch(static_cast<NoteSequenceItem::SequencerItemType>(insertItemType)) {
        case NoteSequenceItem::SequencerItemType::TOP_NOTE:
        case NoteSequenceItem::SequencerItemType::MID_NOTE:
            ImGui::SetNextItemWidth(32);
            if(!ImGui::IsAnyItemActive() && !ImGuiFileDialog::Instance()->IsOpened() && !ImGui::IsMouseClicked(0))
                ImGui::SetKeyboardFocusHere(0);

            if(ImGui::InputText("##addnote_text", addedItem, 2, addItemFlags, utils::filterInputMiddleKey, (void *)chartinfo.keyboardLayout.c_str())) {
                if(addedItem[0] != '\0') {
                    std::string keyText{ addedItem };

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

                    undoStack.push(currAction);
                    utils::emptyActionStack(redoStack);

                    addedItem[0] = '\0';
                    ImGui::CloseCurrentPopup();
                    unsaved = true;
                }
            }
            break;
        case NoteSequenceItem::SequencerItemType::BOT_NOTE:
            static int selectedFuncKey = 0;
            ImGui::SetNextItemWidth(64);

            if(ImGui::BeginCombo("##addfunction_key", constants::FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey).c_str())) {
                for(auto & [keyIdx, keyTxt] : constants::FUNCTION_KEY_COMBO_ITEMS) {
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

            std::string keyText = constants::FUNCTION_KEY_COMBO_ITEMS.at(selectedFuncKey);

            std::shared_ptr<EditAction> currAction;
            auto foundItem = chartinfo.notes.containsItemAt(insertBeat, insertItemType);
            if(foundItem.get()) {
                currAction = std::make_shared<EditNoteAction>(unsaved, insertBeat, (NoteSequenceItem::SequencerItemType)insertItemType, foundItem->displayText, keyText);
                chartinfo.notes.editNote(insertBeat, insertItemType, keyText);
            } else {
                chartinfo.notes.addNote(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (NoteSequenceItem::SequencerItemType)insertItemType, keyText);
                currAction = std::make_shared<PlaceNoteAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos, (NoteSequenceItem::SequencerItemType)insertItemType, keyText);
            }

            undoStack.push(currAction);
            utils::emptyActionStack(redoStack);

            selectedFuncKey = 0;
            ImGui::CloseCurrentPopup();
            unsaved = true;
            insertKey = false;
            leftClickReleased = false;
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

                    undoStack.push(currAction);
                    utils::emptyActionStack(redoStack);

                    ImGui::CloseCurrentPopup();
                    unsaved = true;

                    leftClickReleased = false;
                }
            }
            break;
        case NoteSequenceItem::SequencerItemType::STOP:
            chartinfo.notes.addStop(insertBeat, songpos.absBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);

            auto putAction = std::make_shared<PlaceStopAction>(unsaved, insertBeat, endBeat - insertBeat, insertBeatpos, endBeatpos);
            undoStack.push(putAction);
            utils::emptyActionStack(redoStack);

            ImGui::CloseCurrentPopup();
            unsaved = true;

            break;
    }
}

void Timeline::checkDeleteItem(bool focused) {
    // check to delete item
    if(focused && !ImGuiFileDialog::Instance()->IsOpened() && rightClickedEntity) {
        auto itemToDelete = chartinfo.notes.containsItemAt(clickedBeat, clickedItemType);
        if(itemToDelete.get()) {
            chartinfo.notes.deleteItem(clickedBeat, clickedItemType);
            auto deleteAction = std::make_shared<DeleteNoteAction>(unsaved, itemToDelete->absBeat, itemToDelete->beatEnd - itemToDelete->absBeat,
                itemToDelete->beatpos, itemToDelete->endBeatpos, itemToDelete->getItemType(), itemToDelete->displayText);
            undoStack.push(deleteAction);
            utils::emptyActionStack(redoStack);

            unsaved = true;

            if(itemToDelete->getItemType() == NoteSequenceItem::SequencerItemType::SKIP) {
                songpos.removeSkip(clickedBeat);
            }
        }
    }
}

void Timeline::showHorizontalScroll(AudioSystem * audioSystem) {
    const auto & io = ImGui::GetIO();

    // sideways scroll
    if(focused && !ImGuiFileDialog::Instance()->IsOpened() && io.MouseWheel != 0.f && !io.KeyCtrl) {
        if(!songpos.started) {
            songpos.start();
            songpos.pause();

            // decrease pause counter manually by offset, since skipping
            songpos.pauseCounter += (Uint64)((songpos.offsetMS / 1000.f) * SDL_GetPerformanceFrequency());
        }

        double scrollAmt = io.MouseWheel;
        bool decrease = io.MouseWheel < 0;
        scrollAmt *= (2.0 / zoom);
        auto beatsplitChange = std::lround(scrollAmt);
        if(beatsplitChange == 0)
            beatsplitChange = decrease ? -1 : 1;

        auto fullBeats = (int)std::floor(songpos.absBeat);
        auto fullBeatSplits = (int)((songpos.absBeat - fullBeats) / currentBeatsplitValue + 0.5);
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
