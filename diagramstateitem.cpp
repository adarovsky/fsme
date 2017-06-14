#include <QBrush>
#include <QPen>
#include <QFontMetricsF>
#include <QFont>
#include <QPainter>
#include <QStaticText>
#include <QtDebug>
#include <QUndoCommand>

#include "diagramstateitem.h"
#include "diagramscene.h"
#include "diagramtransitionitem.h"
#include "scopeguard.h"
#include "my_algo.h"

Q_DECLARE_LOGGING_CATEGORY(DiagramStateItemLogging)
Q_LOGGING_CATEGORY(DiagramStateItemLogging, "graphics.diagramscene.stateitem")

class StateMoveCommand : public QUndoCommand {
    DiagramScene * m_scene;
    QWeakPointer<StateMachine> m_document;
    QString m_stateName;
    QPointF m_oldPos;
    QPointF m_newPos;
public:
    StateMoveCommand( DiagramStateItem * item, const QPointF& newPos, QUndoCommand * parent = nullptr );
    virtual int id() const override { return 10001; }
    bool mergeWith(const QUndoCommand *command) override;
    virtual void undo() override;
    virtual void redo() override;
};

DiagramStateItem::DiagramStateItem(QSharedPointer<State> state, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), m_state( state ), m_updating( false )
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setPen( QPen(QBrush(QColor(Qt::black)), 1) );
    setBrush( QBrush(QColor(Qt::lightGray)) );

    QString name = state->name();
    QFontMetricsF metrics(QFont("helvetica", 14, QFont::Medium));

    QRectF bounds = metrics.boundingRect(name).normalized().marginsAdded(QMarginsF(10, 10, 10, 10)).toAlignedRect();
    bounds.moveCenter( QPointF(0, 0) );
    setRect(bounds);

    setPos( state->center() );
}

DiagramStateItem::~DiagramStateItem()
{
    qCDebug(DiagramStateItemLogging) << "deleting graphical for" << m_state.toStrongRef()->name();
}

void DiagramStateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QSharedPointer<State> state(m_state);
    if (state) {
        QPen currentPen;
        QBrush currentBrush;
        if ( isSelected() ) {
            currentPen = QPen( QBrush(QColor(Qt::blue)), 3 );
            currentBrush = brush();
        }
        else {
            currentPen = pen();
            if (state->parent()->initialState == state->name())
                currentPen.setWidth(2);
            currentBrush = brush();
        }

        painter->setBrush( currentBrush );
        painter->fillRect( rect(), currentBrush );
        painter->setPen( currentPen );
        painter->drawRect( rect() );

        painter->setPen( pen() );
        painter->setFont( QFont("helvetica", 14, QFont::Medium) );
        QPointF topLeft = rect().topLeft();
        painter->drawStaticText( topLeft.x() + 10, topLeft.y() + 10, QStaticText(state->name()) );
    }
}

QVariant DiagramStateItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ( change == QGraphicsItem::ItemPositionChange ) {
        SCOPE_GUARD_RETURN(m_updating, value);
        DiagramScene * sc = dynamic_cast<DiagramScene*>( scene() );
        if( !sc ) return value;

//        qCDebug(DiagramStateItemLogging) << "changed pos to " << value << ", current bounds:" << sceneBoundingRect();
        auto s = state();
        if ( s ) {
            StateMoveCommand * cmd = new StateMoveCommand( this, value.toPointF() );
            s->parent()->undoStack()->push( cmd );
        }
    }

    return value;
}

void DiagramStateItem::modelPositionChanged()
{
    DiagramScene * sc = dynamic_cast<DiagramScene*>( scene() );
    if( !sc ) return;

//        qCDebug(DiagramStateItemLogging) << "changed pos to " << value << ", current bounds:" << sceneBoundingRect();
    auto s = state();

    update();
    if ( s ) {
        QString name = s->name();
        QFontMetricsF metrics(QFont("helvetica", 14, QFont::Medium));

        QRectF bounds = metrics.boundingRect(name).normalized().marginsAdded(QMarginsF(10, 10, 10, 10)).toAlignedRect();
        bounds.moveCenter( QPointF(0, 0) );
        setRect(bounds);

        if (!qFuzzyCompare( dist(pos(), s->center()), 0 ) && !m_updating) {
            m_updating = true;
            setPos( s->center() );
            m_updating = false;
        }

        foreach (auto di, sc->graphicTransitionsOut( this )) {
            di->updateEnds();
        }
        foreach (auto di, sc->graphicTransitionsIn( this )) {
            di->updateEnds();
        }
    }
}


StateMoveCommand::StateMoveCommand( DiagramStateItem * item, const QPointF& newPos, QUndoCommand * parent )
    : QUndoCommand( QObject::tr("Move state %1\nmove").arg(item->state()->name()), parent ),
      m_scene( static_cast<DiagramScene*>( item->scene() ) ),
      m_document( item->state()->parent() ),
      m_stateName( item->state()->name() ),
      m_oldPos( item->state()->center() ), m_newPos( newPos )
{
}

void StateMoveCommand::undo()
{
    QUndoCommand::undo();

    auto doc = m_document.toStrongRef();
    if (!doc) return;

    auto state = doc->findState( m_stateName );
    if ( !state ) return;

    state->setCenter( m_oldPos );
}

void StateMoveCommand::redo()
{
    QUndoCommand::redo();

    auto doc = m_document.toStrongRef();
    if (!doc) return;

    auto state = doc->findState( m_stateName );
    if ( !state ) return;

    state->setCenter( m_newPos );
}

bool StateMoveCommand::mergeWith(const QUndoCommand *command)
{
    const StateMoveCommand * other = dynamic_cast<const StateMoveCommand*>( command );
    if (!other || other->m_stateName != m_stateName)
        return false;

//    qCDebug(DiagramStateItemLogging) << "merged m_oldPos:" << m_oldPos << ", m_newPos:" << m_newPos <<
//                "with" << other->m_newPos << ", other old was" << other->m_oldPos;
    m_newPos = other->m_newPos;
    return true;
}
