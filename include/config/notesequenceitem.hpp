#ifndef NOTESEQUENCEITEM_HPP
#define NOTESEQUENCEITEM_HPP

#include <memory>
#include <string>
#include <set>
#include <unordered_map>

#include "config/beatpos.hpp"
#include "IconsFontAwesome6.h"

enum SequencerItemType {
    TOP_NOTE,
    MID_NOTE,
    BOT_NOTE,
    STOP,
    SKIP
};

const std::unordered_map<std::string, std::unordered_map<std::string, std::pair<int,int>>> KEYBOARD_POSITION_MAPS = {
    {"QWERTY", {
        { "1", { 0, 0} }, {"2", { 0, 1}}, {"3", { 0, 2}}, {"4", { 0, 3}}, {"5", { 0, 4}}, {"6", { 0, 5}}, {"7", { 0, 6}}, {"8", { 0, 7}}, {"9", { 0, 8}}, {"0", { 0, 9}},
        { "Q", { 1, 0} }, {"W", { 1, 1}}, {"E", { 1, 2}}, {"R", { 1, 3}}, {"T", { 1, 4}}, {"Y", { 1, 5}}, {"U", { 1, 6}}, {"I", { 1, 7}}, {"O", { 1, 8}}, {"P", { 1, 9}},
        { "A", { 2, 0} }, {"S", { 2, 1}}, {"D", { 2, 2}}, {"F", { 2, 3}}, {"G", { 2, 4}}, {"H", { 2, 5}}, {"J", { 2, 6}}, {"K", { 2, 7}}, {"L", { 2, 8}}, {";", { 2, 9}},
        { "Z", { 3, 0} }, {"X", { 3, 1}}, {"C", { 3, 2}}, {"V", { 3, 3}}, {"B", { 3, 4}}, {"N", { 3, 5}}, {"M", { 3, 6}}, {",", { 3, 7}}, {".", { 3, 8}}, {"/", { 3, 9}},
    }},
    {"DVORAK", {
        { "1", { 0, 0} }, {"2", { 0, 1}}, {"3", { 0, 2}}, {"4", { 0, 3}}, {"5", { 0, 4}}, {"6", { 0, 5}}, {"7", { 0, 6}}, {"8", { 0, 7}}, {"9", { 0, 8}}, {"0", { 0, 9}},
        { "'", { 1, 0} }, {",", { 1, 1}}, {".", { 1, 2}}, {"P", { 1, 3}}, {"Y", { 1, 4}}, {"F", { 1, 5}}, {"G", { 1, 6}}, {"C", { 1, 7}}, {"R", { 1, 8}}, {"L", { 1, 9}},
        { "A", { 2, 0} }, {"O", { 2, 1}}, {"E", { 2, 2}}, {"U", { 2, 3}}, {"I", { 2, 4}}, {"D", { 2, 5}}, {"H", { 2, 6}}, {"T", { 2, 7}}, {"N", { 2, 8}}, {"S", { 2, 9}},
        { ";", { 3, 0} }, {"Q", { 3, 1}}, {"J", { 3, 2}}, {"K", { 3, 3}}, {"X", { 3, 4}}, {"B", { 3, 5}}, {"M", { 3, 6}}, {"W", { 3, 7}}, {"V", { 3, 8}}, {"Z", { 3, 9}},
    }},
    {"AZERTY", {
        { "1", { 0, 0} }, {"2", { 0, 1}}, {"3", { 0, 2}}, {"4", { 0, 3}}, {"5", { 0, 4}}, {"6", { 0, 5}}, {"7", { 0, 6}}, {"8", { 0, 7}}, {"9", { 0, 8}}, {"0", { 0, 9}},
        { "A", { 1, 0} }, {"Z", { 1, 1}}, {"E", { 1, 2}}, {"R", { 1, 3}}, {"T", { 1, 4}}, {"Y", { 1, 5}}, {"U", { 1, 6}}, {"I", { 1, 7}} ,{"O", { 1, 8}}, {"P", { 1, 9}},
        { "Q", { 2, 0} }, {"S", { 2, 1}}, {"D", { 2, 2}}, {"F", { 2, 3}}, {"G", { 2, 4}}, {"H", { 2, 5}}, {"J", { 2, 6}}, {"K", { 2, 7}}, {"L", { 2, 8}}, {"M", { 2, 9}},
        { "W", { 3, 0} }, {"X", { 3, 1}}, {"C", { 3, 2}}, {"V", { 3, 3}}, {"B", { 3, 4}}, {"N", { 3, 5}}, {",", { 3, 6}}, {";", { 3, 7}}, {":", { 3, 8}}, {"!", { 3, 9}},
    }},
    {"COLEMAK", {
        { "1", { 0, 0} }, {"2", { 0, 1}}, {"3", { 0, 2}}, {"4", { 0, 3}}, {"5", { 0, 4}}, {"6", { 0, 5}}, {"7", { 0, 6}}, {"8", { 0, 7}}, {"9", { 0, 8}}, {"0", { 0, 9}},
        { "Q", { 1, 0} }, {"W", { 1, 1}}, {"F", { 1, 2}}, {"P", { 1, 3}}, {"G", { 1, 4}}, {"J", { 1, 5}}, {"L", { 1, 6}}, {"U", { 1, 7}}, {"Y", { 1, 8}}, {";", { 1, 9}},
        { "A", { 2, 0} }, {"R", { 2, 1}}, {"S", { 2, 2}}, {"T", { 2, 3}}, {"D", { 2, 4}}, {"H", { 2, 5}}, {"N", { 2, 6}}, {"E", { 2, 7}}, {"I", { 2, 8}}, {"O", { 2, 9}},
        { "Z", { 3, 0} }, {"X", { 3, 1}}, {"C", { 3, 2}}, {"V", { 3, 3}}, {"B", { 3, 4}}, {"K", { 3, 5}}, {"M", { 3, 6}}, {",", { 3, 7}}, {".", { 3, 8}}, {"/", { 3, 9}},
    }}
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

const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> KEYBOARD_FLIP_MAPS = {
    {"QWERTY", {
        { "1", "0" }, {"2", "9"}, {"3", "8"}, {"4", "7"}, {"5", "6"}, {"6", "5"}, {"7", "4"}, {"8", "3"}, {"9", "2"}, {"0", "1"},
        { "Q", "P" }, {"W", "O"}, {"E", "I"}, {"R", "U"}, {"T", "Y"}, {"Y", "T"}, {"U", "R"}, {"I", "E"}, {"O", "W"}, {"P", "Q"},
        { "A", ";" }, {"S", "L"}, {"D", "K"}, {"F", "J"}, {"G", "H"}, {"H", "G"}, {"J", "F"}, {"K", "D"}, {"L", "S"}, {";", "A"},
        { "Z", "/" }, {"X", "."}, {"C", ","}, {"V", "M"}, {"B", "N"}, {"N", "B"}, {"M", "V"}, {",", "C"}, {".", "X"}, {"/", "Z"},
        { "L" ICON_FA_ARROW_UP, "R" ICON_FA_ARROW_UP }, { "R" ICON_FA_ARROW_UP, "L" ICON_FA_ARROW_UP },
        { ICON_FA_ARROW_UP, ICON_FA_ARROW_LEFT_LONG }, { ICON_FA_ARROW_LEFT_LONG, ICON_FA_ARROW_UP }
    }},
    {"DVORAK", {
        { "1", "0" }, {"2", "9"}, {"3", "8"}, {"4", "7"}, {"5", "6"}, {"6", "5"}, {"7", "4"}, {"8", "3"}, {"9", "2"}, {"0", "1"},
        { "'", "L" }, {",", "R"}, {".", "C"}, {"P", "G"}, {"Y", "F"}, {"F", "Y"}, {"G", "P"}, {"C", "."}, {"R", ","}, {"L", "'"},
        { "A", "S" }, {"O", "N"}, {"E", "T"}, {"U", "H"}, {"I", "D"}, {"D", "I"}, {"H", "U"}, {"T", "E"}, {"N", "O"}, {"S", "A"},
        { ";", "Z" }, {"Q", "V"}, {"J", "W"}, {"K", "M"}, {"X", "B"}, {"B", "X"}, {"M", "K"}, {"W", "J"}, {"V", "Q"}, {"Z", ";"},
        { "L" ICON_FA_ARROW_UP, "R" ICON_FA_ARROW_UP }, { "R" ICON_FA_ARROW_UP, "L" ICON_FA_ARROW_UP },
        { ICON_FA_ARROW_UP, ICON_FA_ARROW_LEFT_LONG }, { ICON_FA_ARROW_LEFT_LONG, ICON_FA_ARROW_UP }
    }},
    {"AZERTY", {
        { "1", "0" }, {"2", "9"}, {"3", "8"}, {"4", "7"}, {"5", "6"}, {"6", "5"}, {"7", "4"}, {"8", "3"}, {"9", "2"}, {"0", "1"},
        { "A", "P" }, {"W", "O"}, {"E", "I"}, {"R", "U"}, {"T", "Y"}, {"Y", "T"}, {"U", "R"}, {"I", "E"}, {"O", "W"}, {"P", "A"},
        { "Q", "M" }, {"S", "L"}, {"D", "K"}, {"F", "J"}, {"G", "H"}, {"H", "G"}, {"J", "F"}, {"K", "D"}, {"L", "S"}, {"M", "Q"},
        { "W", "!" }, {"X", ":"}, {"C", ";"}, {"V", ","}, {"B", "N"}, {"N", "B"}, {",", "V"}, {";", "C"}, {":", "X"}, {"!", "W"},
        { "L" ICON_FA_ARROW_UP, "R" ICON_FA_ARROW_UP }, { "R" ICON_FA_ARROW_UP, "L" ICON_FA_ARROW_UP },
        { ICON_FA_ARROW_UP, ICON_FA_ARROW_LEFT_LONG }, { ICON_FA_ARROW_LEFT_LONG, ICON_FA_ARROW_UP }
    }},
    {"COLEMAK", {
        { "1", "0" }, {"2", "9"}, {"3", "8"}, {"4", "7"}, {"5", "6"}, {"6", "5"}, {"7", "4"}, {"8", "3"}, {"9", "2"}, {"0", "1"},
        { "Q", ";" }, {"W", "Y"}, {"F", "U"}, {"P", "L"}, {"G", "J"}, {"J", "G"}, {"L", "P"}, {"U", "F"}, {"Y", "W"}, {";", "Q"},
        { "A", "O" }, {"R", "I"}, {"S", "E"}, {"T", "N"}, {"D", "H"}, {"H", "D"}, {"N", "T"}, {"E", "S"}, {"I", "R"}, {"O", "A"},
        { "Z", "/" }, {"X", "."}, {"C", ","}, {"V", "M"}, {"B", "K"}, {"K", "B"}, {"M", "V"}, {",", "C"}, {".", "X"}, {"/", "Z"},
        { "L" ICON_FA_ARROW_UP, "R" ICON_FA_ARROW_UP }, { "R" ICON_FA_ARROW_UP, "L" ICON_FA_ARROW_UP },
        { ICON_FA_ARROW_UP, ICON_FA_ARROW_LEFT_LONG }, { ICON_FA_ARROW_LEFT_LONG, ICON_FA_ARROW_UP }
    }},
};

struct NoteSequenceItem {
    NoteSequenceItem(float absBeat, float beatEnd, float songBeat, BeatPos beatpos, BeatPos endBeatpos) :
        absBeat(absBeat), beatEnd(beatEnd), beatpos(beatpos), endBeatpos(endBeatpos), passed(absBeat < songBeat) {}

    virtual SequencerItemType getItemType() const = 0;
    virtual void setItemType(SequencerItemType type) = 0;

    float absBeat;
    float beatEnd;

    BeatPos beatpos;
    BeatPos endBeatpos;

    std::string displayText = "";

    bool passed;
};

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs);
bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs);

#endif // NOTESEQUENCEITEM_HPP