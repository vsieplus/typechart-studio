#ifndef TIMEINFO_HPP
#define TIMEINFO_HPP

#include "config/beatpos.hpp"

namespace constants {
    const int DEFAULT_BEATS_PER_MEASURE = 4;
}

struct Timeinfo {
    Timeinfo() = default;
    Timeinfo(BeatPos beatpos, const Timeinfo * prevTimeinfo, int beatsPerMeasure, double bpm, double interpolateBeatDuration);

    double calculateBeatStart(const Timeinfo * prevTimeinfo) const;
    double calculateTimeStart(const Timeinfo * prevTimeinfo) const;

    BeatPos beatpos;

    int beatsPerMeasure { constants::DEFAULT_BEATS_PER_MEASURE };

    double bpm { 100.0 };

    double absBeatStart { 0.0 };
    double absTimeStart { 0.0 };
    double interpolateBeatDuration { 0.0 };
};

bool operator<(const Timeinfo & lhs, const Timeinfo & rhs);

#endif // TIMEINFO_HPP
