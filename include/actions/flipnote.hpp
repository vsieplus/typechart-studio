#ifndef FLIPNOTE_HPP
#define FLIPNOTE_HPP

#include <string>
#include <string_view>

#include "actions/editaction.hpp"

class FlipNoteAction : public EditAction {
    public:
        FlipNoteAction(int minItemType, int maxItemType, double startBeat, double endBeat, std::string_view keyboardLayout);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;
    private:
        int minItemType;
        int maxItemType;

        double startBeat;
        double endBeat;

        std::string keyboardLayout;
};

#endif // FLIPNOTE_HPP
