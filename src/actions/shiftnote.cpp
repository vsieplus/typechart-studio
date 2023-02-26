#include "actions/shiftnote.hpp"
#include "ui/editwindow.hpp"

ShiftNoteAction::ShiftNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat, float endBeat, std::string keyboardLayout,
        ShiftDirection shiftDirection, std::list<std::shared_ptr<NoteSequenceItem>> items) :
    EditAction(unsaved), minItemType(minItemType), maxItemType(maxItemType), startBeat(startBeat),
    endBeat(endBeat), keyboardLayout(keyboardLayout), shiftDirection(shiftDirection), items(items) {}

void ShiftNoteAction::undoAction(EditWindowData * editWindow) {
    EditAction::undoAction(editWindow);

    ShiftDirection reverseDirection = ShiftNone;
    switch(shiftDirection) {
        case ShiftUp:
            reverseDirection = ShiftDown;
            break;
        case ShiftDown:
            reverseDirection = ShiftUp;
            break;
        case ShiftLeft:
            reverseDirection = ShiftRight;
            break;
        case ShiftRight:
            reverseDirection = ShiftLeft;
            break;
        default:
            break;
    }

    reconcileDeletedItems(editWindow);
    editWindow->chartinfo.notes.shiftItems(keyboardLayout, startBeat, endBeat, items, reverseDirection);
}

void ShiftNoteAction::redoAction(EditWindowData * editWindow) {
    reconcileDeletedItems(editWindow);
    editWindow->chartinfo.notes.shiftItems(keyboardLayout, startBeat, endBeat, items, shiftDirection);
}

void ShiftNoteAction::reconcileDeletedItems(EditWindowData * editWindow) {
    // if any items were deleted, try to find a replacement with matching beat / note
    // otherwise, just delete the item
    for(auto itemIter = items.begin(); itemIter != items.end();) {
        auto item = *itemIter;

        if(item && item->deleted) {
            std::shared_ptr<NoteSequenceItem> replacementItem = nullptr;
            for(auto note : editWindow->chartinfo.notes.myItems) {
                if(note && (note == item)) {
                    replacementItem = note;
                    break;
                }
            }

            itemIter = items.erase(itemIter);
            if(replacementItem) {
                itemIter = items.insert(itemIter, replacementItem);
            }
        } else {
            ++itemIter;
        }
    }
}
