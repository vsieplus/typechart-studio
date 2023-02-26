#ifndef SHIFTNOTE_HPP
#define SHIFTNOTE_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "actions/editaction.hpp"
#include "config/notesequenceitem.hpp"

const std::unordered_map<std::string, std::vector<std::vector<std::string>>> KEYBOARD_LAYOUTS = {
    { "QWERTY", {
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
        { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" },
        { "A", "S", "D", "F", "G", "H", "J", "K", "L", ";" },
        { "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/" },
    }},
    { "DVORAK", {
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
        { "'", ",", ".", "P", "Y", "F", "G", "C", "R", "L" },
        { "A", "O", "E", "U", "I", "D", "H", "T", "N", "S" },
        { ";", "Q", "J", "K", "X", "B", "M", "W", "V", "Z" },
    }},
    { "COLEMAK", {
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
        { "Q", "W", "F", "P", "G", "J", "L", "U", "Y", ";" },
        { "A", "R", "S", "T", "D", "H", "N", "E", "I", "O" },
        { "Z", "X", "C", "V", "B", "K", "M", ",", ".", "/" },
    }},
    { "AZERTY", {
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
        { "A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P" },
        { "Q", "S", "D", "F", "G", "H", "J", "K", "L", "M" },
        { "W", "X", "C", "V", "B", "N", ",", ";", ":", "!" },
    }}
};

class ShiftNoteAction : public EditAction {
    public:
        enum ShiftDirection {
            ShiftUp,
            ShiftDown,
            ShiftLeft,
            ShiftRight,
            ShiftNone
        };

        ShiftNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat, float endBeat, std::string keyboardLayout,
            ShiftDirection shiftDirection, std::vector<std::shared_ptr<NoteSequenceItem>> items);
        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        int minItemType, maxItemType;
        float startBeat, endBeat;

        std::string keyboardLayout;

        ShiftDirection shiftDirection;

        std::vector<std::shared_ptr<NoteSequenceItem>> items;
};

#endif // SHIFTNOTE_HPP
