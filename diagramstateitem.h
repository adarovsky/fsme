#ifndef DIAGRAMSTATEITEM_H
#define DIAGRAMSTATEITEM_H

#include <QGraphicsRectItem>
#include "statemachine.h"

class DiagramStateItem : public QGraphicsRectItem
{
    QWeakPointer<State> m_state;
    bool m_updating;
public:
    DiagramStateItem(QSharedPointer<State> state, QGraphicsItem *parent = Q_NULLPTR);
    ~DiagramStateItem();

    enum { Type = UserType + 15 };
    int type() const Q_DECL_OVERRIDE { return Type;}

    QSharedPointer<State> state() const { return m_state; }
    void modelPositionChanged();
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
};

#endif // DIAGRAMSTATEITEM_H
