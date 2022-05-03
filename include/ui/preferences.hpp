#ifndef PREFERENCES_HPP
#define PREFERENCES_HPP

#include <list>
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

        void addMostRecentFile(std::string path);

        bool getCopyArtAndMusic() const;

        const char * getInputDir() const;
        const char * getSaveDir() const;

        const std::list<std::string> & getMostRecentFiles() const;
    private:
        Preferences() {}

        char inputDir[512] = ".";
        char outputDir[512] = ".";

        float musicVolume = 1.f;
        float soundVolume = 1.f;

        bool copyArtAndMusic = true;

        bool showPreferences;

        std::list<std::string> mostRecentFiles;
};


#endif // PREFERENCES_HPP