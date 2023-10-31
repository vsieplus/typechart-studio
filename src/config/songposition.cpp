#include <algorithm>
#include <cmath>
#include <float.h>

#include "config/songposition.hpp"

#include "imgui.h"

void SongPosition::start() {
    songStart = SDL_GetPerformanceCounter();
    currentSection = 0;
    prevSectionBeats = 0;
    prevSectionTime = 0;

    currSpb = 60.0 / timeinfo.at(currentSection).bpm;

    started = true;
    paused = false;

    bpmInterpolating = false;
    beatSkipped = false;
    beatSkiptimePassed = false;

    currentSkip = 0;
    currSkipDuration = 0.0;
    currSkipStartTimePosition = 0.0;
    currSkipTime = 0.0;
    currSkipSpb = 0.0;
}

void SongPosition::stop() {
    absTime = 0.0;
    absBeat = 0.0;

    started = false;
    paused = false;
}

void SongPosition::update() {
    if(!paused && started) {
        now = SDL_GetPerformanceCounter();
        absTime = (((double)(now - songStart)) / SDL_GetPerformanceFrequency()) - (offsetMS / 1000.0);

        updateBeatPos();

        // updateBPM(); disable for now until figuring out a solution
        
        updateSection();
        updateSkips();

        //printf("Songpos: %.4f, %.4f\n", absTime, absBeat);
    }
}

void SongPosition::updateBPM() {
    if(bpmInterpolating && currentSection < timeinfo.size() - 1) {
        unsigned int nextSection { currentSection + 1 };

        double timeUntilNextSection { timeinfo.at(nextSection).absTimeStart - absTime };
        double bpmInterplationProgress { 1.0 - (timeUntilNextSection / ((timeinfo.at(nextSection).interpolateBeatDuration) * currSpb)) };
        bpmInterplationProgress = std::min(1.0, bpmInterplationProgress);
        bpmInterplationProgress = std::max(0.0, bpmInterplationProgress);

        double currBpm { bpmInterpolateStart + bpmInterplationProgress * (bpmInterpolateEnd - bpmInterpolateStart) };
        currSpb = 60.0 / currBpm;
        prevSectionTime = absTime;
        prevSectionBeats = absBeat;
    }
}

void SongPosition::updateBeatPos() {
    if(beatSkipped) {
        auto timeSinceSkip { static_cast<double>(now - currSkipBegin) / static_cast<double>(SDL_GetPerformanceFrequency()) };

        // check if full skip passed, or if still 'skipping'
        if(timeSinceSkip > currSkipDuration) {
            beatSkipped = false;
            beatSkiptimePassed = false;
        } else if(!beatSkiptimePassed) {
            if(timeSinceSkip < currSkipTime) {
                absBeat = prevSectionBeats + ((currSkipStartTimePosition - prevSectionTime) / currSpb) +
                    (timeSinceSkip / currSkipSpb);
            } else {
                absBeat = prevSectionBeats + ((currSkipStartTimePosition - prevSectionTime + currSkipDuration) / currSpb);
                beatSkiptimePassed = true;
            }
        }
    } else {
        absBeat = prevSectionBeats + ((absTime - prevSectionTime) / currSpb);
    }
}

void SongPosition::updateSection() {
    if(currentSection < timeinfo.size() - 1) {
        unsigned int nextSection { currentSection + 1 };
        const auto & nextTimeinfo { timeinfo.at(nextSection) };

        if(!bpmInterpolating && nextTimeinfo.interpolateBeatDuration > FLT_EPSILON) {
            if(absTime >= nextTimeinfo.absTimeStart - (nextTimeinfo.interpolateBeatDuration * currSpb)) {
                bpmInterpolating = true;
                bpmInterpolateStart = timeinfo.at(currentSection).bpm;
                bpmInterpolateEnd = timeinfo.at(nextSection).bpm;
            }
        } else if(absTime >= nextTimeinfo.absTimeStart) {
            currentSection = nextSection;

            prevSectionBeats = timeinfo.at(currentSection).absBeatStart;
            prevSectionTime = absTime;

            currSpb = 60.0 / timeinfo.at(currentSection).bpm;

            bpmInterpolating = false;
        }
    }
}

