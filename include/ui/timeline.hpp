#ifndef TIMELINE_HPP
#define TIMELINE_HPP

#include "imgui.h"

#include <vector>

class AudioSystem;

class Timeline {
public:
    Timeline() = default;

    void showContents(AudioSystem * audioSystem, std::vector<bool> & keysPressed);
private:

    void showAddItem(int insertItemType, double insertBeat, double endBeat, ImGuiInputTextFlags addItemFlags, std::vector <bool> & keysPressed);
    void checkDeleteItem(double clickedBeat, int clickedItemType);

    void showHorizontalScroll(AudioSystem * audioSystem, double timelineZoom, float currentBeatsplitValue);

};

#endif // TIMELINE_HPP
