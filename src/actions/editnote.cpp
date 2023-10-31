#include "actions/editnote.hpp"
#include "ui/editwindow.hpp"

EditNoteAction::EditNoteAction(double absBeat, NoteSequenceItem::SequencerItemType itemType, std::string_view oldDisplayText, std::string_view newDisplayText)
    : absBeat(absBeat)
    , itemType(itemType)
    , oldDisplayText(oldDisplayText)
    , newDisplayText(newDisplayText) {}

void EditNoteAction::undoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.editNote(absBeat, itemType, oldDisplayText);
}

void EditNoteAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.editNote(absBeat, itemType, newDisplayText);
}
