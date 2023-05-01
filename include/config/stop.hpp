#ifndef STOP_HPP
#define STOP_HPP

#include "config/notesequenceitem.hpp"

struct Stop : public NoteSequenceItem {
    Stop(float absBeat, float beatDuration, bool passed, BeatPos beatpos, BeatPos endBeatpos) :
        NoteSequenceItem(absBeat, absBeat + beatDuration, passed, beatpos, endBeatpos), beatDuration(beatDuration) {}

    virtual SequencerItemType getItemType() const override {
        return NoteSequenceItem::SequencerItemType::STOP;
    }

    void setItemType(SequencerItemType type) override {}

    float beatDuration;
};

#endif // STOP_HPP
