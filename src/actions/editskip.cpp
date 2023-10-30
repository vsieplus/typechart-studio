#include "actions/editskip.hpp"
#include "ui/editwindow.hpp"

EditSkipAction::EditSkipAction(double absBeat,double prevSkipbeats, double newSkipbeats)
    : absBeat(absBeat)
    , prevSkipbeats(prevSkipbeats)
    , newSkipbeats(newSkipbeats) {}

void EditSkipAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);
    editWindow->chartinfo.notes.editSkip(absBeat, prevSkipbeats);
}

void EditSkipAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.editSkip(absBeat, newSkipbeats);
}
