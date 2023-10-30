#include "actions/deleteitems.hpp"
#include "ui/editwindow.hpp"

DeleteItemsAction::DeleteItemsAction(int itemTypeStart, int itemTypeEnd, double startBeat, double endBeat, const std::list<std::shared_ptr<NoteSequenceItem>> & items)
    : itemTypeStart(itemTypeStart)
    , itemTypeEnd(itemTypeEnd)
    , startBeat(startBeat)
    , endBeat(endBeat)
    , items(items) {}

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
