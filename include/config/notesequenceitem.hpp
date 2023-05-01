#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

#include <memory>
#include <string>
#include <set>
#include <unordered_map>

#include "config/beatpos.hpp"

enum SequencerItemType {
    TOP_NOTE,
    MID_NOTE,
    BOT_NOTE,
    STOP,
    SKIP
};

struct NoteSequenceItem {
    NoteSequenceItem(float absBeat, float beatEnd, float songBeat, BeatPos beatpos, BeatPos endBeatpos) :
        absBeat(absBeat), beatEnd(beatEnd), beatpos(beatpos), endBeatpos(endBeatpos), passed(absBeat < songBeat) {}

    virtual SequencerItemType getItemType() const = 0;
    virtual void setItemType(SequencerItemType type) = 0;

    float absBeat = 0.f;
    float beatEnd = 0.f;

    BeatPos beatpos;
    BeatPos endBeatpos;

    std::string displayText = "";

    bool passed = false;
    bool deleted = false;
};

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs);
bool operator==(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);
bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP
