#include "config/chartinfo.hpp"
#include "config/constants.hpp"
#include "config/notemaps.hpp"
#include "config/songposition.hpp"
#include "config/utils.hpp"
#include "ui/editwindow.hpp"

#include <fstream>
#include <iostream>
#include <memory>

ChartInfo::ChartInfo(int level, std::string_view typist, std::string_view keyboardLayout, std::string_view difficulty)
    : level(level)
    , typist(typist)
    , keyboardLayout(keyboardLayout)
    , difficulty(difficulty) {}

bool ChartInfo::loadChart(const fs::path & chartPath, SongPosition & songpos) {
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

void ChartInfo::loadChartTimeInfo(ordered_json chartinfoJSON, SongPosition & songpos) const {
    std::vector<ordered_json> timeinfo = chartinfoJSON[constants::TIMEINFO_KEY];
    const Timeinfo * prevTimeinfo { nullptr };

    for(auto & sectionJSON : timeinfo) {
        std::vector<int> pos = sectionJSON[constants::POS_KEY];

        double bpm { sectionJSON.value(constants::BPM_KEY, constants::BPM_VALUE_DEFAULT) };
        double interpolateBeatDuration { sectionJSON.value(constants::INTERPOLATE_BEAT_DURATION_KEY, constants::INTERPOLATE_BEAT_DURATION_VALUE_DEFAULT) };
        int beatsPerMeasure { sectionJSON.value(constants::BEATS_PER_MEASURE_KEY, constants::BEATS_PER_MEASURE_VALUE_DEFAULT) };

        if(pos.size() == constants::NUM_BEATPOS_ELEMENTS) {
            BeatPos sectionStartPos { pos.at(0), pos.at(1), pos.at(2) };

            songpos.timeinfo.emplace_back(sectionStartPos, prevTimeinfo, beatsPerMeasure, bpm, interpolateBeatDuration);
            prevTimeinfo = &(songpos.timeinfo.back());
        }
    }

    std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());
}

void ChartInfo::loadChartStops(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> stopsJSON = chartinfoJSON[constants::STOPS_KEY];
    for(auto & stopJSON : stopsJSON) {
        std::vector<int> pos { stopJSON[constants::POS_KEY] };
        double beatDuration { stopJSON.value(constants::DURATION_KEY, constants::DURATION_VALUE_DEFAULT) };

        if(pos.size() == constants::NUM_BEATPOS_ELEMENTS) {
            BeatPos beatpos { pos.at(0), pos.at(1), pos.at(2) };
            double absBeat { songpos.calculateAbsBeat(beatpos) };
            BeatPos endBeatpos { utils::calculateBeatpos(absBeat + beatDuration, pos.at(1), songpos.timeinfo) };

            notes.addStop(absBeat, songpos.absBeat, beatDuration, beatpos, endBeatpos);
        }
    }
}

void ChartInfo::loadChartSkips(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> skipsJSON = chartinfoJSON[constants::SKIPS_KEY];
    for(auto & skipJSON : skipsJSON) {
        std::vector<int> pos { skipJSON[constants::POS_KEY] };
        double beatDuration { skipJSON.value(constants::DURATION_KEY, constants::DURATION_VALUE_DEFAULT) };
        double skipTime { skipJSON.value(constants::SKIPTIME_KEY, constants::SKIPTIME_VALUE_DEFAULT) };

        if(pos.size() == constants::NUM_BEATPOS_ELEMENTS) {
            BeatPos beatpos { pos.at(0), pos.at(1), pos.at(2) };
            double absBeat { songpos.calculateAbsBeat(beatpos) };
            BeatPos endBeatpos { utils::calculateBeatpos(absBeat + beatDuration, pos.at(1), songpos.timeinfo) };

            auto skip { notes.addSkip(absBeat, songpos.absBeat, skipTime, beatDuration, beatpos, endBeatpos) };
            songpos.addSkip(skip);
        }
    }
}

