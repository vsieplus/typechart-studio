#ifndef INSERTITEMS_HPP
#define INSERTITEMS_HPP

#include <list>

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class InsertItemsAction : public EditAction {
    public:
        InsertItemsAction(bool unsaved, int itemTypeStart, int itemTypeEnd, float startBeat,
            std::list<std::shared_ptr<NoteSequenceItem>> & itemsInserted,
            std::list<std::shared_ptr<NoteSequenceItem>> & itemsDeleted);

        virtual void undoAction(EditWindow * editWindow) override;
        virtual void redoAction(EditWindow * editWindow) override;
    private:
        int itemTypeStart;
        int itemTypeEnd;

        float startBeat;

        std::list<std::shared_ptr<NoteSequenceItem>> itemsInserted;
        std::list<std::shared_ptr<NoteSequenceItem>> itemsDeleted;
};

#endif // INSERTITEMS_HPP
