#include "actions/insertitems.hpp"
#include "ui/editwindow.hpp"

InsertItemsAction::InsertItemsAction(int itemTypeStart, int itemTypeEnd, double startBeat,
    const std::list<std::shared_ptr<NoteSequenceItem>> & itemsInserted,
    const std::list<std::shared_ptr<NoteSequenceItem>> & itemsDeleted)
    : itemTypeStart(itemTypeStart)
    , itemTypeEnd(itemTypeEnd)
    , startBeat(startBeat)
    , itemsInserted(itemsInserted)
    , itemsDeleted(itemsDeleted) {}

void InsertItemsAction::undoAction(EditWindow * editWindow) {
    if(!itemsInserted.empty()) {
        auto endBeat = startBeat + (itemsInserted.back()->absBeat - itemsInserted.front()->absBeat);
        editWindow->chartinfo.notes.deleteItems(startBeat, endBeat, itemTypeStart, itemTypeEnd);
    }

    if(!itemsDeleted.empty()) {
        editWindow->chartinfo.notes.insertItems(itemsDeleted.front()->absBeat, editWindow->songpos.absBeat, itemTypeStart,
            itemTypeEnd, editWindow->songpos.timeinfo, itemsDeleted);
    }
}

void InsertItemsAction::redoAction(EditWindow * editWindow) {
    if(!itemsInserted.empty()) {
        editWindow->chartinfo.notes.insertItems(startBeat, editWindow->songpos.absBeat, itemTypeStart,
            itemTypeEnd, editWindow->songpos.timeinfo, itemsInserted);
    }
}
