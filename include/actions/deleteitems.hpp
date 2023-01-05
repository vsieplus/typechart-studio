#ifndef DELETEITEMS_HPP
#define DELETEITEMS_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class DeleteItemsAction : public EditAction {
    public:
        DeleteItemsAction(bool unsaved, int beatsplit, int itemTypeStart, int itemTypeEnd, float startBeat, float endBeat, 
            std::vector<std::shared_ptr<NoteSequenceItem>> & items);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        int beatsplit;
        int itemTypeStart;
        int itemTypeEnd;

        float startBeat;
        float endBeat;

        std::vector<std::shared_ptr<NoteSequenceItem>> items;
};

#endif // DELETEITEMS_HPP
