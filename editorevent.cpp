#include "editorevent.h"
#include "statemachine.h"
#include "ui_editorbase.h"
#include "scopeguard.h"

EditorEvent::EditorEvent(QWidget *parent)
    : QWidget(parent),
      ui( new Ui::EditorBase )
{
    ui->setupUi( this );
    connect(ui->nameField, &QLineEdit::textChanged, [=] ( const QString &newValue ) {
        SCOPE_GUARD(m_editing);
        auto cmd = object()->renameCommand(newValue);
        if (cmd)
            object()->parent()->undoStack()->push(cmd);
    });
    connect(ui->commentField, &QPlainTextEdit::textChanged, [=] () {
        SCOPE_GUARD(m_editing);
        auto cmd = new EventChangeCommentCommand( object(), ui->commentField->toPlainText() );
        object()->parent()->undoStack()->push( cmd );
    });
}

bool EditorEvent::canStartEditing(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    return dynamic_cast<Event*>(p) != nullptr;
}

Event * EditorEvent::object() const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( m_editedObject.internalPointer() );
    return dynamic_cast<Event*>(p);
}

void EditorEvent::updateUI()
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
}
