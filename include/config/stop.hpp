#ifndef STOP_HPP
#define STOP_HPP

#include "config/notesequenceitem.hpp"

struct Stop : public NoteSequenceItem {
    Stop(float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType) :
        NoteSequenceItem(absBeat, absBeat + beatDuration, beatpos, endBeatpos, itemType), beatDuration(beatDuration) {}

    float beatDuration;
};

#endif // STOP_HPP