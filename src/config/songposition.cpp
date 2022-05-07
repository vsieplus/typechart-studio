#include "config/songposition.hpp"


void SongPosition::start() {
    songStart = SDL_GetPerformanceCounter();
    currentSection = 0;
    prevSectionBeats = 0;
    prevSectionTime = 0;

    currSpb = 60.f / timeinfo.at(currentSection).bpm;

    started = true;
    paused = false;
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
        absTime = (((double)(now - songStart)) / SDL_GetPerformanceFrequency()) - (offsetMS / 1000.f) ;

        updateBeatPos();
        updateSection();
    }
}

void SongPosition::updateBeatPos() {
    absBeat = prevSectionBeats + ((absTime - prevSectionTime) / currSpb);
}

void SongPosition::updateSection() {
    if(currentSection < timeinfo.size() - 1) {
        int nextSection = currentSection + 1;
        if(absTime + (offsetMS / 1000.f) >= timeinfo.at(nextSection).absTimeStart) {
            currentSection = nextSection;

            prevSectionBeats = timeinfo.at(currentSection).absBeatStart;
            prevSectionTime = absTime;

            currSpb = 60.f / timeinfo.at(currentSection).bpm;
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