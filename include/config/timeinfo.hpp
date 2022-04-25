#ifndef TIMEINFO_HPP
#define TIMEINFO_HPP

#include "config/beatpos.hpp"

struct Timeinfo {
    Timeinfo(BeatPos beatpos, Timeinfo * prevTimeinfo, int beatsPerMeasure, float bpm);

    float calculateBeatStart();
    float calculateTimeStart(Timeinfo * prevTimeinfo);

    BeatPos beatpos;
    int beatsPerMeasure;

    float bpm;

    float absBeatStart;
    float absTimeStart;
};


#endif // TIMEINFO_HPP