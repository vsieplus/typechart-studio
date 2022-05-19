#include "actions/placestop.hpp"
#include "ui/editwindow.hpp"


PlaceStopAction::PlaceStopAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) :
    EditAction(unsaved), absBeat(absBeat), beatDuration(beatDuration), beatpos(beatpos), endBeatpos(endBeatpos) {}

void PlaceStopAction::undoAction(EditWindowData * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.deleteItem(absBeat, SequencerItemType::STOP);
}

void PlaceStopAction::redoAction(EditWindowData * editWindow) {
    editWindow->chartinfo.notes.addStop(absBeat, beatDuration, beatpos, endBeatpos);
}