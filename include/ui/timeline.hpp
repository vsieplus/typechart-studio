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

class Timeline {
public:
    Timeline() = default;

    void showContents(int musicSourceIdx, bool focused, AudioSystem * audioSystem, ChartInfo & chartinfo, SongPosition & songpos, std::vector<bool> & keysPressed);

    int getUndoStackSize() const;
    int getRedoStackSize() const;

    void setCopy(bool copy);
    void setPaste(bool paste);
    void setCut(bool cut);
    void setFlip(bool flip);
private:
    void showBeatsplit();
    void showCurrentBeat(int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem);
    void showBeatpos(const SongPosition & songpos);
    void showZoom(bool focused);
    void showSequencer(bool focused, ChartInfo & chartinfo, SongPosition & songpos);

    void checkResetClicks();
    void checkUpdatedBeat();
    void checkUpdatedBeat(bool focused, int musicSourceIdx, ChartInfo & chartinfo, SongPosition & songpos, AudioSystem * audioSystem);
    void prepUpdateEntity(bool focused, std::string_view addItemPopup, const SongPosition & songpos);
    void setEntityType(bool focused, std::string_view addItemPopup, const SongPosition & songpos);

    void checkEditActions(bool focused, std::vector<bool> & keysPressed);
    void editCopy();
    void editCut(ChartInfo & chartinfo);
    void editFlip(ChartInfo & chartinfo);

    void showAddItem(std::vector <bool> & keysPressed);
    void checkDeleteItem(bool focused);

    void showHorizontalScroll(AudioSystem * audioSystem);

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
