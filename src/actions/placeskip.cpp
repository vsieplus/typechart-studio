#include "actions/placeskip.hpp"
#include "ui/editwindow.hpp"


PlaceSkipAction::PlaceSkipAction(bool unsaved, float absBeat, float skipBeats, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) :
    EditAction(unsaved), absBeat(absBeat), skipBeats(skipBeats), beatDuration(beatDuration), beatpos(beatpos), endBeatpos(endBeatpos) {}

void PlaceSkipAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.deleteItem(absBeat, NoteSequenceItem::SequencerItemType::SKIP);
    editWindow->songpos.removeSkip(absBeat);
}

void PlaceSkipAction::redoAction(EditWindow * editWindow) {
    auto skip = editWindow->chartinfo.notes.addSkip(absBeat, editWindow->songpos.absBeat, skipBeats, beatDuration, beatpos, endBeatpos);
    editWindow->songpos.addSkip(skip);
}
