#ifndef SONGPOSITION_HPP
#define SONGPOSITION_HPP

#include <vector>
#include <SDL2/SDL.h>

#include "config/timeinfo.hpp"

struct SongPosition {
    void start();
    void stop();
    void update();

    void pause();
    void unpause();

    void setSongPosition(float absPosition, bool isTime);

    bool started = false;
    bool paused = false;

    float absTime = 0.f;
    float absBeat = 0.f;

    Uint64 songStart = 0;
    Uint64 pauseCounter = 0;

    int currentSection = 0;

    std::vector<Timeinfo> timeinfo;
};

#endif // SONGPOSITION_HPP