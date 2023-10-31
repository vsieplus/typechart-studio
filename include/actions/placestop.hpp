#ifndef PLACESTOP_HPP
#define PLACESTOP_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceStopAction : public EditAction {
    public:
        PlaceStopAction(double absBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;
    private:
        double absBeat;
        double beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;
};

#endif // PLACESTOP_HPP
