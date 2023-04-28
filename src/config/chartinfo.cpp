#include "config/chartinfo.hpp"
#include "config/songposition.hpp"
#include "ui/editwindow.hpp"

#include <fstream>
#include <memory>
#include <json.hpp>
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

float calculateAbsBeat(BeatPos beatpos, std::vector<Timeinfo> & timeinfo) {
    float absBeat = 0.f;
    int prevSectionBeatsPerMeasure = 4;
    BeatPos prevSectionBeatpos = {0, 1, 0};
    
    for(auto & section: timeinfo) {
        if(beatpos < section.beatpos || section.absBeatStart == timeinfo.back().absBeatStart) {
            float prevSectionMeasure = prevSectionBeatpos.measure + (prevSectionBeatpos.split / (float)prevSectionBeatpos.measureSplit);
            float currSectionMeasure = beatpos.measure + (beatpos.split / (float)beatpos.measureSplit);

            float currSectionBeats = (currSectionMeasure - prevSectionMeasure) * prevSectionBeatsPerMeasure;
            absBeat += currSectionBeats;
            break;
        } else {
            absBeat = section.absBeatStart;
            prevSectionBeatpos = section.beatpos;
            prevSectionBeatsPerMeasure = section.beatsPerMeasure;
        }
    }

    return absBeat;
}

ChartInfo::ChartInfo() {}

ChartInfo::ChartInfo(int level, std::string typist, std::string keyboardLayout, std::string difficulty) : 
        level(level), typist(typist), keyboardLayout(keyboardLayout), difficulty(difficulty) {}

bool ChartInfo::loadChart(fs::path chartPath, SongPosition & songpos) {
    savePath = chartPath;

    ordered_json chartinfoJSON;

    try {
        std::ifstream in(chartPath);
        in >> chartinfoJSON;
    
        typist = chartinfoJSON["typist"];
        keyboardLayout = chartinfoJSON["keyboard"];
        difficulty = chartinfoJSON["difficulty"];
        level = chartinfoJSON["level"];
        songpos.offsetMS = chartinfoJSON["offsetMS"];

        std::vector<ordered_json> timeinfo = chartinfoJSON["timeinfo"];
        Timeinfo * prevTimeinfo = nullptr;
        
        for(auto & sectionJSON : timeinfo) {
            std::vector<int> pos = sectionJSON["pos"];
            float bpm = sectionJSON["bpm"];
            float interpolateBeatDuration = 0.f;
            int beatsPerMeasure = sectionJSON["beatsPerMeasure"];

            if(sectionJSON.contains("interpolateBeatDuration")) {
                interpolateBeatDuration = sectionJSON["interpolateBeatDuration"];
            }

            songpos.timeinfo.push_back(Timeinfo((BeatPos){pos.at(0), pos.at(1), pos.at(2)}, prevTimeinfo, beatsPerMeasure, bpm, interpolateBeatDuration));
            prevTimeinfo = &(songpos.timeinfo.back());
        }

        std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());

        if(!chartinfoJSON["stops"].empty()) {
            std::vector<ordered_json> stopsJSON = chartinfoJSON["stops"];
            for(auto & stopJSON : stopsJSON) {
                std::vector<int> pos = stopJSON["pos"];
                float beatDuration = stopJSON["duration"];

                BeatPos beatpos = (BeatPos){pos.at(0), pos.at(1), pos.at(2)};
                float absBeat = calculateAbsBeat(beatpos, songpos.timeinfo);

                BeatPos endBeatpos = calculateBeatpos(absBeat + beatDuration, pos.at(1), songpos.timeinfo);

                notes.addStop(absBeat, songpos.absBeat, beatDuration, beatpos, endBeatpos);
            }
        }

        if(!chartinfoJSON["skips"].empty()) {
            std::vector<ordered_json> skipsJSON = chartinfoJSON["skips"];
            for(auto & skipJSON : skipsJSON) {
                std::vector<int> pos = skipJSON["pos"];
                float beatDuration = skipJSON["duration"];
                float skipTime = skipJSON["skiptime"];

                BeatPos beatpos = (BeatPos){pos.at(0), pos.at(1), pos.at(2)};
                float absBeat = calculateAbsBeat(beatpos, songpos.timeinfo);

                BeatPos endBeatpos = calculateBeatpos(absBeat + beatDuration, pos.at(1), songpos.timeinfo);

                auto skip = notes.addSkip(absBeat, songpos.absBeat, skipTime, beatDuration, beatpos, endBeatpos);
                songpos.addSkip(skip);
            }
        }

        std::vector<ordered_json> notesJSON = chartinfoJSON["notes"];
        for(auto iter = notesJSON.begin(); iter != notesJSON.end(); iter++) {
            auto & noteJSON = *iter;

            NoteType noteType = (NoteType)(noteJSON["type"].get<int>());
            
            // release notes handled separately below
            if(noteType == NoteType::KEYHOLDRELEASE) {
                continue;
            }

            std::vector<int> pos = noteJSON["pos"];
            if(pos.size() != 3) {
                continue;
            }

            BeatPos beatpos = (BeatPos){pos.at(0), pos.at(1), pos.at(2)};
            BeatPos endBeatpos = beatpos;
            std::string keyText = noteJSON["key"];
            if(STR_TO_FUNCTION_KEY.find(keyText) != STR_TO_FUNCTION_KEY.end()) {
                keyText = STR_TO_FUNCTION_KEY.at(keyText);
            }

            switch(noteType) {
                case NoteType::KEYPRESS:
                    break;
                case NoteType::KEYHOLDSTART:
                    // find next matching release note
                    for(auto nIter = std::next(iter); nIter != notesJSON.end(); nIter++) {
                        auto & nextNoteJSON = *nIter;

                        NoteType nextNoteType = (NoteType)(nextNoteJSON["type"].get<int>());
                        std::string nextNoteKeyText = nextNoteJSON["key"];
                        if(STR_TO_FUNCTION_KEY.find(nextNoteKeyText) != STR_TO_FUNCTION_KEY.end()) {
                            nextNoteKeyText = STR_TO_FUNCTION_KEY.at(nextNoteKeyText);
                        }

                        if(nextNoteKeyText == keyText && nextNoteType == NoteType::KEYHOLDRELEASE) {
                            std::vector<int> nextNotepos = nextNoteJSON["pos"];
                            endBeatpos = (BeatPos) { nextNotepos.at(0), nextNotepos.at(1), nextNotepos.at(2) };
                            break;
                        }
                    }

                    break;
                case NoteType::KEYHOLDRELEASE:
                    continue;                    
            }

            SequencerItemType itemType;
            if(MIDDLE_ROW_KEYS.find(keyboardLayout) != MIDDLE_ROW_KEYS.end()) {
                auto & validKeys = MIDDLE_ROW_KEYS.at(keyboardLayout);
                if(FUNCTION_KEY_TO_STR.find(keyText) != FUNCTION_KEY_TO_STR.end()) {
                    itemType = SequencerItemType::BOT_NOTE;
                } else if(validKeys.find(keyText.at(0)) != validKeys.end()) {
                    itemType = SequencerItemType::MID_NOTE;
                } else if(keyText.length() == 1 && isdigit(keyText.at(0))) {
                    itemType = SequencerItemType::TOP_NOTE;
                }
            } else {
                // unsupported keyboard layout
                return false;
            }

            float absBeat = calculateAbsBeat(beatpos, songpos.timeinfo);
            float absBeatEnd = calculateAbsBeat(endBeatpos, songpos.timeinfo);

            float beatDuration = absBeatEnd - absBeat;

            notes.addNote(absBeat, songpos.absBeat, beatDuration, beatpos, endBeatpos, itemType, keyText);
        }
    } catch(...) {
        return false;
    }

    std::sort(notes.myItems.begin(), notes.myItems.end());
    notes.resetItemCounts();

    return true;
}

