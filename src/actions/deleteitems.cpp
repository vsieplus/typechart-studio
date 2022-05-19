#include "actions/deleteitems.hpp"
#include "ui/editwindow.hpp"


DeleteItemsAction::DeleteItemsAction(bool unsaved, int beatsplit, int itemTypeStart, int itemTypeEnd, float startBeat, float endBeat,
        std::vector<std::shared_ptr<NoteSequenceItem>> & items) : 
    EditAction(unsaved), beatsplit(beatsplit), itemTypeStart(itemTypeStart), itemTypeEnd(itemTypeEnd), startBeat(startBeat), endBeat(endBeat), items(items) {}

void DeleteItemsAction::undoAction(EditWindowData * editWindow) {
    EditAction::undoAction(editWindow);
    if(!items.empty()) {
        editWindow->chartinfo.notes.insertItems(items.front()->absBeat, beatsplit, itemTypeStart, itemTypeEnd, editWindow->songpos.timeinfo, items);
    }
}

void DeleteItemsAction::redoAction(EditWindowData * editWindow) {
    editWindow->chartinfo.notes.deleteItems(startBeat, endBeat, itemTypeStart, itemTypeEnd);
}