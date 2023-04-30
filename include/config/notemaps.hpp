#ifndef NOTEMAPS_HPP
#define NOTEMAPS_HPP

#include <string>
#include <unordered_map>

#include "IconsFontAwesome6.h"

namespace notemaps {
    const std::unordered_map<std::string, std::string> FUNCTION_KEY_TO_STR = {
        { "L" ICON_FA_ARROW_UP, "Left Shift" },
        { "R" ICON_FA_ARROW_UP, "Right Shift" },
        { ICON_FA_ARROW_UP, "CapsLock" },
        { ICON_FA_ARROW_LEFT_LONG, "Return" },
        { "_", "Space" },
    };

    const std::unordered_map<std::string, std::string> STR_TO_FUNCTION_KEY = {
        { "Left Shift", "L" ICON_FA_ARROW_UP },
        { "Right Shift", "R" ICON_FA_ARROW_UP },
        { "CapsLock", ICON_FA_ARROW_UP },
        { "Return", ICON_FA_ARROW_LEFT_LONG },
        { "Space", "_" },
    };
};

#endif // NOTEMAPS_HPP