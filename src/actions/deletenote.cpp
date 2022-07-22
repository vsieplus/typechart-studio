#include "actions/deletenote.hpp"
#include "ui/editwindow.hpp"

DeleteNoteAction::DeleteNoteAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType, std::string displayText) :
    EditAction(unsaved), absBeat(absBeat), beatDuration(beatDuration), beatpos(beatpos), endBeatpos(endBeatpos), itemType(itemType), displayText(displayText) {}

void DeleteNoteAction::undoAction(EditWindowData * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.addNote(absBeat, editWindow->songpos.absBeat, beatDuration, beatpos, endBeatpos, itemType, displayText);
}

void DeleteNoteAction::redoAction(EditWindowData * editWindow) {
    editWindow->chartinfo.notes.deleteItem(absBeat, itemType);
}