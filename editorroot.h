#ifndef EDITORROOT_H
#define EDITORROOT_H

#include "editorbase.h"

class StateMachine;
class Root;

namespace Ui {
    class EditorRoot;
}

class EditorRoot : public QWidget,
                   public EditorBase
{
    Q_OBJECT
    Ui::EditorRoot * ui;

    Root * object() const;
public:
    explicit EditorRoot(QWidget *parent = 0);

    virtual bool canStartEditing(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateUI() Q_DECL_OVERRIDE;

public slots:
};

#endif // EDITORROOT_H
