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
#include "statemachine.h"

// ======================= State ============================

#include <QAbstractItemModel>
#include <QDataStream>
#include <QDomDocument>
#include <QEnableSharedFromThis>
#include <QObject>
#include <QPair>
#include <QPoint>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWeakPointer>
#include "gsavers.h"

//==========================================================
//                     State Machine
//==========================================================

StateMachine::StateMachine()
    : m_undoStack(new QUndoStack(this))
{
}

StateMachine::~StateMachine()
{
}


void StateMachine::add( QSharedPointer<Event> e )
{
    beginInsertRows( eventsFolder(), events.count(), events.count() + 1 );
    events.append( e );
    endInsertRows();
    ensureEventsSorted();
}

void StateMachine::add( QSharedPointer<Input>  i )
{
    beginInsertRows( inputsFolder(), inputs.count() - 1, inputs.count() );
    inputs.append( i );
    endInsertRows();
    ensureInputsSorted();
}

void StateMachine::add( QSharedPointer<Output>  o )
{
    beginInsertRows( outputsFolder(), outputs.count(), outputs.count() + 1 );
    outputs.append( o );
    endInsertRows();
    ensureOutputsSorted();
}

void StateMachine::add( QSharedPointer<State>  s )
{
    beginInsertRows( statesFolder(), states.count(), states.count() + 1 );
    states.append( s );
    endInsertRows();
    ensureStatesSorted();
}


void StateMachine::del( QSharedPointer<Event>  e )
{
    beginRemoveRows( eventsFolder(), e->index(), e->index() );
    events.removeAll( e );
    endRemoveRows();
}

void StateMachine::del( QSharedPointer<Input>  i )
{
    beginRemoveRows( inputsFolder(), i->index(), i->index() );
    inputs.removeAll( i );
    endRemoveRows();
}

void StateMachine::del( QSharedPointer<Output>  o )
{
    int index = o->index();
    beginRemoveRows( outputsFolder(), index, index );
    outputs.removeAll( o );
    endRemoveRows();
}

void StateMachine::del( QSharedPointer<State>  s )
{
    beginRemoveRows( statesFolder(), s->index(), s->index() );
    states.removeAll( s );
    endRemoveRows();
}

void StateMachine::ensureEventsSorted()
{
    QPersistentModelIndex index(eventsFolder());
    QList<QPersistentModelIndex> parents;
    parents << index;
    emit layoutAboutToBeChanged(parents);
    QList<QModelIndex> originals;
    int i = 0;
    foreach (QSharedPointer<Event> o, events) {
        originals << createIndex(i++, 0, o.data());
    }

    std::sort(events.begin(), events.end(), [] ( const QSharedPointer<Event> x1, const QSharedPointer<Event> x2 ) {
        return x1->name().toLower() < x2->name().toLower();
    });

    foreach (QModelIndex orig, originals) {
        int newIndex = 0;
        foreach (QSharedPointer<Event> o, events) {
            if (reinterpret_cast<EIOBase*>(orig.internalPointer()) == o.data()) {
                changePersistentIndex(orig, createIndex(newIndex, 0, o.data()));
                break;
            }
            ++newIndex;
        }
    }
    emit layoutChanged(parents);
}

void StateMachine::ensureInputsSorted()
{
    QPersistentModelIndex index(createIndex(1, 0, nullptr));
    QList<QPersistentModelIndex> parents;
    parents << index;
    emit layoutAboutToBeChanged(parents);
    QList<QModelIndex> originals;
    int i = 0;
    foreach (QSharedPointer<Input> o, inputs) {
        originals << createIndex(i++, 0, o.data());
    }

    std::sort(inputs.begin(), inputs.end(), [] ( const QSharedPointer<Input> x1, const QSharedPointer<Input> x2 ) {
        return x1->name().toLower() < x2->name().toLower();
    });

    foreach (QModelIndex orig, originals) {
        int newIndex = 0;
        foreach (QSharedPointer<Input> o, inputs) {
            if (reinterpret_cast<EIOBase*>(orig.internalPointer()) == o.data()) {
                changePersistentIndex(orig, createIndex(newIndex, 0, o.data()));
                break;
            }
            ++newIndex;
        }
    }
    emit layoutChanged(parents);
}

