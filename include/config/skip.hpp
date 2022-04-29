#ifndef SKIP_HPP
#define SKIP_HPP

#include "config/notesequenceitem.hpp"

struct Skip : public NoteSequenceItem {
    Skip(float absBeat, float skipTime, float beatDuration, SequencerItemType itemType) : 
        NoteSequenceItem(absBeat, absBeat + beatDuration, itemType), skipTime(skipTime), beatDuration(beatDuration) {}

    float skipTime;
    float beatDuration;
};

#endif // STOP_HPP