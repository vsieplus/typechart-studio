#ifndef EDITSKIP_HPP
#define EDITSKIP_HPP

#include "actions/editaction.hpp"
#include "config/notesequence.hpp"

class EditSkipAction : public EditAction {
    public:
        EditSkipAction(bool unsaved, float absBeat, float prevSkipbeats, float newSkipbeats);

        virtual void undoAction(EditWindowData * editWindow) override;
        virtual void redoAction(EditWindowData * editWindow) override;

    private:
        float absBeat;
        float prevSkipbeats;
        float newSkipbeats;
};

#endif // EDITSKIP_HPP