void StateMachine::ensureOutputsSorted()
{
    QPersistentModelIndex index(createIndex(2, 0, nullptr));
    QList<QPersistentModelIndex> parents;
    parents << index;
    emit layoutAboutToBeChanged(parents);
    QList<QModelIndex> originals;
    int i = 0;
    foreach (QSharedPointer<Output> o, outputs) {
        originals << createIndex(i++, 0, o.data());
    }

    std::sort(outputs.begin(), outputs.end(), [] ( const QSharedPointer<Output> x1, const QSharedPointer<Output> x2 ) {
        return x1->name().toLower() < x2->name().toLower();
    });

    foreach (QModelIndex orig, originals) {
        int newIndex = 0;
        foreach (QSharedPointer<Output> o, outputs) {
            if (reinterpret_cast<EIOBase*>(orig.internalPointer()) == o.data()) {
                changePersistentIndex(orig, createIndex(newIndex, 0, o.data()));
                break;
            }
            ++newIndex;
        }
    }
    emit layoutChanged(parents);
}

void StateMachine::ensureStatesSorted()
{
    QPersistentModelIndex index(createIndex(3, 0, nullptr));
    QList<QPersistentModelIndex> parents;
    parents << index;
    emit layoutAboutToBeChanged(parents);
    QList<QModelIndex> originals;
    int i = 0;
    foreach (QSharedPointer<State> o, states) {
        originals << createIndex(i++, 0, o.data());
    }

    std::sort(states.begin(), states.end(), [] ( const QSharedPointer<State> x1, const QSharedPointer<State> x2 ) {
        return x1->name().toLower() < x2->name().toLower();
    });

    foreach (QModelIndex orig, originals) {
        int newIndex = 0;
        foreach (QSharedPointer<State> o, states) {
            if (reinterpret_cast<EIOBase*>(orig.internalPointer()) == o.data()) {
                changePersistentIndex(orig, createIndex(newIndex, 0, o.data()));
                break;
            }
            ++newIndex;
        }
    }
    emit layoutChanged(parents);
}


QSharedPointer<Event> StateMachine::findEvent( const QString& name ) const
{
    QList<QSharedPointer<Event> >::const_iterator e = std::find_if(events.begin(), events.end(), [&] (const QSharedPointer<Event>& e) {
        return e->name() == name;
    });
    return e != events.end() ? *e : QSharedPointer<Event>();
}

QSharedPointer<Input>  StateMachine::findInput ( const QString& name ) const
{
    QList<QSharedPointer<Input> >::const_iterator e = std::find_if(inputs.begin(), inputs.end(), [&] (const QSharedPointer<Input>& e) {
        return e->name() == name;
    });
    return e != inputs.end() ? *e : QSharedPointer<Input>();
}

QSharedPointer<Output> StateMachine::findOutput( const QString& name ) const
{
    QList<QSharedPointer<Output> >::const_iterator e = std::find_if(outputs.begin(), outputs.end(), [&] (const QSharedPointer<Output>& e) {
        return e->name() == name;
    });
    return e != outputs.end() ? *e : QSharedPointer<Output>();
}

QSharedPointer<State>  StateMachine::findState ( const QString& name ) const
{
    QList<QSharedPointer<State> >::const_iterator e = std::find_if(states.begin(), states.end(), [&] (const QSharedPointer<State>& e) {
        return e->name() == name;
    });
    return e != states.end() ? *e : QSharedPointer<State>();
}



#define CHECK_ELEMENT(e)                                        \
if( e.isNull() ) {                                              \
  qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );    \
  return false;                                                 \
}

void StateMachine::clear()
{
    _root.reset( new Root( sharedFromThis() ) );
    events.clear();
    inputs.clear();
    outputs.clear();
    states.clear();
}

