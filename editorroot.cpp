#include "editorroot.h"
#include "statemachine.h"
#include "ui_editorroot.h"
#include "scopeguard.h"
#include "rootundocommand.h"

EditorRoot::EditorRoot(QWidget *parent)
    : QWidget(parent),
      ui( new Ui::EditorRoot )
{
    ui->setupUi( this );
    connect(ui->nameField, &QLineEdit::textChanged, [=] ( const QString &newValue ) {
        SCOPE_GUARD(m_editing);
        object()->parent()->undoStack()->push(object()->renameCommand(newValue));
    });
    connect(ui->commentField, &QPlainTextEdit::textChanged, [=] () {
        SCOPE_GUARD(m_editing);
        auto cmd = new RootChangeCommentCommand( object(), ui->commentField->toPlainText() );
        object()->parent()->undoStack()->push( cmd );
    });
    connect(ui->baseClassField,&QLineEdit::textChanged, [=] ( const QString &newValue ) {
        SCOPE_GUARD(m_editing);
        auto cmd = new RootChangeBaseClassCommand( object(), newValue );
        object()->parent()->undoStack()->push( cmd );
    });
    connect(ui->initialStateCombo, &QComboBox::currentTextChanged, [=] (const QString& text) {
        SCOPE_GUARD(m_editing);
        QString old = object()->initialState();
        if (old != text) {
            auto cmd = new RootChangeInitialStateCommand( object(), text );
            object()->parent()->undoStack()->push( cmd );
        }
    });
}

bool EditorRoot::canStartEditing(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    return dynamic_cast<Root*>(p) != nullptr;
}

Root * EditorRoot::object() const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( m_editedObject.internalPointer() );
    return dynamic_cast<Root*>(p);
}

void EditorRoot::updateUI()
{
    SCOPE_GUARD(m_editing);
    {
        QSignalBlocker _b(ui->nameField);
        ui->nameField->setText( object()->name() );
    }

    {
        QSignalBlocker _b(ui->commentField);
        ui->commentField->setPlainText( object()->comment() );
    }

    {
        QSignalBlocker _b(ui->baseClassField);
        ui->baseClassField->setText( object()->baseClass() );
    }

    {
        QSignalBlocker _b(ui->initialStateCombo);
        ui->initialStateCombo->setModel( object()->parent().data() );
        ui->initialStateCombo->setRootModelIndex( object()->parent()->statesFolder() );
        ui->initialStateCombo->setCurrentText( object()->initialState() );
    }
}
