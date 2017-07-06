#include "event.h"
#include "statemachine.h"

RENAME_BOILERPLATE(Event, event)
UNDO_BASE_BOILERPLATE_IMPL(Event, event)

Event::Event( QString name, QSharedPointer<StateMachine> parent )
    : EIOBase( name, parent )
{
    if( parent->findEvent(name) )
        throw NameConflictException( name );
}

int Event::index() const
{
    QSharedPointer<StateMachine> p(parent());
    if (p) {
        for (int i = 0; i < p->events.count(); ++i) {
            if (p->events[i].data() == this)
                return i;
        }
    }

    return -1;
}

bool Event::rename( QString newName )
{
    if( parent()->findEvent(_name) != this || parent()->findEvent(newName) )
        return false;
    else {
        _name = newName;
        parent()->informItemChanged(this);
        parent()->ensureEventsSorted();
        return true;
    }
}

QDomNode Event::save( QDomDocument doc )
{
    QDomElement e = doc.createElement( "event" );
    QDomElement eName = doc.createElement( "name" );
    QDomText nameText = doc.createTextNode( name() );
    QDomElement eComment = doc.createElement( "comment" );
    QDomText commentText = doc.createTextNode( comment() );
    e.appendChild( eName );
    e.appendChild( eComment );
    eName.appendChild( nameText );
    eComment.appendChild( commentText );
    return e;
}

EventDeleteCommand::EventDeleteCommand(Event * obj, QUndoCommand * parent)
    : QUndoCommand(QObject::tr("delete %1").arg(obj->name()), parent),
      m_name(obj->name()), m_document(obj->parent())
{
    new EventChangeCommentCommand(obj, QString(), this);
}

void RenameEventCommand::undo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<Event> obj = fsm->findEvent( m_newName );
        if ( obj )
            obj->rename( m_oldName );

        EIOBase::renameInCondition(fsm, m_newName, m_oldName );
    }
}

void RenameEventCommand::redo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<Event> obj = fsm->findEvent( m_oldName );
        if ( obj )
            obj->rename( m_newName );

        EIOBase::renameInCondition(fsm, m_oldName, m_newName );
    }
}
