#include "config/timeinfo.hpp"

Timeinfo::Timeinfo(BeatPos beatpos, Timeinfo * prevTimeinfo, int beatsPerMeasure, float bpm) :
        beatpos(beatpos), beatsPerMeasure(beatsPerMeasure), bpm(bpm), absBeatStart(calculateBeatStart(prevTimeinfo)),
        absTimeStart(calculateTimeStart(prevTimeinfo)) {}

float Timeinfo::calculateBeatStart(Timeinfo * prevTimeinfo) {
    auto absMeasure = beatpos.measure + (beatpos.split / (float)beatpos.beatsplit);

    if(!prevTimeinfo) {
        return absMeasure * beatsPerMeasure;
    } else {
        auto prevAbsMeasure = prevTimeinfo->beatpos.measure + ((float)prevTimeinfo->beatpos.split / prevTimeinfo->beatpos.beatsplit);
        auto absMeasureDiff = absMeasure - prevAbsMeasure;

        return prevTimeinfo->absBeatStart + (absMeasureDiff * prevTimeinfo->beatsPerMeasure);
    }
}

float Timeinfo::calculateTimeStart(Timeinfo * prevTimeinfo) {
    if(!prevTimeinfo) {
        return 0.f;
    } else {
        auto prevSectionBeatLength = absBeatStart - prevTimeinfo->absBeatStart;
        return prevTimeinfo->absTimeStart + prevSectionBeatLength * (60.f / prevTimeinfo->bpm); 
    }
}

bool operator<(const Timeinfo & lhs, const Timeinfo & rhs) {
    return lhs.beatpos < rhs.beatpos;
}