#define ADD_BOLIERPLATE(Type)                                                                       \
QSharedPointer<Type> StateMachine::add##Type ( const QString& name )                                 \
{                                                                                                   \
    if ( find##Type(name) )                                                                         \
        return find##Type(name);                                                                    \
    auto obj = QSharedPointer<Type>( new Type(name, sharedFromThis() ) );                           \
    add( obj );                                                                                     \
    return obj;                                                                                     \
}

ADD_BOLIERPLATE(Event)
ADD_BOLIERPLATE(Input)
ADD_BOLIERPLATE(Output)

QSharedPointer<State> StateMachine::addState ( const QString& name, const QPointF& pos )
{
    if ( findState(name) )
        return findState(name);
    auto obj = QSharedPointer<State>( new State(name, sharedFromThis() ) );
    obj->setCenter( pos );
    add( obj );
    return obj;
}


bool StateMachine::loadCore(const QDomElement& sme)
{
    m_undoStack->setActive(false);
    clear();
    //    qDebug( "StateMachine::load: got element '%s'", sme.tagName().toLocal8Bit().data() );
    if( sme.isNull() ) { // the node was really an element.
        qWarning( "%s is not a state machine tag", sme.tagName().toLocal8Bit().data() );
        return false;
    }
    QDomNode n = sme.firstChild();


    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "StateMachine::load: %s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }

        QString tag = e.tagName();
        QString text = e.text();
        if( tag == "name" ) name = text;
        else if( tag == "comment" ) comment = text;
        else if( tag == "initialstate" ) initialState = text;
        else if( tag == "baseclass" ) baseClass = text;
        else if( tag == "input" )  readInput( e );
        else if( tag == "event" )  readEvent( e );
        else if( tag == "output" ) readOutput( e );
        else if( tag == "state" )  readState( e );

        n = n.nextSibling();
    }

    n = sme.firstChild();

    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "StateMachine::load: %s is not an element", e.nodeName().toLocal8Bit().data() );
            m_undoStack->setActive(true);
            return false;
        }

        QString tag = e.tagName();
        if( tag == "state" )  fillState( e );

        n = n.nextSibling();
    }

    m_undoStack->setActive(true);
    m_undoStack->clear();
    return true;
}

bool StateMachine::loadGraphics(const QDomElement &graphics)
{
    QDomNode n = graphics.firstChild();
    while( !n.isNull() ) {
        Q_ASSERT( n.isElement() );
        auto s = Streamable::load( n.toElement() );
        if( s ) {
            auto gs = s.dynamicCast<GSaver>();
            if( gs )
                gs->load( sharedFromThis() );
        }
        else
            qDebug("Cannot find streamable for element %s", n.nodeName().toLocal8Bit().data());
        n = n.nextSibling();
    }

    return true;
}

bool StateMachine::load(const QDomDocument &doc)
{
    beginResetModel();
    QDomElement node = doc.documentElement().firstChildElement( "statemachine" );
    if ( node.isNull() ) {
        qWarning("cannot find statemachine element");
        return false;
    }

    if ( !loadCore( node ) ) {
        endResetModel();
        return false;
    }

    node = node.nextSiblingElement( "graphics" );

    bool ok = !node.isNull() && loadGraphics( node );
    endResetModel();

    return ok;
}

bool StateMachine::readEvent( const QDomElement& e )
{
    // qDebug("reading input");
    QSharedPointer<Event> el(new Event( "1", sharedFromThis() ));
    add(el);

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        CHECK_ELEMENT(e);
        if( e.tagName() == "name" ) {
            el->rename( e.text() );
        }
        else if( e.tagName() == "comment" ) {
            el->setComment( e.text() );
        }

        n = n.nextSibling();
    }
    return true;
}


bool StateMachine::readInput( const QDomElement& e )
{
    // qDebug("reading input");
    QSharedPointer<Input> el( new Input( "1", sharedFromThis() ) );
    add(el);

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        CHECK_ELEMENT(e);
        if( e.tagName() == "name" ) {
            el->rename( e.text() );
        }
        else if( e.tagName() == "comment" ) {
            el->setComment( e.text() );
        }

        n = n.nextSibling();
    }
    return true;
}

bool StateMachine::readOutput( const QDomElement& e )
{
    // qDebug("reading input");
    QSharedPointer<Output> el( new Output( "1", sharedFromThis() ) );
    add(el);

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        CHECK_ELEMENT(e);
        if( e.tagName() == "name" ) {
            el->rename( e.text() );
        }
        else if( e.tagName() == "comment" ) {
            el->setComment( e.text() );
        }

        n = n.nextSibling();
    }
    return true;
}


