#include <filesystem>
namespace fs = std::filesystem;

#include "imgui.h"

#include "versionconfig.h"

#include "ui/editwindow.hpp"
#include "ui/menubar.hpp"
#include "ui/preferences.hpp"

void showMenuBar(ImFont * menuFont, SDL_Renderer * renderer, AudioSystem * audioSystem) {
    // styling
    if(menuFont)
        ImGui::PushFont(menuFont);

    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            showFileMenu(renderer, audioSystem);
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

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x *.95);
        ImGui::BeginChild("versiontext", ImVec2(0, 0), false);
        ImGui::Text("v%d.%d.%d", TYPECHART_STUDIO_VERSION_MAJOR, TYPECHART_STUDIO_VERSION_MINOR, TYPECHART_STUDIO_VERSION_PATCH);
        ImGui::EndChild();

        ImGui::EndMainMenuBar();
    }

    if(menuFont)
        ImGui::PopFont();
}

void showFileMenu(SDL_Renderer * renderer, AudioSystem * audioSystem) {
    if(ImGui::MenuItem("New", "Ctrl+N")) {
        startNewEditWindow();
    }

    if(ImGui::MenuItem("Open", "Ctrl+O")) {
        startOpenChart();
    }

    if(ImGui::BeginMenu("Recent")) {
        for(auto & recentPath : Preferences::Instance().getMostRecentFiles()) {
            auto labelName = fs::path(recentPath.c_str()).parent_path().stem().string() + "/" + fs::path(recentPath.c_str()).filename().string();
            if(ImGui::MenuItem(labelName.c_str())) {
                loadEditWindow(renderer, audioSystem, recentPath);
            }
        }
        ImGui::EndMenu();
    }

    if(ImGui::MenuItem("Save", "Ctrl+S")) {
        startSaveCurrentChart();
    }

    if(ImGui::MenuItem("Save As...")) {
        startSaveCurrentChart(true);
    }
}

void showEditMenu() {
    if(ImGui::MenuItem("Undo", "Ctrl+Z")) {
        setUndo();
    }

    if(ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {
        setRedo();
    }

    ImGui::Separator();
    
    if(ImGui::MenuItem("Cut", "Ctrl+X")) {
        setCut();
    }

    if(ImGui::MenuItem("Copy", "Ctrl+C")) {
        setCopy();
    }

    if(ImGui::MenuItem("Paste", "Ctrl+V")) {
        setPaste();
    }
}

void showOptionMenu() {
    if(ImGui::MenuItem("Preferences", "Ctrl+P")) {
        Preferences::Instance().setShowPreferences(true);
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