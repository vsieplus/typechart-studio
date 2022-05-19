#ifndef PLACESTOP_HPP
#define PLACESTOP_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceStopAction : public EditAction {
    public:
        PlaceStopAction(bool unsaved, float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        float absBeat;
        float beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;
};

#endif // PLACESTOP_HPP