#include "config/chartinfo.hpp"
#include "config/constants.hpp"
#include "config/notemaps.hpp"
#include "config/songposition.hpp"
#include "ui/editwindow.hpp"

#include <fstream>
#include <iostream>
#include <memory>

ChartInfo::ChartInfo() {}

ChartInfo::ChartInfo(int level, std::string typist, std::string keyboardLayout, std::string difficulty) : 
        level(level), typist(typist), keyboardLayout(keyboardLayout), difficulty(difficulty) {}

bool ChartInfo::loadChart(fs::path chartPath, SongPosition & songpos) {
    savePath = chartPath;

    ordered_json chartinfoJSON;

    try {
        std::ifstream in(chartPath);
        in >> chartinfoJSON;

        loadChartMetadata(chartinfoJSON);
        loadChartTimeInfo(chartinfoJSON, songpos);
        loadChartStops(chartinfoJSON, songpos);
        loadChartSkips(chartinfoJSON, songpos);
        loadChartNotes(chartinfoJSON, songpos);
    } catch(const nlohmann::detail::parse_error & e) {
        std::cerr << "Error parsing chart file: " << e.what() << std::endl;
        return false;
    } catch(const std::runtime_error & e) {
        std::cerr << "Error loading chart file: " << e.what() << std::endl;
        return false;
    }

    std::sort(notes.myItems.begin(), notes.myItems.end());
    notes.resetItemCounts();

    return true;
}

void ChartInfo::loadChartMetadata(ordered_json chartinfoJSON) {
    typist = chartinfoJSON.value(constants::TYPIST_KEY, constants::TYPIST_VALUE_DEFAULT);
    keyboardLayout = chartinfoJSON.value(constants::KEYBOARD_KEY, constants::KEYBOARD_VALUE_DEFAULT);
    difficulty = chartinfoJSON.value(constants::DIFFICULTY_KEY, constants::DIFFICULTY_VALUE_DEFAULT);
    level = chartinfoJSON.value(constants::LEVEL_KEY, constants::LEVEL_VALUE_DEFAULT);
}

void ChartInfo::loadChartTimeInfo(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> timeinfo = chartinfoJSON[constants::TIMEINFO_KEY];
    Timeinfo * prevTimeinfo = nullptr;

    for(auto & sectionJSON : timeinfo) {
        std::vector<int> pos = sectionJSON[constants::POS_KEY];

        float bpm = sectionJSON.value(constants::BPM_KEY, constants::BPM_VALUE_DEFAULT);
        float interpolateBeatDuration = sectionJSON.value(constants::INTERPOLATE_BEAT_DURATION_KEY, constants::INTERPOLATE_BEAT_DURATION_VALUE_DEFAULT);
        int beatsPerMeasure = sectionJSON.value(constants::BEATS_PER_MEASURE_KEY, constants::BEATS_PER_MEASURE_VALUE_DEFAULT);

        if(pos.size() == constants::NUM_BEATPOS_ELEMENTS) {
            auto sectionStartPos = BeatPos(pos.at(0), pos.at(1), pos.at(2));
            auto sectionTimeInfo = Timeinfo(sectionStartPos, prevTimeinfo, beatsPerMeasure, bpm, interpolateBeatDuration);

            songpos.timeinfo.push_back(sectionTimeInfo);
            prevTimeinfo = &(songpos.timeinfo.back());
        }
    }

    std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());
}

void ChartInfo::loadChartStops(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> stopsJSON = chartinfoJSON[constants::STOPS_KEY];
    for(auto & stopJSON : stopsJSON) {
        std::vector<int> pos = stopJSON[constants::POS_KEY];
        float beatDuration = stopJSON.value(constants::DURATION_KEY, constants::DURATION_VALUE_DEFAULT);

        if(pos.size() == constants::NUM_BEATPOS_ELEMENTS) {
            BeatPos beatpos = BeatPos(pos.at(0), pos.at(1), pos.at(2));
            float absBeat = songpos.calculateAbsBeat(beatpos);

            BeatPos endBeatpos = calculateBeatpos(absBeat + beatDuration, pos.at(1), songpos.timeinfo);

            notes.addStop(absBeat, songpos.absBeat, beatDuration, beatpos, endBeatpos);
        }
    }
}

