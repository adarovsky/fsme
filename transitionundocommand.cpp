#include <QFontMetricsF>
#include "output.h"
#include "state.h"
#include "transition.h"
#include "statemachine.h"
#include "my_algo.h"
#include "transitionundocommand.h"

TransitionUndoCommand::TransitionUndoCommand(Transition * e, const QString& text, QUndoCommand * parent)
    : QUndoCommand( text, parent ),
      m_stateName(e->parentState()->name()),
      m_pos(e->index()),
      m_document(e->parent())
{
}

QSharedPointer<State> TransitionUndoCommand::state() const
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<State> state = fsm->findState( m_stateName );
        return state;
    }

    return QSharedPointer<State>();
}

QSharedPointer<Transition> TransitionUndoCommand::transition() const
{
    auto s = state();
    if (s) return s->transitionList()[m_pos];
    return QSharedPointer<Transition>();
}

TransitionAddCommand::TransitionAddCommand(QSharedPointer<State> source, QSharedPointer<State> dest, QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Create transition"), parent), m_document( source->parent() ),
      m_stateName( source->name() ), m_destName( dest->name() ), m_pos( 0 )
{
    // add a loop
    if (source == dest) {
        QString name = m_stateName;
        QFontMetricsF metrics(QFont("helvetica", 14, QFont::Medium));
        QRectF bounds = metrics.boundingRect(name).normalized().marginsAdded(QMarginsF(10, 10, 10, 10)).toAlignedRect();
        bounds.moveCenter( source->center() );
        m_initialCP = make_loop(bounds);
    }
}

bool TransitionAddCommand::mergeWith(const QUndoCommand *command)
{
    auto other = dynamic_cast<const RenameTransitionCommand*>( command );
    if ( other ) {
        m_newName = other->m_newName;
        return true;
    }
    return false;
}

void TransitionAddCommand::undo()
{
    QUndoCommand::undo();
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<State> state = fsm->findState( m_stateName );
        QSharedPointer<Transition> transition = state->transitionList()[m_pos];
        state->del( transition );
    }
}

void TransitionAddCommand::redo()
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<State> state = fsm->findState( m_stateName );
        QSharedPointer<State> dest = fsm->findState( m_destName );
        m_pos = state->transitionList().count();
        auto t = state->createTransition( m_pos, dest );
        t->setControlPoints(m_initialCP);
        if ( !m_newName.isEmpty() )
            t->rename( m_newName );
    }
    QUndoCommand::redo();
}

QSharedPointer<Transition> TransitionAddCommand::transition() const
{
    QSharedPointer<StateMachine> fsm( m_document );
    if (fsm) {
        QSharedPointer<State> state = fsm->findState( m_stateName );
        QSharedPointer<Transition> transition = state->transitionList()[m_pos];

        return transition;
    }

    return QSharedPointer<Transition>();
}

RenameTransitionCommand::RenameTransitionCommand(Transition *e, const QString &newName, QUndoCommand * parent)
    : TransitionUndoCommand(e, QObject::tr("Rename transition %1:%2 to %3\nrename transition").arg(e->parentState()->name(), e->name(), newName), parent),
      m_oldName(e->name()), m_newName(newName)
{
}

void RenameTransitionCommand::undo()
{
    TransitionUndoCommand::undo();
    auto t = transition();

    if ( t ) {
        t->rename( m_oldName );
    }
}

void RenameTransitionCommand::redo()
{
    auto t = transition();

    if ( t ) {
        t->rename( m_newName );
    }
    TransitionUndoCommand::redo();
}


TransitionChangeCommentCommand::TransitionChangeCommentCommand(Transition * e, const QString& newComment, QUndoCommand * parent)
    : TransitionUndoCommand(e, QObject::tr("Change comment"), parent), m_newComment(newComment), m_oldComment(e->comment())
{
}

void TransitionChangeCommentCommand::undo()
{
    TransitionUndoCommand::undo();
    transition()->setComment( m_oldComment );
}
void TransitionChangeCommentCommand::redo()
{
    transition()->setComment( m_newComment );
    TransitionUndoCommand::redo();
}
bool TransitionChangeCommentCommand::mergeWith(const QUndoCommand * command)
{
    auto other = dynamic_cast<const TransitionChangeCommentCommand*>( command );
    if (other && transition() == other->transition()) {
        m_newComment = other->m_newComment;
        return true;
    }
    return false;
}

TransitionChangeDestinationCommand::TransitionChangeDestinationCommand(Transition * e, const QString& newDestination, QUndoCommand * parent)
    : TransitionUndoCommand(e, QObject::tr("Change Destination"), parent), m_newDestination(newDestination)
{
    auto t = e->target.toStrongRef();
    if (t)
        m_oldDestination = t->name();

    QVector<QPointF> m_initialCP;
    // make a loop
    if (newDestination.isEmpty() || newDestination == e->parentState()->name()) {
        QString name = e->parentState()->name();
        QFontMetricsF metrics(QFont("helvetica", 14, QFont::Medium));
        QRectF bounds = metrics.boundingRect(name).normalized().marginsAdded(QMarginsF(10, 10, 10, 10)).toAlignedRect();
        bounds.moveCenter( state()->center() );
        m_initialCP = make_loop(bounds);
    }

    new TransitionTransformCommand(e, m_initialCP, this);
}

void TransitionChangeDestinationCommand::undo()
{
    transition()->target = transition()->parent()->findState( m_oldDestination );
    TransitionUndoCommand::undo();
}

