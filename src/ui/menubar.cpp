#include "ui/menubar.hpp"

#include "imgui.h"

void showMenuBar() {
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
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void showFileMenu() {
    if(ImGui::MenuItem("New")) {

    }

    if(ImGui::MenuItem("Open", "Ctrl+O")) {

    }

    if(ImGui::BeginMenu("Open Recent")) {
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