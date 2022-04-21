#ifndef PREFERENCES_HPP
#define PREFERENCES_HPP

#include <string>

struct Preferences {
    Preferences();

    static void setShowPreferences();
    static void showPreferencesWindow();

    inline static char outputDir[256];
};


#endif // PREFERENCES_HPP