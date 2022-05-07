#ifndef NOTESEQUENCE_HPP
#define NOTESEQUENCE_HPP

#include <algorithm>
#include <float.h>
#include <list>
#include <memory>
#include <vector>
#include <unordered_map>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ImSequencer.h"

#include "config/note.hpp"
#include "config/skip.hpp"
#include "config/stop.hpp"

#include "systems/audiosystem.hpp"

static const char* SequencerItemTypeNames[] = { "Lane 1 [Top]", "Lane 2 [Middle]", "Lane 3 [Bottom]", "Stops", "Skips" };

const std::unordered_map<int, NoteSplit> DENOMINATOR_TO_NOTESPLIT = {
    { 1, NoteSplit::WHOLE },
    { 2, NoteSplit::HALF },
    { 4, NoteSplit::QUARTER },
    { 8, NoteSplit::EIGHTH },
    { 16, NoteSplit::SIXTEENTH },
    { 32, NoteSplit::THIRTYSECOND },
    { 64, NoteSplit::SIXTYFOURTH },
    { 128, NoteSplit::ONETWENTYEIGHTH },
    { 3, NoteSplit::TWELFTH },
    { 6, NoteSplit::TWELFTH },
    { 12, NoteSplit::TWELFTH },
    { 24, NoteSplit::TWENTYFOURTH },
    { 48, NoteSplit::FORTYEIGHTH },
    { 96, NoteSplit::NINETYSIXTH }
};

struct NoteSequence : public ImSequencer::SequenceInterface {
    // my datas
    NoteSequence() : mFrameMin(0.f), mFrameMax(1000.f) {}

    float mFrameMin, mFrameMax;

    std::vector<std::shared_ptr<NoteSequenceItem>> myItems;

    NoteSplit computeNoteSplit(int noteSplitNumerator, int noteSplitDenominator) {
        auto gcd = std::__gcd(noteSplitNumerator, noteSplitDenominator);
        noteSplitNumerator /= gcd;
        noteSplitDenominator /= gcd;

        if(DENOMINATOR_TO_NOTESPLIT.find(noteSplitDenominator) != DENOMINATOR_TO_NOTESPLIT.end()) {
            return DENOMINATOR_TO_NOTESPLIT.at(noteSplitDenominator);
        } else {
            return NoteSplit::WHOLE;
        }    
    }

    void resetPassed(float songBeat, float currSpb) {
        float currBeatDelay = .1f / currSpb;
        
        for(auto & item : myItems) {
            switch(item->getItemType()) {
                case SequencerItemType::TOP_NOTE:
                case SequencerItemType::MID_NOTE:
                case SequencerItemType::BOT_NOTE:
                    item->passed = item->absBeat + currBeatDelay < songBeat;
                    break;
                case SequencerItemType::SKIP:
                    break;
                case SequencerItemType::STOP:
                    break;
            }
        }
    }

    void update(float songBeat, float currSpb, AudioSystem * audioSystem) {        
        for(auto & item : myItems) {
            switch(item->getItemType()) {
                case SequencerItemType::TOP_NOTE:
                case SequencerItemType::MID_NOTE:
                case SequencerItemType::BOT_NOTE:
                    if(item->passed) {
                        break;
                    } else if(item->absBeat < songBeat) {
                        item->passed = true;
                        audioSystem->playSound("keypress");
                    }
                    break;
                case SequencerItemType::SKIP:
                    break;
                case SequencerItemType::STOP:
                    break;
            }
        }
    }

    void addNote(float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType, std::string displayText) {
        NoteType noteType = NoteType::KEYPRESS;
        if(beatDuration > FLT_EPSILON) {
            noteType = NoteType::KEYHOLDSTART;
        }

        std::shared_ptr<NoteSequenceItem> newNote = std::make_shared<Note>(absBeat, absBeat + beatDuration, beatpos, endBeatpos, noteType, NoteSplit::EIGHTH, itemType, displayText);
        myItems.push_back(newNote);

        std::sort(myItems.begin(), myItems.end());
    }

    void addStop(float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) {

    }

    void addSkip(float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) {

    }

    void addItem(float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, int itemType, std::string displayText) {
        switch(itemType) {
            case SequencerItemType::TOP_NOTE:
            case SequencerItemType::MID_NOTE:
            case SequencerItemType::BOT_NOTE:
                addNote(absBeat, beatDuration, beatpos, endBeatpos, (SequencerItemType)itemType, displayText);
                break;
            case SequencerItemType::STOP:
                addStop(absBeat, beatDuration, beatpos, endBeatpos);
                break;
            case SequencerItemType::SKIP:
                addSkip(absBeat, beatDuration, beatpos, endBeatpos);
                break;
        }
    }

    void editItem(float absBeat, int itemType, std::string displayText) {
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                
                seqItem->displayText = displayText;
                break;
            }
        }
    }

    void deleteItem(float absBeat, int itemType) {
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                myItems.erase(iter);
                break;
            }
        }
    }

    bool containsItemAt(float absBeat, int itemType) {
        for(auto & seqItem : myItems) {
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                return true;
            }
        }

        return false;
    }

    virtual int GetFrameMin() const {
        return mFrameMin;
    }
    virtual int GetFrameMax() const {
        return mFrameMax;
    }

    virtual int GetItemCount() const { return myItems.size(); }

    virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
    virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
    virtual const char* GetItemLabel(int index) const { return ""; }

    virtual void Get(int index, float** start, float** end, int* type, unsigned int* color, const char** displayText)
    {
        auto & item = myItems[index];
        if (color)
            *color = 0xFFAA8080; // same color for everyone, return color based on type
        if (start)
            *start = &(item->absBeat);
        if (end)
            *end = &(item->beatEnd);
        if(type)
            *type = (int)(item->getItemType());

        if(displayText)
            *displayText = item->displayText.c_str();
    }

    virtual size_t GetCustomHeight(int index) { return 30; }
};

#endif // NOTESEQUENCE_HPP