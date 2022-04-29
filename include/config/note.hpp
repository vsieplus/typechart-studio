#ifndef NOTE_HPP
#define NOTE_HPP

#include <string>

#include "config/notesequenceitem.hpp"

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

struct Note : public NoteSequenceItem {
    Note(float absBeat, float beatEnd, NoteType noteType, NoteSplit noteSplit, SequencerItemType itemType, std::string key) :
        NoteSequenceItem(absBeat, beatEnd, itemType), noteType(noteType), noteSplit(noteSplit), key(key) {}

    NoteType noteType;
    NoteSplit noteSplit;

    std::string key;
};

#endif // NOTE_HPP