bool StateMachine::readState( const QDomElement& e )
{
    // qDebug("reading state");
    QSharedPointer<State> s( new State( "1", sharedFromThis() ) );
    add(s);

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }

        if( e.tagName() == "name" ) {
            s->rename( e.text() );
        }
        else if( e.tagName() == "comment" ) {
            s->setComment( e.text() );
        }
        n = n.nextSibling();
    }
    return true;
}

bool StateMachine::fillState( const QDomElement& e )
{
    QString name = e.namedItem("name").toElement().text();
    QSharedPointer<State> s = findState(name);
    Q_ASSERT(s);

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }

        if( e.tagName() == "incomeactions" )
            readIncomeActions( e, *s );
        else if( e.tagName() == "outcomeactions" )
            readOutcomeActions( e, *s );
        else if( e.tagName() == "transitions" )
            readTransitions( e, *s );
        n = n.nextSibling();
    }
    return true;
}

bool StateMachine::readTransitions( const QDomElement& e, State& state )
{
    QDomNode n = e.firstChild();
    int pos = 0;
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }

        if( e.tagName() == "transition" )
            readTransition( e, state, pos++ );

        n = n.nextSibling();
    }
    return true;
}

bool StateMachine::readTransition ( const QDomElement& e, State& state, int pos )
{
    QSharedPointer<Transition> t( state.createTransition( pos ) );
    t->transitionType = Transition::parseTransitionType( e.attribute( "type", "simple" ) );
    // qDebug( "reading transition" );
    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }

        if( e.tagName() == "name" ) {
            t->rename( e.text() );
            // qDebug( "   name = %s", e.text().toLocal8Bit().data() );
        }
        else if( e.tagName() == "destination" ) {
            t->target = state.parent()->findState( e.text() );
            // qDebug( "   destination = %s", e.text().toLocal8Bit().data() );
        }
        else if( e.tagName() == "transite_destination" ) {
            t->transiteTarget = state.parent()->findState( e.text() );
            // qDebug( "   destination = %s", e.text().toLocal8Bit().data() );
        }
        else if( e.tagName() == "condition" ) {
            t->setCondition( e.text() );
            // qDebug( "   condition = %s", e.text().toLocal8Bit().data() );
        }
        else if( e.tagName() == "comment" ) {
            t->setComment( e.text() );
            // qDebug( "   condition = %s", e.text().toLocal8Bit().data() );
        }
        else if( e.tagName() == "transitionactions" ) {
            QDomNode tr_n = e.firstChild();
            while( !tr_n.isNull() ) {
                QDomElement e = tr_n.toElement();

                if( e.isNull() ) {
                    qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
                    return false;
                }


                if( e.tagName() == "action" ) {
                    // qDebug( "   transitions action = %s", e.text().toLocal8Bit().data() );
                    QSharedPointer<EIOBase> b;

                    b = state.parent()->findOutput( e.text() );
                    if (b)
                        t->actions.append( b );
                    else {
                        b = state.parent()->findEvent( e.text() );
                        if (b)
                            t->actions.append( b );
                        else
                            qWarning("%s: skipping unknown action '%s'", state.name().toLocal8Bit().data(), e.text().toLocal8Bit().data());
                    }
                }

                tr_n = tr_n.nextSibling();
            }
        }

        n = n.nextSibling();
    }
    return true;
}

bool StateMachine::readIncomeActions( const QDomElement& e, State& state )
{
    // qDebug("reading incomeActions");

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }


        if( e.tagName() == "action" ) {
            // qDebug( "action = %s", e.text().toLocal8Bit().data() );
            QSharedPointer<EIOBase> b;

            b = state.parent()->findOutput( e.text() );
            if (b)
                state.incomeActions.append( b );
            else {
                b = state.parent()->findEvent( e.text() );
                if (b)
                    state.incomeActions.append( b );
                else
                    qWarning("%s: skipping unknown action '%s'", state.name().toLocal8Bit().data(), e.text().toLocal8Bit().data());
            }
        }

        n = n.nextSibling();
    }

    return true;
}