void SongPosition::updateSkips() {
    if(!beatSkipped && currentSkip <= static_cast<int>(skips.size() - 1)) {
        double currSkipBeat { skips.at(currentSkip)->absBeat };

        if(absBeat > currSkipBeat) {
            beatSkipped = true;

            currSkipDuration = skips.at(currentSkip)->beatDuration * currSpb;
            currSkipBegin = SDL_GetPerformanceCounter();
            currSkipStartTimePosition = (static_cast<double>(currSkipBegin - songStart) / static_cast<double>(SDL_GetPerformanceFrequency())) - (offsetMS / 1000.0);

            currSkipTime = skips.at(currentSkip)->skipTime;
            currSkipSpb = currSpb * (currSkipTime / currSkipDuration);
            
            beatSkiptimePassed = false;

            currentSkip++;
        }
    }
}

bool skipCompare(std::shared_ptr<Skip> a, std::shared_ptr<Skip> b) {
    return a->absBeat < b->absBeat; 
}

void SongPosition::addSkip(std::shared_ptr<Skip> skip) {
    skips.push_back(skip);
    std::sort(skips.begin(), skips.end(), skipCompare);
    resetCurrskip();
}

void SongPosition::removeSkip(double skipBeat) {
    for(auto skipIter = skips.begin(); skipIter != skips.end();) {
        auto currSkip = *skipIter;
        if(currSkip->absBeat <= skipBeat && skipBeat <= currSkip->absBeat + currSkip->beatDuration) {
            skipIter = skips.erase(skipIter);
            resetCurrskip();
            break;
        } else {
            skipIter++;
        }
    }
}

void SongPosition::resetCurrskip() {
    currentSkip = static_cast<int>(skips.size());
    for(unsigned int i = 0; i < skips.size(); i++) {
        if(absBeat <= skips.at(i)->absBeat) {
            currentSkip = i;
            break;
        }
    }
}

bool SongPosition::addSection(int newBeatsPerMeasure, double newBPM, double newInterpolateDuration, BeatPos newBeatpos) {
    Timeinfo * prevSection { nullptr };

    for(auto & section : timeinfo) {
        if(newBeatpos == section.beatpos) {
            ImGui::OpenPopup("Invalid input");
            return false;
        } else if(section.beatpos < newBeatpos || section.beatpos == timeinfo.back().beatpos) {
            prevSection = &section;
        }
    }

    Timeinfo newSection { newBeatpos, prevSection, newBeatsPerMeasure, newBPM, newInterpolateDuration };
    timeinfo.push_back(newSection);

    std::sort(timeinfo.begin(), timeinfo.end());
    prevSection = &(timeinfo.front());

    // update following section(s) time start after adding new section
    for(auto & section : timeinfo) {
        if(*prevSection < section) {
            section.absTimeStart = section.calculateTimeStart(prevSection);
            prevSection = &section;
        }
    }

    setSongBeatPosition(absBeat);

    return true;
}

bool SongPosition::editSection(int origSectionIndex, int newBeatsPerMeasure, double newBPM, double newInterpolateDuration, BeatPos newBeatpos) {
    // remove original section, add new section
    removeSection(origSectionIndex);
    return addSection(newBeatsPerMeasure, newBPM, newInterpolateDuration, newBeatpos);
}

bool SongPosition::removeSection(int sectionIndex) {
    if(sectionIndex < 0 || sectionIndex >= (int)timeinfo.size()) {
        return false;
    }

    auto currSectionBeatpos = timeinfo.at(sectionIndex).beatpos;

    Timeinfo * prevSection = nullptr;

    // update following section(s) time start after adding new section
    for(auto iter = timeinfo.begin(); iter != timeinfo.end(); iter++) {
        auto & section = *iter;

        if(section.beatpos == currSectionBeatpos) {
            iter = timeinfo.erase(iter);
            if(iter != timeinfo.begin())
                iter--;

            prevSection = &(*iter);
        } else if(currSectionBeatpos < section.beatpos) {
            section.absTimeStart = section.calculateTimeStart(prevSection);
            prevSection = &section;
        }
    }

    setSongBeatPosition(absBeat);

    return true;
}

