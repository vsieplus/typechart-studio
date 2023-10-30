#include "config/timeinfo.hpp"

Timeinfo::Timeinfo(BeatPos beatpos, const Timeinfo * prevTimeinfo, int beatsPerMeasure, double bpm, double interpolateBeatDuration)
    : beatpos(beatpos)
    , beatsPerMeasure(beatsPerMeasure)
    , bpm(bpm)
    , absBeatStart(calculateBeatStart(prevTimeinfo))
    , absTimeStart(calculateTimeStart(prevTimeinfo))
    , interpolateBeatDuration(interpolateBeatDuration) {}

double Timeinfo::calculateBeatStart(const Timeinfo * prevTimeinfo) const {
    auto absMeasure = beatpos.measure + (beatpos.split / (double)beatpos.measureSplit);

    if(!prevTimeinfo) {
        return absMeasure * beatsPerMeasure;
    } else {
        auto prevAbsMeasure = prevTimeinfo->beatpos.measure + ((double)prevTimeinfo->beatpos.split / prevTimeinfo->beatpos.measureSplit);
        auto absMeasureDiff = absMeasure - prevAbsMeasure;

        return prevTimeinfo->absBeatStart + (absMeasureDiff * prevTimeinfo->beatsPerMeasure);
    }
}

double Timeinfo::calculateTimeStart(const Timeinfo * prevTimeinfo) const {
    if(!prevTimeinfo) {
        return 0.0;
    } else {
        auto prevSectionBeatLength = absBeatStart - prevTimeinfo->absBeatStart;
        return prevTimeinfo->absTimeStart + prevSectionBeatLength * (60.0 / prevTimeinfo->bpm); 
    }
}

bool operator<(const Timeinfo & lhs, const Timeinfo & rhs) {
    return lhs.beatpos < rhs.beatpos;
}