void ChartInfo::loadChartNotes(ordered_json chartinfoJSON, SongPosition & songpos) {
    std::vector<ordered_json> notesJSON = chartinfoJSON[constants::NOTES_KEY];
    for(auto iter = notesJSON.begin(); iter != notesJSON.end(); iter++) {
        auto & noteJSON { *iter };
        auto noteType { static_cast<Note::NoteType>(noteJSON.value(constants::NOTE_TYPE_KEY, constants::NOTE_TYPE_VALUE_DEFAULT)) };

        // release notes handled separately below
        if(noteType == Note::NoteType::KEYHOLDRELEASE) {
            continue;
        }

        std::vector<int> pos = noteJSON[constants::POS_KEY];
        if(pos.size() != constants::NUM_BEATPOS_ELEMENTS) {
            continue;
        }

        BeatPos beatpos { pos.at(0), pos.at(1), pos.at(2) };
        BeatPos endBeatpos { beatpos };
        std::string keyText = noteJSON.value(constants::NOTE_KEY_KEY, constants::NOTE_KEY_VALUE_DEFAULT);
        if(notemaps::STR_TO_FUNCTION_KEY.find(keyText) != notemaps::STR_TO_FUNCTION_KEY.end()) {
            keyText = notemaps::STR_TO_FUNCTION_KEY.at(keyText);
        }

        switch(noteType) {
            case Note::NoteType::KEYPRESS:
                break;
            case Note::NoteType::KEYHOLDSTART:
                endBeatpos = findMatchingReleaseNote(keyText, iter, notesJSON);
                break;
            case Note::NoteType::KEYHOLDRELEASE:
                continue;
        }

        NoteSequenceItem::SequencerItemType itemType { determineItemType(keyText) };
        double absBeat { songpos.calculateAbsBeat(beatpos) };
        double absBeatEnd { songpos.calculateAbsBeat(endBeatpos) };
        double beatDuration { absBeatEnd - absBeat };

        notes.addNote(absBeat, songpos.absBeat, beatDuration, beatpos, endBeatpos, itemType, keyText);
    }
}

BeatPos ChartInfo::findMatchingReleaseNote(std::string_view keyText, std::vector<ordered_json>::iterator iter, std::vector<ordered_json> notesJSON) const {
    // find next matching release note
    for(auto nIter = std::next(iter); nIter != notesJSON.end(); nIter++) {
        auto & nextNoteJSON = *nIter;

        auto nextNoteType = static_cast<Note::NoteType>(nextNoteJSON.value(constants::NOTE_TYPE_KEY, constants::NOTE_TYPE_VALUE_DEFAULT));
        std::string nextNoteKeyText = nextNoteJSON[constants::NOTE_KEY_KEY];
        if(notemaps::STR_TO_FUNCTION_KEY.find(nextNoteKeyText) != notemaps::STR_TO_FUNCTION_KEY.end()) {
            nextNoteKeyText = notemaps::STR_TO_FUNCTION_KEY.at(nextNoteKeyText);
        }

        if(nextNoteKeyText == keyText && nextNoteType == Note::NoteType::KEYHOLDRELEASE) {
            std::vector<int> nextNotepos = nextNoteJSON[constants::POS_KEY];
            if(nextNotepos.size() == constants::NUM_BEATPOS_ELEMENTS) {
                return BeatPos{ nextNotepos.at(0), nextNotepos.at(1), nextNotepos.at(2) };
            }
        }
    }

    return BeatPos{ 0, 0, 1 };
}

NoteSequenceItem::SequencerItemType ChartInfo::determineItemType(const std::string & keyText) const {
    NoteSequenceItem::SequencerItemType itemType { NoteSequenceItem::SequencerItemType::MID_NOTE };
    if(notemaps::MIDDLE_ROW_KEYS.find(keyboardLayout) != notemaps::MIDDLE_ROW_KEYS.end()) {
        auto & validKeys = notemaps::MIDDLE_ROW_KEYS.at(keyboardLayout);
        if(notemaps::FUNCTION_KEY_TO_STR.find(keyText) != notemaps::FUNCTION_KEY_TO_STR.end()) {
            itemType = NoteSequenceItem::SequencerItemType::BOT_NOTE;
        } else if(validKeys.find(keyText.at(0)) != validKeys.end()) {
            itemType = NoteSequenceItem::SequencerItemType::MID_NOTE;
        } else if(keyText.length() == 1 && isdigit(keyText.at(0))) {
            itemType = NoteSequenceItem::SequencerItemType::TOP_NOTE;
        }
    }

    return itemType;
}

ordered_json ChartInfo::saveChartMetadata(const SongPosition & songpos) const {
    ordered_json chartinfo;

    chartinfo[constants::TYPIST_KEY] = typist;
    chartinfo[constants::KEYBOARD_KEY] = keyboardLayout;
    chartinfo[constants::DIFFICULTY_KEY] = difficulty;
    chartinfo[constants::LEVEL_KEY] = level;

    return chartinfo;
}

ordered_json ChartInfo::saveChartTimeInfo(SongPosition & songpos) const {
    ordered_json timeinfo;

    std::sort(songpos.timeinfo.begin(), songpos.timeinfo.end());

    for(const auto & section : songpos.timeinfo) {
        ordered_json sectionJSON;
        sectionJSON[constants::POS_KEY] = { section.beatpos.measure, section.beatpos.measureSplit, section.beatpos.split };
        sectionJSON[constants::BPM_KEY] = section.bpm;
        sectionJSON[constants::BEATS_PER_MEASURE_KEY] = section.beatsPerMeasure;
        sectionJSON[constants::INTERPOLATE_BEAT_DURATION_KEY] = section.interpolateBeatDuration;

        timeinfo.push_back(sectionJSON);
    }

    return timeinfo;
}

void ChartInfo::saveChart(const fs::path & chartPath, SongPosition & songpos) {
    savePath = chartPath;

    auto chartinfoJSON = saveChartMetadata(songpos);
    chartinfoJSON[constants::TIMEINFO_KEY] = saveChartTimeInfo(songpos);

    ordered_json stopsJSON = ordered_json::array();
    ordered_json skipsJSON = ordered_json::array();
    ordered_json notesJSON = ordered_json::array();

    std::sort(notes.myItems.begin(), notes.myItems.end());

    for(auto item : notes.myItems) {
        ordered_json itemJSON;
        ordered_json itemBeatpos = {item->beatpos.measure, item->beatpos.measureSplit, item->beatpos.split};

        switch(item->itemType) {
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
                    Note::NoteType currNoteType = currNote->noteType;

                    itemJSON[constants::POS_KEY] = itemBeatpos;

                    std::string keyText = currNote->displayText;
                    if (notemaps::FUNCTION_KEY_TO_STR.find(currNote->displayText) != notemaps::FUNCTION_KEY_TO_STR.end()) {
                        keyText = notemaps::FUNCTION_KEY_TO_STR.at(currNote->displayText);
                    }

                    itemJSON[constants::NOTE_KEY_KEY] = keyText;
                    itemJSON[constants::NOTE_TYPE_KEY] = static_cast<int>(currNoteType);
                    notesJSON.push_back(itemJSON);

                    // add release note if hold start
                    if(currNoteType == Note::NoteType::KEYHOLDSTART) {
                        ordered_json item2JSON;

                        item2JSON[constants::POS_KEY] = {currNote->endBeatpos.measure, currNote->endBeatpos.measureSplit, currNote->endBeatpos.split};
                        item2JSON[constants::NOTE_KEY_KEY] = keyText;
                        item2JSON[constants::NOTE_TYPE_KEY] = static_cast<int>(Note::NoteType::KEYHOLDRELEASE);
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