void TransitionChangeDestinationCommand::redo()
{
    transition()->target = transition()->parent()->findState( m_newDestination );
    TransitionUndoCommand::redo();
}

TransitionChangeConditionCommand::TransitionChangeConditionCommand(Transition * e, const QString& newCondition, QUndoCommand * parent)
    : TransitionUndoCommand(e, QObject::tr("Change condition"), parent), m_newCondition(newCondition), m_oldCondition(e->condition() )
{
}

void TransitionChangeConditionCommand::undo()
{
    TransitionUndoCommand::undo();
    transition()->setCondition( m_oldCondition );
}
void TransitionChangeConditionCommand::redo()
{
    transition()->setCondition( m_newCondition );
    TransitionUndoCommand::redo();
}
bool TransitionChangeConditionCommand::mergeWith(const QUndoCommand * command)
{
    auto other = dynamic_cast<const TransitionChangeConditionCommand*>( command );
    if (other && transition() == other->transition()) {
        m_newCondition = other->m_newCondition;
        return true;
    }
    return false;
}

TransitionTransformCommand::TransitionTransformCommand(Transition * e, const QVector<QPointF>& controlPoints, QUndoCommand * parent )
    : TransitionUndoCommand(e, QObject::tr("Transform transition %1\ntransform").arg(e->name()), parent),
      m_newControlPoints( controlPoints ), m_oldControlPoints( e->controlPoints() )
{
}

void TransitionTransformCommand::undo()
{
    TransitionUndoCommand::undo();
    auto t = transition();
    if (t)
        t->setControlPoints( m_oldControlPoints );
}

void TransitionTransformCommand::redo()
{
    TransitionUndoCommand::redo();
    auto t = transition();
    if (t)
        t->setControlPoints( m_newControlPoints );
}

bool TransitionTransformCommand::mergeWith(const QUndoCommand *command)
{
    auto other = dynamic_cast<const TransitionTransformCommand*>( command );
    if ( other && transition() == other->transition() ) {
        m_newControlPoints = other->m_newControlPoints;
        return true;
    }

    return false;
}

TransitionActionCommand::TransitionActionCommand(Transition * s, EIOBase * target, const QString & text,QUndoCommand * parent)
    : TransitionUndoCommand(s, text, parent)
{
    QModelIndex i = target->parent()->indexForItem(target);
    m_folder = i.parent();
    m_index = i.row();
}

QSharedPointer<EIOBase> TransitionActionCommand::target() const
{
    QModelIndex i = m_folder.child(m_index, 0);
    return transition()->parent()->item( i );
}

OutputListModel * TransitionActionCommand::targetList() const
{
    return &transition()->actions;
}

QSharedPointer<EIOBase> TransitionActionCommand::itemAt(Transition * s, int pos)
{
    return s->actions.at(pos);
}

TransitionAddActionCommand::TransitionAddActionCommand(Transition * s, EIOBase * target, int pos, QUndoCommand * parent)
    : TransitionActionCommand(s, target, QObject::tr("add action %1").arg(target->name()), parent), m_pos( pos )
{
}

void TransitionAddActionCommand::undo()
{
    TransitionActionCommand::undo();
    targetList()->removeAt( m_pos );
}

void TransitionAddActionCommand::redo()
{
    targetList()->insert(m_pos, target());
    TransitionActionCommand::redo();
}

TransitionRemoveActionCommand::TransitionRemoveActionCommand(Transition *s, int pos, QUndoCommand *parent)
    : TransitionActionCommand(s, itemAt(s, pos).data(),
                         QObject::tr("remove action %1").arg(itemAt(s, pos)->name()), parent), m_pos (pos)
{
}

void TransitionRemoveActionCommand::undo()
{
    TransitionActionCommand::undo();
    targetList()->insert(m_pos, target());
}

void TransitionRemoveActionCommand::redo()
{
    targetList()->removeAt( m_pos );
    TransitionActionCommand::redo();
}

TransitionMoveActionCommand::TransitionMoveActionCommand(Transition * s, int oldPos, int newPos, QUndoCommand * parent)
    : TransitionActionCommand(s, itemAt(s, oldPos).data(), QObject::tr("move action"), parent),
      m_oldPos( oldPos ), m_newPos( newPos )
{
}

void TransitionMoveActionCommand::undo()
{
    TransitionActionCommand::undo();
    targetList()->moveRow(QModelIndex(), m_newPos, QModelIndex(), m_oldPos);
}

void TransitionMoveActionCommand::redo()
{
    targetList()->moveRow(QModelIndex(), m_oldPos, QModelIndex(), m_newPos);
    TransitionActionCommand::redo();
}

TransitionDeleteCommand::TransitionDeleteCommand(Transition * obj, QUndoCommand * parent)
    : TransitionUndoCommand(obj, QObject::tr("delete %1").arg(obj->name()), parent)
{
    new RenameTransitionCommand(obj, "", this);
    new TransitionChangeCommentCommand(obj, QString(), this);
    new TransitionChangeDestinationCommand(obj, "", this);
    new TransitionChangeConditionCommand(obj, "", this);

    for( int i = obj->actions.rowCount() - 1; i >= 0; --i)
        new TransitionRemoveActionCommand(obj, i, this);
}

void TransitionDeleteCommand::undo()
{
    auto s = state();
    s->createTransition(m_pos);

    QUndoCommand::undo();
}

void TransitionDeleteCommand::redo()
{
    QUndoCommand::redo();

    auto s = state();
    if (s) {
        s->del( transition() );
    }
}
