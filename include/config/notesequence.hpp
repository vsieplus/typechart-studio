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

#include "config/constants.hpp"
#include "config/note.hpp"
#include "config/skip.hpp"
#include "config/stop.hpp"
#include "config/timeinfo.hpp"
#include "config/utils.hpp"

#include "actions/shiftnote.hpp"

#include "systems/audiosystem.hpp"

struct NoteSequence : public ImSequencer::SequenceInterface {
    NoteSequence() = default;

    float mFrameMin { 0.f };
    float mFrameMax { 1000.f };
    int numTopNotes { 0 };
    int numMidNotes { 0 };
    int numBotNotes { 0 };

    std::vector<std::shared_ptr<NoteSequenceItem>> myItems;
    std::map<std::string, int> keyFrequencies;
    std::vector<std::pair<std::string, int>> keyFreqsSorted;

    void update(double songBeat, AudioSystem * audioSystem, bool notesoundEnabled);
    void resetPassed(double songBeat);

    void addNote(double absBeat, double songBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos,
        NoteSequenceItem::SequencerItemType itemType, std::string_view displayText);
    void editNote(double absBeat, NoteSequenceItem::SequencerItemType itemType, std::string_view displayText);

    void addStop(double absBeat, double songBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos);

    std::shared_ptr<Skip> addSkip(double absBeat, double songBeat, double skipTime, double beatDuration, BeatPos beatpos, BeatPos endBeatpos);
    void editSkip(double absBeat, double skipTime);

    void flipNotes(std::string_view keyboardLayout, double startBeat, double endBeat, int minItemType, int maxItemType);

    std::list<std::shared_ptr<NoteSequenceItem>> shiftNotes(std::string_view keyboardLayout, double startBeat, double endBeat,
        int minItemType, int maxItemType, ShiftNoteAction::ShiftDirection shiftDirection);
    std::list<std::shared_ptr<NoteSequenceItem>> shiftItems(std::string_view keyboardLayout, double startBeat, double endBeat,
        const std::list<std::shared_ptr<NoteSequenceItem>> & items, ShiftNoteAction::ShiftDirection shiftDirection);
    bool shiftNoteSequenceItem(ShiftNoteAction::ShiftDirection shiftDirection, std::shared_ptr<NoteSequenceItem> item, std::string_view keyboardLayout);

    std::list<std::shared_ptr<NoteSequenceItem>> getItems(double startBeat, double endBeat, int minItemType, int maxItemType) const;
    std::shared_ptr<NoteSequenceItem> containsItemAt(double absBeat, NoteSequenceItem::SequencerItemType itemType);
    int getLaneItemCount(NoteSequenceItem::SequencerItemType lane) const;
    void resetItemCounts();

    void insertItems(double insertBeat, double songBeat, int minItemType, int maxItemType, const std::vector<Timeinfo> & timeinfo, std::list<std::shared_ptr<NoteSequenceItem>> items);
    void deleteItems(double startBeat, double endBeat, int minItemType, int maxItemType);
    void deleteItem(double absBeat, NoteSequenceItem::SequencerItemType itemType);

    const std::pair<std::string, int> & getKeyItemData(int frequencyRank) const;

    void updateKeyFrequencies() {
        keyFreqsSorted = std::vector<std::pair<std::string, int>>(keyFrequencies.begin(), keyFrequencies.end());
        std::sort(keyFreqsSorted.begin(), keyFreqsSorted.end(), utils::cmpSecond);
    }

    int GetFrameMin() const override { return static_cast<int>(mFrameMin); }
    int GetFrameMax() const override { return static_cast<int>(mFrameMax); }
    int GetItemCount() const override { return static_cast<int>(myItems.size()); }
    int GetItemTypeCount() const override { return static_cast<int>(constants::SEQUENCER_ITEM_TYPES.size()); }
    const char* GetItemTypeName(int typeIndex) const override { return constants::SEQUENCER_ITEM_TYPES[typeIndex].c_str(); }
    const char* GetItemLabel(int index) const override { return ""; }

    void Get(int index, float** start, float** end, int* type, unsigned int* color, const char** displayText) override;
    size_t GetCustomHeight(int index) override { return 30; }
};

#endif // NOTESEQUENCE_HPP
