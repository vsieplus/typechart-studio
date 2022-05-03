#ifndef SKIP_HPP
#define SKIP_HPP

#include "config/notesequenceitem.hpp"

struct Skip : public NoteSequenceItem {
    Skip(float absBeat, float skipTime, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) : 
        NoteSequenceItem(absBeat, absBeat + beatDuration, beatpos, endBeatpos), skipTime(skipTime), beatDuration(beatDuration) {}

    virtual SequencerItemType getItemType() const override {
        return SequencerItemType::SKIP;
    }

    float skipTime;
    float beatDuration;
};

#endif // STOP_HPP