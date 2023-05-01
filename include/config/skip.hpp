#ifndef SKIP_HPP
#define SKIP_HPP

#include "config/notesequenceitem.hpp"

struct Skip : public NoteSequenceItem {
    Skip(float absBeat, float skipTime, float beatDuration, bool passed, BeatPos beatpos, BeatPos endBeatpos) : 
        NoteSequenceItem(absBeat, absBeat + beatDuration, passed, beatpos, endBeatpos), skipTime(skipTime), beatDuration(beatDuration) {}

    virtual SequencerItemType getItemType() const override {
        return NoteSequenceItem::SequencerItemType::SKIP;
    }

    void setItemType(SequencerItemType type) override {}

    float skipTime;
    float beatDuration;
};

#endif // SKIP_HPP
