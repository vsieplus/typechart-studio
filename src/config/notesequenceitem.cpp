#include "config/notesequenceitem.hpp"

NoteSequenceItem::NoteSequenceItem(double absBeat, double beatEnd, bool passed, BeatPos beatpos, BeatPos endBeatpos) :
    absBeat(absBeat), beatEnd(beatEnd), beatpos(beatpos), endBeatpos(endBeatpos), passed(passed) {}

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs) {
    return lhs && rhs && (*lhs.get()) < (*rhs.get());
}

bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs) {
    return lhs.absBeat < rhs.absBeat;
}

bool operator==(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs) {
    return (lhs.getItemType() == rhs.getItemType()) &&
        (lhs.absBeat == rhs.absBeat) &&
        (lhs.beatEnd == rhs.beatEnd) &&
        (lhs.beatpos == rhs.beatpos) &&
        (lhs.endBeatpos == rhs.endBeatpos);
}
