#ifndef EDITACTION_HPP
#define EDITACTION_HPP

class EditAction {
    public:
        virtual void undoAction() = 0;
        virtual void redoAction() = 0;

};

#endif // EDITACTION_HPP