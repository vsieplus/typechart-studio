#ifndef TIMEINFO_HPP
#define TIMEINFO_HPP

#include "config/beatpos.hpp"

struct Timeinfo {
    Timeinfo(BeatPos beatpos, Timeinfo * prevTimeinfo, int beatsPerMeasure, float bpm);

    float calculateBeatStart(Timeinfo * prevTimeinfo);
    float calculateTimeStart(Timeinfo * prevTimeinfo);

    BeatPos beatpos;
    int beatsPerMeasure;

    float bpm;

    float absBeatStart;
    float absTimeStart;
};


bool operator<(const Timeinfo & lhs, const Timeinfo & rhs);


#endif // TIMEINFO_HPP