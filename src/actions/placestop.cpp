#include "actions/placestop.hpp"
#include "ui/editwindow.hpp"


PlaceStopAction::PlaceStopAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) :
    EditAction(unsaved), absBeat(absBeat), beatDuration(beatDuration), beatpos(beatpos), endBeatpos(endBeatpos) {}

void PlaceStopAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.deleteItem(absBeat, NoteSequenceItem::SequencerItemType::STOP);
}

void PlaceStopAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.addStop(absBeat, editWindow->songpos.absBeat, beatDuration, beatpos, endBeatpos);
}
