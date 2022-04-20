#include "imgui.h"

#include "ui/editwindow.hpp"

bool newEditStarted = false;

std::string DEFAULT_WINDOW_NAME = "Untitled";


void createNewEditWindow() {
    newEditStarted = true;
}

void showInitEditWindow() {
    if(newEditStarted) {
        ImGui::Begin("New Chart", &newEditStarted);
        ImGui::Text("Chart Configuration");

        static char buf1[64] = "";
        static int level = 0;

        static int keyboardLayout = 0;

        ImGui::InputTextWithHint(" ", "Typist", buf1, 64);
        ImGui::Combo("Keyboard", &keyboardLayout, "QWERTY\0DVORAK\0AZERTY\0\0");
        ImGui::InputInt("Level", &level);

        if(ImGui::Button("Create")) {
            std::string windowName = DEFAULT_WINDOW_NAME;
            int windowID = 0;

            if(!availableWindowIDs.empty()) {
                windowID = availableWindowIDs.back();
                availableWindowIDs.pop();

                if(windowID > 0)
                    windowName += std::to_string(windowID);
            } else if(editWindows.size() > 0) {
                windowID = editWindows.size();
                windowName += std::to_string(windowID);
            }

            EditWindowData newWindow = EditWindowData(true, windowID, windowName);
            editWindows.push_back(newWindow);

            newEditStarted = false;

            level = 0;
        }

        ImGui::End();
    }
}

void closeWindow(EditWindowData & currWindow, std::list<EditWindowData>::iterator & iter) {
    availableWindowIDs.push(currWindow.ID);
    iter = editWindows.erase(iter);
}

void showEditWindows() {
    for(auto iter = editWindows.begin(); iter != editWindows.end(); iter++) {
        auto & currWindow = *iter;

        ImGuiWindowFlags windowFlags = 0;
        if(currWindow.unsaved)  windowFlags |= ImGuiWindowFlags_UnsavedDocument;
        if(!currWindow.open)    windowFlags |= ImGuiWindowFlags_NoInputs;

        ImGui::Begin(currWindow.name.c_str(), &(currWindow.open), windowFlags);


        ImGui::End();

        if(!currWindow.open) {
            if(currWindow.unsaved) {
                char msg[128];
                snprintf(msg, 128, "Unsaved work! [%s]", currWindow.name.c_str());
                ImGui::Begin(msg);

                ImGui::Text("Save before closing?");
                if(ImGui::Button("Yes")) {
                    // save chart
                }

                ImGui::SameLine();
                if(ImGui::Button("No")) {
                    closeWindow(currWindow, iter);
                }

                ImGui::SameLine();
                if(ImGui::Button("Cancel")) {
                    currWindow.open = true;
                }

                ImGui::End();
            } else {
                closeWindow(currWindow, iter);
            }
        }
    }
}