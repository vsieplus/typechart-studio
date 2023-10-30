#include "actions/deleteitems.hpp"
#include "ui/editwindow.hpp"

DeleteItemsAction::DeleteItemsAction(bool unsaved, int itemTypeStart, int itemTypeEnd, float startBeat, float endBeat,
    std::list<std::shared_ptr<NoteSequenceItem>> & items) : 
        EditAction(unsaved),
        itemTypeStart(itemTypeStart),
        itemTypeEnd(itemTypeEnd),
        startBeat(startBeat),
        endBeat(endBeat),
        items(items) {}

void DeleteItemsAction::undoAction(EditWindow * editWindow) {
    EditAction::undoAction(editWindow);
    if(!items.empty()) {
        editWindow->chartinfo.notes.insertItems(items.front()->absBeat, editWindow->songpos.absBeat, itemTypeStart, 
            itemTypeEnd, editWindow->songpos.timeinfo, items);
    }
}

void DeleteItemsAction::redoAction(EditWindow * editWindow) {
    editWindow->chartinfo.notes.deleteItems(startBeat, endBeat, itemTypeStart, itemTypeEnd);
}
