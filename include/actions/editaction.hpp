#ifndef EDITACTION_HPP
#define EDITACTION_HPP

class EditWindow;

class EditAction {
    public:
        EditAction(bool & unsaved) : unsaved(unsaved), wasUnsaved(unsaved) {}

        virtual void undoAction(EditWindow * editWindow);
        virtual void redoAction(EditWindow * editWindow) = 0;
    private:
        bool & unsaved;
        bool wasUnsaved;
};

#endif // EDITACTION_HPP
