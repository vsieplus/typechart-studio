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
    void updateBeatPos();
    void updateSection();

    void updateSkips();

    void addSkip(std::shared_ptr<Skip> skip);
    void removeSkip(double skipBeat);
    void resetCurrskip();

    bool addSection(int newBeatsPerMeasure, double newBPM, double newInterpolateDuration, BeatPos newBeatpos);
    bool editSection(int origSectionIndex, int newBeatsPerMeasure, double newBPM, double newInterpolateDuration, BeatPos newBeatpos);
    bool removeSection(int sectionIndex);

    void pause();
    void unpause();

    void setSongTimePosition(double absTime);
    void setSongBeatPosition(double absBeat);

    double calculateAbsBeat(BeatPos beatpos);

    bool started = false;
    bool paused = false;
    bool bpmInterpolating = false;

    bool beatSkipped = false;
    bool beatSkiptimePassed = false;

    double currSpb = 0.0;
    double bpmInterpolateStart = 0.0;
    double bpmInterpolateEnd = 0.0;

    double absTime = 0.0;
    double absBeat = 0.0;

    double prevSectionTime = 0.0;
    double prevSectionBeats = 0.0;

    double currSkipDuration = 0.0;
    double currSkipStartTimePosition = 0.0;
    double currSkipTime = 0.0;
    double currSkipSpb = 0.0;

    int offsetMS = 0;
    int currentSkip = 0;

    Uint64 currSkipBegin = 0;

    Uint64 now = 0;
    Uint64 songStart = 0;
    Uint64 pauseCounter = 0;

    unsigned int currentSection = 0;
    
    std::vector<Timeinfo> timeinfo;
    std::vector<std::shared_ptr<Skip>> skips;
};

#endif // SONGPOSITION_HPP
