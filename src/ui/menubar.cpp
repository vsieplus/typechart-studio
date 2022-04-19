#include "ui/menubar.hpp"

#include "imgui.h"

void showMenuBar(ImFont * menuFont, ImFont * submenuFont) {
    // styling
    if(menuFont)
        ImGui::PushFont(menuFont);

    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            showFileMenu(submenuFont);         
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit")) {
            showEditMenu(submenuFont);
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Options")) {
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if(menuFont)
        ImGui::PopFont();
}

void showFileMenu(ImFont * submenuFont) {
    if(submenuFont)
        ImGui::PushFont(submenuFont);

    if(ImGui::MenuItem("New", "Ctrl+N")) {

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

    if(submenuFont)
        ImGui::PopFont();
}

void showEditMenu(ImFont * submenuFont) {
    if(submenuFont)
        ImGui::PushFont(submenuFont);

    if(ImGui::MenuItem("Undo", "CTRL+Z")) {

    }

    if(ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {

    }

    ImGui::Separator();
    
    if(ImGui::MenuItem("Cut", "CTRL+X")) {

    }

    if(ImGui::MenuItem("Copy", "CTRL+C")) {

    }

    if(ImGui::MenuItem("Paste", "CTRL+V")) {

    }

    if(submenuFont)
        ImGui::PopFont();
}