#include "actions/flipnote.hpp"
#include "ui/editwindow.hpp"

FlipNoteAction::FlipNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat,
        float endBeat, std::string keyboardLayout) : 
    EditAction(unsaved), minItemType(minItemType), maxItemType(maxItemType), startBeat(startBeat),
    endBeat(endBeat), keyboardLayout(keyboardLayout) {}

void FlipNoteAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);

    // undo/redo is the same behavior
    redoAction(editWindow);
}

void FlipNoteAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.flipNotes(keyboardLayout, startBeat, endBeat, minItemType, maxItemType);
}
