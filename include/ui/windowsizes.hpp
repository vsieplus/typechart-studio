#ifndef WINDOWSIZES_HPP
#define WINDOWSIZES_HPP

#include "imgui.h"

namespace constants {

constexpr int SCREEN_WIDTH { 1280 };
constexpr int SCREEN_HEIGHT { 720 };

// default window sizes
constexpr ImVec2 maxFDSize { SCREEN_WIDTH*3.f/4, SCREEN_HEIGHT*3.f/4 };
constexpr ImVec2 minFDSize { SCREEN_WIDTH/2.f, SCREEN_HEIGHT/2.f };
constexpr ImVec2 newEditWindowSize { 600, 570 };
constexpr ImVec2 editWindowSize { 640, 480 };

}

#endif
