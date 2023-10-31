#ifndef PLACESKIP_HPP
#define PLACESKIP_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class PlaceSkipAction : public EditAction {
    public:
        PlaceSkipAction(double absBeat, double skipBeats, double beatDuration, BeatPos beatpos, BeatPos endBeatpos);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;
    private:
        double absBeat;
        double skipBeats;
        double beatDuration;

        BeatPos beatpos;
        BeatPos endBeatpos;
};

#endif // PLACESKIP_HPP
