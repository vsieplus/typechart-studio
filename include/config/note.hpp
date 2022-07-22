#ifndef NOTE_HPP
#define NOTE_HPP

#include <string>
#include <unordered_map>

#include "config/notesequenceitem.hpp"
#include "IconsFontAwesome6.h"

enum NoteSplit {
    WHOLE,
    HALF,
    QUARTER,
    EIGHTH,
    SIXTEENTH,
    THIRTYSECOND,
    SIXTYFOURTH,
    ONETWENTYEIGHTH,
    TWELFTH,
    TWENTYFOURTH,
    FORTYEIGHTH,
    NINETYSIXTH,
    DUMMY
};

enum NoteType {
    KEYPRESS,
    KEYHOLDSTART,
    KEYHOLDRELEASE
};

const std::unordered_map<std::string, std::string> FUNCTION_KEY_TO_STR = {
    { "L" ICON_FA_ARROW_UP, "Left Shift" },
    { "R" ICON_FA_ARROW_UP, "Right Shift" },
    { ICON_FA_ARROW_UP, "CapsLock" },
    { ICON_FA_ARROW_LEFT_LONG, "Return" },
    { "_", "Space" },
};

const std::unordered_map<std::string, std::string> STR_TO_FUNCTION_KEY = {
    { "Left Shift", "L" ICON_FA_ARROW_UP },
    { "Right Shift", "R" ICON_FA_ARROW_UP },
    { "CapsLock", ICON_FA_ARROW_UP },
    { "Return", ICON_FA_ARROW_LEFT_LONG },
    { "Space", "_" },
};

struct Note : public NoteSequenceItem {
    Note(float absBeat, float beatEnd, float songBeat, BeatPos beatpos, BeatPos endBeatpos, NoteType noteType, NoteSplit noteSplit, SequencerItemType itemType, std::string key) :
        NoteSequenceItem(absBeat, beatEnd, songBeat, beatpos, endBeatpos), noteType(noteType), noteSplit(noteSplit), itemType(itemType) { displayText = key; }

    virtual SequencerItemType getItemType() const override {
        return itemType;
    }

    NoteType noteType;
    NoteSplit noteSplit;

    SequencerItemType itemType;

    std::string key;
};

#endif // NOTE_HPP