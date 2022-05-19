#ifndef PLACENOTE_HPP
#define PLACENOTE_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceNoteAction : public EditAction {
    public:
        PlaceNoteAction(NoteSequence & noteSequence, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType, std::string displayText);

        virtual void undoAction() override;
        virtual void redoAction() override;

    private:
        NoteSequence & noteSequence;

        float absBeat;
        float beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;

        SequencerItemType itemType;

        std::string displayText;
};

#endif // PLACENOTE_HPP