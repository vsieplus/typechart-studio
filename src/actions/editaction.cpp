#include "actions/editaction.hpp"
#include "ui/editwindow.hpp"

void EditAction::undoAction(EditWindowData * editWindow) {
    editWindow->unsaved = wasUnsaved;
}