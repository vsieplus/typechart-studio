#ifndef WINDOWSIZES_HPP
#define WINDOWSIZES_HPP

#include "imgui.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// default window sizes
inline ImVec2 maxFDSize = ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT);
inline ImVec2 minFDSize = ImVec2(SCREEN_WIDTH*3.f/4, SCREEN_HEIGHT*3.f/4);
inline ImVec2 newEditWindowSize = ImVec2(500, 450);

#endif