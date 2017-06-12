#include "input.h"
#include "statemachine.h"

RENAME_BOILERPLATE(Input, input)
UNDO_BASE_BOILERPLATE_IMPL(Input, input)
Input::Input( QString name, QSharedPointer<StateMachine> parent )
    : EIOBase( name, parent )
{
    if( parent->findInput(name) )
        throw NameConflictException( name );
}

int Input::index() const
{
    QSharedPointer<StateMachine> p(parent());
    if (p) {
        for (int i = 0; i < p->inputs.count(); ++i) {
            if (p->inputs[i].data() == this)
                return i;
        }
    }

    return -1;
}

bool Input::rename( QString newName )
{
    if( parent()->findInput(_name) != this || parent()->findInput(newName) )
        return false;
    else {
        _name = newName;
        parent()->informItemChanged(this);
        parent()->ensureInputsSorted();
        return true;
    }
}

QDomNode Input::save( QDomDocument doc )
{
    QDomElement e = doc.createElement( "input" );
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

InputDeleteCommand::InputDeleteCommand(Input * obj, QUndoCommand * parent)
    : QUndoCommand(QObject::tr("delete %1").arg(obj->name()), parent),
      m_name(obj->name()), m_document(obj->parent())
{
    new InputChangeCommentCommand(obj, QString(), this);
}