void ChartInfo::loadChartSkips(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> skipsJSON = chartinfoJSON[constants::SKIPS_KEY];
    for(auto & skipJSON : skipsJSON) {
        std::vector<int> pos = skipJSON[constants::POS_KEY];
        float beatDuration = skipJSON.value(constants::DURATION_KEY, constants::DURATION_VALUE_DEFAULT);
        float skipTime = skipJSON.value(constants::SKIPTIME_KEY, constants::SKIPTIME_VALUE_DEFAULT);

        if(pos.size() == constants::NUM_BEATPOS_ELEMENTS) {
            BeatPos beatpos = BeatPos(pos.at(0), pos.at(1), pos.at(2));
            float absBeat = songpos.calculateAbsBeat(beatpos);

            BeatPos endBeatpos = calculateBeatpos(absBeat + beatDuration, pos.at(1), songpos.timeinfo);

            auto skip = notes.addSkip(absBeat, songpos.absBeat, skipTime, beatDuration, beatpos, endBeatpos);
            songpos.addSkip(skip);
        }
    }
}

void ChartInfo::loadChartNotes(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> notesJSON = chartinfoJSON[constants::NOTES_KEY];
    for(auto iter = notesJSON.begin(); iter != notesJSON.end(); iter++) {
        auto & noteJSON = *iter;

        auto noteType = (NoteType)(noteJSON.value(constants::NOTE_TYPE_KEY, constants::NOTE_TYPE_VALUE_DEFAULT));
        
        // release notes handled separately below
        if(noteType == NoteType::KEYHOLDRELEASE) {
            continue;
        }

        std::vector<int> pos = noteJSON[constants::POS_KEY];
        if(pos.size() != constants::NUM_BEATPOS_ELEMENTS) {
            continue;
        }

        BeatPos beatpos = BeatPos(pos.at(0), pos.at(1), pos.at(2));
        BeatPos endBeatpos = beatpos;
        std::string keyText = noteJSON.value(constants::NOTE_KEY_KEY, constants::NOTE_KEY_VALUE_DEFAULT);
        if(notemaps::STR_TO_FUNCTION_KEY.find(keyText) != notemaps::STR_TO_FUNCTION_KEY.end()) {
            keyText = notemaps::STR_TO_FUNCTION_KEY.at(keyText);
        }

        switch(noteType) {
            case NoteType::KEYPRESS:
                break;
            case NoteType::KEYHOLDSTART:
                // find next matching release note
                for(auto nIter = std::next(iter); nIter != notesJSON.end(); nIter++) {
                    auto & nextNoteJSON = *nIter;

                    NoteType nextNoteType = (NoteType)(nextNoteJSON.value(constants::NOTE_TYPE_KEY, constants::NOTE_TYPE_VALUE_DEFAULT));
                    std::string nextNoteKeyText = nextNoteJSON[constants::NOTE_KEY_KEY];
                    if(notemaps::STR_TO_FUNCTION_KEY.find(nextNoteKeyText) != notemaps::STR_TO_FUNCTION_KEY.end()) {
                        nextNoteKeyText = notemaps::STR_TO_FUNCTION_KEY.at(nextNoteKeyText);
                    }

                    if(nextNoteKeyText == keyText && nextNoteType == NoteType::KEYHOLDRELEASE) {
                        std::vector<int> nextNotepos = nextNoteJSON[constants::POS_KEY];
                        if(nextNotepos.size() == constants::NUM_BEATPOS_ELEMENTS) {
                            endBeatpos = (BeatPos) { nextNotepos.at(0), nextNotepos.at(1), nextNotepos.at(2) };
                            break;
                        }
                    }
                }

                break;
            case NoteType::KEYHOLDRELEASE:
                continue;
        }

        NoteSequenceItem::SequencerItemType itemType;
        if(notemaps::MIDDLE_ROW_KEYS.find(keyboardLayout) != notemaps::MIDDLE_ROW_KEYS.end()) {
            auto & validKeys = notemaps::MIDDLE_ROW_KEYS.at(keyboardLayout);
            if(notemaps::FUNCTION_KEY_TO_STR.find(keyText) != notemaps::FUNCTION_KEY_TO_STR.end()) {
                itemType = NoteSequenceItem::SequencerItemType::BOT_NOTE;
            } else if(validKeys.find(keyText.at(0)) != validKeys.end()) {
                itemType = NoteSequenceItem::SequencerItemType::MID_NOTE;
            } else if(keyText.length() == 1 && isdigit(keyText.at(0))) {
                itemType = NoteSequenceItem::SequencerItemType::TOP_NOTE;
            }
        } else {
            // unsupported keyboard layout
            throw std::runtime_error("Unsupported keyboard layout");
        }

        float absBeat = songpos.calculateAbsBeat(beatpos);
        float absBeatEnd = songpos.calculateAbsBeat(endBeatpos);

        float beatDuration = absBeatEnd - absBeat;

        notes.addNote(absBeat, songpos.absBeat, beatDuration, beatpos, endBeatpos, itemType, keyText);
    }
}

