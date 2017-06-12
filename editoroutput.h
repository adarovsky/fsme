#ifndef EDITOROUTPUT_H
#define EDITOROUTPUT_H

#include "editorbase.h"

class StateMachine;
class Output;

namespace Ui {
    class EditorBase;
}

class EditorOutput : public QWidget,
                    public EditorBase
{
    Q_OBJECT
    Ui::EditorBase * ui;

    Output * output() const;
public:
    explicit EditorOutput(QWidget *parent = 0);

    virtual bool canStartEditing(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateUI() Q_DECL_OVERRIDE;

public slots:
};

#endif // EDITOROUTPUT_H
