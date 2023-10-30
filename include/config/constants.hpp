#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <array>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace constants {
    // song metadata
    const std::string SONGINFO_FILENAME = "songinfo.json";

    const std::string TITLE_KEY = "title";
    const std::string ARTIST_KEY = "artist";
    const std::string GENRE_KEY = "genre";
    const std::string COVERART_KEY = "coverart";
    const std::string MUSIC_KEY = "music";
    const std::string BPMTEXT_KEY = "bpmtext";
    const std::string OFFSET_KEY = "offsetMS";
    const std::string MUSIC_PREVIEW_START_KEY = "musicPreviewStart";
    const std::string MUSIC_PREVIEW_STOP_KEY = "musicPreviewStop";

    // chart metadata
    const std::string TYPIST_KEY = "typist";
    const std::string KEYBOARD_KEY = "keyboard";
    const std::string DIFFICULTY_KEY = "difficulty";
    const std::string LEVEL_KEY = "level";

    const std::string TYPIST_VALUE_DEFAULT = "Unknown";
    const std::string KEYBOARD_VALUE_DEFAULT = "QWERTY";
    const std::string DIFFICULTY_VALUE_DEFAULT = "Easy";
    const int LEVEL_VALUE_DEFAULT = 1;
    const int OFFSET_VALUE_DEFAULT = 0;

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

    const std::string ADD_ITEM_POPUP_PREFIX { "add_item_" };

    const double BPM_VALUE_DEFAULT = 0.0;
    const double INTERPOLATE_BEAT_DURATION_VALUE_DEFAULT = 0.0;
    const double DURATION_VALUE_DEFAULT = 0.0;
    const double SKIPTIME_VALUE_DEFAULT = 0.0;

    constexpr double ZOOM_STEP = 0.25;

    constexpr int BEATS_PER_MEASURE_VALUE_DEFAULT = 4;
    constexpr int NOTE_TYPE_VALUE_DEFAULT = 1;

    // bg color
    constexpr uint8_t BG_R = 20;
    constexpr uint8_t BG_G = 20;
    constexpr uint8_t BG_B = 20;
    constexpr uint8_t BG_A = 255;

    // edit windows
    const std::string DEFAULT_WINDOW_NAME = "Untitled";

    const std::string saveFileFilter = "(*.type){.type}";
    const std::string songinfoFileFilter = "(*.json){.json}";
    const std::string imageFileFilters = "(*.jpg *.png){.jpg,.png}";
    const std::string musicFileFilters = "(*.flac *.mp3 *.ogg *.wav){.flac,.mp3,.ogg,.wav}";

    constexpr int MENU_FONT_SIZE = 28;

    const std::string PREFERENCES_PATH = "preferences.json";

    const fs::path FONTS_DIR = fs::path("fonts");
    const fs::path MENU_FONT_PATH = FONTS_DIR / fs::path("NotoSans-Regular.ttf");
    const fs::path ICON_FONT_PATH = FONTS_DIR / fs::path("fa-solid-900.ttf");

    const fs::path IMAGES_DIR = fs::path("images");
    const fs::path WINDOW_ICON_PATH = IMAGES_DIR / fs::path("windowIcon.png");

    const fs::path KEYPRESS_SOUND_PATH = fs::path("sounds") / fs::path("keypress.wav");

    const std::unordered_map<int, std::string> ID_TO_KEYBOARDLAYOUT {
        { 0, "QWERTY" },
        { 1, "DVORAK" },
        { 2, "AZERTY" },
        { 3, "COLEMAK" }
    };

    const std::unordered_map<std::string, int> KEYBOARDLAYOUT_TO_ID {
        { "QWERTY", 0 },
        { "DVORAK", 1 },
        { "AZERTY", 2 },
        { "COLEMAK", 3 }
    };

    const std::unordered_map<int, std::string> ID_TO_DIFFICULTY {
        { 0, "easy" },
        { 1, "normal" },
        { 2, "hard" },
        { 3, "expert" },
        { 4, "unknown" }
    };

    const std::unordered_map<std::string, int> DIFFICULTY_TO_ID {
        { "easy", 0 },
        { "normal", 1 },
        { "hard", 2 },
        { "expert", 3 },
        { "unknown", 4 }
    };

    const std::unordered_map<int, std::string> FUNCTION_KEY_COMBO_ITEMS {
        { 0, "_" },
    };

    const std::array<std::string, 5> SEQUENCER_ITEM_TYPES { 
        "Lane 1 [Top]",
        "Lane 2 [Middle]",
        "Lane 3 [Bottom]",
        "Stops",
        "Skips"
    };
} // namespace constants

#endif // CONSTANTS_HPP
