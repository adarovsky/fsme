#include <QtDebug>

#include "editorstate.h"
#include "scopeguard.h"
#include "statemachine.h"
#include "stateundocommand.h"
#include "ui_editorstate.h"
#include "availableactionsmodel.h"

EditorState::EditorState(QWidget *parent)
    : QWidget(parent),
      ui( new Ui::EditorState )
{
    ui->setupUi( this );
    connect(ui->nameField, &QLineEdit::textChanged, [=] ( const QString &newValue ) {
        SCOPE_GUARD(m_editing);
        auto cmd = state()->renameCommand(newValue);
        if (cmd)
            state()->parent()->undoStack()->push(cmd);
    });
    connect(ui->commentField, &QPlainTextEdit::textChanged, [=] () {
        SCOPE_GUARD(m_editing);
        auto cmd = new StateChangeCommentCommand( state(), ui->commentField->toPlainText() );
        state()->parent()->undoStack()->push( cmd );
    });
}

EditorState::~EditorState()
{
    ui->availableActions->setModel( nullptr );
    ui->inActions->setModel( nullptr );
    ui->outActions->setModel( nullptr );
}

bool EditorState::canStartEditing(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    return dynamic_cast<State*>(p) != nullptr;
}

State * EditorState::state() const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( m_editedObject.internalPointer() );
    return dynamic_cast<State*>(p);
}

void EditorState::updateUI()
{
    SCOPE_GUARD(m_editing);

    {
        QSignalBlocker _b(ui->nameField);
        ui->nameField->setText( state()->name() );
    }

    {
        QSignalBlocker _b(ui->commentField);
        ui->commentField->setPlainText( state()->comment() );
    }

    {
        QSignalBlocker _b(ui->availableActions);

        m_availableActions.reset( new AvailableActionsModel );
        m_availableActions->setSourceModel( state()->parent().data() );
        ui->availableActions->setModel( m_availableActions.data() );
        connect(ui->availableActions->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditorState::availSelectionChanged);
        connect(ui->availableActions->model(), &QAbstractItemModel::rowsMoved, this, &EditorState::availSelectionChanged);
        availSelectionChanged();
    }

    {
        QSignalBlocker _b(ui->inActions);
        ui->inActions->setModel( &state()->incomeActions );
        connect(ui->inActions->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditorState::inSelectionChanged);
        connect(ui->inActions->model(), &QAbstractItemModel::rowsMoved, this, &EditorState::inSelectionChanged);
        inSelectionChanged();
    }

    {
        QSignalBlocker _b(ui->outActions);
        ui->outActions->setModel( &state()->outcomeActions );
        connect(ui->outActions->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditorState::outSelectionChanged);
        connect(ui->outActions->model(), &QAbstractItemModel::rowsMoved, this, &EditorState::outSelectionChanged);
        outSelectionChanged();
    }
}

void EditorState::inSelectionChanged()
{
    QModelIndex current;
    if (ui->inActions->selectionModel() && ui->inActions->selectionModel()->selectedRows().count() > 0)
        current = ui->inActions->selectionModel()->selectedRows().first();
    bool selected = current.isValid();
    ui->moveInUp->setEnabled( selected && current.row() > 0 );
    ui->moveInDown->setEnabled( selected && current.row() < ui->inActions->model()->rowCount() - 1);
    ui->removeIn->setEnabled( selected );
}

void EditorState::outSelectionChanged()
{
    QModelIndex current;
    if (ui->outActions->selectionModel() && ui->outActions->selectionModel()->selectedRows().count() > 0)
        current = ui->outActions->selectionModel()->selectedRows().first();
    bool selected = current.isValid();
    ui->moveOutUp->setEnabled( selected && current.row() > 0);
    ui->moveOutDown->setEnabled( selected && current.row() < ui->outActions->model()->rowCount() - 1);
    ui->removeOut->setEnabled( selected );
}

void EditorState::availSelectionChanged()
{
    QModelIndex current;
    if (ui->availableActions->selectionModel() && ui->availableActions->selectionModel()->selectedRows().count() > 0)
        current = ui->availableActions->selectionModel()->selectedRows().first();
    bool selected = current.isValid();
    ui->addIn->setEnabled( selected );
    ui->addOut->setEnabled( selected );
}

void EditorState::moveInUp()
{
    auto sel = ui->inActions->selectionModel();
    auto cmd = new StateMoveActionCommand(state(), sel->currentIndex().row(), sel->currentIndex().row() - 1, StateActionCommand::In);
    state()->parent()->undoStack()->push(cmd);
}

void EditorState::moveInDown()
{
    auto sel = ui->inActions->selectionModel();
    auto cmd = new StateMoveActionCommand(state(), sel->currentIndex().row(), sel->currentIndex().row() + 1, StateActionCommand::In);
    state()->parent()->undoStack()->push(cmd);
}

void EditorState::moveOutUp()
{
    auto sel = ui->outActions->selectionModel();
    auto cmd = new StateMoveActionCommand(state(), sel->currentIndex().row(), sel->currentIndex().row() - 1, StateActionCommand::Out);
    state()->parent()->undoStack()->push(cmd);
}

void EditorState::moveOutDown()
{
    auto sel = ui->outActions->selectionModel();
    auto cmd = new StateMoveActionCommand(state(), sel->currentIndex().row(), sel->currentIndex().row() + 1, StateActionCommand::Out);
    state()->parent()->undoStack()->push(cmd);
}

void EditorState::addIn()
{
    auto selected = ui->availableActions->selectionModel()->selectedIndexes();
    auto fsm = state()->parent();
    if (selected.count() > 0 && fsm) {
        EIOBase * item = reinterpret_cast<EIOBase*>(selected.first().internalPointer());
        if (item) {
            int pos = ui->inActions->currentIndex().isValid() ? ui->inActions->currentIndex().row() + 1 : state()->incomeActions.rowCount();
            auto cmd = new StateAddActionCommand(state(), item, pos, StateActionCommand::In);
            fsm->undoStack()->push( cmd );
        }
    }
}

void EditorState::removeIn()
{
    auto selected = ui->inActions->selectionModel()->selectedIndexes();
    auto fsm = state()->parent();
    if (selected.count() > 0 && fsm) {
        auto cmd = new StateRemoveActionCommand(state(), selected.first().row(), StateActionCommand::In);
        fsm->undoStack()->push( cmd );
    }
}

void EditorState::addOut()
{
    auto selected = ui->availableActions->selectionModel()->selectedIndexes();
    auto fsm = state()->parent();
    if (selected.count() > 0 && fsm) {
        EIOBase * item = reinterpret_cast<EIOBase*>(selected.first().internalPointer());
        if (item) {
            int pos = ui->outActions->currentIndex().isValid() ? ui->outActions->currentIndex().row() + 1 : state()->outcomeActions.rowCount();
            auto cmd = new StateAddActionCommand(state(), item, pos, StateActionCommand::Out);
            fsm->undoStack()->push( cmd );
        }
    }
}

void EditorState::removeOut()
{
    auto selected = ui->outActions->selectionModel()->selectedIndexes();
    auto fsm = state()->parent();
    if (selected.count() > 0 && fsm) {
        auto cmd = new StateRemoveActionCommand(state(), selected.first().row(), StateActionCommand::Out);
        fsm->undoStack()->push( cmd );
    }
}

