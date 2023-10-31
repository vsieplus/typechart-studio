#ifndef NOTE_HPP
#define NOTE_HPP

#include <string>
#include <unordered_map>

#include "config/notesequenceitem.hpp"

struct Note : public NoteSequenceItem {
    enum class NoteSplit {
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

    enum class NoteType {
        KEYPRESS,
        KEYHOLDSTART,
        KEYHOLDRELEASE
    };

    Note(SequencerItemType itemType, bool passed, double absBeat, double beatEnd, BeatPos beatpos, BeatPos endBeatpos, NoteType noteType, NoteSplit noteSplit, std::string_view key)
    : NoteSequenceItem(itemType, passed, absBeat, beatEnd, beatpos, endBeatpos, key)
    , noteType(noteType)
    , noteSplit(noteSplit) {}

    NoteType noteType;
    NoteSplit noteSplit;

    std::string key;
};

#endif // NOTE_HPP
