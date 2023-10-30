#ifndef SKIP_HPP
#define SKIP_HPP

#include "config/notesequenceitem.hpp"

struct Skip : public NoteSequenceItem {
    Skip(double absBeat, double skipTime, double beatDuration, bool passed, BeatPos beatpos, BeatPos endBeatpos)
    : NoteSequenceItem(SequencerItemType::SKIP, passed, absBeat, absBeat + beatDuration, beatpos, endBeatpos, "")
    , skipTime(skipTime)
    , beatDuration(beatDuration) {}

    double skipTime;
    double beatDuration;
};

#endif // SKIP_HPP
