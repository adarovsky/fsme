#include "chaikinarrow.h"

static QVector<QPointF> make_chaikin_step( const QVector<QPointF>& arr )
{
    QVector<QPointF> ch( 2*(arr.count()-2)+2 );
    int j = 0;
    for( int i = 0; i < arr.count(); ++i ) {
        if( i == 0 || i == arr.count() - 1 )
            ch[j++] = arr[i];
        else {
            const QPointF& p1 = arr[i-1];
            const QPointF& p2 = arr[i];
            const QPointF& p3 = arr[i+1];
            ch[j++] = QPointF( 3*p2.x() + p1.x(), 3*p2.y() + p1.y() ) / 4;
            ch[j++] = QPointF( 3*p2.x() + p3.x(), 3*p2.y() + p3.y() ) / 4;
        }
    }
    return ch;
}

static bool intersects(const QRectF& r, const QLineF& line, QPointF * outPoint)
{
    return (QLineF(r.topLeft(),     r.topRight()   ).intersect(line, outPoint) == QLineF::BoundedIntersection) ||
           (QLineF(r.topRight(),    r.bottomRight()).intersect(line, outPoint) == QLineF::BoundedIntersection) ||
           (QLineF(r.bottomRight(), r.bottomLeft() ).intersect(line, outPoint) == QLineF::BoundedIntersection) ||
           (QLineF(r.bottomLeft(),  r.topLeft()    ).intersect(line, outPoint) == QLineF::BoundedIntersection);
}

static QPointF neck( const QVector<QPointF>& arr )
{
    if ( arr.count() < 2)
        return QPointF();

    return arr[ arr.count() - 2 ];
}

ChaikinArrow::ChaikinArrow(QGraphicsItem *parent )
    : QGraphicsPathItem( parent )
{

}

void ChaikinArrow::setStartRect(QRectF rect)
{
    m_start = rect;
    updateGeometry();
}

void ChaikinArrow::setEndRect( QRectF rect )
{
    m_end = rect;
    updateGeometry();
}

void ChaikinArrow::setControlPoints( const QVector<QPointF>& points )
{
    m_controlPoints = points;
    updateGeometry();
}

void ChaikinArrow::updateGeometry()
{
    QVector<QPointF> spline;
    spline += m_start.center();
    spline += m_controlPoints;
    spline += m_end.center();

    for( int step = 0; step < 3; ++step )
        spline = make_chaikin_step( spline );


    while( spline.count() >= 2 ) {
        if ( m_start.contains(spline[0]) && m_start.contains( spline[1] ) ) {
            spline.removeFirst();
        }
        else if ( m_start.contains(spline[0]) && !m_start.contains( spline[1] ) ) {
            QLineF firstLine( spline[0], spline[1] );
            QPointF firstPoint;
            bool ok = intersects( m_start, firstLine, &firstPoint );
            Q_ASSERT(ok);

            spline.removeFirst();
            // if last line is too short, remove two points instead of one and use intersection point as final
            if ( QLineF(firstLine.p2(), firstPoint).length() < 20 )
                spline.removeFirst();

            spline.prepend( firstPoint );
            break;
        }
        else
            break;
    }

    while( spline.count() >= 2 ) {
        if ( m_end.contains(spline.last()) && m_end.contains( neck(spline) ) ) {
            spline.removeLast();
        }
        else if ( m_end.contains(spline.last()) && !m_end.contains( neck(spline) ) ) {
            QLineF lastLine( spline.last(), neck(spline) );
            QPointF lastPoint;
            bool ok = intersects( m_end, lastLine, &lastPoint );
            Q_ASSERT(ok);

            spline.removeLast();
            // if last line is too short, remove two points instead of one and use intersection point as final
            if ( QLineF(lastLine.p2(), lastPoint).length() < 20 )
                spline.removeLast();

            spline += lastPoint;
            break;
        }
        else
            break;
    }

    m_spline = spline;

    QPainterPath path;
    QVector<QPointF>::ConstIterator i = spline.constBegin();
    path.moveTo( *i++ );

    for (; i != spline.constEnd(); ++i)
        path.lineTo( *i );

    setPath(path);
}
