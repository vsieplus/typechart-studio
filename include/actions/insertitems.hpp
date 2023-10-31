#ifndef INSERTITEMS_HPP
#define INSERTITEMS_HPP

#include <list>

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class InsertItemsAction : public EditAction {
    public:
        InsertItemsAction(int itemTypeStart, int itemTypeEnd, double startBeat,
            const std::list<std::shared_ptr<NoteSequenceItem>> & itemsInserted,
            const std::list<std::shared_ptr<NoteSequenceItem>> & itemsDeleted);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;
    private:
        int itemTypeStart;
        int itemTypeEnd;

        double startBeat;

        std::list<std::shared_ptr<NoteSequenceItem>> itemsInserted;
        std::list<std::shared_ptr<NoteSequenceItem>> itemsDeleted;
};

#endif // INSERTITEMS_HPP
