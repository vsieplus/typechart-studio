#include "ui/preferences.hpp"

#include "imgui.h"

static ImGuiWindowFlags preferencesWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
static ImVec2 preferencesWindowSize = ImVec2(500, 500);

static bool showPreferences = false;

void setShowPreferences() {
    showPreferences = true;
}

void showPreferencesWindow() {
    if(showPreferences) {
        ImGui::SetNextWindowSize(preferencesWindowSize);
        ImGui::Begin("Preferences", &showPreferences, preferencesWindowFlags);

        ImGui::End();
    }
}