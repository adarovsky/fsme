#include <QPlainTextEdit>
#include <QCompleter>

#include "editortransition.h"
#include "ui_editortransition.h"
#include "scopeguard.h"
#include "statemachine.h"
#include "availableactionsmodel.h"
#include "transitionundocommand.h"
#include "stateundocommand.h"
#include "syntaxhighlighter.h"
#include "eventsinputsmodel.h"
#include "transitiontargetsmodel.h"
#include "textedit.h"

EditorTransition::EditorTransition(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditorTransition), m_editing(false), m_eventsInputs( new EventsInputsModel ), m_targets( new TransitionTargetsModel )
{
    ui->setupUi(this);

    m_syntaxHighlighter.reset( new EIOHighlighter(ui->conditionField->document() ) );
    m_completer.reset( new QCompleter(m_eventsInputs.data()) );
    ui->conditionField->setCompleter(m_completer.data());

    connect(ui->nameField, &QLineEdit::textChanged, [=] ( const QString &newValue ) {
        SCOPE_GUARD(m_editing);
        auto t = transition();
        if ( t ) {
            t->parent()->undoStack()->push(transition()->renameCommand(newValue));
        }
    });
    connect(ui->commentField, &QPlainTextEdit::textChanged, [=] () {
        SCOPE_GUARD(m_editing);
        auto t = transition();
        if ( t ) {
            auto cmd = new TransitionChangeCommentCommand( t.data(), ui->commentField->toPlainText() );
            t->parent()->undoStack()->push( cmd );
        }
    });
    connect(ui->conditionField, &QPlainTextEdit::textChanged, [=] () {
        SCOPE_GUARD(m_editing);
        auto t = transition();
        if ( t ) {
            auto cmd = new TransitionChangeConditionCommand( t.data(), ui->conditionField->toPlainText() );
            t->parent()->undoStack()->push( cmd );
        }
    });
}

EditorTransition::~EditorTransition()
{
    delete ui;
}

void EditorTransition::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool EditorTransition::canStartEditing(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    return dynamic_cast<Transition*>(p) != nullptr;
}

QSharedPointer<Transition> EditorTransition::transition() const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( m_editedObject.internalPointer() );
    if (!p) return QSharedPointer<Transition>();
    return p->sharedFromThis().dynamicCast<Transition>();
}

void EditorTransition::updateUI()
{
    SCOPE_GUARD(m_editing);

    {
        QSignalBlocker _b(ui->nameField);
        ui->nameField->setText( transition()->name() );
    }

    {
        QSignalBlocker _b(ui->commentField);
        ui->commentField->setPlainText( transition()->comment() );
    }

    {
        QSignalBlocker _b(ui->conditionField);
        m_syntaxHighlighter->setStateMachine( transition()->parent() );
        m_eventsInputs->setSourceModel( transition()->parent().data() );
        ui->conditionField->setPlainText( transition()->condition() );
    }

    {
        QSignalBlocker _b(ui->destinationField);
        m_targets->setSourceModel( transition()->parent().data() );
        ui->destinationField->setModel( m_targets.data() );
        auto t = transition()->target.toStrongRef();
        ui->destinationField->setCurrentText( t ? t->name() : QString::null );
    }

    {
        QSignalBlocker _b(ui->availableActions);

        m_availableActions.reset( new AvailableActionsModel );
        m_availableActions->setSourceModel( transition()->parent().data() );
        ui->availableActions->setModel( m_availableActions.data() );
        connect(ui->availableActions->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditorTransition::availSelectionChanged);
        connect(ui->availableActions->model(), &QAbstractItemModel::rowsMoved, this, &EditorTransition::availSelectionChanged);
        availSelectionChanged();
    }

    {
        QSignalBlocker _b(ui->actionList);
        ui->actionList->setModel( &transition()->actions );
        connect(ui->actionList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditorTransition::actionSelectionChanged);
        connect(ui->actionList->model(), &QAbstractItemModel::rowsMoved, this, &EditorTransition::actionSelectionChanged);
        actionSelectionChanged();
    }

    ui->moveTransitionUp->setEnabled( transition()->index() > 0 );
    ui->moveTransitionDown->setEnabled( transition()->index() < transition()->parentState()->transitionList().count() - 1);
}


