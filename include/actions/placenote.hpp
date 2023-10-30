#ifndef PLACENOTE_HPP
#define PLACENOTE_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceNoteAction : public EditAction {
    public:
        PlaceNoteAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, NoteSequenceItem::SequencerItemType itemType, std::string displayText);

        virtual void undoAction(EditWindow * editWindow) override;
        virtual void redoAction(EditWindow * editWindow) override;

    private:
        float absBeat;
        float beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;

        NoteSequenceItem::SequencerItemType itemType;

        std::string displayText;
};

#endif // PLACENOTE_HPP
