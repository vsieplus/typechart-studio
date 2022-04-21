#include "imgui.h"

#include "ui/editwindow.hpp"
#include "ui/menubar.hpp"
#include "ui/preferences.hpp"

void showMenuBar(ImFont * menuFont) {
    // styling
    if(menuFont)
        ImGui::PushFont(menuFont);

    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            showFileMenu();
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit")) {
            showEditMenu();
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Options")) {
            showOptionMenu();
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help")) {
            showHelpMenu();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if(menuFont)
        ImGui::PopFont();
}

void showFileMenu() {
    if(ImGui::MenuItem("New", "Ctrl+N")) {
        startNewEditWindow();
    }

    if(ImGui::MenuItem("Open", "Ctrl+O")) {

    }

    if(ImGui::BeginMenu("Recent")) {
        ImGui::MenuItem("Test1.type");
        ImGui::EndMenu();
    }

    if(ImGui::MenuItem("Save", "Ctrl+S")) {

    }

    if(ImGui::MenuItem("Save As..")) {

    }
}

void showEditMenu() {
    if(ImGui::MenuItem("Undo", "Ctrl+Z")) {

    }

    if(ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {

    }

    ImGui::Separator();
    
    if(ImGui::MenuItem("Cut", "Ctrl+X")) {

    }

    if(ImGui::MenuItem("Copy", "Ctrl+C")) {

    }

    if(ImGui::MenuItem("Paste", "Ctrl+V")) {

    }
}

void showOptionMenu() {
    if(ImGui::MenuItem("Preferences", "Ctrl+P")) {
        setShowPreferences();
    }

    if(ImGui::BeginMenu("Theme")) {
        if(ImGui::MenuItem("Dark")) {
            ImGui::StyleColorsDark();
        }

        if(ImGui::MenuItem("Light")) {
            ImGui::StyleColorsLight();
        }

        if(ImGui::MenuItem("Classic")) {
            ImGui::StyleColorsClassic();
        }

        ImGui::EndMenu();
    }
}

void showHelpMenu() {
    if(ImGui::MenuItem("Tutorial")) {
        // open link
    }
}