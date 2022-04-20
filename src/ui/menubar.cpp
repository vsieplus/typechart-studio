#include "imgui.h"

#include "ui/editwindow.hpp"
#include "ui/menubar.hpp"

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

        ImGui::EndMainMenuBar();
    }

    if(menuFont)
        ImGui::PopFont();
}

void showFileMenu() {
    if(ImGui::MenuItem("New", "Ctrl+N")) {
        createNewEditWindow();
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
}

void showOptionMenu() {
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