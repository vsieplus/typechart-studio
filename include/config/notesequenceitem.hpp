#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

#include <memory>
#include <string>
#include <set>
#include <unordered_map>

#include "config/beatpos.hpp"

struct NoteSequenceItem {
    enum class SequencerItemType {
        TOP_NOTE,
        MID_NOTE,
        BOT_NOTE,
        STOP,
        SKIP
    };

    NoteSequenceItem(SequencerItemType itemType, bool passed, double absBeat, double beatEnd, BeatPos beatpos, BeatPos endBeatpos, std::string_view displayText);
    virtual ~NoteSequenceItem() = default;

    SequencerItemType itemType;

    bool passed { false };
    bool deleted { false };

    double absBeat { 0.f };
    double beatEnd { 0.f };

    BeatPos beatpos;
    BeatPos endBeatpos;

    std::string displayText {};
};

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs);
bool operator==(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);
bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP
