#ifndef EDITORTRANSITION_H
#define EDITORTRANSITION_H

#include <QWidget>
#include "editorbase.h"
#include "statemachine.h"

namespace Ui {
class EditorTransition;
}

class AvailableActionsModel;
class EventsInputsModel;
class EIOHighlighter;
class QCompleter;
class TransitionTargetsModel;
class EditorTransition : public QWidget,
                         public EditorBase
{
    Q_OBJECT
public:
    explicit EditorTransition(QWidget *parent = 0);
    ~EditorTransition();

    virtual bool canStartEditing(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateUI() Q_DECL_OVERRIDE;

protected:
    void changeEvent(QEvent *e) Q_DECL_OVERRIDE;

public slots:
    void availSelectionChanged();
    void actionSelectionChanged();
    void moveActionUp();
    void moveActionDown();
    void addAction();
    void removeAction();
    void moveTransitionUp();
    void moveTransitionDown();
    void setDestination( const QString& );

private:
    QSharedPointer<Transition> transition() const;
    Ui::EditorTransition *ui;
    bool m_editing;
    QScopedPointer<AvailableActionsModel> m_availableActions;
    QScopedPointer<EventsInputsModel> m_eventsInputs;
    QScopedPointer<TransitionTargetsModel> m_targets;
    QScopedPointer<EIOHighlighter> m_syntaxHighlighter;
    QScopedPointer<QCompleter> m_completer;
};

#endif // EDITORTRANSITION_H
