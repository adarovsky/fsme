#ifndef EDITOREVENT_H
#define EDITOREVENT_H

#include "editorbase.h"

class StateMachine;
class Event;

namespace Ui {
    class EditorBase;
}

class EditorEvent : public QWidget,
                    public EditorBase
{
    Q_OBJECT
    Ui::EditorBase * ui;

    Event * object() const;
public:
    explicit EditorEvent(QWidget *parent = 0);

    virtual bool canStartEditing(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateUI() Q_DECL_OVERRIDE;

public slots:
};

#endif // EDITOREVENT_H
