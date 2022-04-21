#include <map>

#include "imgui.h"

#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"

#include "ui/editwindow.hpp"

static bool newEditStarted = false;

std::string DEFAULT_WINDOW_NAME = "Untitled";

const std::map<int, std::string> ID_TO_KEYBOARDLAYOUT = {
    { 0 , "QWERTY" },
    { 1, "DVORAK" },
    { 2, "AZERTY" }
};

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void startNewEditWindow() {
    newEditStarted = true;
}

static char musicFilename[128] = "";

static char title[64] = "";
static char artist[64] = "";
static char bpmtext[16] = "";

void showSongConfig() {
    // song config
    ImGui::Text("Song configuration");
    ImGui::Button("Load from existing...");
    ImGui::SameLine();
    HelpMarker("Load a pre-existing songinfo.json file");
    ImGui::InputText("Music", musicFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...")) {
        // open file dialog
    }

    ImGui::InputText("Title", title, 64);
    ImGui::InputText("Artist", artist, 64);
    ImGui::InputText("BPM", bpmtext, 64);
    ImGui::SameLine();
    HelpMarker("This value is only used as the 'displayed' BPM");
}


static char typist[64] = "";
static int level = 1;
static int keyboardLayout = 0;

void showChartConfig() {
    ImGui::Text("Chart configuration");

    ImGui::InputText("Typist", typist, 64);
    ImGui::Combo("Keyboard", &keyboardLayout, "QWERTY\0DVORAK\0AZERTY\0\0");
    ImGui::SameLine();
    HelpMarker("Choose the keyboard layout that this chart is\n"
                "intended to be played with. Charts can then be\n"
                "accordingly 'translated' to other keyboard layouts\n"
                "when loaded into Typing Tempo.");
    ImGui::InputInt("Level", &level);
}

void createNewEditWindow() {
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

    ChartInfo chartInfo = ChartInfo(level, typist, ID_TO_KEYBOARDLAYOUT.at(keyboardLayout));

    EditWindowData newWindow = EditWindowData(true, windowID, windowName);
    editWindows.push_back(newWindow);

    newEditStarted = false;

    // reset fields
    level = 1;
    musicFilename[0] = '\0';
    title[0] = '\0';
    artist[0] = '\0';
    bpmtext[0] = '\0';
}

void showInitEditWindow() {
    if(newEditStarted) {
        ImGui::Begin("New Chart", &newEditStarted);

        showSongConfig();
        ImGui::Separator();
        showChartConfig();

        if(ImGui::Button("Create")) {
            // check valid music

            createNewEditWindow();
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