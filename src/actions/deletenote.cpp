#include "actions/deletenote.hpp"
#include "ui/editwindow.hpp"

DeleteNoteAction::DeleteNoteAction(double absBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos, NoteSequenceItem::SequencerItemType itemType, std::string_view displayText)
    : absBeat(absBeat)
    , beatDuration(beatDuration)
    , beatpos(beatpos)
    , endBeatpos(endBeatpos)
    , itemType(itemType)
    , displayText(displayText) {}

void DeleteNoteAction::undoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.addNote(absBeat, editWindow->songpos.absBeat, beatDuration, beatpos, endBeatpos, itemType, displayText);
}

void DeleteNoteAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.deleteItem(absBeat, itemType);
}
