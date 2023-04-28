#ifndef FLIPNOTE_HPP
#define FLIPNOTE_HPP

#include <string>

#include "actions/editaction.hpp"

class FlipNoteAction : public EditAction {
    public:
        FlipNoteAction(bool unsaved, int minItemType, int maxItemType, float startBeat,
            float endBeat, std::string keyboardLayout);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;
    private:
        int minItemType, maxItemType;
        float startBeat, endBeat;

        std::string keyboardLayout;
};

#endif // FLIPNOTE_HPP
