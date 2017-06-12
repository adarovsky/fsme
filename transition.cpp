#include "statemachine.h"
#include "transitionundocommand.h"

Transition::TransitionType Transition::parseTransitionType( const QString& tt )
{
    if( tt == "simple" )
        return Transition::Simple;
    else if( tt == "save_source" )
        return Transition::SaveSource;
    else if( tt == "transite" )
        return Transition::Transite;
    else if( tt == "return" )
        return Transition::Return;
    else return Transition::Simple;
}

QString Transition::unparseTransitionType( Transition::TransitionType tt )
{
    switch( tt ) {
    case Transition::Simple:
        return "simple";
    case Transition::SaveSource:
        return "save_source";
    case Transition::Transite:
        return "transite";
    case Transition::Return:
        return "return";
    }
    return "simple";
}

Transition::Transition( QSharedPointer<State> parent )
    : EIOBase( "", parent->parent() ), _parent( parent ), actions(this),  transitionType( Simple )
{
}

bool Transition::rename( QString newName )
{
    _name = newName;
    parent()->informItemChanged(this);
    return true;
}

int Transition::index() const
{
    QSharedPointer<State> state(_parent);
    if (state) {
        for (int i = 0; i < state->_transitions.count(); ++i) {
            if (state->_transitions[i].data() == this)
                return i;
        }
    }

    return -1;
}

bool Transition::canDelete() const
{
    return true;
}

QUndoCommand * Transition::deleteCommand()
{
    return new TransitionDeleteCommand( this );
}


QDomNode Transition::save( QDomDocument doc )
{
    QDomElement e = doc.createElement( "transition" );
    e.setAttribute( "type", unparseTransitionType(transitionType) );

    QDomElement eName = doc.createElement( "name" );
    QDomText nameText = doc.createTextNode( name() );
    QDomElement eComment = doc.createElement( "comment" );
    QDomText commentText = doc.createTextNode( comment() );
    e.appendChild( eName );
    e.appendChild( eComment );
    eName.appendChild( nameText );
    eComment.appendChild( commentText );

    QDomElement eCondition = doc.createElement( "condition" );
    QDomText conditionText = doc.createTextNode( condition() );
    e.appendChild( eCondition );
    eCondition.appendChild( conditionText );

    QDomElement eTarget = doc.createElement( "destination" );
    QSharedPointer<State> targetState(target);
    QDomText targetText = doc.createTextNode( targetState ? targetState->name() : "");
    e.appendChild( eTarget );
    eTarget.appendChild( targetText );

    QDomElement eTTarget = doc.createElement( "transite_destination" );
    QSharedPointer<State> transiteTargetState(target);
    QDomText ttargetText = doc.createTextNode( transiteTargetState ? transiteTargetState->name() : "" );
    e.appendChild( eTTarget );
    eTTarget.appendChild( ttargetText );

    QDomElement eTrans = doc.createElement( "transitionactions" );
    e.appendChild( eTrans );
    for( auto i = actions.begin(); i != actions.end(); ++i ) {
        auto b = i->toStrongRef();

        QDomElement eAction = doc.createElement( "action" );
        eAction.setAttribute( "type", b.dynamicCast<Output>() ? "output" : "event" );
        QDomText actionText = doc.createTextNode( b->name() );
        eTrans.appendChild( eAction );
        eAction.appendChild( actionText );
    }

    return e;
}

void Transition::setControlPoints( const QVector<QPointF> &cp )
{
    m_controlPoints = cp;
    parent()->informItemChanged( this, StateMachine::GraphicPositionRole );
}

void Transition::setCondition( const QString& condition )
{
    m_condition = condition;
    parent()->informItemChanged( this, StateMachine::ContentChangedRole );
}

QUndoCommand * Transition::renameCommand(const QString &newName)
{
    return new RenameTransitionCommand(this, newName);
}
