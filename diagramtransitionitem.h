#ifndef DIAGRAMTRANSITIONITEM_H
#define DIAGRAMTRANSITIONITEM_H
#include "chaikinarrow.h"
#include "statemachine.h"

class DiagramTransitionItem : public ChaikinArrow {
    QWeakPointer<Transition> m_transition;
    QPolygonF m_arrowHead;
    void updateArrowGeometry();
    void updatePoints( const QPointF& newHead, const QPointF& newTail );
public:
    DiagramTransitionItem(QSharedPointer<Transition> transition, QGraphicsItem *parent = Q_NULLPTR);

    enum { Type = UserType + 17 };
    int type() const Q_DECL_OVERRIDE { return Type;}
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

    QSharedPointer<Transition> transition() const { return m_transition; }

    void updateEnds();

    void updateGeometry();
};

#endif // DIAGRAMTRANSITIONITEM_H
