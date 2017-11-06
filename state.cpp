#include <QtDebug>
#include <QLoggingCategory>
#include "statemachine.h"
#include "stateundocommand.h"

RENAME_BOILERPLATE(State, state)

Q_DECLARE_LOGGING_CATEGORY(StateLogging)
Q_LOGGING_CATEGORY(StateLogging, "statemachine.state")

State::State( QString name, QSharedPointer<StateMachine> parent )
    : EIOBase( name, parent ), incomeActions( this ), outcomeActions( this )
{
    if( parent->findState(name) )
        throw NameConflictException( name );
}

State::~State()
{
    qCDebug(StateLogging) << "deleting state" << name();
}


int State::index() const
{
    QSharedPointer<StateMachine> p(parent());
    if (p) {
        for (int i = 0; i < p->states.count(); ++i) {
            if (p->states[i].data() == this)
                return i;
        }
    }

    return -1;
}

bool State::rename( QString newName )
{
    if( parent()->findState(_name) != this || parent()->findState(newName) )
        return false;
    else {
        _name = newName;
        parent()->informItemChanged(this);
        parent()->ensureStatesSorted();
        return true;
    }
}

bool State::moveTransition( int i, int to )
{
    if( i >= 0 && i < _transitions.count() && to >= 0 && to < _transitions.count() ) {
        auto fsm = parent();
        auto me = fsm->indexForItem( this );

        int dc = i < to ? to + 1 : to;

        if ( fsm->beginMoveRows( me, i, i, me, dc ) ) {
            _transitions.move(i, to);
            fsm->endMoveRows();
        }
        return true;
    }
    return false;
}

void State::add( QSharedPointer<Transition> t )
{
    TransitionList::Iterator i = _transitions.begin();
    while( i != _transitions.end() && dynamic_cast<Transition*>(i->data()) ) ++i;
    if( i == _transitions.end() )
        _transitions.append( t );
    else
        _transitions.insert( i, t );
}

void State::insert( QSharedPointer<Transition> t, int pos )
{
    TransitionList::Iterator it = _transitions.begin();
    for( int i = 0; i < pos; ++i )
        ++it;
    _transitions.insert( it, t );
}

void State::del( QSharedPointer<Transition> t )
{
    auto fsm = parent();
    int index = t->index();
    qCDebug(StateLogging) << "removing transition at index" << index << ", total transitions:" << _transitions.count();
    fsm->beginRemoveRows( fsm->indexForItem(this), index, index );
    _transitions.removeAt( index );
    fsm->endRemoveRows();
}

State::TransitionList State::transitionList() const
{
    return _transitions;
}

QSharedPointer<Transition> State::createTransition(int pos, QSharedPointer<State> target)
{
    QSharedPointer<Transition> t( new Transition( sharedFromThis().dynamicCast<State>() ) );
    t->target = target;
    auto fsm = parent();
    fsm->beginInsertRows( fsm->indexForItem(this), pos, pos + 1 );
    _transitions.insert(pos, t);
    fsm->endInsertRows();
    return t;
}

QDomNode State::save( QDomDocument doc )
{
    QDomElement e = doc.createElement( "state" );
    QDomElement eName = doc.createElement( "name" );
    QDomText nameText = doc.createTextNode( name() );
    QDomElement eComment = doc.createElement( "comment" );
    QDomText commentText = doc.createTextNode( comment() );
    e.appendChild( eName );
    e.appendChild( eComment );
    eName.appendChild( nameText );
    eComment.appendChild( commentText );

    QDomElement eIncome = doc.createElement( "incomeactions" );
    e.appendChild( eIncome );
    for( auto i = incomeActions.begin(); i != incomeActions.end(); ++i ) {
        auto b = i->toStrongRef();
        if (!b) continue; // skip empty
        QDomElement eAction = doc.createElement( "action" );
        eAction.setAttribute( "type", b.dynamicCast<Output>() ? "output" : "event" );
        QDomText actionText = doc.createTextNode( b->name() );
        eIncome.appendChild( eAction );
        eAction.appendChild( actionText );
    }

    QDomElement eOutcome = doc.createElement( "outcomeactions" );
    e.appendChild( eOutcome );
    for( auto i = outcomeActions.begin(); i != outcomeActions.end(); ++i ) {
        auto b = i->toStrongRef();
        if (!b) continue; // skip empty
        QDomElement eAction = doc.createElement( "action" );
        eAction.setAttribute( "type", b.dynamicCast<Output>() ? "output" : "event" );
        QDomText actionText = doc.createTextNode( b->name() );
        eOutcome.appendChild( eAction );
        eAction.appendChild( actionText );
    }

    QDomElement transitionsEl = doc.createElement( "transitions" );
    e.appendChild( transitionsEl );
    for( TransitionList::Iterator i = _transitions.begin(); i != _transitions.end(); ++i ) {
        QDomNode n = (*i)->save(doc);
        if( !n.isNull() )
            transitionsEl.appendChild( n );
    }

    return e;
}

void State::setCenter( const QPointF &c )
{
    m_center = c;
    parent()->informItemChanged( this, StateMachine::GraphicPositionRole );
}

void RenameStateCommand::undo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<State> obj = fsm->findState( m_newName );
        if ( obj )
            obj->rename( m_oldName );
    }
}

void RenameStateCommand::redo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<State> obj = fsm->findState( m_oldName );
        if ( obj )
            obj->rename( m_newName );
    }
}

