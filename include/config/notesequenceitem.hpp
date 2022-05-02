#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

#include <string>

#include "config/beatpos.hpp"

enum SequencerItemType {
    TOP_NOTE,
    MID_NOTE,
    BOT_NOTE,
    STOP,
    SKIP
};

struct NoteSequenceItem {
    NoteSequenceItem(float absBeat, float beatEnd, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType) :
        absBeat(absBeat), beatEnd(beatEnd), beatpos(beatpos), endBeatpos(endBeatpos), itemType(itemType) {}
    virtual ~NoteSequenceItem() {}

    float absBeat;
    float beatEnd;

    BeatPos beatpos;
    BeatPos endBeatpos;

    SequencerItemType itemType;

    std::string displayText = "";

    bool passed = false;
};

bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP