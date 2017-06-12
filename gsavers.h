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
#ifndef __GSAVER_H_
#define __GSAVER_H_
#include <QPointF>
#include "streamable.h"

QDomNode& operator << ( QDomNode& n, const QPointF& value );
bool operator >> ( QDomNode& n, QPointF& value );

class StateMachine;
class DiagramView;
class State;
class Transition;
class GSaver : public Streamable {
    DECLARE_STREAMABLE( GSaver )
public:
    virtual void load( QSharedPointer<StateMachine> sm ) = 0;
};

class GStateSaver : public GSaver {
    DECLARE_CUSTOM_STREAMABLE( GStateSaver )
    QString  stateName;
    QPointF  position;
public:
    GStateSaver();
    GStateSaver( QSharedPointer<State> s );
    virtual void load( QSharedPointer<StateMachine> sm );
};

class GTransitionSaver : public GSaver {
    DECLARE_CUSTOM_STREAMABLE( GTransitionSaver )
    QString            stateName;
    int                transition;
    QList<QPointF> points;
public:
    GTransitionSaver();
    GTransitionSaver( QSharedPointer<Transition> s );
    virtual void load( QSharedPointer<StateMachine> sm );
};

#endif

/// Local Variables:
/// mode: C++
/// c-basic-offset: 4
/// End:
