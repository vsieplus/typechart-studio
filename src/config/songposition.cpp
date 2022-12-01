#include <algorithm>
#include <float.h>

#include "config/songposition.hpp"

void SongPosition::start() {
    songStart = SDL_GetPerformanceCounter();
    currentSection = 0;
    prevSectionBeats = 0;
    prevSectionTime = 0;

    currSpb = 60.f / timeinfo.at(currentSection).bpm;

    started = true;
    paused = false;

    bpmInterpolating = false;
    beatSkipped = false;
    beatSkiptimePassed = false;

    currentSkip = 0;
    currSkipDuration = 0.f;
    currSkipStartTimePosition = 0.f;
    currSkipTime = 0.f;
    currSkipSpb = 0.f;
}

void SongPosition::stop() {
    absTime = 0.f;
    absBeat = 0.f;

    started = false;
    paused = false;
}

void SongPosition::update() {
    if(!paused && started) {
        auto now = SDL_GetPerformanceCounter();
        absTime = (((double)(now - songStart)) / SDL_GetPerformanceFrequency()) - (offsetMS / 1000.f);

        updateBeatPos(now);

        // updateBPM(); disable for now until figuring out a solution
        
        updateSection();
        updateSkips();
    }
}

void SongPosition::updateBPM() {
    if(bpmInterpolating && currentSection < timeinfo.size() - 1) {
        int nextSection = currentSection + 1;

        float timeUntilNextSection = timeinfo.at(nextSection).absTimeStart - absTime;
        float bpmInterplationProgress = 1 - (timeUntilNextSection / ((timeinfo.at(nextSection).interpolateBeatDuration) * currSpb));
        bpmInterplationProgress = std::min(1.f, bpmInterplationProgress);
        bpmInterplationProgress = std::max(0.f, bpmInterplationProgress);

        float currBpm = bpmInterpolateStart + bpmInterplationProgress * (bpmInterpolateEnd - bpmInterpolateStart);
        currSpb = 60.f / currBpm;
        prevSectionTime = absTime;
        prevSectionBeats = absBeat;
    }
}

void SongPosition::updateBeatPos(uint64_t now) {
    if(beatSkipped) {
        auto timeSinceSkip = (now - currSkipBegin) / (double)SDL_GetPerformanceFrequency();

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
        int nextSection = currentSection + 1;

        auto & nextTimeinfo = timeinfo.at(nextSection);

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

            currSpb = 60.f / timeinfo.at(currentSection).bpm;

            bpmInterpolating = false;
        }
    }
}

void SongPosition::updateSkips() {
    if(!beatSkipped && currentSkip <= (int)(skips.size() - 1)) {
        float currSkipBeat = skips.at(currentSkip)->absBeat;

        if(absBeat > currSkipBeat) {
            beatSkipped = true;

            currSkipDuration = skips.at(currentSkip)->beatDuration * currSpb;
            currSkipBegin = SDL_GetPerformanceCounter();
            currSkipStartTimePosition = ((currSkipBegin - songStart) / (double)SDL_GetPerformanceFrequency()) - (offsetMS / 1000.f);

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

void SongPosition::removeSkip(float absBeat) {
    for(auto skipIter = skips.begin(); skipIter != skips.end(); skipIter++) {
        if(std::abs((*skipIter)->absBeat - absBeat) < FLT_EPSILON) {
            skips.erase(skipIter);
            resetCurrskip();
            break;
        }
    }
}

void SongPosition::resetCurrskip() {
    currentSkip = skips.size();
    for(unsigned int i = 0; i < skips.size(); i++) {
        if(absBeat <= skips.at(i)->absBeat) {
            currentSkip = i;
            break;
        }
    }
}

void SongPosition::setSongTimePosition(float absTime) {
    float timeDiff = absTime - this->absTime;
    // absTime - thisabstime = ((now - songStart_target) / sdlgpf) - ((now - songstart) / sdlgpf)
    // timeDiff = (now - songstart_t - (now - songstart)) / sdlgpf
    // timeDiff = (songstart - songstart_t) / sdlgpf
    // timeDiff * sdlgpf = songstart - songstart_t
    // songstart_t = songstart - (timeDiff * sdlgpf)
    float counterDiff = (timeDiff * SDL_GetPerformanceFrequency());
    songStart -= counterDiff;

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

    currSpb = 60.f / timeinfo.at(currentSection).bpm;

    resetCurrskip();
}

void SongPosition::setSongBeatPosition(float absBeat) {
    // calculate absTime from absBeat
    float absBeatTime = 0.f;
    float prevAbsBeatStart = 0.f;
    float prevSpb = 60.f / timeinfo.front().bpm;
    for(auto const & tInfo : timeinfo) {
        if(absBeat >= tInfo.absBeatStart) {
            absBeatTime += prevSpb * (tInfo.absBeatStart - prevAbsBeatStart);

            prevAbsBeatStart = tInfo.absBeatStart;
            prevSpb = (60.f / tInfo.bpm);
        }
    }

    // add the remainder
    absBeatTime += prevSpb * (absBeat - prevAbsBeatStart);

    this->absBeat = absBeat;
    setSongTimePosition(absBeatTime);
}

void SongPosition::pause() {
    pauseCounter = SDL_GetPerformanceCounter();
    paused = true;
}

void SongPosition::unpause() {
    auto now = SDL_GetPerformanceCounter();
    songStart += (now - pauseCounter);

    paused = false;
}