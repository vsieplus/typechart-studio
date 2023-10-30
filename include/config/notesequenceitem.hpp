#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

#include <memory>
#include <string>
#include <set>
#include <unordered_map>

#include "config/beatpos.hpp"

struct NoteSequenceItem {
    NoteSequenceItem(double absBeat, double beatEnd, bool passed, BeatPos beatpos, BeatPos endBeatpos);
    virtual ~NoteSequenceItem() = default;

    enum class SequencerItemType {
        TOP_NOTE,
        MID_NOTE,
        BOT_NOTE,
        STOP,
        SKIP
    };

    virtual SequencerItemType getItemType() const = 0;
    virtual void setItemType(SequencerItemType type) = 0;

    BeatPos beatpos;
    BeatPos endBeatpos;

    double absBeat { 0.f };
    double beatEnd { 0.f };

    bool passed { false };
    bool deleted { false };

    std::string displayText {};
};

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs);
bool operator==(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);
bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP
