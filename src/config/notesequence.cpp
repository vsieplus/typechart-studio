#include "config/notesequence.hpp"

#include "config/notemaps.hpp"

void NoteSequence::update(double songBeat, AudioSystem * audioSystem, bool notesoundEnabled) {
    for(const auto & item : myItems) {
        switch(item->itemType) {
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                if(item->passed) {
                    break;
                } else if(item->absBeat < songBeat) {
                    item->passed = true;

                    if(notesoundEnabled) {
                        audioSystem->playSound("keypress");
                    }
                }
                break;
            case NoteSequenceItem::SequencerItemType::SKIP:
                break;
            case NoteSequenceItem::SequencerItemType::STOP:
                break;
        }
    }
}

void NoteSequence::resetPassed(double songBeat) {
    for(const auto & item : myItems) {
        switch(item->itemType) {
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                item->passed = item->absBeat < songBeat;
                break;
            case NoteSequenceItem::SequencerItemType::SKIP:
                break;
            case NoteSequenceItem::SequencerItemType::STOP:
                break;
        }
    }
}

void NoteSequence::addNote(double absBeat, double songBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos,
        NoteSequenceItem::SequencerItemType itemType, std::string_view displayText)
{
    NoteType noteType = NoteType::KEYPRESS;
    if(beatDuration > FLT_EPSILON) {
        noteType = NoteType::KEYHOLDSTART;
    }

    bool passed = absBeat < songBeat;

    std::shared_ptr<NoteSequenceItem> newNote = std::make_shared<Note>(absBeat, absBeat + beatDuration, passed, beatpos, endBeatpos, 
        noteType, NoteSplit::EIGHTH, itemType, displayText);
    myItems.push_back(newNote);

    std::sort(myItems.begin(), myItems.end());

    switch(itemType) {
        case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            numTopNotes++;
            break;
        case NoteSequenceItem::SequencerItemType::MID_NOTE:
            numMidNotes++;
            break;
        case NoteSequenceItem::SequencerItemType::BOT_NOTE:
            numBotNotes++;
            break;
        default:
            break;
    }

    keyFrequencies[displayText] += 1;

    updateKeyFrequencies();
}

