#ifndef STOP_HPP
#define STOP_HPP

#include "config/notesequenceitem.hpp"

struct Stop : public NoteSequenceItem {
    Stop(double absBeat, double beatDuration, bool passed, BeatPos beatpos, BeatPos endBeatpos)
    : NoteSequenceItem(SequencerItemType::STOP, passed, absBeat, absBeat + beatDuration, beatpos, endBeatpos, "")
    , beatDuration(beatDuration) {}

    double beatDuration;
};

#endif // STOP_HPP
