#include <QItemSelectionModel>
#include <QItemSelection>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QKeyEvent>

#include <QtDebug>
#include <QLoggingCategory>

#include "scopeguard.h"
#include "diagramscene.h"
#include "diagramstateitem.h"
#include "stateaddplaceholder.h"
#include "stateundocommand.h"
#include "diagramtransitionitem.h"
#include "diagramtransitionmanipulator.h"
#include "transitionundocommand.h"

Q_DECLARE_LOGGING_CATEGORY(DiagramSceneLogging)
Q_LOGGING_CATEGORY(DiagramSceneLogging, "graphics.diagramscene")

DiagramScene::DiagramScene( QObject * parent )
    : QGraphicsScene(parent), m_manipilator( nullptr ), m_selectionIsUpdating( false ), fsm( this )
{
    connect( this, SIGNAL(selectionChanged()), SLOT(diagramSelectionChanged()) );
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    m_eventHandle = Forward;
    fsm.processEvent( DiagramSceneFSM::MousePress, QVariantMap {
        { "pos", mouseEvent->scenePos() }
    });

    switch (m_eventHandle) {
    case Accept:
        mouseEvent->accept();
        break;
    case Reject:
        mouseEvent->ignore();
    default:
        QGraphicsScene::mousePressEvent( mouseEvent );
    }
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    m_eventHandle = Forward;
    fsm.processEvent( DiagramSceneFSM::MouseMove, QVariantMap {
        { "pos", mouseEvent->scenePos() }
    });

    switch (m_eventHandle) {
    case Accept:
        mouseEvent->accept();
        break;
    case Reject:
        mouseEvent->ignore();
    default:
        QGraphicsScene::mouseMoveEvent( mouseEvent );
    }
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    m_eventHandle = Forward;
    fsm.processEvent( DiagramSceneFSM::MouseRelease, QVariantMap {
        { "pos", mouseEvent->scenePos() }
    });

    switch (m_eventHandle) {
    case Accept:
        mouseEvent->accept();
        break;
    case Reject:
        mouseEvent->ignore();
    default:
        QGraphicsScene::mouseReleaseEvent( mouseEvent );
    }
}

bool DiagramScene::handleCancel(QKeyEvent * event)
{
    m_eventHandle = Forward;
    if (event && event->matches(QKeySequence::Cancel) )
        fsm.processEvent(DiagramSceneFSM::Cancel, QVariantMap());
    return m_eventHandle == Accept;
}

void DiagramScene::setStateMachine( QSharedPointer<StateMachine> fsm )
{
    m_document = fsm;
    connect( fsm.data(), SIGNAL(modelAboutToBeReset()), SLOT(modelAboutToBeReset()) );
    connect( fsm.data(), SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)) );
    connect( fsm.data(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );
    connect( fsm.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)) );

    modelAboutToBeReset();
    modelReset();
}

void DiagramScene::setItemSelectionModel(QItemSelectionModel *selectionModel)
{
    m_selectionModel = selectionModel;
    m_selectionIsUpdating = false;
    connect( selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)) );
}

void DiagramScene::modelAboutToBeReset()
{
    clear();
    m_manipilator = nullptr;
}

DiagramStateItem * DiagramScene::graphicState(QSharedPointer<State> state) const
{
    foreach ( auto item, items(Qt::DescendingOrder) ) {
        DiagramStateItem * s = dynamic_cast<DiagramStateItem*>(item);
        if (s && s->state() == state) {
//            qCDebug(DiagramSceneLogging) << "found graphic for " << state->name() << ":" << state->topLeft() << ":" << s->sceneBoundingRect();
            return s;
        }
    }

    qWarning() << "cannot find graphical state for" << state->name() << ":" << state->center();

    return nullptr;
}

DiagramTransitionItem * DiagramScene::graphicTransition( QSharedPointer<Transition> transition ) const
{
    foreach ( auto item, items(Qt::AscendingOrder) ) {
        DiagramTransitionItem * t = dynamic_cast<DiagramTransitionItem*>(item);
        if (t && t->transition() == transition)
            return t;
    }

    qWarning() << "cannot find graphical item for" << transition->parentState()->name() << "." << transition->name();

    return nullptr;
}

QList<DiagramTransitionItem*> DiagramScene::graphicTransitionsOut( DiagramStateItem * state ) const
{
    QList<DiagramTransitionItem*> result;

    foreach ( auto i, items(Qt::AscendingOrder) ) {
        DiagramTransitionItem * di = dynamic_cast<DiagramTransitionItem*>( i );
        if ( di ) {
            // outgoing
            if ( di->transition()->parentState() == state->state() )
                result << di;
        }
    }

    return result;
}