void NoteSequence::editNote(double absBeat, NoteSequenceItem::SequencerItemType itemType, std::string_view displayText) {
    for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {

    std::for_each(myItems.begin(), myItems.end(), [&](auto seqItem) {
        if(seqItem->itemType == itemType && absBeat >= seqItem->absBeat &&
            (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {
            
            auto oldText = seqItem->displayText;
            seqItem->displayText = displayText;

            keyFrequencies[oldText] -= 1;
            keyFrequencies[displayText] += 1;

            updateKeyFrequencies();

            return;
        }
    });
}

void NoteSequence::addStop(double absBeat, double songBeat, double beatDuration, BeatPos beatpos, BeatPos endBeatpos) {
    bool passed { absBeat < songBeat };
    auto newStop { std::make_shared<Stop>(absBeat, beatDuration, passed, beatpos, endBeatpos) };
    newStop->displayText = std::to_string(beatDuration);
    myItems.push_back(newStop);

    std::sort(myItems.begin(), myItems.end());
}

std::shared_ptr<Skip> NoteSequence::addSkip(double absBeat, double songBeat, double skipTime, double beatDuration, BeatPos beatpos, BeatPos endBeatpos) {
    bool passed = absBeat < songBeat;
    auto newSkip { std::make_shared<Skip>(absBeat, skipTime, passed, beatDuration, beatpos, endBeatpos) };
    newSkip->displayText = std::to_string(skipTime);
    myItems.push_back(newSkip);

    std::sort(myItems.begin(), myItems.end());

    return newSkip;
}

void NoteSequence::editSkip(double absBeat, double skipTime) {
    std::for_each(myItems.begin(), myItems.end(), [&](auto seqItem) {
        if(seqItem->itemType == NoteSequenceItem::SequencerItemType::SKIP && absBeat >= seqItem->absBeat &&
            (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd))) {

            auto currSkip = std::dynamic_pointer_cast<Skip>(seqItem);
            currSkip->displayText = std::to_string(skipTime);
            currSkip->skipTime = skipTime;

            return;
        }
    });
}

void NoteSequence::flipNotes(std::string_view keyboardLayout, double startBeat, double endBeat, int minItemType, int maxItemType) {
    if(notemaps::KEYBOARD_FLIP_MAPS.find(keyboardLayout) != notemaps::KEYBOARD_FLIP_MAPS.end()) {
        auto & flipMap = notemaps::KEYBOARD_FLIP_MAPS.at(keyboardLayout);

        auto items = getItems(startBeat, endBeat, minItemType, maxItemType);
        for(auto item: items) {
            switch(item->itemType) {
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                    if(flipMap.find(item->displayText) != flipMap.end()) {
                        keyFrequencies[item->displayText] -= 1;
                        item->displayText = flipMap.at(item->displayText);
                        keyFrequencies[item->displayText] += 1;
                    }
                    break;
                default:
                    break;
            }
        }

        updateKeyFrequencies();
    }
}

std::list<std::shared_ptr<NoteSequenceItem>> NoteSequence::shiftNotes(std::string_view keyboardLayout, double startBeat, double endBeat,
    int minItemType, int maxItemType, ShiftNoteAction::ShiftDirection shiftDirection)
{
    auto items = getItems(startBeat, endBeat, minItemType, maxItemType);
    return shiftItems(keyboardLayout, startBeat, endBeat, items, shiftDirection);
}

std::list<std::shared_ptr<NoteSequenceItem>> NoteSequence::shiftItems(std::string_view keyboardLayout, double startBeat, double endBeat,
    const std::list<std::shared_ptr<NoteSequenceItem>> & items, ShiftNoteAction::ShiftDirection shiftDirection)
{
    std::list<std::shared_ptr<NoteSequenceItem>> shiftedItems;

    if (notemaps::KEYBOARD_LAYOUTS.find(keyboardLayout) != notemaps::KEYBOARD_LAYOUTS.end() &&
        notemaps::KEYBOARD_POSITION_MAPS.find(keyboardLayout) != notemaps::KEYBOARD_POSITION_MAPS.end()) 
    {
        for(auto item: items) {
            if(shiftNoteSequenceItem(shiftDirection, item, keyboardLayout)) {
                shiftedItems.push_back(item);
            }
        }

        updateKeyFrequencies();
    }

    return shiftedItems;
}

bool NoteSequence::shiftNoteSequenceItem(ShiftNoteAction::ShiftDirection shiftDirection, std::shared_ptr<NoteSequenceItem> item, std::string_view keyboardLayout) {
    auto & keyboardLayoutMap = notemaps::KEYBOARD_LAYOUTS.at(keyboardLayout);
    auto & keyboardPositionMap = notemaps::KEYBOARD_POSITION_MAPS.at(keyboardLayout);

    auto itemKey = item->displayText;
    auto itemType = item->itemType;

    switch(itemType) {
        case NoteSequenceItem::SequencerItemType::TOP_NOTE:
        case NoteSequenceItem::SequencerItemType::MID_NOTE:
        case NoteSequenceItem::SequencerItemType::BOT_NOTE:
            if(keyboardPositionMap.find(itemKey) != keyboardPositionMap.end()) {
                auto keyPos = keyboardPositionMap.at(itemKey);
                int keyRow = keyPos.first;
                int keyCol = keyPos.second;

                int newRow = keyRow;
                int newCol = keyCol;
                switch(shiftDirection) {
                    case ShiftNoteAction::ShiftDirection::ShiftUp:
                        newRow = keyRow - 1;
                        break;
                    case ShiftNoteAction::ShiftDirection::ShiftDown:
                        newRow = keyRow + 1;
                        break;
                    case ShiftNoteAction::ShiftDirection::ShiftLeft:
                        newCol = keyCol - 1;
                        break;
                    case ShiftNoteAction::ShiftDirection::ShiftRight:
                        newCol = keyCol + 1;
                        break;
                    default:
                        break;
                }

                newRow = std::max(0, std::min(newRow, 3));
                newCol = std::max(0, std::min(newCol, 9));

                if(newRow == keyRow && newCol == keyCol) {
                    return false;
                }

                if(itemType == NoteSequenceItem::SequencerItemType::TOP_NOTE && newRow > 0) {
                    item->itemType = NoteSequenceItem::SequencerItemType::MID_NOTE;
                }

                if(itemType == NoteSequenceItem::SequencerItemType::MID_NOTE && newRow < 1) {
                    item->itemType = NoteSequenceItem::SequencerItemType::TOP_NOTE;
                }

                auto newKey = keyboardLayoutMap[newRow][newCol];
                keyFrequencies[itemKey] -= 1;
                item->displayText = newKey;
                keyFrequencies[newKey] += 1;
            }
            break;
        default:
            break;
    }

    return true;
}

std::list<std::shared_ptr<NoteSequenceItem>> NoteSequence::getItems(double startBeat, double endBeat, int minItemType, int maxItemType) {
    std::list<std::shared_ptr<NoteSequenceItem>> currItems;

    for(auto iter = myItems.begin(); iter != myItems.end(); iter++) {
        auto & seqItem = *iter;

        int seqItemType = (int)(seqItem->itemType);

        if(seqItemType >= minItemType && seqItemType <= maxItemType && startBeat <= seqItem->absBeat && seqItem->absBeat <= endBeat) {
            currItems.push_back(seqItem);
        } else if(endBeat <= seqItem->absBeat) {
            break;
        }
    }

    return currItems;
}

std::shared_ptr<NoteSequenceItem> NoteSequence::containsItemAt(double absBeat, NoteSequenceItem::SequencerItemType itemType) {
    for(auto seqItem : myItems) {
        if(seqItem->itemType == itemType && absBeat >= seqItem->absBeat &&
            (absBeat < seqItem->beatEnd || (seqItem->absBeat == seqItem->beatEnd && absBeat <= seqItem->beatEnd)))
        {
            return seqItem;
        }
    }

    return nullptr;
}

int NoteSequence::getLaneItemCount(NoteSequenceItem::SequencerItemType lane) const {
    switch(lane) {
        case NoteSequenceItem::SequencerItemType::TOP_NOTE:
            return numTopNotes;
        case NoteSequenceItem::SequencerItemType::MID_NOTE:
            return numMidNotes;
        case NoteSequenceItem::SequencerItemType::BOT_NOTE:
            return numBotNotes;
        default:
            return 0;
    }
}

void NoteSequence::resetItemCounts() {
    numTopNotes = 0;
    numMidNotes = 0;
    numBotNotes = 0;

    for(auto item : myItems) {
        bool insertedKey = false;

        switch(item->itemType) {
            case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                numTopNotes++;
                insertedKey = true;
                break;
            case NoteSequenceItem::SequencerItemType::MID_NOTE:
                numMidNotes++;
                insertedKey = true;
                break;
            case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                numBotNotes++;
                insertedKey = true;
                break;
            default:
                break;
        }

        if(insertedKey) {
            keyFrequencies[item->displayText] += 1;
        }
    }

    updateKeyFrequencies();
}

void NoteSequence::insertItems(double insertBeat, double songBeat, int minItemType, int maxItemType,
    const std::vector<Timeinfo> & timeinfo, std::list<std::shared_ptr<NoteSequenceItem>> items)
{
    if(!items.empty()) {
        double firstBeat = items.front()->absBeat;
        BeatPos firstBeatPos = items.front()->beatpos;
        BeatPos insertBeatPos = utils::calculateBeatpos(insertBeat, firstBeatPos.measureSplit, timeinfo);

        deleteItems(insertBeat, insertBeat + (items.back()->beatEnd - firstBeat), minItemType, maxItemType);
        
        for(auto item : items) {
            double currBeat = insertBeat + (item->absBeat - firstBeat);

            BeatPos currBeatPos = insertBeatPos + (item->beatpos - firstBeatPos);
            BeatPos currEndBeatPos = insertBeatPos + (item->endBeatpos - firstBeatPos);

            switch(item->itemType) {
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                    addNote(currBeat, songBeat, item->beatEnd - item->absBeat, currBeatPos, currEndBeatPos, item->itemType, item->displayText);
                    break;
                case NoteSequenceItem::SequencerItemType::STOP:
                    addStop(currBeat, songBeat, item->beatEnd - item->absBeat, currBeatPos, currEndBeatPos);
                    break;
                case NoteSequenceItem::SequencerItemType::SKIP:
                    auto currSkip = std::dynamic_pointer_cast<Skip>(item);
                    addSkip(currBeat, songBeat, currSkip->skipTime, item->beatEnd - item->absBeat, currBeatPos, currEndBeatPos);
                    break;
            }
        }
    }
}

 void NoteSequence::deleteItems(double startBeat, double endBeat, int minItemType, int maxItemType) {
    for(auto iter = myItems.begin(); iter != myItems.end();) {
        bool removeNote = false;
        const auto & seqItem = *iter;

        auto seqItemType = static_cast<int>(seqItem->itemType);

        if(seqItemType >= minItemType && seqItemType <= maxItemType && startBeat <= seqItem->absBeat && seqItem->absBeat <= endBeat) {
            switch(seqItem->itemType) {
                case NoteSequenceItem::SequencerItemType::TOP_NOTE:
                    numTopNotes--;
                    removeNote = true;
                    break;
                case NoteSequenceItem::SequencerItemType::MID_NOTE:
                    numMidNotes--;
                    removeNote = true;
                    break;
                case NoteSequenceItem::SequencerItemType::BOT_NOTE:
                    numBotNotes--;
                    removeNote = true;
                    break;
                default:
                    break;
            }

            if(removeNote) {
                keyFrequencies[seqItem->displayText] -= 1;
                if(keyFrequencies[seqItem->displayText] == 0) {
                    keyFrequencies.erase(seqItem->displayText);
                }
            }

            // in case dangling pointers in undo/redo stack refer to this item
            seqItem->deleted = true;

            iter = myItems.erase(iter);
        } else {
            iter++;
        }
    }

    updateKeyFrequencies();
}

void deleteItem(double absBeat, NoteSequenceItem::SequencerItemType itemType) {
    deleteItems(absBeat, absBeat, itemType, itemType);
}

const std::pair<std::string, int> & NoteSequence::getKeyItemData(int frequencyRank) const {
    return keyFreqsSorted.at(frequencyRank);
}

void NoteSequence::Get(int index, float** start, float** end, int* type, unsigned int* color, const char** displayText) override {
    auto item = myItems[index];

    if(color) {
        *color = 0xFFAA8080; // same color for everyone, return color based on type
    }

    if(start) {
        *start = &(item->absBeat);
    }

    if(end) {
        *end = &(item->beatEnd);
    }

    if(type) {
        *type = (int)(item->itemType);
    }

    if(displayText) {
        *displayText = item->displayText.c_str();
    }
}
