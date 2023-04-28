#include "actions/editskip.hpp"
#include "ui/editwindow.hpp"

EditSkipAction::EditSkipAction(bool unsaved, float absBeat,float prevSkipbeats, float newSkipbeats) :
    EditAction(unsaved), absBeat(absBeat), prevSkipbeats(prevSkipbeats), newSkipbeats(newSkipbeats) {}

void EditSkipAction::undoAction(EditWindowData * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.editSkip(absBeat, prevSkipbeats);
}

void EditSkipAction::redoAction(EditWindowData * editWindow) {
    editWindow->chartinfo.notes.editSkip(absBeat, newSkipbeats);
}
