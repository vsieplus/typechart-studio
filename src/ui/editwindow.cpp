#include <cstring>
#include <map>

#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "config/chartinfo.hpp"
#include "config/songinfo.hpp"

#include "ui/editwindow.hpp"
#include "ui/windowsizes.hpp"

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

static char UImusicFilename[128] = "";
static char UIcoverArt[128] = "";

static char UItitle[64] = "";
static char UIartist[64] = "";
static char UIbpmtext[16] = "";

static std::string UImusicFilepath = "";
static std::string UIcoverArtFilepath = "";

const char * songinfoFileFilter = "(*.json){.json}";
const char * imageFileFilters = "(*.jpg *.png){.jpg,.png}";
const char * musicFileFilters = "(*.flac *.mp3 *.ogg){.flac,.mp3,.ogg}";

void loadSonginfo(std::string songinfoPath) {
    
}

void showSongConfig() {
    // song config
    ImGui::Text("Song configuration");
    if(ImGui::Button("Load from existing...")) {
        ImGuiFileDialog::Instance()->OpenDialog("selectSonginfo", "Select songinfo.json", songinfoFileFilter, ".");
    }
    
    ImGui::SameLine();
    HelpMarker("Load song data from a pre-existing songinfo.json file");

    // songinfo dialog
    if(ImGuiFileDialog::Instance()->Display("selectSonginfo", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string songinfoPath = ImGuiFileDialog::Instance()->GetFilePathName();
            loadSonginfo(songinfoPath);
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText("Music", UImusicFilename, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##music")) {
        ImGuiFileDialog::Instance()->OpenDialog("selectMusicFile", "Select Music", musicFileFilters, ".");
    }

    // music file dialog
    if(ImGuiFileDialog::Instance()->Display("selectMusicFile", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            UImusicFilepath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            strcpy(UImusicFilename, fileName.c_str());
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText("Art", UIcoverArt, 128, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if(ImGui::Button("Browse...##art")) {
        ImGuiFileDialog::Instance()->OpenDialog("selectArt", "Select Art", imageFileFilters, ".");
    }

    // art file dialog
    if(ImGuiFileDialog::Instance()->Display("selectArt", ImGuiWindowFlags_NoCollapse, minFDSize, maxFDSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            UIcoverArtFilepath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            strcpy(UIcoverArt, fileName.c_str());
        }

        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputText("Title", UItitle, 64);
    ImGui::InputText("Artist", UIartist, 64);
    ImGui::InputText("BPM", UIbpmtext, 64);
    ImGui::SameLine();
    HelpMarker("This value is only used as the 'displayed' BPM");
}


static char UItypist[64] = "";
static int UIlevel = 1;
static int UIkeyboardLayout = 0;

void showChartConfig() {
    ImGui::Text("Chart configuration");

    ImGui::InputText("Typist", UItypist, 64);
    ImGui::Combo("Keyboard", &UIkeyboardLayout, "QWERTY\0DVORAK\0AZERTY\0\0");
    ImGui::SameLine();
    HelpMarker("Choose the keyboard layout that this chart is\n"
                "intended to be played with. Charts will then be\n"
                "accordingly 'translated' to other keyboard layouts\n"
                "when loaded into Typing Tempo.");
    ImGui::InputInt("Level", &UIlevel);
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

    ChartInfo chartInfo = ChartInfo(UIlevel, UItypist, ID_TO_KEYBOARDLAYOUT.at(UIkeyboardLayout));

    EditWindowData newWindow = EditWindowData(true, windowID, windowName);
    editWindows.push_back(newWindow);

    newEditStarted = false;

    // reset fields
    UIlevel = 1;
    UImusicFilename[0] = '\0';
    UIcoverArt[0] = '\0';
    UItitle[0] = '\0';
    UIartist[0] = '\0';
    UIbpmtext[0] = '\0';
}

void showInitEditWindow() {
    if(newEditStarted) {
        ImGui::SetNextWindowSize(newEditWindowSize);
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