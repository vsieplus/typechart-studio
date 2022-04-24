#ifndef NOTESEQUENCE_HPP
#define NOTESEQUENCE_HPP

#include <algorithm>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ImSequencer.h"

static const char* SequencerItemTypeNames[] = { "Top [#]", "Middle [A-Z,./;':]", "Bottom [Fn]", "Stops", "Skips" };

struct NoteSequence : public ImSequencer::SequenceInterface {
    // my datas
    NoteSequence() : mFrameMin(-10.f), mFrameMax(100.f) {
        myItems.push_back(NoteSequence::NoteSequenceItem{ SequencerItemType::TOP_NOTE, 10, 10 });
    }

    enum SequencerItemType {
        TOP_NOTE,
        MID_NOTE,
        BOT_NOTE,
        STOP,
        SKIP
    };

    float mFrameMin, mFrameMax;
    struct NoteSequenceItem
    {
        SequencerItemType mType;
        float mFrameStart, mFrameEnd;
    };

    std::vector<NoteSequenceItem> myItems;

    virtual int GetFrameMin() const {
        return mFrameMin;
    }
    virtual int GetFrameMax() const {
        return mFrameMax;
    }
    virtual int GetItemCount() const { return (int)myItems.size(); }

    virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
    virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
    virtual const char* GetItemLabel(int index) const
    {
        static char tmps[512];
        snprintf(tmps, 512, "%s", SequencerItemTypeNames[myItems[index].mType]);
        return tmps;
    }

    virtual void Get(int index, float** start, float** end, int* type, unsigned int* color)
    {
        NoteSequenceItem& item = myItems[index];
        if (color)
            *color = 0xFFAA8080; // same color for everyone, return color based on type
        if (start)
            *start = &item.mFrameStart;
        if (end)
            *end = &item.mFrameEnd;
        if (type)
            *type = item.mType;
    }
    virtual void Add(int type) { myItems.push_back(NoteSequenceItem{ (SequencerItemType)type, 0, 10 }); };
    virtual void Del(int index) { myItems.erase(myItems.begin() + index); }
    virtual void Duplicate(int index) { myItems.push_back(myItems[index]); }

    virtual size_t GetCustomHeight(int index) { return 30; }
};

#endif // NOTESEQUENCE_HPP