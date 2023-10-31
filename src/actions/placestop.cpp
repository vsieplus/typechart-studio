#include "actions/placestop.hpp"
#include "ui/editwindow.hpp"


PlaceStopAction::PlaceStopAction(double absBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos)
    : absBeat(absBeat)
    , beatDuration(beatDuration)
    , beatpos(beatpos)
    , endBeatpos(endBeatpos) {}

void PlaceStopAction::undoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.deleteItem(absBeat, NoteSequenceItem::SequencerItemType::STOP);
}

void PlaceStopAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.addStop(absBeat, editWindow->songpos.absBeat, beatDuration, beatpos, endBeatpos);
}
