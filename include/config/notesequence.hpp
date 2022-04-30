#ifndef NOTESEQUENCE_HPP
#define NOTESEQUENCE_HPP

#include <algorithm>
#include <list>
#include <vector>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ImSequencer.h"

#include "config/note.hpp"
#include "config/skip.hpp"
#include "config/stop.hpp"

#include "systems/audiosystem.hpp"

static const char* SequencerItemTypeNames[] = { "Lane 1 [Top]", "Lane 2 [Middle]", "Lane 3 [Bottom]", "Stops", "Skips" };

struct NoteSequence : public ImSequencer::SequenceInterface {
    // my datas
    NoteSequence() : mFrameMin(0.f), mFrameMax(1000.f) {}

    float mFrameMin, mFrameMax;

    std::vector<NoteSequenceItem> myItems;

    void update(float songBeat, float currSpb, AudioSystem * audioSystem) {
        float currBeatDelay = .1f / currSpb;
        
        for(auto & item : myItems) {
            switch(item.itemType) {
                case SequencerItemType::TOP_NOTE:
                case SequencerItemType::MID_NOTE:
                case SequencerItemType::BOT_NOTE:
                    if(item.passed) {
                        break;
                    } else if(item.absBeat + currBeatDelay < songBeat) {
                        item.passed = true;
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

    void addNote(float absBeat, float beatDuration, SequencerItemType itemType) {
        Note newNote = Note(absBeat, absBeat + beatDuration, NoteType::KEYPRESS, NoteSplit::EIGHTH, itemType, "A");
        myItems.push_back(newNote);

        std::sort(myItems.begin(), myItems.end());
    }

    void addStop(float absBeat, float beatDuration) {

    }

    void addSkip(float absBeat, float beatDuration) {

    }

    void addItem(float absBeat, float beatDuration, int itemType) {
        switch(itemType) {
            case SequencerItemType::TOP_NOTE:
            case SequencerItemType::MID_NOTE:
            case SequencerItemType::BOT_NOTE:
                addNote(absBeat, beatDuration, (SequencerItemType)itemType);
                break;
            case SequencerItemType::STOP:
                addStop(absBeat, beatDuration);
                break;
            case SequencerItemType::SKIP:
                addSkip(absBeat, beatDuration);
                break;
        }
    }

    void editItem(float absBeat, int itemType) {

    }

    void deleteItem(float absBeat, int itemType) {
        for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
            auto seqItem = *iter;
            if((int)(seqItem.itemType) == itemType && absBeat >= seqItem.absBeat &&
                (absBeat < seqItem.beatEnd || (seqItem.absBeat == seqItem.beatEnd && absBeat <= seqItem.beatEnd))) {
                myItems.erase(iter);
                break;
            }
        }
    }

    bool containsItemAt(float absBeat, int itemType) {
        for(auto seqItem : myItems) {
            if((int)(seqItem.itemType) == itemType && absBeat >= seqItem.absBeat &&
                (absBeat < seqItem.beatEnd || (seqItem.absBeat == seqItem.beatEnd && absBeat <= seqItem.beatEnd))) {
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
        NoteSequenceItem & item = myItems[index];
        if (color)
            *color = 0xFFAA8080; // same color for everyone, return color based on type
        if (start)
            *start = &(item.absBeat);
        if (end)
            *end = &(item.beatEnd);
        if(type)
            *type = (int)(item.itemType);

        if(displayText)
            *displayText = item.displayText.c_str();
    }

    virtual size_t GetCustomHeight(int index) { return 30; }
};

#endif // NOTESEQUENCE_HPP