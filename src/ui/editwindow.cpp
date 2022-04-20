#include "imgui.h"

#include "ui/editwindow.hpp"

std::string DEFAULT_WINDOW_NAME = "Untitled";

bool newEditStarted = false;

void createNewEditWindow() {
    newEditStarted = true;
}

void showInitEditWindow() {
    if(newEditStarted) {
        ImGui::Begin("New Chart", &newEditStarted, ImGuiWindowFlags_NoResize);
        ImGui::Text("Chart configuration settings");

        if(ImGui::Button("Create")) {
            std::string windowName = DEFAULT_WINDOW_NAME;
            int windowID = 0;

            if(!availableWindowIDs.empty()) {
                windowName += std::to_string(availableWindowIDs.back());
                availableWindowIDs.pop();
            } else if(editWindows.size() > 0) {
                windowName += std::to_string(editWindows.size());
            }

            EditWindowData newWindow = EditWindowData(true, windowID, windowName);
            editWindows.push_back(newWindow);

            newEditStarted = false;
        }

        ImGui::End();
    }
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
                    availableWindowIDs.push(currWindow.ID);
                    iter = editWindows.erase(iter);
                }

                ImGui::SameLine();
                if(ImGui::Button("Cancel")) {
                    currWindow.open = true;
                }

                ImGui::End();
            } else {
                availableWindowIDs.push(currWindow.ID);
                iter = editWindows.erase(iter);
            }
        }
    }
}