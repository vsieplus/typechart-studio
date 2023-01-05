#ifndef SHIFTNOTE_HPP
#define SHIFTNOTE_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "actions/editaction.hpp"

const std::unordered_map<std::string, std::vector<std::vector<std::string>>> KEYBOARD_LAYOUTS = {
    { "QWERTY", {
        { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
        { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" },
        { "A", "S", "D", "F", "G", "H", "J", "K", "L", ";" },
        { "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/" },
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

        ShiftNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat,
            float endBeat, std::string keyboardLayout, ShiftDirection shiftDirection);
        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        int minItemType, maxItemType;
        float startBeat, endBeat;

        std::string keyboardLayout;

        ShiftDirection shiftDirection;
};

#endif // SHIFTNOTE_HPP
