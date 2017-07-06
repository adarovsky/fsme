#include "output.h"
#include "statemachine.h"
#include "transitionundocommand.h"
#include "stateundocommand.h"

RENAME_BOILERPLATE(Output, output)
UNDO_BASE_BOILERPLATE_IMPL(Output, output)
int Output::index() const
{
    QSharedPointer<StateMachine> p(parent());
    if (p) {
        for (int i = 0; i < p->outputs.count(); ++i) {
            if (p->outputs[i].data() == this)
                return i;
        }
    }

    return -1;
}

Output::Output( QString name, QSharedPointer<StateMachine> parent )
    : EIOBase( name, parent )
{
    if( parent->findOutput(name) )
        throw NameConflictException( name );
}

bool Output::rename( QString newName )
{
    if( parent()->findOutput(_name) != this || parent()->findOutput(newName) )
        return false;
    else {
        _name = newName;
        parent()->informItemChanged(this);
        parent()->ensureOutputsSorted();
        return true;
    }
}

QDomNode Output::save( QDomDocument doc )
{
    QDomElement e = doc.createElement( "output" );
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

OutputDeleteCommand::OutputDeleteCommand(Output * obj, QUndoCommand * parent)
    : QUndoCommand(QObject::tr("delete %1").arg(obj->name()), parent),
      m_name(obj->name()), m_document(obj->parent())
{
    new OutputChangeCommentCommand(obj, QString(), this);

    auto fsm = m_document.toStrongRef();

    if (fsm) {
        for( auto s = fsm->statesBegin(); s != fsm->statesEnd(); ++s ) {
            for (int o = (*s)->incomeActions.rowCount() - 1; o >= 0; --o) {
                if ((*s)->incomeActions.at(o) == obj) {
                    new StateRemoveActionCommand(s->data(), o, StateActionCommand::In, this);
                }
            }

            for (int o = (*s)->outcomeActions.rowCount() - 1; o >= 0; --o) {
                if ((*s)->outcomeActions.at(o) == obj) {
                    new StateRemoveActionCommand(s->data(), o, StateActionCommand::Out, this);
                }
            }

            auto tl = (*s)->transitionList();
            for ( auto t = tl.begin(); t != tl.end(); ++ t ) {
                for (int o = (*t)->actions.rowCount() - 1; o >= 0; --o) {
                    if ((*t)->actions.at(o) == obj) {
                        new TransitionRemoveActionCommand(t->data(), o, this);
                    }
                }
            }
        }
    }
}

void RenameOutputCommand::undo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<Output> obj = fsm->findOutput( m_newName );
        if ( obj )
            obj->rename( m_oldName );
    }
}

void RenameOutputCommand::redo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<Output> obj = fsm->findOutput( m_oldName );
        if ( obj )
            obj->rename( m_newName );
    }
}

