#ifndef EDITACTION_HPP
#define EDITACTION_HPP

class EditWindow;

class EditAction {
public:
    EditAction() = default;
    virtual ~EditAction() = default;

    virtual void undoAction(EditWindow * editWindow) = 0;
    virtual void redoAction(EditWindow * editWindow) = 0;
};

#endif // EDITACTION_HPP
