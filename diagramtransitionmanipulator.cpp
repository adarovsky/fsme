#include <QPen>
#include <QBrush>
#include <QtDebug>
#include <QPainter>
#include <QUndoCommand>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#include <limits>
#include <cmath>

#include "diagramtransitionmanipulator.h"
#include "diagramtransitionitem.h"
#include "transitionundocommand.h"
#include "diagramscene.h"
#include "statemachine.h"
#include "my_algo.h"

DiagramTransitionManipulator::DiagramTransitionManipulator()
    : QGraphicsPathItem(), m_fsm(this), m_editingTransition( nullptr ), m_hoverPoint(), m_hoverVisible( false )
{
    setAcceptHoverEvents( true );
    setPen( QPen( QBrush( Qt::black ), 3) );
}

void DiagramTransitionManipulator::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverVisible = true;
    m_hoverPoint = alignedPoint( event->pos() );
    QGraphicsPathItem::hoverEnterEvent( event );
}

void DiagramTransitionManipulator::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverPoint = alignedPoint( event->pos() );
    QGraphicsPathItem::hoverMoveEvent( event );
    update();
}

void DiagramTransitionManipulator::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverVisible = false;
    QGraphicsPathItem::hoverLeaveEvent( event );
}

void DiagramTransitionManipulator::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressPoint =
    m_mousePoint = event->pos();
    m_hoverVisible = false;

    m_fsm.processEvent( ManipFSM::MousePressed, QVariantMap { {"pos", m_mousePoint} } );
}

void DiagramTransitionManipulator::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePoint = event->pos();

    m_fsm.processEvent( ManipFSM::MouseMoved, QVariantMap { {"pos", m_mousePoint} } );
}

void DiagramTransitionManipulator::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePoint = event->pos();

    m_fsm.processEvent( ManipFSM::MouseReleased, QVariantMap { {"pos", m_mousePoint} } );
}

void DiagramTransitionManipulator::editTransition(DiagramTransitionItem *item)
{
    m_editingTransition = item;

    if ( item ) {
        QPainterPath path;
        path.moveTo( item->startRect().center() );
        foreach ( auto p, item->controlPoints() ) {
            path.lineTo( p );
        }
        path.lineTo( item->endRect().center() );
        setZValue( -500.0 );

        setPath( path );
    }

    setVisible( item != nullptr );
}

QRectF DiagramTransitionManipulator::boundingRect() const
{
    qreal extra = (pen().width() + 20) / 2.0;
    QRectF bounds = QGraphicsPathItem::boundingRect();

    return bounds
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath DiagramTransitionManipulator::shape() const
{
    QPainterPath p = QGraphicsPathItem::path();

    QPainterPathStroker stroker;
    stroker.setWidth( 20 );
    stroker.setJoinStyle(Qt::RoundJoin);

    QPainterPath path = ( stroker.createStroke( p ) ).simplified();

    return path;
}

QPointF DiagramTransitionManipulator::alignedPoint( const QPointF& original ) const
{
    auto controlPoints = m_editingTransition->controlPoints();
    controlPoints.prepend( m_editingTransition->startRect().center() );
    controlPoints.append( m_editingTransition->endRect().center() );

    QPointF result = controlPoints.first();
    qreal minimumDistance = std::numeric_limits<qreal>::max();

    for( int i = 0; i < controlPoints.count() - 1; ++i ) {
        QLineF line( controlPoints[ i + 1 ], controlPoints[ i ] );
        QPointF v1 = original - line.p2();

        if ( sqrt(v1.x() * v1.x() + v1.y() * v1.y()) <= 10 )
            return line.p2();

        double ds = line.length();

        if ( qFuzzyCompare(ds + 1, 1) ) {
            continue;
        }

        QPointF v = line.p1() - line.p2();

        // single vector
        double sx = v.x() / ds;
        double sy = v.y() / ds;

        qreal k = (v1.x()*sx + v1.y()*sy) / ds;
        qreal d = cross_prod( 0, 0, sx, sy, v1.x(), v1.y() );

        if ( k >= 0 && k <= 1 && fabs(d) < minimumDistance ) {
            minimumDistance = fabs(d);
            result = line.p2() + QPointF( v.x() * k, v.y() * k );
        }
    }

    return result;
}

void DiagramTransitionManipulator::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    QGraphicsPathItem::paint( painter, option, widget );
    if( !m_editingTransition ) return;

    painter->setBrush( QBrush( Qt::black ) );
    foreach (auto cp, m_editingTransition->controlPoints()) {
        if ( m_hoverVisible && QLineF(cp, m_hoverPoint).length() < 10 )
            painter->drawEllipse( cp, 5, 5 );
        else
            painter->drawEllipse( cp, 3, 3 );
    }

    if ( m_hoverVisible ) {
        painter->drawEllipse( m_hoverPoint, 2, 2 );
    }
}


// FSM

