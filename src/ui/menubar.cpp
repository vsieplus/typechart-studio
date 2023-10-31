#include <filesystem>
namespace fs = std::filesystem;

#include "imgui.h"

#include "versionconfig.h"

#include "ui/editwindowmanager.hpp"

#include "ui/editwindow.hpp"
#include "ui/menubar.hpp"
#include "ui/preferences.hpp"

namespace menubar {

void showMenuBar(ImFont * menuFont, SDL_Renderer * renderer, AudioSystem * audioSystem, EditWindowManager & editWindowManager) {
    // styling
    if(menuFont)
        ImGui::PushFont(menuFont);

    std::string popupID {};

    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            popupID = showFileMenu(renderer, audioSystem, editWindowManager);
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit")) {
            showEditMenu(editWindowManager);
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Options")) {
            showOptionMenu();
            ImGui::EndMenu();
        }

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x *.95f);
        ImGui::BeginChild("versiontext", ImVec2(0, 0), false);
        ImGui::Text("v%d.%d.%d", TYPECHART_STUDIO_VERSION_MAJOR, TYPECHART_STUDIO_VERSION_MINOR, TYPECHART_STUDIO_VERSION_PATCH);
        ImGui::EndChild();

        ImGui::EndMainMenuBar();
    }

    if(popupID.size() > 0) {
        ImGui::OpenPopup(popupID.c_str());
    }

    if(menuFont)
        ImGui::PopFont();
}

std::string showFileMenu(SDL_Renderer * renderer, AudioSystem * audioSystem, EditWindowManager & editWindowManager) {
    std::string popupID {};

    if(ImGui::MenuItem("New", "Ctrl+N")) {
        editWindowManager.startNewEditWindow();
    }

    if(ImGui::MenuItem("Open", "Ctrl+O")) {
        editWindowManager.startOpenChart();
    }

    if(ImGui::BeginMenu("Recent")) {
        for(const auto & recentPath : Preferences::Instance().getMostRecentFiles()) {
            fs::path fsRecentPath(recentPath.c_str());
            auto labelName = fsRecentPath.parent_path().stem().string() + "/" + fsRecentPath.filename().string();
            if(ImGui::MenuItem(labelName.c_str())) {
                popupID = editWindowManager.loadEditWindow(renderer, audioSystem, fsRecentPath);
            }
        }
        ImGui::EndMenu();
    }

    if(ImGui::MenuItem("Save", "Ctrl+S")) {
        editWindowManager.startSaveCurrentChart();
    }

    if(ImGui::MenuItem("Save As...")) {
        editWindowManager.startSaveCurrentChart(true);
    }

    return popupID;
}

void showEditMenu(EditWindowManager & editWindowManager) {
    if(ImGui::MenuItem("Undo", "Ctrl+Z")) {
        editWindowManager.setUndo(true);
    }

    if(ImGui::MenuItem("Redo", "Ctrl+Y")) {
        editWindowManager.setRedo(true);
    }

    ImGui::Separator();
    
    if(ImGui::MenuItem("Cut", "Ctrl+X")) {
        editWindowManager.setCut(true);
    }

    if(ImGui::MenuItem("Copy", "Ctrl+C")) {
        editWindowManager.setCopy(true);
    }

    if(ImGui::MenuItem("Paste", "Ctrl+V")) {
        editWindowManager.setPaste(true);
    }

    ImGui::Separator();

    if(ImGui::MenuItem("Flip", "F")) {
        editWindowManager.setFlip(true);
    }
}

void showOptionMenu() {
    if(ImGui::MenuItem("Preferences", "Ctrl+P")) {
        Preferences::Instance().setShowPreferences(true);
    }

    if(ImGui::BeginMenu("Theme")) {
        if(ImGui::MenuItem("Dark")) {
            ImGui::StyleColorsDark();
            Preferences::Instance().setDarkTheme(true);
        }

        if(ImGui::MenuItem("Light")) {
            ImGui::StyleColorsLight();
            Preferences::Instance().setDarkTheme(false);
        }

        ImGui::EndMenu();
    }
}

} // namespace menubar
