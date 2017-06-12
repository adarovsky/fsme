#ifndef EDITORINPUT_H
#define EDITORINPUT_H

#include "editorbase.h"

class StateMachine;
class Input;

namespace Ui {
    class EditorBase;
}

class EditorInput : public QWidget,
                    public EditorBase
{
    Q_OBJECT
    Ui::EditorBase * ui;

    Input * input() const;
public:
    explicit EditorInput(QWidget *parent = 0);

    virtual bool canStartEditing(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateUI() Q_DECL_OVERRIDE;

public slots:
};

#endif // EDITORINPUT_H