void EditorTransition::availSelectionChanged()
{
    QModelIndex current;
    if (ui->availableActions->selectionModel() && ui->availableActions->selectionModel()->selectedRows().count() > 0)
        current = ui->availableActions->selectionModel()->selectedRows().first();
    bool selected = current.isValid();
    ui->addAction->setEnabled( selected );
}

void EditorTransition::actionSelectionChanged()
{
    QModelIndex current;
    if (ui->actionList->selectionModel() && ui->actionList->selectionModel()->selectedRows().count() > 0)
        current = ui->actionList->selectionModel()->selectedRows().first();
    bool selected = current.isValid();
    ui->moveActionUp->setEnabled( selected && current.row() > 0);
    ui->moveActionDown->setEnabled( selected && current.row() < ui->actionList->model()->rowCount() - 1);
    ui->removeAction->setEnabled( selected );
}

void EditorTransition::moveActionUp()
{
    auto sel = ui->actionList->selectionModel();
    auto cmd = new TransitionMoveActionCommand(transition().data(), sel->currentIndex().row(), sel->currentIndex().row() - 1);
    transition()->parent()->undoStack()->push(cmd);
}

void EditorTransition::moveActionDown()
{
    auto sel = ui->actionList->selectionModel();
    auto cmd = new TransitionMoveActionCommand(transition().data(), sel->currentIndex().row(), sel->currentIndex().row() + 1);
    transition()->parent()->undoStack()->push(cmd);
}

void EditorTransition::addAction()
{
    auto selected = ui->availableActions->selectionModel()->selectedIndexes();
    auto fsm = transition()->parent();
    if (selected.count() > 0 && fsm) {
        EIOBase * item = reinterpret_cast<EIOBase*>(selected.first().internalPointer());
        if (item) {
            int pos = ui->actionList->currentIndex().isValid() ? ui->actionList->currentIndex().row() + 1 : transition()->actions.rowCount();
            auto cmd = new TransitionAddActionCommand(transition().data(), item, pos);
            fsm->undoStack()->push( cmd );
        }
    }
}

void EditorTransition::removeAction()
{
    auto selected = ui->actionList->selectionModel()->selectedIndexes();
    auto fsm = transition()->parent();
    if (selected.count() > 0 && fsm) {
        auto cmd = new TransitionRemoveActionCommand(transition().data(), selected.first().row());
        fsm->undoStack()->push( cmd );
    }
}

void EditorTransition::setDestination(const QString & destination)
{
    SCOPE_GUARD(m_editing);
    auto cmd = new TransitionChangeDestinationCommand( transition().data(), destination );
    transition()->parent()->undoStack()->push( cmd );
}

void EditorTransition::moveTransitionUp()
{
    SCOPE_GUARD(m_editing);
    auto cmd = new TransitionMoveCommand( transition().data(), transition()->index() - 1 );
    transition()->parent()->undoStack()->push( cmd );
    ui->moveTransitionUp->setEnabled( transition()->index() > 0 );
    ui->moveTransitionDown->setEnabled( transition()->index() < transition()->parentState()->transitionList().count() - 1);
}

void EditorTransition::moveTransitionDown()
{
    SCOPE_GUARD(m_editing);
    auto cmd = new TransitionMoveCommand( transition().data(), transition()->index() + 1 );
    transition()->parent()->undoStack()->push( cmd );
    ui->moveTransitionUp->setEnabled( transition()->index() > 0 );
    ui->moveTransitionDown->setEnabled( transition()->index() < transition()->parentState()->transitionList().count() - 1);
}

