#include <QPainterPathStroker>
#include <QPainter>
#include <QtDebug>
#include <cmath>

#include "diagramtransitionitem.h"
#include "diagramscene.h"
#include "diagramstateitem.h"
#include "my_algo.h"

Q_DECLARE_LOGGING_CATEGORY(DiagramTransitionItemLogging)
Q_LOGGING_CATEGORY(DiagramTransitionItemLogging, "graphics.diagramscene.transition")

DiagramTransitionItem::DiagramTransitionItem(QSharedPointer<Transition> transition, QGraphicsItem *parent )
    : ChaikinArrow( parent ), m_transition(transition), m_arrowHead( 3 )
{
    setPen( QPen( QBrush(QColor(Qt::black)), 1) );

    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void DiagramTransitionItem::updateGeometry()
{
    QSharedPointer<Transition> t = m_transition;
    if (!t) return;

    QSharedPointer<State> source = t->parentState();
    QSharedPointer<State> destination = t->target ? t->target.toStrongRef() : source;
    if (!source) return;

    DiagramStateItem * sg = dynamic_cast<DiagramScene*>( scene() )->graphicState( source );
    DiagramStateItem * st = dynamic_cast<DiagramScene*>( scene() )->graphicState( destination );

    m_start = sg->rect();
    m_start.moveCenter( m_start.center() + sg->pos() );
    m_end = st->rect();
    m_end.moveCenter( m_end.center() + st->pos() );

    m_controlPoints = t->controlPoints();

    ChaikinArrow::updateGeometry();

    updateArrowGeometry();

    setZValue( -1000.f );
}

static qreal headLength = 20;
static qreal headWidth = 10;
void DiagramTransitionItem::updateArrowGeometry()
{
    if (m_spline.count() < 2) return;

    // calculate arrow position
    QLineF lastLine( m_spline.last(), m_spline[ m_spline.count() - 2 ] );

    qreal l = lastLine.length();
    if (l > 0) {
        double ph1x = lastLine.p1().x() + (lastLine.p2().x()-lastLine.p1().x())*headLength/l;
        double ph1y = lastLine.p1().y() + (lastLine.p2().y()-lastLine.p1().y())*headLength/l;
        double nx =  (lastLine.p2()-lastLine.p1()).y() * (headWidth/2.0) / l; // normal vector
        double ny = -(lastLine.p2()-lastLine.p1()).x() * (headWidth/2.0) / l; // normal vector

        m_arrowHead[0] = lastLine.p1();
        m_arrowHead[1] = QPointF(int(ph1x + nx), int(ph1y+ny));
        m_arrowHead[2] = QPointF(int(ph1x - nx), int(ph1y-ny));
    }
}

void DiagramTransitionItem::updatePoints( const QPointF& newHead, const QPointF& newTail )
{
    std::vector<double> k( m_controlPoints.count() );
    std::vector<double> d( m_controlPoints.count() );

    QLineF line( m_end.center(), m_start.center() );
    double ds = line.length();
    qCDebug(DiagramTransitionItemLogging)  << "updatePoints: (" << line.p2() << "," << line.p1() << ") -> (" << newHead << "," << newTail << ")";

    // it is a loop
    if ( qFuzzyCompare(ds + 1, 1) ) {

        for( int i = 0; i < m_controlPoints.count(); ++i ) {
            m_controlPoints[i]+= (newHead - line.p2());
        }

        auto t = transition();
        if (t) t->setControlPoints( m_controlPoints );

        return;
    }

    QPointF v = line.p1() - line.p2();

    // single vector
    double sx = v.x() / ds;
    double sy = v.y() / ds;

    for( int i = 0; i < m_controlPoints.count(); ++i ) {
        QPointF v = m_controlPoints[i] - line.p2();
        k[i] = (v.x()*sx + v.y()*sy) / ds;
        d[i] = cross_prod( 0, 0, sx, sy, v.x(), v.y() );
    }

    line.setP1( newHead );
    line.setP2( newTail );
    ds = line.length();

    v  = line.p1()-line.p2();
    sx = v.x() / ds;
    sy = v.y() / ds;

    for( int i = 0; i < m_controlPoints.count(); ++i ) {
        double newX = line.p2().x()     // start point
            + sx  *k[i]*ds  // distance parallel
            + -sy *d[i];    // distance perpendicular
        double newY = line.p2().y()    // start point
            + sy  *k[i]*ds  // distance parallel
            + sx  *d[i];    // distance perpendicular

        m_controlPoints[i] = QPointF(newX, newY);
    }

    auto t = transition();
    if (t) t->setControlPoints( m_controlPoints );
}

void DiagramTransitionItem::updateEnds()
{
    QSharedPointer<Transition> t = m_transition;
    if (!t) return;

    QSharedPointer<State> source = t->parentState();
    QSharedPointer<State> destination = t->target ? t->target.toStrongRef() : source;
    if (!source) return;

    DiagramStateItem * sg = dynamic_cast<DiagramScene*>( scene() )->graphicState( source );
    DiagramStateItem * sd = dynamic_cast<DiagramScene*>( scene() )->graphicState( destination );

    if (sg && sd) {
        updatePoints( sd->rect().center() + sd->pos(),  sg->rect().center() + sg->pos() );
        updateGeometry();
    }
}

QRectF DiagramTransitionItem::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;
    QRectF bounds = ChaikinArrow::boundingRect();

    return bounds
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath DiagramTransitionItem::shape() const
{
    QPainterPath p = ChaikinArrow::path();

    QPainterPathStroker stroker;
    stroker.setWidth( 20 );
    stroker.setJoinStyle(Qt::RoundJoin);

    QPainterPath path = ( stroker.createStroke( p ) ).simplified();

    return path;
}

void DiagramTransitionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_spline.count() < 2) return;

    painter->setPen( isSelected() ? QPen(Qt::DotLine) : pen() );
    painter->setBrush( Qt::NoBrush );
//    painter->setBrush( QBrush( QColor( Qt::black ) ) );

    QPainterPath path = ChaikinArrow::path();

    painter->drawPath( ChaikinArrow::path() );


    painter->setBrush( QBrush( isSelected() ? QColor( Qt::lightGray ) : QColor( Qt::black ) ) );
    painter->drawPolygon( m_arrowHead );

    if ( isSelected() )
        return;

    if ( path.angleAtPercent( 0.5 ) > 90 && path.angleAtPercent( 0.5 ) < 270 )
        path = path.toReversed();

    QString title = transition()->name();
    QFont f = painter->font();
    f.setPixelSize( 10 );
    painter->setFont( f );

    QFontMetrics m = painter->fontMetrics();
    qreal pathLength = path.length();
    qreal textLength = m.width( title );

    if ( textLength > pathLength - 22 ) {
        f.setPixelSize( std::floor(10 * (pathLength - 25) / textLength) );
        painter->setFont( f );
        m = painter->fontMetrics();
        textLength = m.width( title );
    }

    if (path.length() > 0 && textLength > 0) {
        qreal startPercent = 0.5 * (1 - textLength / (pathLength - 22));
        int i; qreal p;
        for (i = 0, p = startPercent; i < title.length(); ++i) {
            QPointF point = path.pointAtPercent(p);
            qreal angle = path.angleAtPercent(p);

            QString letter = QString(title[i]);
            p += m.width( letter ) / pathLength;

            painter->save();
            painter->translate(point);
            painter->rotate(-angle);
            painter->drawText( QPoint(0, -2), letter );
            painter->restore();
        }
    }
}
