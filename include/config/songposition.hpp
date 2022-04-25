#ifndef SONGPOSITION_HPP
#define SONGPOSITION_HPP

#include <vector>
#include <SDL2/SDL.h>

#include "config/timeinfo.hpp"

struct SongPosition {
    void start();
    void stop();

    void update();
    void updateBeatPos();
    void updateSection();

    void pause();
    void unpause();

    void setSongTimePosition(float absTime);
    void setSongBeatPosition(float absBeat);

    bool started = false;
    bool paused = false;

    float absTime = 0.f;
    float absBeat = 0.f;

    float prevSectionTime = 0.f;
    float prevSectionBeats = 0.f;

    int offsetMS = 0;

    Uint64 songStart = 0;
    Uint64 pauseCounter = 0;

    unsigned int currentSection = 0;

    std::vector<Timeinfo> timeinfo;
};

#endif // SONGPOSITION_HPP