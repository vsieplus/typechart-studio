#ifndef SONGPOSITION_HPP
#define SONGPOSITION_HPP

#include <list>
#include <vector>

#include <SDL2/SDL.h>

#include "config/timeinfo.hpp"
#include "config/skip.hpp"

struct SongPosition {
    void start();
    void stop();

    void update();
    void updateBPM();
    void updateBeatPos(uint64_t now);
    void updateSection();

    void updateSkips();

    void addSkip(std::shared_ptr<Skip> skip);
    void removeSkip(float absBeat);
    void resetCurrskip();

    void pause();
    void unpause();

    void setSongTimePosition(float absTime);
    void setSongBeatPosition(float absBeat);

    bool started = false;
    bool paused = false;
    bool bpmInterpolating = false;

    bool beatSkipped = false;
    bool beatSkiptimePassed = false;

    float currSpb = 0.f;
    float bpmInterpolateStart = 0.f;
    float bpmInterpolateEnd = 0.f;

    float absTime = 0.f;
    float absBeat = 0.f;

    float prevSectionTime = 0.f;
    float prevSectionBeats = 0.f;

    float currSkipDuration = 0.f;
    float currSkipStartTimePosition = 0.f;
    float currSkipTime = 0.f;
    float currSkipSpb = 0.f;

    int offsetMS = 0;
    int currentSkip = 0;

    Uint64 currSkipBegin = 0;

    Uint64 songStart = 0;
    Uint64 pauseCounter = 0;

    unsigned int currentSection = 0;
    
    std::vector<Timeinfo> timeinfo;
    std::vector<std::shared_ptr<Skip>> skips;
};

#endif // SONGPOSITION_HPP