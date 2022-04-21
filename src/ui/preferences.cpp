#include "ui/preferences.hpp"

#include "imgui.h"
#include "ImGuiFileDialog.h"

static ImGuiWindowFlags preferencesWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
static ImVec2 preferencesWindowSize = ImVec2(500, 500);

static bool showPreferences = false;

Preferences::Preferences() {}

void Preferences::setShowPreferences() {
    showPreferences = true;
}

void Preferences::showPreferencesWindow() {
    if(showPreferences) {
        ImGui::SetNextWindowSize(preferencesWindowSize);
        ImGui::Begin("Preferences", &showPreferences, preferencesWindowFlags);

        ImGui::InputText("Default Output Folder", Preferences::outputDir, 256, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        

        ImGui::End();
    }
}