bool DiagramTransitionManipulator::isDragPosReached(const QVariantMap& /*params*/) const
{
    return dist(m_pressPoint, m_mousePoint) > qApp->startDragDistance();
}

bool DiagramTransitionManipulator::isNearLine(const QVariantMap& /*params*/) const
{
    auto controlPoints = m_editingTransition->controlPoints();
    controlPoints.prepend( m_editingTransition->startRect().center() );
    controlPoints.append( m_editingTransition->endRect().center() );

    for( int i = 0; i < controlPoints.count() - 1; ++i ) {
        if( is_close(controlPoints[i], controlPoints[i + 1], m_mousePoint, 10) )
            return true;
    }

    return false;
}

bool DiagramTransitionManipulator::isNearPoint(const QVariantMap & /*params*/) const
{
    foreach (auto p, m_editingTransition->controlPoints()) {
        if( dist(m_mousePoint, p) < 10)
            return true;
    }

    return false;
}

void DiagramTransitionManipulator::addPoint(const QVariantMap& /*params*/)
{
    auto a = m_editingTransition->controlPoints();

    QVector<QPointF> n;
    for( int i = 0; i < m_editingSegmentIndex; ++i )
        n += a[ i ];

    n += m_mousePoint;
    m_editingPointIndex = n.count() - 1;

    for( int i = m_editingSegmentIndex; i < a.count(); ++i )
        n += a[ i ];

    commitPoints( n );
}

void DiagramTransitionManipulator::calculateLineIndex(const QVariantMap& /*params*/)
{
    auto controlPoints = m_editingTransition->controlPoints();
    controlPoints.prepend( m_editingTransition->startRect().center() );
    controlPoints.append( m_editingTransition->endRect().center() );

    m_editingSegmentIndex = 0;
    qreal minimumDistance = std::numeric_limits<qreal>::max();

    for( int i = 0; i < controlPoints.count() - 1; ++i ) {
        QLineF line( controlPoints[ i + 1 ], controlPoints[ i ] );
        QPointF v1 = m_mousePoint - line.p2();

        double ds = line.length();

        if ( qFuzzyCompare(ds + 1, 1) ) {
            continue;
        }

        QPointF v = line.p1() - line.p2();

        // single vector
        double sx = v.x() / ds;
        double sy = v.y() / ds;

        qreal k = (v1.x()*sx + v1.y()*sy) / ds;
        qreal d = cross_prod( 0, 0, sx, sy, v1.x(), v1.y() );

        if ( k >= 0 && k <= 1 && fabs(d) < minimumDistance ) {
            minimumDistance = fabs(d);
            m_editingSegmentIndex = i;
        }
    }
}

void DiagramTransitionManipulator::calculatePointIndex(const QVariantMap& /*params*/)
{
    auto controlPoints = m_editingTransition->controlPoints();

    m_editingPointIndex = 0;
    qreal minimumDistance = std::numeric_limits<qreal>::max();

    for( int i = 0; i < controlPoints.count(); ++i) {
        auto p = controlPoints[ i ];
        if( minimumDistance > dist(p, m_mousePoint) ) {
            m_editingPointIndex = i;
            minimumDistance = dist(p, m_mousePoint);
        }
    }
}

void DiagramTransitionManipulator::movePoint(const QVariantMap & /*params*/)
{
    auto controlPoints = m_editingTransition->controlPoints();

    controlPoints[ m_editingPointIndex ] = m_mousePoint;
    commitPoints( controlPoints );
}

void DiagramTransitionManipulator::optimize(const QVariantMap & /*params*/)
{
    auto p = m_editingTransition->controlPoints();

    auto lp = p;
    lp.prepend( m_editingTransition->startRect().center() );
    lp.append( m_editingTransition->endRect().center() );

    QVector<QPointF>::iterator prev, next, cur;
    prev = cur = next = lp.begin();
    if( ++cur, ++next == lp.end() )
        return;
    ++next;
    while( next != lp.end() ) {
        double l1 = dist(*prev, *cur);
        double l2 = dist(*cur, *next);

        double cp = fabs(cross_prod( *prev, *cur, *next - *cur + *prev ));

        if( cp / (l1*l2) < asin(10*3.1415926/180) ||
            dist(*prev, *cur) < 10 ) {
            if( (cur = lp.erase(cur)) == lp.end() )
                break;
        }
        else
            prev = cur++;
        next = cur; ++next;
    }

    if( (!qFuzzyCompare( QLineF(lp.first(), lp.last()).length() + 1, 1) || lp.count() > 2) &&  p.count() + 2 != lp.count() ) {
        p.resize( lp.count() - 2);

        std::copy( lp.constBegin() + 1, lp.constEnd() - 1, p.begin() );
    }

    commitPoints( p );
}

void DiagramTransitionManipulator::commitPoints( const QVector<QPointF>& points )
{
    TransitionTransformCommand * cmd = new TransitionTransformCommand( m_editingTransition->transition().data(), points );
    m_editingTransition->transition()->parent()->undoStack()->push( cmd );
}
