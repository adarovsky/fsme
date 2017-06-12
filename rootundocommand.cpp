#include "rootundocommand.h"

RootUndoCommand::RootUndoCommand(Root *e, const QString& text, QUndoCommand *parent)
    : QUndoCommand(text, parent), m_document( e->parent() )
{
}

QSharedPointer<Root> RootUndoCommand::root() const
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        return fsm->root();
    }

    return QSharedPointer<Root>();
}

RenameRootCommand::RenameRootCommand(Root *e, const QString &newName, QUndoCommand * parent)
    : RootUndoCommand(e, QObject::tr("Rename state machine from %1 to %2\nrename").arg(e->name(), newName), parent),
      m_oldName(e->name()), m_newName(newName), m_document(e->parent())
{
}

void RenameRootCommand::undo()
{
    QUndoCommand::undo();

    root()->rename( m_oldName );
}

void RenameRootCommand::redo()
{
    root()->rename( m_newName );
    QUndoCommand::redo();
}


RootChangeCommentCommand::RootChangeCommentCommand(Root * e, const QString& newComment, QUndoCommand * parent)
    : RootUndoCommand(e, QObject::tr("change comment"), parent), m_oldComment(e->comment()), m_newComment(newComment)
{
}

void RootChangeCommentCommand::undo()
{
    RootUndoCommand::undo();
    root()->setComment( m_oldComment );
}
void RootChangeCommentCommand::redo()
{
    root()->setComment( m_newComment );
    RootUndoCommand::redo();
}

bool RootChangeCommentCommand::mergeWith(const QUndoCommand * command)
{
    auto other = dynamic_cast<const RootChangeCommentCommand*>( command );
    if (other) {
        m_newComment = other->m_newComment;
        return true;
    }
    return false;
}

RootChangeInitialStateCommand::RootChangeInitialStateCommand(Root * e, const QString& newInitialstate, QUndoCommand * parent)
    : RootUndoCommand(e, QObject::tr("change initialstate"), parent), m_oldInitialState(e->initialState()), m_newInitialState(newInitialstate)
{
}

void RootChangeInitialStateCommand::undo()
{
    RootUndoCommand::undo();
    root()->setInitialState( m_oldInitialState );
}
void RootChangeInitialStateCommand::redo()
{
    root()->setInitialState( m_newInitialState );
    RootUndoCommand::redo();
}
