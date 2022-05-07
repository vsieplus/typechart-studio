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


const std::unordered_map<std::string, std::set<char>> MIDDLE_ROW_KEYS = {
    {"QWERTY", {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
                'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
                'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'}},
    {"DVORAK", {'\'', ',', '.', 'P', 'Y', 'F', 'G', 'C', 'R', 'L',
                'A', 'O', 'E', 'U', 'I', 'D', 'H', 'T', 'N', 'S',
                ';', 'Q', 'J', 'K', 'X', 'B', 'M', 'W', 'V', 'Z'}},
    {"AZERTY", {'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
                'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',
                'W', 'X', 'C', 'V', 'B', 'N', ',', ';', ':', '!'}},
    {"COLEMAK", {'Q', 'W', 'F', 'P', 'G', 'J', 'L', 'U', 'Y', ';',
                'A', 'R', 'S', 'T', 'D', 'H', 'N', 'E', 'I', 'O',
                'Z', 'X', 'C', 'V', 'B', 'K', 'M', ',', '.', '/'}}
};

struct NoteSequenceItem {
    NoteSequenceItem(float absBeat, float beatEnd, BeatPos beatpos, BeatPos endBeatpos) :
        absBeat(absBeat), beatEnd(beatEnd), beatpos(beatpos), endBeatpos(endBeatpos) {}

    virtual SequencerItemType getItemType() const = 0;

    float absBeat;
    float beatEnd;

    BeatPos beatpos;
    BeatPos endBeatpos;

    std::string displayText = "";

    bool passed = false;
};

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs);
bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP