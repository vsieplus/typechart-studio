#include "config/notesequenceitem.hpp"

NoteSequenceItem::NoteSequenceItem(SequencerItemType itemType, bool passed, double absBeat, double beatEnd, BeatPos beatpos, BeatPos endBeatpos, std::string_view displayText)
    : itemType(itemType)
    , passed(passed)
    , absBeat(absBeat)
    , beatEnd(beatEnd)
    , beatpos(beatpos)
    , endBeatpos(endBeatpos)
    , displayText(displayText) {}

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs) {
    return lhs && rhs && (*lhs.get()) < (*rhs.get());
}

bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs) {
    return lhs.absBeat < rhs.absBeat;
}

bool operator==(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs) {
    return (lhs.itemType == rhs.itemType) &&
        (lhs.absBeat == rhs.absBeat) &&
        (lhs.beatEnd == rhs.beatEnd) &&
        (lhs.beatpos == rhs.beatpos) &&
        (lhs.endBeatpos == rhs.endBeatpos);
}
