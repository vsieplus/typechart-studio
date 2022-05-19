#ifndef EDITACTION_HPP
#define EDITACTION_HPP

class EditWindowData;

class EditAction {
    public:
        EditAction(bool & unsaved) : unsaved(unsaved), wasUnsaved(unsaved) {}

        virtual void undoAction(EditWindowData * editWindow);
        virtual void redoAction(EditWindowData * editWindow) = 0;
    private:
        bool & unsaved;
        bool wasUnsaved;
};

#endif // EDITACTION_HPP