QList<DiagramTransitionItem*> DiagramScene::graphicTransitionsIn( DiagramStateItem * state ) const
{
    QList<DiagramTransitionItem*> result;
    auto s = state->state();

    foreach ( auto i, items(Qt::AscendingOrder) ) {
        DiagramTransitionItem * di = dynamic_cast<DiagramTransitionItem*>( i );
        if ( di ) {
            auto t = di->transition();
            if ( t && (t->target == s || (!t->target && t->parentState() == s)) ) {
                result << di;
            }
        }
    }

    return result;
}

void DiagramScene::modelReset()
{
    QSharedPointer<StateMachine> fsm = m_document;
    QRectF totalRect;
    if (fsm) {
        QModelIndex start = fsm->statesFolder();
        int stateCount = fsm->rowCount (start);
        for (int i = 0; i < stateCount; ++i) {
            QModelIndex index = fsm->index(i, 0, start);
            EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());
            State * s = dynamic_cast<State*>(p);
            if (s) {
                auto g = new DiagramStateItem(s->sharedFromThis().dynamicCast<State>());
                addItem(g);
                totalRect |= g->boundingRect();
            }
        }

        for (int i = 0; i < stateCount; ++i) {
            QModelIndex index = fsm->index(i, 0, start);
            EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());
            State * s = dynamic_cast<State*>(p);
            if (s) {
                foreach (auto t, s->transitionList()) {
                    auto tg = new DiagramTransitionItem( t );
                    addItem( tg );
                    tg->updateGeometry();
                    totalRect |= tg->boundingRect();
                }
            }
        }
    }
}

void DiagramScene::rowsInserted(const QModelIndex &parent, int first, int last)
{
    auto fsm = m_document.toStrongRef();
    if ( !fsm ) return;

    if ( parent == fsm->statesFolder() ) {
        for (int i = first; i < last; ++i) {
            QModelIndex index = fsm->index(i, 0, parent);
            EIOBase * p = reinterpret_cast<EIOBase*>(index.internalPointer());
            State * s = dynamic_cast<State*>(p);
            if (s) {
                auto g = new DiagramStateItem(s->sharedFromThis().dynamicCast<State>());
                addItem(g);
            }
        }
    }
    else {
        State * s = dynamic_cast<State*>( reinterpret_cast<EIOBase*>( parent.internalPointer() ) );
        if ( s ) {
            auto list = s->transitionList();
            for( int i = first; i < last; ++i ) {
                auto t = list[ i ];
                auto tg = new DiagramTransitionItem( t );
                addItem( tg );
                tg->updateGeometry();
            }
        }
    }
}

void DiagramScene::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &/*roles*/)
{
    auto fsm = m_document.toStrongRef();
    if ( !fsm ) return;

    QModelIndex parent = topLeft.parent();

    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        QModelIndex index = parent.child( i, 0 );
        State * s = dynamic_cast<State*>( reinterpret_cast<EIOBase*>( index.internalPointer() ) );

        if ( s ) {
            auto state = s->sharedFromThis().dynamicCast<State>();
            DiagramStateItem * g = graphicState( state );
            if ( g )
                g->modelPositionChanged();
            continue;
        }

        Transition * t = dynamic_cast<Transition*>( reinterpret_cast<EIOBase*>( index.internalPointer() ) );
        if ( t ) {
            auto transition = t->parentState()->transitionList()[ i ];
            DiagramTransitionItem * g = graphicTransition( transition );
            if ( g ) {
                g->updateGeometry();
                g->update();
            }

            if ( selectedItems().contains( g ) && m_manipilator->isVisible() ) {
                m_manipilator->editTransition( g );
            }
            continue;
        }
    }
}

