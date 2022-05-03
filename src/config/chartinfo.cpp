#include "config/chartinfo.hpp"
#include "config/songposition.hpp"

#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

ChartInfo::ChartInfo(std::string chartPath) {
    savePath = chartPath;
}

ChartInfo::ChartInfo(int level, std::string typist, std::string keyboardLayout) : 
        level(level), typist(typist), keyboardLayout(keyboardLayout) {}

void ChartInfo::saveChart(std::string chartPath, SongPosition & songpos) {
    savePath = chartPath;

    ordered_json chartinfo;

    chartinfo["typist"] = typist;
    chartinfo["keyboard"] = keyboardLayout;
    chartinfo["level"] = level;
    chartinfo["offsetMS"] = songpos.offsetMS;

    ordered_json timeinfo;

    for(auto & section : songpos.timeinfo) {
        json sectionJSON;
        sectionJSON["pos"] = { section.beatpos.measure, section.beatpos.beatsplit, section.beatpos.split };
        sectionJSON["bpm"] = section.bpm;
        sectionJSON["beatsPerMeasure"] = section.beatsPerMeasure;

        timeinfo.push_back(sectionJSON);
    }

    chartinfo["timeinfo"] = timeinfo;

    ordered_json stopsJSON;
    ordered_json skipsJSON;
    ordered_json notesJSON;

    for(auto & item : notes.myItems) {
        ordered_json itemJSON;
        json itemBeatpos = {item->beatpos.measure, item->beatpos.beatsplit, item->beatpos.split};

        switch(item->getItemType()) {
            case SequencerItemType::STOP:
                itemJSON["pos"] = itemBeatpos;
                itemJSON["duration"] = item->beatEnd - item->absBeat;
                stopsJSON.push_back(itemJSON);
                break;
            case SequencerItemType::SKIP:
                {
                    auto currSkip = std::dynamic_pointer_cast<Skip>(item);

                    itemJSON["pos"] = itemBeatpos;
                    itemJSON["duration"] = currSkip->beatEnd - currSkip->absBeat;
                    itemJSON["skiptime"] = currSkip->skipTime;
                    skipsJSON.push_back(itemJSON);
                }
                break;
            case SequencerItemType::TOP_NOTE:
            case SequencerItemType::MID_NOTE:
            case SequencerItemType::BOT_NOTE:
                {
                    auto currNote = std::dynamic_pointer_cast<Note>(item);   
                    NoteType currNoteType = currNote->noteType;

                    itemJSON["pos"] = itemBeatpos;

                    std::string keyText = currNote->displayText;
                    if (FUNCTION_KEY_TO_STR.find(currNote->displayText) != FUNCTION_KEY_TO_STR.end()) {
                        keyText = FUNCTION_KEY_TO_STR.at(currNote->displayText);
                    }

                    itemJSON["key"] = keyText;
                    itemJSON["type"] = (int)currNoteType;
                    notesJSON.push_back(itemJSON);

                    // add release note if hold start
                    if(currNoteType == NoteType::KEYHOLDSTART) {
                        ordered_json item2JSON;

                        item2JSON["pos"] = {currNote->endBeatpos.measure, currNote->endBeatpos.beatsplit, currNote->endBeatpos.split};
                        item2JSON["key"] = keyText;
                        item2JSON["type"] = (int)(NoteType::KEYHOLDRELEASE);
                        notesJSON.push_back(item2JSON);
                    }
                }
                break;
        }
    }

    chartinfo["stops"] = stopsJSON;
    chartinfo["skips"] = skipsJSON;
    chartinfo["notes"] = notesJSON;

    // write to file
    std::ofstream file(savePath.c_str());
    file << chartinfo << std::endl;
}