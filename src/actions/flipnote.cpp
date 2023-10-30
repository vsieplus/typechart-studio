#include "actions/flipnote.hpp"
#include "ui/editwindow.hpp"

FlipNoteAction::FlipNoteAction(int minItemType, int maxItemType, double startBeat, double endBeat, std::string_view keyboardLayout)
    : minItemType(minItemType)
    , maxItemType(maxItemType)
    , startBeat(startBeat)
    , endBeat(endBeat)
    , keyboardLayout(keyboardLayout) {}

void FlipNoteAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);

    // undo/redo is the same behavior
    redoAction(editWindow);
}

void FlipNoteAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.flipNotes(keyboardLayout, startBeat, endBeat, minItemType, maxItemType);
}
