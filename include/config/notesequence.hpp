#ifndef NOTESEQUENCE_HPP
#define NOTESEQUENCE_HPP

#include <algorithm>
#include <float.h>
#include <list>
#include <map>
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
    int numTopNotes = 0;
    int numMidNotes = 0;
    int numBotNotes = 0;

    std::vector<std::shared_ptr<NoteSequenceItem>> myItems;
    std::map<std::string, int> keyFrequencies;
    std::vector<std::pair<std::string, int>> keyFreqsSorted;

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

    void resetPassed(float songBeat) {        
        for(auto & item : myItems) {
            switch(item->getItemType()) {
                case SequencerItemType::TOP_NOTE:
                case SequencerItemType::MID_NOTE:
                case SequencerItemType::BOT_NOTE:
                    item->passed = item->absBeat < songBeat;
                    break;
                case SequencerItemType::SKIP:
                    break;
                case SequencerItemType::STOP:
                    break;
            }
        }
    }

    void update(float songBeat, AudioSystem * audioSystem) {        
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

    static bool cmpSecond(const std::pair<std::string, int> & l, const std::pair<std::string, int> & r) {
        return l.second > r.second;
    }

    void addNote(float absBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, SequencerItemType itemType, std::string displayText) {
        NoteType noteType = NoteType::KEYPRESS;
        if(beatDuration > FLT_EPSILON) {
            noteType = NoteType::KEYHOLDSTART;
        }

        std::shared_ptr<NoteSequenceItem> newNote = std::make_shared<Note>(absBeat, absBeat + beatDuration, beatpos, endBeatpos, noteType, NoteSplit::EIGHTH, itemType, displayText);
        myItems.push_back(newNote);

        std::sort(myItems.begin(), myItems.end());

        switch(itemType) {
            case SequencerItemType::TOP_NOTE:
                numTopNotes++;
                break;
            case SequencerItemType::MID_NOTE:
                numMidNotes++;
                break;
            case SequencerItemType::BOT_NOTE:
                numBotNotes++;
                break;
            default:
                break;
        }

        keyFrequencies[displayText] += 1;

        keyFreqsSorted = std::vector<std::pair<std::string, int>>(keyFrequencies.begin(), keyFrequencies.end());
        std::sort(keyFreqsSorted.begin(), keyFreqsSorted.end(), cmpSecond);
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
        bool removeNote = false;
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                myItems.erase(iter);

                switch(seqItem->getItemType()) {
                    case SequencerItemType::TOP_NOTE:
                        numTopNotes--;
                        removeNote = true;
                        break;
                    case SequencerItemType::MID_NOTE:
                        numMidNotes--;
                        removeNote = true;
                        break;
                    case SequencerItemType::BOT_NOTE:
                        numBotNotes--;
                        removeNote = true;
                        break;
                    default:
                        break;
                }

                if(removeNote) {
                    keyFrequencies[seqItem->displayText] -= 1;

                    keyFreqsSorted = std::vector<std::pair<std::string, int>>(keyFrequencies.begin(), keyFrequencies.end());
                    std::sort(keyFreqsSorted.begin(), keyFreqsSorted.end(), cmpSecond);
                }

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

    int getLaneItemCount(SequencerItemType lane) const {
        switch(lane) {
            case SequencerItemType::TOP_NOTE:
                return numTopNotes;
            case SequencerItemType::MID_NOTE:
                return numMidNotes;
            case SequencerItemType::BOT_NOTE:
                return numBotNotes;
            default:
                return 0;
        }
    }

    std::pair<std::string, int> & getKeyItemData(int frequencyRank) {
        return keyFreqsSorted.at(frequencyRank);
    }

    void resetItemCounts() {
        numTopNotes = 0;
        numMidNotes = 0;
        numBotNotes = 0;

        for(auto & item : myItems) {
            bool insertedKey = false;

            switch(item->getItemType()) {
                case SequencerItemType::TOP_NOTE:
                    numTopNotes++;
                    insertedKey = true;
                    break;
                case SequencerItemType::MID_NOTE:
                    numMidNotes++;
                    insertedKey = true;
                    break;
                case SequencerItemType::BOT_NOTE:
                    numBotNotes++;
                    insertedKey = true;
                    break;
                default:
                    break;
            }

            if(insertedKey) {
                keyFrequencies[item->displayText] += 1;
            }
        }

        keyFreqsSorted = std::vector<std::pair<std::string, int>>(keyFrequencies.begin(), keyFrequencies.end());
        std::sort(keyFreqsSorted.begin(), keyFreqsSorted.end(), cmpSecond);
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