void ChartInfo::saveChart(fs::path chartPath, SongPosition & songpos) {
    savePath = chartPath;

    ordered_json chartinfo;

    chartinfo["typist"] = typist;
    chartinfo["keyboard"] = keyboardLayout;
    chartinfo["difficulty"] = difficulty;
    chartinfo["level"] = level;
    chartinfo["offsetMS"] = songpos.offsetMS;

    ordered_json timeinfo;

    std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());

    for(auto & section : songpos.timeinfo) {
        ordered_json sectionJSON;
        sectionJSON["pos"] = { section.beatpos.measure, section.beatpos.measureSplit, section.beatpos.split };
        sectionJSON["bpm"] = section.bpm;
        sectionJSON["beatsPerMeasure"] = section.beatsPerMeasure;
        sectionJSON["interpolateBeatDuration"] = section.interpolateBeatDuration;

        timeinfo.push_back(sectionJSON);
    }

    chartinfo["timeinfo"] = timeinfo;

    ordered_json stopsJSON = ordered_json::array();
    ordered_json skipsJSON = ordered_json::array();
    ordered_json notesJSON = ordered_json::array();

    std::sort(notes.myItems.begin(), notes.myItems.end());

    for(auto & item : notes.myItems) {
        ordered_json itemJSON;
        ordered_json itemBeatpos = {item->beatpos.measure, item->beatpos.measureSplit, item->beatpos.split};

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

                        item2JSON["pos"] = {currNote->endBeatpos.measure, currNote->endBeatpos.measureSplit, currNote->endBeatpos.split};
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
    file << std::setw(4) << chartinfo << std::endl;
}
