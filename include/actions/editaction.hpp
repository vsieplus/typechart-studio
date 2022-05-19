#ifndef EDITACTION_HPP
#define EDITACTION_HPP

class EditAction {
    public:
        EditAction(bool & unsaved) : unsaved(unsaved), wasUnsaved(unsaved) {}

        virtual void undoAction() {
            unsaved = wasUnsaved;
        }

        virtual void redoAction() = 0;
    private:
        bool & unsaved;
        bool wasUnsaved;
};

#endif // EDITACTION_HPP