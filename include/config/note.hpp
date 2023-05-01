#ifndef NOTE_HPP
#define NOTE_HPP

#include <string>
#include <unordered_map>

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
    Note(float absBeat, float beatEnd, bool passed, BeatPos beatpos, BeatPos endBeatpos, NoteType noteType, NoteSplit noteSplit, SequencerItemType itemType, std::string key) :
        NoteSequenceItem(absBeat, beatEnd, passed, beatpos, endBeatpos), noteType(noteType), noteSplit(noteSplit), itemType(itemType) { displayText = key; }

    virtual SequencerItemType getItemType() const override {
        return itemType;
    }

    void setItemType(SequencerItemType type) override { 
        itemType = type;
    }

    NoteType noteType;
    NoteSplit noteSplit;

    SequencerItemType itemType;

    std::string key;
};

#endif // NOTE_HPP
