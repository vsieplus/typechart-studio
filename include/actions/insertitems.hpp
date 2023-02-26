#ifndef INSERTITEMS_HPP
#define INSERTITEMS_HPP

#include <list>

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class InsertItemsAction : public EditAction {
    public:
        InsertItemsAction(bool unsaved, int beatsplit, int itemTypeStart, int itemTypeEnd, float startBeat,
            std::list<std::shared_ptr<NoteSequenceItem>> & itemsInserted,
            std::list<std::shared_ptr<NoteSequenceItem>> & itemsDeleted);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        int beatsplit;
        int itemTypeStart;
        int itemTypeEnd;

        float startBeat;

        std::list<std::shared_ptr<NoteSequenceItem>> itemsInserted;
        std::list<std::shared_ptr<NoteSequenceItem>> itemsDeleted;
};

#endif // INSERTITEMS_HPP
