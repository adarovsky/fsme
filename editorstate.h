#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include <QWidget>
#include "editorbase.h"
class State;
class AvailableActionsModel;
namespace Ui {
    class EditorState;
}


class EditorState : public QWidget,
                    public EditorBase
{
    Q_OBJECT
    State * state() const;
    Ui::EditorState * ui;
    QScopedPointer<AvailableActionsModel> m_availableActions;
public:
    explicit EditorState(QWidget *parent = 0);
    virtual ~EditorState();

    virtual bool canStartEditing(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateUI() Q_DECL_OVERRIDE;

signals:

public slots:
    void inSelectionChanged();
    void outSelectionChanged();
    void availSelectionChanged();
    void moveInUp();
    void moveInDown();
    void moveOutUp();
    void moveOutDown();
    void addIn();
    void removeIn();
    void addOut();
    void removeOut();
};

#endif // EDITORSTATE_H
