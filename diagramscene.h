#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include "statemachine.h"
#include "diagramscenefsm.h"

class DiagramStateItem;
class DiagramTransitionItem;
class QItemSelectionModel;
class QItemSelection;
class DiagramTransitionManipulator;
class StateAddPlaceholder;
class QGraphicsLineItem;

class DiagramScene : public QGraphicsScene,
                     public DiagramSceneFSM::Delegate
{
    Q_OBJECT
    QWeakPointer<StateMachine> m_document;
    QItemSelectionModel * m_selectionModel;
    DiagramTransitionManipulator * m_manipilator;
    StateAddPlaceholder * m_statePlaceholder;
    QGraphicsLineItem *   m_transitionPlaceholder;
    bool m_selectionIsUpdating;
    DiagramSceneFSM fsm;
    enum EventHandle {
        Accept,
        Reject,
        Forward
    } m_eventHandle;

    // gets or creates manipulator
    DiagramTransitionManipulator * editTransition( DiagramTransitionItem * item );
public:
    explicit DiagramScene( QObject * parent = nullptr );
    void setStateMachine( QSharedPointer<StateMachine> fsm );

    void setItemSelectionModel( QItemSelectionModel * selectionModel );
    QItemSelectionModel * itemSelectionModel() const { return m_selectionModel; }

    DiagramStateItem * graphicState( QSharedPointer<State> state ) const;
    DiagramTransitionItem * graphicTransition(QSharedPointer<Transition> transition ) const;

    QList<DiagramTransitionItem*> graphicTransitionsIn( DiagramStateItem * state ) const;
    QList<DiagramTransitionItem*> graphicTransitionsOut( DiagramStateItem * state ) const;

public:
    virtual bool isPointingToState(const QVariantMap& params) const Q_DECL_OVERRIDE;
    virtual bool isStartingFromState(const QVariantMap& params) const Q_DECL_OVERRIDE;

    virtual void acceptEvent(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void addNameField(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void addState(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void addTransition(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void removeNameField(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void removePreviewArrow(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void selectState(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void setPointerToArrow(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void setPointerToCross(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void showPreviewArrow(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void updatePreviewArrow(const QVariantMap& params) Q_DECL_OVERRIDE;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
public:
    bool handleCancel(QKeyEvent * event);

public slots:
    void prepareToAddState();
    void prepareToAddTransition();
    void stateNameEntered(const QString& text);
    void cancelAdding();

    void modelAboutToBeReset();
    void modelReset();
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);

    // originated from selection model
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // originated from diagram
    void diagramSelectionChanged();
//    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
signals:
    void requestEditingItem( const QModelIndex& item );
};

#endif // DIAGRAMSCENE_H
