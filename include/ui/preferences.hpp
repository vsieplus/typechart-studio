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

        bool isNotesoundEnabled() const;
        bool isDarkTheme () const;
        void setDarkTheme(bool dark);

        bool getCopyArtAndMusic() const;

        std::string getInputDir() const;
        std::string getSaveDir() const;

        const std::list<std::string> & getMostRecentFiles() const;
    private:
        Preferences() {}

        float musicVolume = 1.f;
        float soundVolume = 1.f;

        bool copyArtAndMusic = true;
        bool showPreferences = false;

        bool enableNotesound = true;

        bool darkTheme = true;

        std::string inputDir;
        std::string saveDir;

        std::list<std::string> mostRecentFilepaths;
};


#endif // PREFERENCES_HPP