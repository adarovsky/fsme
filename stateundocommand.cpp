#include "stateundocommand.h"
#include "transitionundocommand.h"
#include "rootundocommand.h"

StateUndoCommand::StateUndoCommand(State * s, const QString& text, QUndoCommand * parent)
    : QUndoCommand( text, parent ), m_stateName( s->name() ), m_document( s->parent() )
{

}

QSharedPointer<State> StateUndoCommand::state() const
{
    auto fsm = m_document.toStrongRef();

    if (fsm)
        return fsm->findState( m_stateName );

    return QSharedPointer<State>();
}

StateAddCommand::StateAddCommand(const QString &stateName, const QPointF &pos, QSharedPointer<StateMachine> document, QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Add State %1\nadd state").arg(stateName), parent), m_stateName(stateName), m_pos( pos ), m_document( document )
{
    auto fsm = m_document.toStrongRef();
    if (fsm && fsm->rowCount(fsm->statesFolder()) == 0)
        new RootChangeInitialStateCommand(fsm->root().data(), stateName, this);
}

void StateAddCommand::undo()
{
    QUndoCommand::undo();
    auto fsm = m_document.toStrongRef();

    if (fsm) {
        auto s = fsm->findState( m_stateName );
        if ( s ) {
            fsm->del( s );
        }
    }
}

void StateAddCommand::redo()
{
    auto fsm = m_document.toStrongRef();

    if (fsm) {
        fsm->addState( m_stateName, m_pos );
    }
    QUndoCommand::redo();
}

StateActionCommand::StateActionCommand(State * s, EIOBase * target, const QString & text, ActionType type, QUndoCommand * parent)
    : StateUndoCommand(s, text, parent), m_type(type)
{
    QModelIndex i = target->parent()->indexForItem(target);
    m_folder = i.parent();
    m_index = i.row();
}

QSharedPointer<EIOBase> StateActionCommand::target() const
{
    QModelIndex i = m_folder.child(m_index, 0);
    return state()->parent()->item( i );
}

OutputListModel * StateActionCommand::targetList() const
{
    return m_type == In ? &(state()->incomeActions) : &(state()->outcomeActions);
}

QSharedPointer<EIOBase> StateActionCommand::itemAt(State * s, int pos, ActionType type)
{
    return (type == In ? s->incomeActions : s->outcomeActions).at(pos);
}


StateAddActionCommand::StateAddActionCommand(State * s, EIOBase * target, int pos, ActionType type, QUndoCommand * parent)
    : StateActionCommand(s, target, QObject::tr("Add %2 action %1").arg(target->name(), type == In ? QObject::tr("in") : QObject::tr("out")), type, parent), m_pos( pos )
{
}

void StateAddActionCommand::undo()
{
    StateActionCommand::undo();
    targetList()->removeAt( m_pos );
}

void StateAddActionCommand::redo()
{
    targetList()->insert(m_pos, target());
    StateActionCommand::redo();
}

StateRemoveActionCommand::StateRemoveActionCommand(State *s, int pos, ActionType type, QUndoCommand *parent)
    : StateActionCommand(s, (type == In ? s->incomeActions : s->outcomeActions).at(pos).data(),
                         QObject::tr("remove %2 action %1").arg(itemAt(s, pos, type)->name(), type == In ?
                                                                    QObject::tr("in") : QObject::tr("out")),
                         type, parent), m_pos (pos)
{
}

void StateRemoveActionCommand::undo()
{
    StateActionCommand::undo();
    targetList()->insert(m_pos, target());
}

void StateRemoveActionCommand::redo()
{
    targetList()->removeAt( m_pos );
    StateActionCommand::redo();
}

StateMoveActionCommand::StateMoveActionCommand(State * s, int oldPos, int newPos, ActionType type, QUndoCommand * parent)
    : StateActionCommand(s, itemAt(s, oldPos, type).data(), QObject::tr("Move %1").arg(type == In ? QObject::tr("in") : QObject::tr("out")), type, parent),
      m_oldPos( oldPos ), m_newPos( newPos )
{
}

void StateMoveActionCommand::undo()
{
    StateActionCommand::undo();
    targetList()->moveRow(QModelIndex(), m_newPos, QModelIndex(), m_oldPos);
}

void StateMoveActionCommand::redo()
{
    targetList()->moveRow(QModelIndex(), m_oldPos, QModelIndex(), m_newPos);
    StateActionCommand::redo();
}

StateChangeCommentCommand::StateChangeCommentCommand(State * e, const QString& newComment, QUndoCommand * parent)
    : StateUndoCommand(e, QObject::tr("change comment"), parent), m_newComment(newComment), m_oldComment(e->comment())
{
}

void StateChangeCommentCommand::undo()
{
    StateUndoCommand::undo();
    state()->setComment( m_oldComment );
}
void StateChangeCommentCommand::redo()
{
    state()->setComment( m_newComment );
    StateUndoCommand::redo();
}
bool StateChangeCommentCommand::mergeWith(const QUndoCommand * command)
{
    auto other = dynamic_cast<const StateChangeCommentCommand*>( command );
    if (other && state() == other->state()) {
        m_newComment = other->m_newComment;
        return true;
    }
    return false;
}

TransitionMoveCommand::TransitionMoveCommand(Transition *s, int newPos, QUndoCommand *parent)
    : StateUndoCommand(s->parentState().data(), QObject::tr("move transition"), parent), m_oldPos(s->index()), m_newPos(newPos)
{
}

void TransitionMoveCommand::undo()
{
    auto s = state();
    if (s) {
        s->moveTransition( m_newPos, m_oldPos );
    }
}

void TransitionMoveCommand::redo()
{
    auto s = state();
    if (s) {
        s->moveTransition( m_oldPos, m_newPos );
    }
}


StateDeleteCommand::StateDeleteCommand(State * obj, QUndoCommand * parent)
    : StateUndoCommand(obj, QObject::tr("delete %1").arg(obj->name()), parent), m_pos( obj->center() )
{
    new StateChangeCommentCommand(obj, QString(), this);

    for( int i = obj->incomeActions.rowCount() - 1; i >= 0; --i)
        new StateRemoveActionCommand(obj, i, StateActionCommand::In, this);

    for( int i = obj->outcomeActions.rowCount() - 1; i >= 0; --i)
        new StateRemoveActionCommand(obj, i, StateActionCommand::Out, this);

    auto tl = obj->transitionList();
    for (auto i = tl.rbegin(); i != tl.rend(); ++i) {
        new TransitionDeleteCommand(i->data(), this);
    }

    auto fsm = m_document.toStrongRef();
    if (fsm) {
        // delete incoming transitions
        for( auto i = fsm->statesBegin(); i != fsm->statesEnd(); ++i ) {
            if (i->data() == obj) continue;
            auto tl = (*i)->transitionList();
            for (auto i = tl.rbegin(); i != tl.rend(); ++i) {
                if ((*i)->target.data() == obj)
                new TransitionDeleteCommand(i->data(), this);
            }
        }
    }
}

void StateDeleteCommand::undo()
{
    auto fsm = m_document.toStrongRef();
    if (fsm) {
        fsm->addState(m_stateName, m_pos);
    }
    QUndoCommand::undo();
}
void StateDeleteCommand::redo()
{
    QUndoCommand::redo();
    auto fsm = m_document.toStrongRef();
    if (fsm) {
        fsm->del(state());
    }
}
