#ifndef SHIFTNOTE_HPP
#define SHIFTNOTE_HPP

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "actions/editaction.hpp"
#include "config/notesequenceitem.hpp"

class ShiftNoteAction : public EditAction {
    public:
        enum ShiftDirection {
            ShiftUp,
            ShiftDown,
            ShiftLeft,
            ShiftRight,
            ShiftNone
        };

        ShiftNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat, float endBeat, std::string keyboardLayout,
            ShiftDirection shiftDirection, std::list<std::shared_ptr<NoteSequenceItem>> items);
        virtual void undoAction(EditWindow * editWindow) override;
        virtual void redoAction(EditWindow * editWindow) override;
    private:
        int minItemType, maxItemType;
        float startBeat, endBeat;

        std::string keyboardLayout;

        ShiftDirection shiftDirection;

        std::list<std::shared_ptr<NoteSequenceItem>> items;

        void reconcileDeletedItems(EditWindow * editWindow);
};

#endif // SHIFTNOTE_HPP
