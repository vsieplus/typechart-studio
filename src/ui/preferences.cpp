#include <algorithm>
#include <cstring>

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "systems/audiosystem.hpp"
#include "ui/preferences.hpp"
#include "ui/windowsizes.hpp"

static ImGuiWindowFlags preferencesWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
static ImVec2 preferencesWindowSize = ImVec2(1100, 500);

const static int MAX_MOST_RECENT = 10;

void Preferences::showPreferencesWindow(AudioSystem * audioSystem) {
    if(showPreferences) {
        ImGui::SetNextWindowSize(preferencesWindowSize);
        ImGui::Begin("Preferences", &showPreferences, preferencesWindowFlags);

        char inputDirStr[256];
        strncpy(inputDirStr, inputDir.c_str(), 256);

        if(ImGui::InputText("Default Input Directory", inputDirStr, 256)) {
            inputDir = inputDirStr;
        }
        // ImGui::SameLine();
        // if(ImGui::Button("Browse...##inputdir")) {
        //     ImGuiFileDialog::Instance()->OpenDialog("chooseInputDir", "Choose Default Input Directory", nullptr, Preferences::inputDir);
        // }

        // if(ImGuiFileDialog::Instance()->Display("chooseInputDir", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        //     if (ImGuiFileDialog::Instance()->IsOk()) {
        //         inputDir = ImGuiFileDialog::Instance()->GetFilePathName();
        //     }

        //     ImGuiFileDialog::Instance()->Close();
        // }

        char saveDirStr[256];
        strncpy(saveDirStr, saveDir.c_str(), 256);

        if(ImGui::InputText("Default Save Directory", saveDirStr, 256)) {
            saveDir = saveDirStr;
        }
        // ImGui::SameLine();
        // if(ImGui::Button("Browse...##outputdir")) {
        //     ImGuiFileDialog::Instance()->OpenDialog("chooseOutputDir", "Choose Default Output Directory", nullptr, Preferences::saveDir.c_str());
        // }

        // if(ImGuiFileDialog::Instance()->Display("chooseOutputDir", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        //     if (ImGuiFileDialog::Instance()->IsOk()) {
        //         saveDir = ImGuiFileDialog::Instance()->GetFilePathName();
        //     }

        //     ImGuiFileDialog::Instance()->Close();
        // }

        if(ImGui::SliderFloat("Music volume", &musicVolume, 0.f, 1.f)) {
            audioSystem->setMusicVolume(Preferences::musicVolume);
        }

        if(ImGui::SliderFloat("Sound volume", &soundVolume, 0.f, 1.f)) {
            audioSystem->setSoundVolume(Preferences::soundVolume);
        }

        ImGui::Checkbox("Enable Notesounds", &enableNotesound);
        ImGui::Checkbox("Copy Art and Music when Saving", &copyArtAndMusic);

        ImGui::End();
    }
}

void Preferences::loadFromFile(std::string preferencesPath) {
    ordered_json preferencesJSON;

    try {
        std::ifstream in(preferencesPath);
        in >> preferencesJSON;

        if(preferencesJSON.contains("musicVolume")) {
            musicVolume = preferencesJSON["musicVolume"];
        }
        
        if(preferencesJSON.contains("soundVolume")) {
            soundVolume = preferencesJSON["soundVolume"];
        }

        if(preferencesJSON.contains("enableNotesound")) {
            enableNotesound = preferencesJSON["enableNotesound"];
        }

        if(preferencesJSON.contains("copyAssetsWhenSaving")) {
            copyArtAndMusic = preferencesJSON["copyAssetsWhenSaving"];
        }

        if(preferencesJSON.contains("theme")) {
            darkTheme = preferencesJSON["theme"] == "dark";
        }

        if(preferencesJSON.contains("defaultInputDir")) {
            inputDir = preferencesJSON["defaultInputDir"];
        }
        
        if(preferencesJSON.contains("defaultSaveDir")) {
            saveDir = preferencesJSON["defaultSaveDir"];
        }

        if(preferencesJSON.contains("recentFiles")) {
            mostRecentFilepaths = preferencesJSON["recentFiles"];
        }
    } catch(...) {
        // no preferences file exists, or field(s) are corrupt
    }

    if(darkTheme) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }
}

void Preferences::saveToFile(std::string preferencesPath) {
    // write settings to JSON object
    ordered_json preferencesJSON;

    preferencesJSON["musicVolume"] = musicVolume;
    preferencesJSON["soundVolume"] = soundVolume;
    preferencesJSON["enableNotesound"] = enableNotesound;
    preferencesJSON["copyAssetsWhenSaving"] = copyArtAndMusic;

    preferencesJSON["theme"] = darkTheme ? "dark" : "light";

    preferencesJSON["defaultInputDir"] = inputDir;
    preferencesJSON["defaultSaveDir"] = saveDir;

    preferencesJSON["recentFiles"] = mostRecentFilepaths;

    // write to file
    std::ofstream file(preferencesPath.c_str());
    file << std::setw(4) << preferencesJSON << std::endl;
}

void Preferences::setShowPreferences(bool showPreferences) {
    this->showPreferences = showPreferences;
}

bool Preferences::isNotesoundEnabled() const {
    return enableNotesound;
}

bool Preferences::isDarkTheme() const {
    return darkTheme;
}

void Preferences::setDarkTheme(bool dark) {
    this->darkTheme = dark;
}

bool Preferences::getCopyArtAndMusic() const {
    return copyArtAndMusic;
}

std::string Preferences::getInputDir() const {
    return inputDir;
}

std::string Preferences::getSaveDir() const {
    return saveDir;
}

void Preferences::addMostRecentFile(std::string path) {
    if(mostRecentFilepaths.size() == MAX_MOST_RECENT) {
        mostRecentFilepaths.pop_back();
    }
    
    for(auto iter = mostRecentFilepaths.begin(); iter != mostRecentFilepaths.end(); iter++) {
        if(path == *iter) {
            mostRecentFilepaths.erase(iter);
            break;
        }
    }

    mostRecentFilepaths.push_front(path);
}

const std::list<std::string> & Preferences::getMostRecentFiles() const {
    return mostRecentFilepaths;
}
