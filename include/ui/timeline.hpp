#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include "actions/editaction.hpp"
#include "config/chartinfo.hpp"
#include "config/songposition.hpp"

#include "imgui.h"

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>

class AudioSystem;

namespace utils {
    void updateAudioPosition(AudioSystem * audioSystem, const SongPosition & songpos, int musicSourceIdx);
}

struct Timeline {
    Timeline() = default;

    void showContents(int musicSourceIdx, bool focused, bool & unsaved, AudioSystem * audioSystem, ChartInfo & chartinfo, SongPosition & songpos, std::vector<bool> & keysPressed);

    int getUndoStackSize() const;
    int getRedoStackSize() const;

    void showBeatsplit();
    void showCurrentBeat(int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem) const;
    void showBeatpos(const SongPosition & songpos) const;
    void showZoom(bool focused);
    void showSequencer(bool focused, ChartInfo & chartinfo, SongPosition & songpos);

    void checkResetClicks();
    void checkUpdatedBeat(bool focused, int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem) const;
    void prepUpdateEntity(bool focused, const std::string & addItemPopup, const SongPosition & songpos);
    void setEntityType(bool focused, const std::string & addItemPopup, const SongPosition & songpos);

    void checkEditActions(bool focused, bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos, std::vector<bool> & keysPressed);
    void editCopy(const ChartInfo & chartinfo);
    void editCut(bool & unsaved, ChartInfo & chartinfo);
    void editFlip(bool & unsaved, ChartInfo & chartinfo);
    void editShiftNotes(bool & unsaved, ChartInfo & chartinfo, const std::vector<bool> & keysPressed);
    void editDelete(bool & unsaved, ChartInfo & chartinfo);
    void editPaste(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos);

    void showAddItem(bool & unsaved, ChartInfo & chartinfo, SongPosition & songpos, std::vector <bool> & keysPressed);
    void showTopMidNote(bool & unsaved, char * addedItem, ChartInfo & chartinfo, const SongPosition & songpos);
    void showBotNote(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos);
    void showSkip(bool & unsaved, ChartInfo & chartinfo, SongPosition & songpos, const std::vector<bool> & keysPressed);
    void showStop(bool & unsaved, ChartInfo & chartinfo, const SongPosition & songpos);

    void checkDeleteItem(bool focused, bool & unsaved, ChartInfo & chartinfo, SongPosition & songpos);
    void checkUpdateNotes(bool focused, AudioSystem * audioSystem, ChartInfo & chartinfo, const SongPosition & songpos);

    void showHorizontalScroll(bool focused, int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem);

    bool updatedBeat { false };

    bool leftClickedEntity { false };
    bool leftClickReleased { false };
    bool leftClickShift { false };
    bool haveSelection { false };
    bool rightClickedEntity { false };
    bool startedNote { false };

    bool activateCopy { false };
    bool activatePaste { false };
    bool activateCut { false };
    bool activateFlip { false };

    int currentBeatsplit { 4 };
    int clickedItemType { 0 };
    int releasedItemType { 0 };
    int insertItemType { 0 };
    int insertItemTypeEnd { 0 };

    double currentBeatsplitValue { 0.0 };
    double clickedBeat { 0.0 };
    double hoveredBeat { 0.0 };
    double zoom { 2.0 };

    double insertBeat { 0.0 };
    double endBeat { 0.0 };

    BeatPos insertBeatpos;
    BeatPos endBeatpos;

    ImGuiInputTextFlags addItemFlags { 0 };
    ImGuiInputTextCallbackData addItemCallbackData;

    std::stack<std::shared_ptr<EditAction>> undoStack;
    std::stack<std::shared_ptr<EditAction>> redoStack;

    std::list<std::shared_ptr<NoteSequenceItem>> copiedItems;
};

#endif // TIMELINE_HPP
