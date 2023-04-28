#ifndef PLACESKIP_HPP
#define PLACESKIP_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceSkipAction : public EditAction {
    public:
        PlaceSkipAction(bool unsaved, float absBeat, float skipBeats, float beatDuration, BeatPos beatpos, BeatPos endBeatpos);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        float absBeat;
        float skipBeats;
        float beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;
};

#endif // PLACESKIP_HPP
