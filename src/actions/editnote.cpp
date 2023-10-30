#include "actions/editnote.hpp"
#include "ui/editwindow.hpp"

EditNoteAction::EditNoteAction(bool unsaved, float absBeat, NoteSequenceItem::SequencerItemType itemType, std::string oldDisplayText, std::string newDisplayText) :
    EditAction(unsaved), absBeat(absBeat), itemType(itemType), oldDisplayText(oldDisplayText), newDisplayText(newDisplayText) {}

void EditNoteAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.editNote(absBeat, itemType, oldDisplayText);
}

void EditNoteAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.editNote(absBeat, itemType, newDisplayText);
}
