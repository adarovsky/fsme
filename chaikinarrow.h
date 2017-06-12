#ifndef CHAIKINARROW_H
#define CHAIKINARROW_H

#include <QRectF>
#include <QVector>
#include <QGraphicsPathItem>

class ChaikinArrow : public QGraphicsPathItem {
protected:
    QRectF m_start;
    QRectF m_end;
    QVector<QPointF> m_controlPoints;
    QVector<QPointF> m_spline;
public:
    ChaikinArrow(QGraphicsItem *parent = Q_NULLPTR);

    enum { Type = UserType + 16 };
    int type() const Q_DECL_OVERRIDE { return Type;}

    QVector<QPointF> spline() const  { return m_spline; }

    void setStartRect( QRectF rect );
    void setEndRect( QRectF rect );
    void setControlPoints(const QVector<QPointF> & points );

    QRectF startRect() const { return m_start; }
    QRectF endRect() const { return m_end; }
    QVector<QPointF> controlPoints() const { return m_controlPoints; }

    void updateGeometry();
};


#endif // CHAIKINARROW_H
