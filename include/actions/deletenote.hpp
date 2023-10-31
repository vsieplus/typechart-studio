#ifndef DELETENOTE_HPP
#define DELETENOTE_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class DeleteNoteAction : public EditAction {
    public:
        DeleteNoteAction(double absBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos, NoteSequenceItem::SequencerItemType itemType, std::string_view displayText);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;

    private:
        double absBeat;
        double beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;

        NoteSequenceItem::SequencerItemType itemType;

        std::string displayText;
};

#endif // DELETENOTE_HPP