bool StateMachine::readOutcomeActions( const QDomElement& e, State& state )
{
    // qDebug("reading incomeActions");

    QDomNode n = e.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            qWarning( "%s is not an element", e.nodeName().toLocal8Bit().data() );
            return false;
        }

        if( e.tagName() == "action" ) {
            // qDebug( "action = %s", e.text().toLocal8Bit().data() );

            QSharedPointer<EIOBase> b;

            b = state.parent()->findOutput( e.text() );
            if (b)
                state.outcomeActions.append( b );
            else {
                b = state.parent()->findEvent( e.text() );
                if (b)
                    state.outcomeActions.append( b );
                else
                    qWarning("%s: skipping unknown action '%s'", state.name().toLocal8Bit().data(), e.text().toLocal8Bit().data());
            }

        }

        n = n.nextSibling();
    }

    return true;
}


void StateMachine::save( QDomDocument doc, QDomElement root )
{
    QDomElement smEl = doc.createElement( "statemachine" );

    root.appendChild( smEl );
    saveCore( doc, smEl );

    QDomElement graphics = doc.createElement( "graphics" );
    root.appendChild( graphics );
    saveGraphics( doc, graphics );
}

void StateMachine::saveCore(QDomDocument doc, QDomElement smEl)
{
    QDomElement eName = doc.createElement( "name" );
    QDomText nameText = doc.createTextNode( name );
    eName.appendChild( nameText );
    smEl.appendChild( eName );

    QDomElement eComment = doc.createElement( "comment" );
    QDomText commentText = doc.createTextNode( comment );
    eComment.appendChild( commentText );
    smEl.appendChild( eComment );


    if( !baseClass.isEmpty() ) {
        QDomElement eBase = doc.createElement( "baseclass" );
        QDomText baseText = doc.createTextNode( baseClass );
        eBase.appendChild( baseText );
        smEl.appendChild( eBase );
    }

    QDomElement eInitial = doc.createElement( "initialstate" );
    QDomText initialText = doc.createTextNode( initialState );
    eInitial.appendChild( initialText );
    smEl.appendChild( eInitial );

    foreach (const QSharedPointer<Event> e, events) {
        QDomNode n = e->save( doc );
        if( !n.isNull() )
            smEl.appendChild( n );
    }

    foreach (const QSharedPointer<Input> i, inputs) {
        QDomNode n = i->save( doc );
        if( !n.isNull() )
            smEl.appendChild( n );
    }

    foreach (const QSharedPointer<Output> o, outputs) {
        QDomNode n = o->save( doc );
        if( !n.isNull() )
            smEl.appendChild( n );
    }

    foreach (const QSharedPointer<State> s, states) {
        QDomNode n = s->save( doc );
        if( !n.isNull() )
            smEl.appendChild( n );
    }
}

void StateMachine::saveGraphics(QDomDocument doc, QDomElement root)
{
    QDomElement graphics = doc.createElement( "graphics" );

    foreach (auto s, states) {
        GStateSaver saver( s );
        saver.save( root );
    }

    foreach (auto s, states) {
        auto tl = s->transitionList();
        for( auto t = tl.begin(); t != tl.end(); ++t) {
            GTransitionSaver saver( *t );
            saver.save( root );
        }
    }
}

QModelIndex StateMachine::eventsFolder() const
{
    return createIndex(0, 0, nullptr);
}

QModelIndex StateMachine::inputsFolder() const
{
    return createIndex(1, 0, nullptr);
}

QModelIndex StateMachine::outputsFolder() const
{
    return createIndex(2, 0, nullptr);
}

QModelIndex StateMachine::statesFolder() const
{
    return createIndex(3, 0, nullptr);
}

QModelIndex StateMachine::indexForItem( EIOBase * item ) const
{
    if ( !item )
        return QModelIndex();

    return createIndex( item->index(), 0, item );
}


