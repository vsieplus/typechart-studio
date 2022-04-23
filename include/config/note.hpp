#ifndef NOTE_HPP
#define NOTE_HPP

#include <string>

#include "config/beatpos.hpp"

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

struct Note {
    Note(float absTime, BeatPos absBeat, NoteType noteType, NoteSplit noteSplit, std::string key) :
        absTime(absTime), absBeat(absBeat), noteType(noteType), noteSplit(noteSplit), key(key) {}

    float absTime;

    BeatPos absBeat;

    NoteType noteType;
    NoteSplit noteSplit;

    std::string key;
};

#endif // NOTE_HPP