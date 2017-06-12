/*
FSME - A State Machine Editor
Copyright (c) 2003-2005 Alexander Darovsky

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <QFontMetricsF>

#include "qdombasic.h"
#include "qdomqvaluelist.h"
#include "gsavers.h"
#include "statemachine.h"
#include "diagramview.h"
#include "qvector.h"

IMPLEMENT_ABSTRACT_STREAMABLE( GSaver )

IMPLEMENT_STREAMABLE1( GStateSaver, GSaver )
IMPLEMENT_STORED_VARS2( GStateSaver, stateName, position )
IMPLEMENT_STREAMABLE1( GTransitionSaver, GSaver )
IMPLEMENT_STORED_VARS3( GTransitionSaver, stateName, transition, points )


bool operator >> ( QDomNode& element, QPointF& value )
{
    Q_ASSERT( element.nodeName() == "QPoint" );
    QDomNode nx = element.firstChild();
    int x, y;
    bool ok = (nx >> x) && (nx >> y);
    if( ok ) {
        value = QPointF( x, y );
        element = element.nextSibling();
    }
    return ok;
}

QDomNode& operator << ( QDomNode& n, const QPointF& value )
{
    QDomElement point = n.ownerDocument().createElement( "QPoint" );
    n.appendChild( point );
    point << (int)value.x();
    point << (int)value.y();
    return n;
}



GStateSaver::GStateSaver()
{
}

GStateSaver::GStateSaver(QSharedPointer<State> s )
    : stateName( s->name() )
{
    QString name = stateName;
    QFontMetricsF metrics(QFont("helvetica", 14, QFont::Medium));

    QRectF bounds = metrics.boundingRect(name).normalized().marginsAdded(QMarginsF(10, 10, 10, 10)).toAlignedRect();

    position = s->center() - bounds.center();
}

void GStateSaver::load(QSharedPointer<StateMachine> sm)
{
    auto s = sm->findState( stateName );
    Q_ASSERT( s );

    QString name = stateName;
    QFontMetricsF metrics(QFont("helvetica", 14, QFont::Medium));

    QRectF bounds = metrics.boundingRect(name).normalized().marginsAdded(QMarginsF(10, 10, 10, 10)).toAlignedRect();

    s->setCenter( position + bounds.center() );
}


GTransitionSaver::GTransitionSaver()
{
}

GTransitionSaver::GTransitionSaver(QSharedPointer<Transition> s )
    : stateName( s->parentState()->name() )
{
    transition = s->index();

    foreach (auto cp, s->controlPoints()) {
        points.append( cp );
    }
}

void GTransitionSaver::load(QSharedPointer<StateMachine> sm)
{
    auto s = sm->findState( stateName );
    Q_ASSERT( s );
    State::TransitionList l = s->transitionList();
    Q_ASSERT( transition < (int)l.count() );
    auto t = l[transition];

    QVector<QPointF> cp( points.count() );

    // Always set control points, even for an empty array, to adjust head and tail
    int pos; QList<QPointF>::Iterator i;
    for( pos = 0, i = points.begin(); pos < cp.count() && i != points.end(); ++pos, ++i ) {
        cp[pos] = *i;
//         qDebug( "   Point: (%.2f, %.2f)", (*i).x(), (*i).y() );
    }
    t->setControlPoints( cp );
}

