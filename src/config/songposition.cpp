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
}

void SongPosition::update() {
    if(!paused && started) {
        auto now = SDL_GetPerformanceCounter();
        absTime = ((double)(now - songStart)) / SDL_GetPerformanceFrequency();
    }
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