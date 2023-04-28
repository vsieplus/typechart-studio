#ifndef DELETEITEMS_HPP
#define DELETEITEMS_HPP

#include <list>

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class DeleteItemsAction : public EditAction {
    public:
        DeleteItemsAction(bool unsaved, int itemTypeStart, int itemTypeEnd, float startBeat, float endBeat, 
            std::list<std::shared_ptr<NoteSequenceItem>> & items);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        int itemTypeStart;
        int itemTypeEnd;

        float startBeat;
        float endBeat;

        std::list<std::shared_ptr<NoteSequenceItem>> items;
};

#endif // DELETEITEMS_HPP
