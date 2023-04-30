#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

namespace constants {

    // chart metadata
    const std::string TYPIST_KEY = "typist";
    const std::string KEYBOARD_KEY = "keyboard";
    const std::string DIFFICULTY_KEY = "difficulty";
    const std::string LEVEL_KEY = "level";
    const std::string OFFSET_KEY = "offsetMS";

    const std::string TYPIST_VALUE_DEFAULT = "Unknown";
    const std::string KEYBOARD_VALUE_DEFAULT = "QWERTY";
    const std::string DIFFICULTY_VALUE_DEFAULT = "Easy";
    int LEVEL_VALUE_DEFAULT = 1;
    int OFFSET_VALUE_DEFAULT = 0;

    // chart data
    const std::string TIMEINFO_KEY = "timeinfo";
    const std::string POS_KEY = "pos";
    const std::string BPM_KEY = "bpm";
    const std::string INTERPOLATE_BEAT_DURATION_KEY = "interpolateBeatDuration";
    const std::string BEATS_PER_MEASURE_KEY = "beatsPerMeasure";
    
    const std::string STOPS_KEY = "stops";
    const std::string DURATION_KEY = "duration";
    
    const std::string SKIPS_KEY = "skips";
    const std::string SKIPTIME_KEY = "skiptime";

    const std::string NOTES_KEY = "notes";
    const std::string NOTE_TYPE_KEY = "type";
    const std::string NOTE_KEY_KEY = "key";
    const std::string NOTE_KEY_VALUE_DEFAULT = "A";

    const float BPM_VALUE_DEFAULT = 0.f;
    const float INTERPOLATE_BEAT_DURATION_VALUE_DEFAULT = 0.f;
    const float DURATION_VALUE_DEFAULT = 0.f;
    const float SKIPTIME_VALUE_DEFAULT = 0.f;

    const int BEATS_PER_MEASURE_VALUE_DEFAULT = 4;
    const int NOTE_TYPE_VALUE_DEFAULT = 1;
};

#endif // CONSTANTS_HPP
