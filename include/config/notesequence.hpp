#ifndef NOTESEQUENCE_HPP
#define NOTESEQUENCE_HPP

#include <algorithm>
#include <float.h>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <vector>
#include <unordered_map>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ImSequencer.h"

#include "config/note.hpp"
#include "config/notemaps.hpp"
#include "config/skip.hpp"
#include "config/stop.hpp"
#include "config/timeinfo.hpp"
#include "config/utils.hpp"

#include "actions/shiftnote.hpp"

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
        auto gcd = std::gcd(noteSplitNumerator, noteSplitDenominator);
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
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                    item->passed = item->absBeat < songBeat;
                    break;
                case NoteSequenceItem::SequencerItemType::SKIP:
                    break;
                case NoteSequenceItem::SequencerItemType::STOP:
                    break;
            }
        }
    }

    void update(float songBeat, AudioSystem * audioSystem, bool notesoundEnabled) {
        for(auto & item : myItems) {
            switch(item->getItemType()) {
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                    if(item->passed) {
                        break;
                    } else if(item->absBeat < songBeat) {
                        item->passed = true;

                        if(notesoundEnabled) {
                            audioSystem->playSound("keypress");
                        }
                    }
                    break;
                case NoteSequenceItem::SequencerItemType::SKIP:
                    break;
                case NoteSequenceItem::SequencerItemType::STOP:
                    break;
            }
        }
    }

    static bool cmpSecond(const std::pair<std::string, int> & l, const std::pair<std::string, int> & r) {
        return l.second > r.second;
    }

    void addNote(float absBeat, float songBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos, NoteSequenceItem::SequencerItemType itemType, std::string displayText) {
        NoteType noteType = NoteType::KEYPRESS;
        if(beatDuration > FLT_EPSILON) {
            noteType = NoteType::KEYHOLDSTART;
        }

        bool passed = absBeat < songBeat;

        std::shared_ptr<NoteSequenceItem> newNote = std::make_shared<Note>(absBeat, absBeat + beatDuration, passed, beatpos, endBeatpos, 
            noteType, NoteSplit::EIGHTH, itemType, displayText);
        myItems.push_back(newNote);

        std::sort(myItems.begin(), myItems.end());

        switch(itemType) {
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                numTopNotes++;
                break;
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
                numMidNotes++;
                break;
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                numBotNotes++;
                break;
            default:
                break;
        }

        keyFrequencies[displayText] += 1;

        updateKeyFrequencies();
    }

    void flipNotes(std::string keyboardLayout, float startBeat, float endBeat, int minItemType, int maxItemType) {
        if(notemaps::KEYBOARD_FLIP_MAPS.find(keyboardLayout) != notemaps::KEYBOARD_FLIP_MAPS.end()) {
            auto & flipMap = notemaps::KEYBOARD_FLIP_MAPS.at(keyboardLayout);

            auto items = getItems(startBeat, endBeat, minItemType, maxItemType);
            for(auto item: items) {
                switch(item->getItemType()) {
                    case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                    case NoteSequenceItem::SequencerItemType::MID_NOTE:
                    case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                        if(flipMap.find(item->displayText) != flipMap.end()) {
                            keyFrequencies[item->displayText] -= 1;
                            item->displayText = flipMap.at(item->displayText);
                            keyFrequencies[item->displayText] += 1;
                        }
                        break;
                    default:
                        break;
                }
            }

            updateKeyFrequencies();
        }
    }

    std::list<std::shared_ptr<NoteSequenceItem>> shiftNotes(std::string keyboardLayout, float startBeat, float endBeat,
        int minItemType, int maxItemType, ShiftNoteAction::ShiftDirection shiftDirection)
    {
        auto items = getItems(startBeat, endBeat, minItemType, maxItemType);
        return shiftItems(keyboardLayout, startBeat, endBeat, items, shiftDirection);
    }

    std::list<std::shared_ptr<NoteSequenceItem>> shiftItems(std::string keyboardLayout, float startBeat, float endBeat,
        const std::list<std::shared_ptr<NoteSequenceItem>> & items, ShiftNoteAction::ShiftDirection shiftDirection)
    {
        std::list<std::shared_ptr<NoteSequenceItem>> shiftedItems;

        if(notemaps::KEYBOARD_LAYOUTS.find(keyboardLayout) != notemaps::KEYBOARD_LAYOUTS.end() && notemaps::KEYBOARD_POSITION_MAPS.find(keyboardLayout) != notemaps::KEYBOARD_POSITION_MAPS.end()) {
            auto & keyboardLayoutMap = notemaps::KEYBOARD_LAYOUTS.at(keyboardLayout);
            auto & keyboardPositionMap = notemaps::KEYBOARD_POSITION_MAPS.at(keyboardLayout);

            for(auto item: items) {
                auto shifted = shiftNoteSequenceItem(shiftDirection, item, keyboardLayoutMap, keyboardPositionMap);
                if(shifted) {
                    shiftedItems.push_back(item);
                }
            }

            updateKeyFrequencies();
        }

        return shiftedItems;
    }

    bool shiftNoteSequenceItem(ShiftNoteAction::ShiftDirection shiftDirection, std::shared_ptr<NoteSequenceItem> item,
        const std::vector<std::vector<std::string>> & keyboardLayoutMap, const std::unordered_map<std::string, std::pair<int, int>> & keyboardPositionMap)
    {
        auto itemKey = item->displayText;
        auto itemType = item->getItemType();

        switch(itemType) {
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                if(keyboardPositionMap.find(itemKey) != keyboardPositionMap.end()) {
                    auto keyPos = keyboardPositionMap.at(itemKey);
                    int keyRow = keyPos.first;
                    int keyCol = keyPos.second;

                    int newRow = keyRow;
                    int newCol = keyCol;
                    switch(shiftDirection) {
                        case ShiftNoteAction::ShiftDirection::ShiftUp:
                            newRow = keyRow - 1;
                            break;
                        case ShiftNoteAction::ShiftDirection::ShiftDown:
                            newRow = keyRow + 1;
                            break;
                        case ShiftNoteAction::ShiftDirection::ShiftLeft:
                            newCol = keyCol - 1;
                            break;
                        case ShiftNoteAction::ShiftDirection::ShiftRight:
                            newCol = keyCol + 1;
                            break;
                        default:
                            break;
                    }

                    newRow = std::max(0, std::min(newRow, 3));
                    newCol = std::max(0, std::min(newCol, 9));

                    if(newRow == keyRow && newCol == keyCol) {
                        return false;
                    }

                    if(itemType == NoteSequenceItem::SequencerItemType::TOP_NOTE && newRow > 0) {
                        item->setItemType(NoteSequenceItem::SequencerItemType::MID_NOTE);
                    }

                    if(itemType == NoteSequenceItem::SequencerItemType::MID_NOTE && newRow < 1) {
                        item->setItemType(NoteSequenceItem::SequencerItemType::TOP_NOTE);
                    }

                    auto newKey = keyboardLayoutMap[newRow][newCol];
                    keyFrequencies[itemKey] -= 1;
                    item->displayText = newKey;
                    keyFrequencies[newKey] += 1;
                }
                break;
            default:
                break;
        }

        return true;
    }

    void addStop(float absBeat, float songBeat, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) {
        bool passed = absBeat < songBeat;
        std::shared_ptr<Stop> newStop = std::make_shared<Stop>(absBeat, beatDuration, passed, beatpos, endBeatpos);
        newStop->displayText = std::to_string(beatDuration);
        myItems.push_back(newStop);

        std::sort(myItems.begin(), myItems.end());
    }

    std::shared_ptr<Skip> addSkip(float absBeat, float songBeat, float skipTime, float beatDuration, BeatPos beatpos, BeatPos endBeatpos) {
        bool passed = absBeat < songBeat;
        std::shared_ptr<Skip> newSkip = std::make_shared<Skip>(absBeat, skipTime, passed, beatDuration, beatpos, endBeatpos);
        newSkip->displayText = std::to_string(skipTime);
        myItems.push_back(newSkip);

        std::sort(myItems.begin(), myItems.end());

        return newSkip;
    }

    void editSkip(float absBeat, float skipTime) {
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;
            if((int)(seqItem->getItemType()) == NoteSequenceItem::SequencerItemType::SKIP && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {

                auto currSkip = std::dynamic_pointer_cast<Skip>(seqItem);
                currSkip->displayText = std::to_string(skipTime);
                currSkip->skipTime = skipTime;
        
                break;
            }
        }
    }

    void editNote(float absBeat, int itemType, std::string displayText) {
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                
                auto oldText = seqItem->displayText;
                seqItem->displayText = displayText;

                keyFrequencies[oldText] -= 1;
                keyFrequencies[displayText] += 1;

                updateKeyFrequencies();

                break;
            }
        }
    }

    std::list<std::shared_ptr<NoteSequenceItem>> getItems(float startBeat, float endBeat, int minItemType, int maxItemType) {
        std::list<std::shared_ptr<NoteSequenceItem>> currItems;

        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;

            int seqItemType = (int)(seqItem->getItemType());

            if(seqItemType >= minItemType && seqItemType <= maxItemType && startBeat <= seqItem->absBeat && seqItem->absBeat <= endBeat) {
                currItems.push_back(seqItem);
            } else if(endBeat <= seqItem->absBeat) {
                break;
            }
        }

        return currItems;
    }

    void insertItems(float insertBeat, float songBeat, int minItemType, int maxItemType,
        const std::vector<Timeinfo> & timeinfo, std::list<std::shared_ptr<NoteSequenceItem>> items)
    {
        if(!items.empty()) {
            float firstBeat = items.front()->absBeat;
            BeatPos firstBeatPos = items.front()->beatpos;
            BeatPos insertBeatPos = utils::calculateBeatpos(insertBeat, firstBeatPos.measureSplit, timeinfo);

            deleteItems(insertBeat, insertBeat + (items.back()->beatEnd - firstBeat), minItemType, maxItemType);
            
            for(auto item : items) {
                float currBeat = insertBeat + (item->absBeat - firstBeat);

                BeatPos currBeatPos = insertBeatPos + (item->beatpos - firstBeatPos);
                BeatPos currEndBeatPos = insertBeatPos + (item->endBeatpos - firstBeatPos);

                switch(item->getItemType()) {
                    case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                    case NoteSequenceItem::SequencerItemType::MID_NOTE:
                    case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                        addNote(currBeat, songBeat, item->beatEnd - item->absBeat, currBeatPos, currEndBeatPos, item->getItemType(), item->displayText);
                        break;
                    case NoteSequenceItem::SequencerItemType::STOP:
                        addStop(currBeat, songBeat, item->beatEnd - item->absBeat, currBeatPos, currEndBeatPos);
                        break;
                    case NoteSequenceItem::SequencerItemType::SKIP:
                        auto currSkip = std::dynamic_pointer_cast<Skip>(item);
                        addSkip(currBeat, songBeat, currSkip->skipTime, item->beatEnd - item->absBeat, currBeatPos, currEndBeatPos);
                        break;
                }
            }
        }
    }

    void deleteItems(float startBeat, float endBeat, int minItemType, int maxItemType) {
        for(auto iter = myItems.begin(); iter != myItems.end();) {
            bool removeNote = false;
            auto & seqItem = *iter;

            int seqItemType = (int)(seqItem->getItemType());

            if(seqItemType >= minItemType && seqItemType <= maxItemType && startBeat <= seqItem->absBeat && seqItem->absBeat <= endBeat) {
                switch(seqItem->getItemType()) {
                    case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                        numTopNotes--;
                        removeNote = true;
                        break;
                    case NoteSequenceItem::SequencerItemType::MID_NOTE:
                        numMidNotes--;
                        removeNote = true;
                        break;
                    case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                        numBotNotes--;
                        removeNote = true;
                        break;
                    default:
                        break;
                }

                if(removeNote) {
                    keyFrequencies[seqItem->displayText] -= 1;
                    if(keyFrequencies[seqItem->displayText] == 0) {
                        keyFrequencies.erase(seqItem->displayText);
                    }
                }

                // in case dangling pointers in undo/redo stack refer to this item
                seqItem->deleted = true;

                iter = myItems.erase(iter);
            } else {
                iter++;
            }
        }

        updateKeyFrequencies();
    }

    void deleteItem(float absBeat, int itemType) {
        bool removeNote = false;
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto & seqItem = *iter;
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                switch(seqItem->getItemType()) {
                    case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                        numTopNotes--;
                        removeNote = true;
                        break;
                    case NoteSequenceItem::SequencerItemType::MID_NOTE:
                        numMidNotes--;
                        removeNote = true;
                        break;
                    case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                        numBotNotes--;
                        removeNote = true;
                        break;
                    default:
                        break;
                }

                if(removeNote) {
                    keyFrequencies[seqItem->displayText] -= 1;

                    if(keyFrequencies[seqItem->displayText] == 0) {
                        keyFrequencies.erase(seqItem->displayText);
                    }

                    updateKeyFrequencies();
                }

                // in case dangling pointers in undo/redo stack refer to this item
                seqItem->deleted = true;

                myItems.erase(iter);
                break;
            }
        }
    }

    std::shared_ptr<NoteSequenceItem> containsItemAt(float absBeat, int itemType) {
        for(auto & seqItem : myItems) {
            if((int)(seqItem->getItemType()) == itemType && absBeat >= seqItem->absBeat &&
                (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
                return seqItem;
            }
        }

        return nullptr;
    }

    int getLaneItemCount(NoteSequenceItem::SequencerItemType lane) const {
        switch(lane) {
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                return numTopNotes;
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
                return numMidNotes;
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
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
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                    numTopNotes++;
                    insertedKey = true;
                    break;
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                    numMidNotes++;
                    insertedKey = true;
                    break;
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
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

        updateKeyFrequencies();
    }

    void updateKeyFrequencies() {
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
