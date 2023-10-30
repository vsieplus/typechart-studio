#ifndef EDITSKIP_HPP
#define EDITSKIP_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class EditSkipAction : public EditAction {
    public:
        EditSkipAction(double absBeat, double prevSkipbeats, double newSkipbeats);

        void undoAction(EditWindow * editWindow) override;
        void redoAction(EditWindow * editWindow) override;

    private:
        double absBeat;
        double prevSkipbeats;
        double newSkipbeats;
};

#endif // EDITSKIP_HPP
