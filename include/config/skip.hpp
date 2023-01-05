#ifndef SKIP_HPP
#define SKIP_HPP

#include "config/notesequenceitem.hpp"

struct Skip : public NoteSequenceItem {
    Skip(float absBeat, float songBeat, float skipTime, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) : 
        NoteSequenceItem(absBeat, absBeat + beatDuration, songBeat, beatpos, endBeatpos), skipTime(skipTime), beatDuration(beatDuration) {}

    virtual SequencerItemType getItemType() const override {
        return SequencerItemType::SKIP;
    }

    void setItemType(SequencerItemType type) override {}

    float skipTime;
    float beatDuration;
};

#endif // STOP_HPP