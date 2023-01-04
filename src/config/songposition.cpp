#include <algorithm>
#include <float.h>

#include "config/songposition.hpp"

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
        int nextSection = currentSection + 1;

        float timeUntilNextSection = timeinfo.at(nextSection).absTimeStart - absTime;
        float bpmInterplationProgress = 1 - (timeUntilNextSection / ((timeinfo.at(nextSection).interpolateBeatDuration) * currSpb));
        bpmInterplationProgress = std::min(1.f, bpmInterplationProgress);
        bpmInterplationProgress = std::max(0.f, bpmInterplationProgress);

        float currBpm = bpmInterpolateStart + bpmInterplationProgress * (bpmInterpolateEnd - bpmInterpolateStart);
        currSpb = 60.0 / currBpm;
        prevSectionTime = absTime;
        prevSectionBeats = absBeat;
    }
}

void SongPosition::updateBeatPos() {
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

            currSpb = 60.0 / timeinfo.at(currentSection).bpm;

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
            currSkipStartTimePosition = ((currSkipBegin - songStart) / (double)SDL_GetPerformanceFrequency()) - (offsetMS / 1000.0);

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

void SongPosition::pause() {
    pauseCounter = SDL_GetPerformanceCounter();
    paused = true;
}

void SongPosition::unpause() {
    now = SDL_GetPerformanceCounter();
    songStart += (now - pauseCounter);

    paused = false;
}