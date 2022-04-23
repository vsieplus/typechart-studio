#include <cstring>

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "systems/audiosystem.hpp"
#include "ui/preferences.hpp"
#include "ui/windowsizes.hpp"

static ImGuiWindowFlags preferencesWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
static ImVec2 preferencesWindowSize = ImVec2(1100, 500);

void Preferences::setShowPreferences(bool showPreferences) {
    this->showPreferences = showPreferences;
}

void Preferences::showPreferencesWindow(AudioSystem * audioSystem) {
    if(showPreferences) {
        ImGui::SetNextWindowSize(preferencesWindowSize);
        ImGui::Begin("Preferences", &showPreferences, preferencesWindowFlags);

        ImGui::InputText("Default Input Directory", Preferences::inputDir, 256, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        if(ImGui::Button("Browse...##inputdir")) {
            ImGuiFileDialog::Instance()->OpenDialog("chooseInputDir", "Choose Default Input Directory", nullptr, Preferences::inputDir);
        }

        if(ImGuiFileDialog::Instance()->Display("chooseInputDir", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                strcpy(Preferences::inputDir, ImGuiFileDialog::Instance()->GetFilePathName().c_str());
            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::InputText("Default Save Directory", Preferences::outputDir, 256, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        if(ImGui::Button("Browse...##outputdir")) {
            ImGuiFileDialog::Instance()->OpenDialog("chooseOutputDir", "Choose Default Output Directory", nullptr, Preferences::outputDir);
        }

        if(ImGuiFileDialog::Instance()->Display("chooseOutputDir", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                strcpy(Preferences::outputDir, ImGuiFileDialog::Instance()->GetFilePathName().c_str());
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if(ImGui::SliderFloat("Music volume", &musicVolume, 0.f, 1.f)) {
            audioSystem->setMusicVolume(Preferences::musicVolume);
        }

        if(ImGui::SliderFloat("Sound volume", &soundVolume, 0.f, 1.f)) {
            audioSystem->setSoundVolume(Preferences::soundVolume);
        }

        ImGui::End();
    }
}

const char * Preferences::getInputDir() const {
    return inputDir;
}