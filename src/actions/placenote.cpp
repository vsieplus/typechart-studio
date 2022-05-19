#include "actions/placenote.hpp"

PlaceNoteAction::PlaceNoteAction(NoteSequence & noteSequence, bool & unsaved, float absBeat, float beatDuration, BeatPos beatpos,
                                 BeatPos endBeatpos, SequencerItemType itemType, std::string displayText) :
    EditAction(unsaved), noteSequence(noteSequence), absBeat(absBeat), beatDuration(beatDuration), beatpos(beatpos), endBeatpos(endBeatpos),
    itemType(itemType), displayText(displayText) {}

void PlaceNoteAction::undoAction() {
    EditAction::undoAction();
    noteSequence.deleteItem(absBeat, itemType);
}

void PlaceNoteAction::redoAction() {
    noteSequence.addNote(absBeat, beatDuration, beatpos, endBeatpos, itemType, displayText);
}