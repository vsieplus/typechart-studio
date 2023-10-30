#include "actions/placenote.hpp"
#include "ui/editwindow.hpp"

PlaceNoteAction::PlaceNoteAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, NoteSequenceItem::SequencerItemType itemType, std::string displayText) :
    EditAction(unsaved), absBeat(absBeat), beatDuration(beatDuration), beatpos(beatpos), endBeatpos(endBeatpos), itemType(itemType), displayText(displayText) {}

void PlaceNoteAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.deleteItem(absBeat, itemType);
}

void PlaceNoteAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.addNote(absBeat, editWindow->songpos.absBeat, beatDuration,beatpos, endBeatpos, itemType, displayText);
}
