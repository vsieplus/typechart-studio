#ifndef DELETEITEMS_HPP
#define DELETEITEMS_HPP

#include <list>

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class DeleteItemsAction : public EditAction {
    public:
        DeleteItemsAction(int itemTypeStart, int itemTypeEnd, double startBeat, double endBeat, const std::list<std::shared_ptr<NoteSequenceItem>> & items);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;
    private:
        int itemTypeStart;
        int itemTypeEnd;

        double startBeat;
        double endBeat;

        std::list<std::shared_ptr<NoteSequenceItem>> items;
};

#endif // DELETEITEMS_HPP