void SongPosition::setSongTimePosition(double absTime) {
    // absTime - thisabstime = [((now - songstart_t) / sdlgpf)] - [((now - songstart) / sdlgpf)]
    // timeDiff = [(now - songstart_t - (now - songstart)) / sdlgpf]
    // timeDiff = [(-songstart_t + songstart) / sdlgpf]
    // timeDiff * sdlgpf = -songstart_t + songstart
    // songstart_t = songstart - (timeDiff * sdlgpf)
    double timeDiff = absTime - this->absTime;
    double counterDiff = (timeDiff * SDL_GetPerformanceFrequency());
    songStart -= counterDiff;

    //printf("absTime: %.8f, absTime_t: %.8f\n", this->absTime, absTime);
    //printf("timeDiff: %.8f, Counter diff: %.4f\n", timeDiff, counterDiff);

    this->absTime = absTime;

    int i = 0;
    for(auto const & tInfo : timeinfo) {
        if(absTime < tInfo.absTimeStart) {
            break;
        }

        i++;
    }

    currentSection = std::max(0, i - 1);
    prevSectionBeats = timeinfo.at(currentSection).absBeatStart;
    prevSectionTime = timeinfo.at(currentSection).absTimeStart;

    currSpb = 60.0 / timeinfo.at(currentSection).bpm;

    resetCurrskip();
}

void SongPosition::setSongBeatPosition(double absBeat) {
    if(!timeinfo.empty()) {
        // calculate absTime from absBeat
        double absBeatTime = 0;
        double prevAbsBeatStart = 0.0;
        double prevSpb = 60.0 / timeinfo.front().bpm;

        for(auto const & tInfo : timeinfo) {
            if(absBeat >= tInfo.absBeatStart) {
                absBeatTime += prevSpb * (tInfo.absBeatStart - prevAbsBeatStart);

                prevAbsBeatStart = tInfo.absBeatStart;
                prevSpb = (60.0 / tInfo.bpm);
            } else {
                break;
            }
        }

        // add the remainder
        absBeatTime += prevSpb * (absBeat - prevAbsBeatStart);
        this->absBeat = absBeat;
        setSongTimePosition(absBeatTime);

        //printf("Setting song beat pos to %.8f, %.4f\n", absBeat, absBeatTime);
    }
}

double SongPosition::calculateAbsBeat(BeatPos beatpos) {
    double absBeat { 0.0 };
    int prevSectionBeatsPerMeasure { 4 };
    BeatPos prevSectionBeatpos { 0, 1, 0 };

    for(const auto & section: timeinfo) {
        if(beatpos < section.beatpos || section.absBeatStart == timeinfo.back().absBeatStart) {
            double prevSectionMeasure = prevSectionBeatpos.measure + (prevSectionBeatpos.split / static_cast<double>(prevSectionBeatpos.measureSplit));
            double currSectionMeasure = beatpos.measure + (beatpos.split / static_cast<double>(beatpos.measureSplit));

            double currSectionBeats = (currSectionMeasure - prevSectionMeasure) * prevSectionBeatsPerMeasure;
            absBeat += currSectionBeats;
            break;
        } else {
            absBeat = section.absBeatStart;
            prevSectionBeatpos = section.beatpos;
            prevSectionBeatsPerMeasure = section.beatsPerMeasure;
        }
    }

    return absBeat;
}

void SongPosition::pause() {
    pauseCounter = SDL_GetPerformanceCounter();
    paused = true;
}

void SongPosition::unpause() {
    now = SDL_GetPerformanceCounter();
    songStart += (now - pauseCounter);

    paused = false;
}