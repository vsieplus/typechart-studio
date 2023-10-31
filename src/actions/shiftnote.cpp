#include "actions/shiftnote.hpp"
#include "ui/editwindow.hpp"

ShiftNoteAction::ShiftNoteAction(int minItemType, int maxItemType, double startBeat, double endBeat, std::string_view keyboardLayout,
    ShiftDirection shiftDirection, const std::list<std::shared_ptr<NoteSequenceItem>> & items)
    : minItemType(minItemType)
    , maxItemType(maxItemType)
    , startBeat(startBeat)
    , endBeat(endBeat)
    , keyboardLayout(keyboardLayout)
    , shiftDirection(shiftDirection)
    , items(items) {}

void ShiftNoteAction::undoAction(EditWindow * editWindow) {
    ShiftDirection reverseDirection = ShiftDirection::ShiftNone;
    switch(shiftDirection) {
        case ShiftDirection::ShiftUp:
            reverseDirection = ShiftDirection::ShiftDown;
            break;
        case ShiftDirection::ShiftDown:
            reverseDirection = ShiftDirection::ShiftUp;
            break;
        case ShiftDirection::ShiftLeft:
            reverseDirection = ShiftDirection::ShiftRight;
            break;
        case ShiftDirection::ShiftRight:
            reverseDirection = ShiftDirection::ShiftLeft;
            break;
        default:
            break;
    }

    reconcileDeletedItems(editWindow);
    editWindow->chartinfo.notes.shiftItems(keyboardLayout, startBeat, endBeat, items, reverseDirection);
}

void ShiftNoteAction::redoAction(EditWindow * editWindow) {
    reconcileDeletedItems(editWindow);
    editWindow->chartinfo.notes.shiftItems(keyboardLayout, startBeat, endBeat, items, shiftDirection);
}

void ShiftNoteAction::reconcileDeletedItems(const EditWindow * editWindow) {
    // if any items were deleted, try to find a replacement with matching beat / note
    // otherwise, just delete the item
    for(auto itemIter = items.begin(); itemIter != items.end();) {
        auto item = *itemIter;

        if(item && item->deleted) {
            std::shared_ptr<NoteSequenceItem> replacementItem { nullptr };
            for(auto note : editWindow->chartinfo.notes.myItems) {
                if(note && (*note == *item)) {
                    replacementItem = note;
                    break;
                }
            }

            itemIter = items.erase(itemIter);
            if(replacementItem) {
                itemIter = items.insert(itemIter, replacementItem);
                itemIter++;
            }
        } else {
            itemIter++;
        }
    }
}
