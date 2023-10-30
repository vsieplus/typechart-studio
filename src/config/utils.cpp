#include <cmath>

#include "actions/editaction.hpp"
#include "config/utils.hpp"

#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome6.h"

namespace utils {

void emptyActionStack(std::stack<std::shared_ptr<EditAction>> & stack) {
    while(!stack.empty())
        stack.pop();
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool showEditableText(const char * label, char * text, size_t bufSize, bool & editingText, std::string & savedText) {
    bool textChanged { false };

    if(editingText) {
        if(ImGui::InputText(label, text, bufSize, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            savedText = text;
            textChanged = true;
            editingText = false;
        }
        if(!ImGui::IsItemHovered() && !ImGuiFileDialog::Instance()->IsOpened() && ImGui::IsMouseClicked(0)) {
            editingText = false;
            strcpy(text, savedText.c_str());
        }
    } else {
        ImGui::Text("%s : %s", label, savedText.c_str());
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            editingText = true;
        }
    }

    return textChanged;
}

BeatPos calculateBeatpos(double absBeat, int currentBeatsplit, const std::vector<Timeinfo> & timeinfo) {
    double absMeasure = 0.f;

    int measure = 0;
    int measureSplit = 0;
    int split = 0;

    int prevBeatsPerMeasure = 0;
    double prevSectionAbsBeat = 0.f;

    unsigned int i = 0;
    for(const auto & time : timeinfo) {
        // track current measure
        if(absBeat >= time.absBeatStart && i > 0) {
            double prevSectionMeasures = (time.absBeatStart - prevSectionAbsBeat) / (double)prevBeatsPerMeasure;
            absMeasure += prevSectionMeasures;
        }

        // calculate the leftover beats from the section the note falls within
        bool isLastSection = i == timeinfo.size() - 1;
        bool beatInPrevSection = absBeat < time.absBeatStart; 
        if(beatInPrevSection || isLastSection) {
            int currBeatsPerMeasure = beatInPrevSection ? prevBeatsPerMeasure : time.beatsPerMeasure;
            double currAbsBeat = beatInPrevSection ? prevSectionAbsBeat : time.absBeatStart;

            absMeasure += (absBeat - currAbsBeat) / currBeatsPerMeasure;
            measure = (int)std::floor(absMeasure);
            measureSplit = currentBeatsplit * currBeatsPerMeasure;

            double leftoverBeats = (absMeasure - measure) * currBeatsPerMeasure;
            split = (int)(leftoverBeats * currentBeatsplit + 0.5);
            break;
        }

        prevSectionAbsBeat = time.absBeatStart;
        prevBeatsPerMeasure = time.beatsPerMeasure;
        i++;
    }

    return BeatPos(measure, measureSplit, split);
}

std::pair<int, double> splitSecsbyMin(double seconds) {
    double minutes = seconds / 60;

    auto fullMinutes = (int)std::floor(minutes);
    double leftoverSecs = (minutes - fullMinutes) * 60.f;

    return std::make_pair(fullMinutes, leftoverSecs);
}

}