QModelIndex StateMachine::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= 1)
            return QModelIndex();

    // return top folder
    if (!parent.isValid())
        return createIndex(row, column, root().data());

    EIOBase * p = reinterpret_cast<EIOBase*>(parent.internalPointer());
    if (!p) {
        switch (parent.row()) {
        case 0:
            return createIndex(row, column, events[row].data());
        case 1:
            return createIndex(row, column, inputs[row].data());
        case 2:
            return createIndex(row, column, outputs[row].data());
        case 3:
            return createIndex(row, column, states[row].data());
        default:
            return QModelIndex();
        }
    }
    else if (dynamic_cast<Root*>(p)) {
        return createIndex(row, column, nullptr);
    }

    State * s = dynamic_cast<State*>(p);
    if (s && row < s->transitionList().count())
        return createIndex(row, column, s->transitionList()[row].data());
    else
        return QModelIndex();
}


QModelIndex StateMachine::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    EIOBase * p = reinterpret_cast<EIOBase*>(child.internalPointer());
    if (p == nullptr)
        return createIndex(0, 0, root().data());
    else if (dynamic_cast<Event*>(p))
        return eventsFolder();
    else if (dynamic_cast<Input*>(p))
        return inputsFolder();
    else if (dynamic_cast<Output*>(p))
        return outputsFolder();
    else if (dynamic_cast<State*>(p))
        return statesFolder();
    else if (dynamic_cast<Transition*>(p)) {
        QSharedPointer<State> state = dynamic_cast<Transition*>(p)->parentState();
        return createIndex( state->index(), 0, state.data() );
    }

    return QModelIndex();
}

int StateMachine::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;

    EIOBase * p = reinterpret_cast<EIOBase*>(parent.internalPointer());
    if (p == nullptr) {
        switch (parent.row()) {
        case 0:
            return events.count();
        case 1:
            return inputs.count();
        case 2:
            return outputs.count();
        case 3:
            return states.count();
        default:
            return 0;
        }
    }
    else if (dynamic_cast<Root*>(p))
        return 4;
    else if (dynamic_cast<State*>(p))
        return dynamic_cast<State*>(p)->transitionList().count();

    return 0;
}

int StateMachine::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

bool StateMachine::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;

    EIOBase * p = reinterpret_cast<EIOBase*>(parent.internalPointer());
    if (p == nullptr) {
        switch (parent.row()) {
        case 0:
            return true;
        case 1:
            return true;
        case 2:
            return true;
        case 3:
            return true;
        default:
            return false;
        }
    }
    else if (dynamic_cast<Root*>(p))
        return true;
    else if (dynamic_cast<State*>(p))
        return true;

    return false;
}

QVariant StateMachine::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return root()->name();
        default:
            return QVariant();
        }
    }

    EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());

    if (p == nullptr) {
        switch (index.row()) {
        case 0:
            switch (role) {
            case Qt::DisplayRole:
                return tr("Events");
            default:
                return QVariant();
            }
        case 1:
            switch (role) {
            case Qt::DisplayRole:
                return tr("Inputs");
            default:
                return QVariant();
            }
        case 2:
            switch (role) {
            case Qt::DisplayRole:
                return tr("Outputs");
            default:
                return QVariant();
            }
        case 3:
            switch (role) {
            case Qt::DisplayRole:
                return tr("States");
            default:
                return QVariant();
            }
        default:
            return QVariant();
        }
    }
    else {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return p->name();
        default:
            return QVariant();
        }
    }
}

QSharedPointer<EIOBase> StateMachine::item(const QModelIndex& index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());

    return p->sharedFromThis();
}


QVariant StateMachine::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (section == 0 && role == Qt::DisplayRole)
        return tr("State Machine");

    return QVariant();
}

Qt::ItemFlags StateMachine::flags(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (p != nullptr)
        flags |= Qt::ItemIsEditable;

    if (dynamic_cast<Event*>(p))
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return flags;
}

bool StateMachine::setData(const QModelIndex &index, const QVariant &value, int role)
{
    EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());

    if (p && role == Qt::EditRole) {
        QUndoCommand * cmd =  p->renameCommand(value.toString());
        if (cmd)
            m_undoStack->push(cmd);
        return cmd != nullptr;
    }

    return false;
}

void StateMachine::informItemChanged(EIOBase *p, int role)
{
    QModelIndex i = indexForItem( p );
    emit dataChanged(i, i, QVector<int>() += role);
}
