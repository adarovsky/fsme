#ifndef DIAGRAMTRANSITIONMANIPULATOR_H
#define DIAGRAMTRANSITIONMANIPULATOR_H

#include <QGraphicsPathItem>
#include "manipfsm.h"

class DiagramTransitionItem;

class DiagramTransitionManipulator : public QGraphicsPathItem,
                                     public ManipFSM::Delegate
{
    ManipFSM m_fsm;
    DiagramTransitionItem * m_editingTransition;
    QPointF                 m_hoverPoint;
    QPointF                 m_pressPoint;
    QPointF                 m_mousePoint;
    bool                    m_hoverVisible;
    int                     m_editingSegmentIndex;
    int                     m_editingPointIndex;
public:
    virtual bool isDragPosReached(const QVariantMap& params) const Q_DECL_OVERRIDE;
    virtual bool isNearLine(const QVariantMap& params) const Q_DECL_OVERRIDE;
    virtual bool isNearPoint(const QVariantMap& params) const Q_DECL_OVERRIDE;


    virtual void addPoint(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void calculateLineIndex(const QVariantMap & params) Q_DECL_OVERRIDE;
    virtual void calculatePointIndex(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void movePoint(const QVariantMap& params) Q_DECL_OVERRIDE;
    virtual void optimize(const QVariantMap& params) Q_DECL_OVERRIDE;
    void commitPoints( const QVector<QPointF>& points );
protected:
    enum { Type = UserType + 100 };
    int type() const Q_DECL_OVERRIDE { return Type;}
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = nullptr, QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    QPointF alignedPoint( const QPointF& original ) const;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
public:
    DiagramTransitionManipulator();

    void editTransition( DiagramTransitionItem * item );
    DiagramTransitionItem * editedTransition() const { return m_editingTransition; }
};

#endif // DIAGRAMTRANSITIONMANIPULATOR_H
