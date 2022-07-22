#include "actions/insertitems.hpp"
#include "ui/editwindow.hpp"

InsertItemsAction::InsertItemsAction(bool unsaved, int beatsplit, int itemTypeStart, int itemTypeEnd, float startBeat,
        std::vector<std::shared_ptr<NoteSequenceItem>> & itemsInserted,
        std::vector<std::shared_ptr<NoteSequenceItem>> & itemsDeleted) : 
    EditAction(unsaved), beatsplit(beatsplit), itemTypeStart(itemTypeStart), itemTypeEnd(itemTypeEnd), startBeat(startBeat),
    itemsInserted(itemsInserted), itemsDeleted(itemsDeleted) {}

void InsertItemsAction::undoAction(EditWindowData * editWindow) {
    EditAction::undoAction(editWindow);

    if(!itemsInserted.empty()) {
        auto endBeat = startBeat + (itemsInserted.back()->absBeat - itemsInserted.front()->absBeat);
        editWindow->chartinfo.notes.deleteItems(startBeat, endBeat, itemTypeStart, itemTypeEnd);
    }

    if(!itemsDeleted.empty()) {
        editWindow->chartinfo.notes.insertItems(itemsDeleted.front()->absBeat, editWindow->songpos.absBeat, beatsplit, itemTypeStart,
            itemTypeEnd, editWindow->songpos.timeinfo, itemsDeleted);
    }
}

void InsertItemsAction::redoAction(EditWindowData * editWindow) {
    if(!itemsInserted.empty()) {
        editWindow->chartinfo.notes.insertItems(startBeat, editWindow->songpos.absBeat, beatsplit, itemTypeStart,
            itemTypeEnd, editWindow->songpos.timeinfo, itemsInserted);
    }
}