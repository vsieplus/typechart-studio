#ifndef STOP_HPP
#define STOP_HPP

#include "config/notesequenceitem.hpp"

struct Stop : public NoteSequenceItem {
    Stop(float absBeat, float beatDuration, SequencerItemType itemType) :
        NoteSequenceItem(absBeat, absBeat + beatDuration, itemType), beatDuration(beatDuration) {}

    float beatDuration;
};

#endif // STOP_HPP