void DiagramScene::rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    auto fsm = m_document.toStrongRef();
    if ( !fsm ) return;

    for (int i = first; i <= last; ++i) {
        QModelIndex index = parent.child( i, 0 );
        State * s = dynamic_cast<State*>( reinterpret_cast<EIOBase*>( index.internalPointer() ) );

        if ( s ) {
            auto state = s->sharedFromThis().dynamicCast<State>();
            DiagramStateItem * g = graphicState( state );
            if ( g ) {
                foreach (auto t, graphicTransitionsIn( g )) {
                    if (m_manipilator && m_manipilator->editedTransition() == t)
                        m_manipilator->editTransition( nullptr );
                    removeItem( t );
                    delete t;
                }
                foreach (auto t, graphicTransitionsOut( g )) {
                    if (m_manipilator && m_manipilator->editedTransition() == t)
                        m_manipilator->editTransition( nullptr );
                    removeItem( t );
                    delete t;
                }

                removeItem( g );
                delete g;
            }
            continue;
        }

        Transition * t = dynamic_cast<Transition*>( reinterpret_cast<EIOBase*>( index.internalPointer() ) );
        if ( t ) {
            auto transition = t->parentState()->transitionList()[ i ];
            DiagramTransitionItem * g = graphicTransition( transition );
            if ( g ) {
                if (m_manipilator && m_manipilator->editedTransition() == g)
                    m_manipilator->editTransition( nullptr );

                removeItem( g );
                delete g;
            }
            continue;
        }
    }
}

void DiagramScene::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if ( m_selectionIsUpdating ) return;

    m_selectionIsUpdating = true;
    scope_guard selGuard = [&]() {
        m_selectionIsUpdating = false;
    };

    foreach ( auto index, deselected.indexes() ) {
        EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
        State * s = dynamic_cast<State*>( p );
        Transition * t = dynamic_cast<Transition*>( p );

        if ( s ) {
            DiagramStateItem * g = graphicState( s->sharedFromThis().dynamicCast<State>() );
            if ( g ) g->setSelected( false );
        }
        else if ( t ) {
            auto state = t->parentState();
            auto transition = state->transitionList()[ t->index() ];
            DiagramTransitionItem * g = graphicTransition( transition );
            if ( g ) g->setSelected( false );
        }
    }

    foreach ( auto index, selected.indexes() ) {
        EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
        State * s = dynamic_cast<State*>( p );
        Transition * t = dynamic_cast<Transition*>( p );

        if ( s ) {
            DiagramStateItem * g = graphicState( s->sharedFromThis().dynamicCast<State>() );
            if ( g ) g->setSelected( true );
        }
        else if ( t ) {
            auto state = t->parentState();
            auto transition = state->transitionList()[ t->index() ];
            DiagramTransitionItem * g = graphicTransition( transition );
            if ( g ) g->setSelected( true );
        }
    }
}

DiagramTransitionManipulator * DiagramScene::editTransition(DiagramTransitionItem *item)
{
    if( m_manipilator == nullptr ) {
        m_manipilator = new DiagramTransitionManipulator();
        m_manipilator->editTransition( item );
        addItem( m_manipilator );
    }
    else
        m_manipilator->editTransition( item );

    return m_manipilator;
}

void DiagramScene::diagramSelectionChanged()
{
    if (selectedItems().count() == 1 && selectedItems().front()->type() == DiagramTransitionItem::Type ) {
        editTransition( dynamic_cast<DiagramTransitionItem*>( selectedItems().front() ) );
    }
    else if ( m_manipilator )
        m_manipilator->hide();

    if ( m_selectionIsUpdating ) return;

    m_selectionIsUpdating = true;
    scope_guard selGuard = [&]() {
        m_selectionIsUpdating = false;
    };

    auto fsm = m_document.toStrongRef();
    if (!fsm) return;

    QItemSelection result;

    foreach (auto selected, selectedItems()) {
        auto s = dynamic_cast<DiagramStateItem*>( selected );
        if ( s && s->state() ) {
            QModelIndex i = fsm->indexForItem( s->state().data() );
            result.select( i, i );
            continue;
        }

        auto t = dynamic_cast<DiagramTransitionItem*>( selected );
        if ( t && t->transition() ) {
            QModelIndex i = fsm->indexForItem( t->transition().data() );
            result.select( i, i );
            continue;
        }
    }

    if ( itemSelectionModel() && result.count() > 0 )
        itemSelectionModel()->select( result, QItemSelectionModel::ClearAndSelect );
}

void DiagramScene::prepareToAddState()
{
    fsm.processEvent(DiagramSceneFSM::AddState, QVariantMap());
}

void DiagramScene::prepareToAddTransition()
{
    fsm.processEvent(DiagramSceneFSM::AddTransition, QVariantMap());
}

void DiagramScene::cancelAdding()
{
    fsm.processEvent(DiagramSceneFSM::Cancel, QVariantMap());
}

void DiagramScene::stateNameEntered(const QString& text)
{
    fsm.processEvent(DiagramSceneFSM::NameEntered, QVariantMap {
        { "name", text }
    });
}

