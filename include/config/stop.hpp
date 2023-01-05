#ifndef STOP_HPP
#define STOP_HPP

#include "config/notesequenceitem.hpp"

struct Stop : public NoteSequenceItem {
    public:
    Stop(float absBeat, float songBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) :
        NoteSequenceItem(absBeat, absBeat + beatDuration, songBeat, beatpos, endBeatpos), beatDuration(beatDuration) {}

    virtual SequencerItemType getItemType() const override {
        return SequencerItemType::STOP;
    }

    void setItemType(SequencerItemType type) override {}

    float beatDuration;
};

#endif // STOP_HPP