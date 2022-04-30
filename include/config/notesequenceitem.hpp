#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

#include <string>

enum SequencerItemType {
    TOP_NOTE,
    MID_NOTE,
    BOT_NOTE,
    STOP,
    SKIP
};

struct NoteSequenceItem {
    NoteSequenceItem(float absBeat, float beatEnd, SequencerItemType itemType) : absBeat(absBeat), beatEnd(beatEnd), itemType(itemType) {}

    float absBeat;
    float beatEnd;

    SequencerItemType itemType;

    std::string displayText = "";

    bool passed = false;
};

bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP