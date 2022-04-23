#ifndef PREFERENCES_HPP
#define PREFERENCES_HPP

#include <string>

class AudioSystem;

class Preferences {
    public:
        // singleton
        static Preferences & Instance() {
            static Preferences p;
            return p;
        }

        Preferences(Preferences const&) = delete;
        void operator=(Preferences const&) = delete;

        void loadFromFile(std::string preferencesPath);
        void saveToFile(std::string preferencesPath);

        void setShowPreferences(bool showPreferences);
        void showPreferencesWindow(AudioSystem * audioSystem);

        const char * getInputDir() const;
    private:
        Preferences() {}

        char inputDir[512] = ".";
        char outputDir[512] = ".";

        float musicVolume = 1.f;
        float soundVolume = 1.f;

        bool showPreferences;
};


#endif // PREFERENCES_HPP