#ifndef PLACENOTE_HPP
#define PLACENOTE_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceNoteAction : public EditAction {
    public:
        PlaceNoteAction(double absBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos, NoteSequenceItem::SequencerItemType itemType, std::string_view displayText);

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

#endif // PLACENOTE_HPP