ordered_json ChartInfo::saveChartMetadata(SongPosition & songpos) {
    ordered_json chartinfo;

    chartinfo[constants::TYPIST_KEY] = typist;
    chartinfo[constants::KEYBOARD_KEY] = keyboardLayout;
    chartinfo[constants::DIFFICULTY_KEY] = difficulty;
    chartinfo[constants::LEVEL_KEY] = level;

    return chartinfo;
}

ordered_json ChartInfo::saveChartTimeInfo(SongPosition & songpos) {
    ordered_json timeinfo;

    std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());

    for(auto & section : songpos.timeinfo) {
        ordered_json sectionJSON;
        sectionJSON[constants::POS_KEY] = { section.beatpos.measure, section.beatpos.measureSplit, section.beatpos.split };
        sectionJSON[constants::BPM_KEY] = section.bpm;
        sectionJSON[constants::BEATS_PER_MEASURE_KEY] = section.beatsPerMeasure;
        sectionJSON[constants::INTERPOLATE_BEAT_DURATION_KEY] = section.interpolateBeatDuration;

        timeinfo.push_back(sectionJSON);
    }

    return timeinfo;
}

void ChartInfo::saveChart(fs::path chartPath, SongPosition & songpos) {
    savePath = chartPath;

    auto chartinfoJSON = saveChartMetadata(songpos);
    chartinfoJSON[constants::TIMEINFO_KEY] = saveChartTimeInfo(songpos);

    ordered_json stopsJSON = ordered_json::array();
    ordered_json skipsJSON = ordered_json::array();
    ordered_json notesJSON = ordered_json::array();

    std::sort(notes.myItems.begin(), notes.myItems.end());

    for(auto & item : notes.myItems) {
        ordered_json itemJSON;
        ordered_json itemBeatpos = {item->beatpos.measure, item->beatpos.measureSplit, item->beatpos.split};

        switch(item->getItemType()) {
            case NoteSequenceItem::SequencerItemType::STOP:
                {
                    itemJSON[constants::POS_KEY] = itemBeatpos;
                    itemJSON[constants::DURATION_KEY] = item->beatEnd - item->absBeat;
                    stopsJSON.push_back(itemJSON);
                }
                break;
            case NoteSequenceItem::SequencerItemType::SKIP:
                {
                    auto currSkip = std::dynamic_pointer_cast<Skip>(item);

                    itemJSON[constants::POS_KEY] = itemBeatpos;
                    itemJSON[constants::DURATION_KEY] = currSkip->beatEnd - currSkip->absBeat;
                    itemJSON[constants::SKIPTIME_KEY] = currSkip->skipTime;
                    skipsJSON.push_back(itemJSON);
                }
                break;
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                {
                    auto currNote = std::dynamic_pointer_cast<Note>(item);
                    NoteType currNoteType = currNote->noteType;

                    itemJSON[constants::POS_KEY] = itemBeatpos;

                    std::string keyText = currNote->displayText;
                    if (notemaps::FUNCTION_KEY_TO_STR.find(currNote->displayText) != notemaps::FUNCTION_KEY_TO_STR.end()) {
                        keyText = notemaps::FUNCTION_KEY_TO_STR.at(currNote->displayText);
                    }

                    itemJSON[constants::NOTE_KEY_KEY] = keyText;
                    itemJSON[constants::NOTE_TYPE_KEY] = (int)currNoteType;
                    notesJSON.push_back(itemJSON);

                    // add release note if hold start
                    if(currNoteType == NoteType::KEYHOLDSTART) {
                        ordered_json item2JSON;

                        item2JSON[constants::POS_KEY] = {currNote->endBeatpos.measure, currNote->endBeatpos.measureSplit, currNote->endBeatpos.split};
                        item2JSON[constants::NOTE_KEY_KEY] = keyText;
                        item2JSON[constants::NOTE_TYPE_KEY] = (int)(NoteType::KEYHOLDRELEASE);
                        notesJSON.push_back(item2JSON);
                    }
                }
                break;
        }
    }

    chartinfoJSON[constants::STOPS_KEY] = stopsJSON;
    chartinfoJSON[constants::SKIPS_KEY] = skipsJSON;
    chartinfoJSON[constants::NOTES_KEY] = notesJSON;

    // write to file
    std::ofstream file(savePath.c_str());
    file << std::setw(4) << chartinfoJSON << std::endl;
}
