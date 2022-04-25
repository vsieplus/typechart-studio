#include "config/songposition.hpp"


void SongPosition::start() {
    songStart = SDL_GetPerformanceCounter();
    currentSection = 0;
    started = true;
}

void SongPosition::stop() {
    absTime = 0.f;
    absBeat = 0.f;
    started = false;

    currentSection = 0;
    prevSectionBeats = 0;
    prevSectionTime = 0;
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
    auto currSpb = 60.f / timeinfo.at(currentSection).bpm;

    absBeat = prevSectionBeats + ((absTime - prevSectionTime) / currSpb);
}

void SongPosition::updateSection() {
    if(currentSection < timeinfo.size() - 1) {
        int nextSection = currentSection + 1;
        if(absTime + (offsetMS / 1000.f) >= timeinfo.at(nextSection).absTimeStart) {
            currentSection = nextSection;

            prevSectionBeats = timeinfo.at(currentSection).absBeatStart;
            prevSectionTime = timeinfo.at(currentSection).absTimeStart;
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
    songStart = songStart - (timeDiff * SDL_GetPerformanceFrequency());

    unsigned int i = 0;
    for(auto const & tInfo : timeinfo) {
        if(absTime >= tInfo.absTimeStart) {
            currentSection = i;

            prevSectionBeats = tInfo.absBeatStart;
            prevSectionTime = tInfo.absTimeStart;

            break;
        }

        i++;
    }
}

void SongPosition::setSongBeatPosition(float absBeat) {
    // calculate absTime from absBeat
    float absTime = 0.f;
    float prevAbsBeatStart = 0.f;
    float prevSpb = 0.f;
    for(auto const & tInfo : timeinfo) {
        if(absBeat >= tInfo.absBeatStart) {
            absTime += prevSpb * (tInfo.absBeatStart - prevAbsBeatStart);

            prevAbsBeatStart = tInfo.absBeatStart;
            prevSpb = (60.f / tInfo.bpm);
        } else {
            absTime += prevSpb * (absBeat - prevAbsBeatStart);
            break;
        }
    }

    this->absBeat = absBeat;
    setSongTimePosition(absTime);
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