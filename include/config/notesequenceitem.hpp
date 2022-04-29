#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

enum SequencerItemType {
    TOP_NOTE,
    MID_NOTE,
    BOT_NOTE,
    STOP,
    SKIP,
    NUM_ITEMTYPES
};

struct NoteSequenceItem {
    NoteSequenceItem(float absBeat, float beatEnd, SequencerItemType itemType) : absBeat(absBeat), beatEnd(beatEnd), itemType(itemType) {}

    float absBeat;
    float beatEnd;

    SequencerItemType itemType;
};

#endif // NOTESEQUENCEITEM_HPP