bool DiagramScene::isPointingToState(const QVariantMap& params) const
{
    Q_ASSERT( params.contains("pos") );
    QPointF pos = params["pos"].toPointF();

    foreach (auto item, items( pos )) {
        if ( item->type() == DiagramStateItem::Type )
            return true;
    }

    return false;
}

bool DiagramScene::isStartingFromState(const QVariantMap&) const
{
    return ( selectedItems().count() == 1 && selectedItems().front()->type() == DiagramStateItem::Type );
}

void DiagramScene::acceptEvent(const QVariantMap&)
{
    m_eventHandle = Accept;
}

void DiagramScene::addNameField(const QVariantMap& params)
{
    Q_ASSERT( params.contains("pos") );
    QPointF pos = params["pos"].toPointF();

    m_statePlaceholder = new StateAddPlaceholder();
    addItem( m_statePlaceholder );
    m_statePlaceholder->setPos( pos );

    connect(m_statePlaceholder, SIGNAL(editCanceled()), SLOT(cancelAdding()));
    connect(m_statePlaceholder, SIGNAL(editFinished(QString)), SLOT(stateNameEntered(QString)));
}

void DiagramScene::addState(const QVariantMap&)
{
    auto fsm = m_document.toStrongRef();
    if (fsm) {
        StateAddCommand * cmd = new StateAddCommand( m_statePlaceholder->toPlainText().simplified(), m_statePlaceholder->pos(), fsm );
        fsm->undoStack()->push( cmd );
    }
}

void DiagramScene::addTransition(const QVariantMap& params)
{
    Q_ASSERT( params.contains("pos") );
    QPointF pos = params["pos"].toPointF();

    QSharedPointer<State> source, dest;
    foreach (auto item, selectedItems()) {
        if ( item->type() == DiagramStateItem::Type ) {
            DiagramStateItem * s = dynamic_cast<DiagramStateItem*>( item );
            source = s->state();
            break;
        }
    }

    foreach (auto item, items( pos )) {
        if ( item->type() == DiagramStateItem::Type ) {
            DiagramStateItem * s = dynamic_cast<DiagramStateItem*>( item );
            dest = s->state();
            break;
        }
    }

    qCDebug(DiagramSceneLogging) << "adding transition from " << source->name() << "to" << dest->name();

    auto fsm = m_document.toStrongRef();
    if (fsm) {
        auto cmd = new TransitionAddCommand( source, dest );
        fsm->undoStack()->push( cmd );
        QModelIndex index = fsm->indexForItem( cmd->transition().data() );
        emit requestEditingItem( index );
    }
}

void DiagramScene::removeNameField(const QVariantMap&)
{
    removeItem( m_statePlaceholder );
    m_statePlaceholder->deleteLater();
    m_statePlaceholder = nullptr;
}

void DiagramScene::removePreviewArrow(const QVariantMap& params)
{
    removeItem( m_transitionPlaceholder );
    delete m_transitionPlaceholder;
    m_transitionPlaceholder = nullptr;
}

void DiagramScene::selectState(const QVariantMap& params)
{
    Q_ASSERT( params.contains("pos") );
    QPointF pos = params["pos"].toPointF();

    foreach (auto item, items( pos )) {
        if ( item->type() == DiagramStateItem::Type )
            item->setSelected( true );
    }
}

void DiagramScene::setPointerToArrow(const QVariantMap& params)
{
    QApplication::restoreOverrideCursor();
}

void DiagramScene::setPointerToCross(const QVariantMap& params)
{
    QApplication::setOverrideCursor( Qt::CrossCursor );
}

void DiagramScene::showPreviewArrow(const QVariantMap& params)
{
    QPointF start;
    foreach (auto item, selectedItems()) {
        if ( item->type() == DiagramStateItem::Type ) {
            start = item->pos() + item->boundingRect().center();
            break;
        }
    }

    m_transitionPlaceholder = new QGraphicsLineItem( QLineF(start, start) );
    m_transitionPlaceholder->setAcceptHoverEvents( true );
    addItem( m_transitionPlaceholder );
}

void DiagramScene::updatePreviewArrow(const QVariantMap& params)
{
    Q_ASSERT( params.contains("pos") );
    QPointF pos = params["pos"].toPointF();

    QLineF line = m_transitionPlaceholder->line();
    line.setP2( pos );
    m_transitionPlaceholder->setLine( line );
}
