#ifndef EDITNOTE_HPP
#define EDITNOTE_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class EditNoteAction : public EditAction {
    public:
        EditNoteAction(bool unsaved, float absBeat, NoteSequenceItem::SequencerItemType itemType, std::string oldDisplayText, std::string newDisplayText);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;

    private:
        float absBeat;
        NoteSequenceItem::SequencerItemType itemType;

        std::string oldDisplayText;
        std::string newDisplayText;
};

#endif // EDITNOTE_HPP
