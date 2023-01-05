#include "actions/shiftnote.hpp"
#include "ui/editwindow.hpp"

ShiftNoteAction::ShiftNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat,
        float endBeat, std::string keyboardLayout, ShiftDirection shiftDirection) : 
    EditAction(unsaved), minItemType(minItemType), maxItemType(maxItemType), startBeat(startBeat),
    endBeat(endBeat), keyboardLayout(keyboardLayout), shiftDirection(shiftDirection) {}

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

    editWindow->chartinfo.notes.shiftNotes(keyboardLayout, startBeat, endBeat, minItemType, maxItemType, reverseDirection);
}

void ShiftNoteAction::redoAction(EditWindowData * editWindow) {
    editWindow->chartinfo.notes.shiftNotes(keyboardLayout, startBeat, endBeat, minItemType, maxItemType, shiftDirection);
}