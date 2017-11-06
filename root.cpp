#include "statemachine.h"
#include "rootundocommand.h"


Root::Root( QSharedPointer<StateMachine> sm )
    : EIOBase( QObject::tr("Root"), sm )
{
}

QString Root::name() const
{
    return parent()->name.isEmpty() ? "State Machine" : parent()->name;
}

int Root::index() const
{
    return 0;
}

bool Root::rename( QString newName )
{
    parent()->name = newName;
    parent()->informItemChanged(this);
    return true;
}

bool Root::canDelete() const
{
    return false;
}

QUndoCommand * Root::deleteCommand()
{
    return nullptr;
}

QDomNode Root::save( QDomDocument )
{
    return QDomNode();
}

QUndoCommand * Root::renameCommand( const QString& newName )
{
    return new RenameRootCommand(this, newName);
}

QString Root::comment() const
{
    return parent()->comment;
}

void Root::setComment(const QString &comment)
{
    parent()->comment = comment;
    parent()->informItemChanged(this, Qt::WhatsThisRole);
}

QString Root::baseClass() const
{
    return parent()->baseClass;
}

void Root::setBaseClass(const QString &baseClass)
{
    parent()->baseClass = baseClass;
    parent()->informItemChanged(this, StateMachine::ContentChangedRole);
}

QString Root::initialState() const
{
    return parent()->initialState;
}

void Root::setInitialState(const QString &initialState)
{
    auto oldInitial = parent()->findState( parent()->initialState );
    auto newInitial = parent()->findState( initialState );

    parent()->initialState = initialState;
    parent()->informItemChanged(this, StateMachine::ContentChangedRole);

    if (oldInitial)
        parent()->informItemChanged( oldInitial.data(), StateMachine::ContentChangedRole);
    if (newInitial)
        parent()->informItemChanged( newInitial.data(), StateMachine::ContentChangedRole);
}
