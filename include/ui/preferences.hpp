#ifndef PREFERENCES_HPP
#define PREFERENCES_HPP

#include <string>

class AudioSystem;

struct Preferences {
    Preferences();

    static void setShowPreferences();
    static void showPreferencesWindow(AudioSystem * audioSystem);

    inline static char inputDir[256] = ".";
    inline static char outputDir[256] = ".";

    inline static float musicVolume = 1.f;
    inline static float soundVolume = 1.f;
};


#endif // PREFERENCES_HPP