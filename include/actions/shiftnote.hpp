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
        enum class ShiftDirection {
            ShiftUp,
            ShiftDown,
            ShiftLeft,
            ShiftRight,
            ShiftNone
        };

        ShiftNoteAction(int minItemType, int maxItemType, double startBeat, double endBeat, std::string_view keyboardLayout,
            ShiftDirection shiftDirection, const std::list<std::shared_ptr<NoteSequenceItem>> & items);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;
    private:
        int minItemType;
        int maxItemType;
        double startBeat;
        double endBeat;

        std::string keyboardLayout;

        ShiftDirection shiftDirection;

        std::list<std::shared_ptr<NoteSequenceItem>> items;

        void reconcileDeletedItems(const EditWindow * editWindow);
};

#endif // SHIFTNOTE_HPP
