#ifndef DELETENOTE_HPP
#define DELETENOTE_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class DeleteNoteAction : public EditAction {
    public:
        DeleteNoteAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType, std::string displayText);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;

    private:
        float absBeat;
        float beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;

        SequencerItemType itemType;

        std::string displayText;
};

#endif // DELETENOTE_HPP