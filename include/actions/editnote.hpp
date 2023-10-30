#ifndef EDITNOTE_HPP
#define EDITNOTE_HPP

#include <string>
#include <string_view>

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class EditNoteAction : public EditAction {
    public:
        EditNoteAction(double absBeat, NoteSequenceItem::SequencerItemType itemType, std::string_view oldDisplayText, std::string_view newDisplayText);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;

    private:
        double absBeat;
        NoteSequenceItem::SequencerItemType itemType;

        std::string oldDisplayText;
        std::string newDisplayText;
};

#endif // EDITNOTE_HPP
