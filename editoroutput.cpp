#include "editoroutput.h"
#include "statemachine.h"
#include "ui_editorbase.h"

EditorOutput::EditorOutput(QWidget *parent)
    : QWidget(parent),
      ui( new Ui::EditorBase )
{
    ui->setupUi( this );
    connect(ui->nameField, &QLineEdit::textChanged, [=] ( const QString &newValue ) {
        auto cmd = output()->renameCommand(newValue);
        if (cmd)
            output()->parent()->undoStack()->push(cmd);
    });
    connect(ui->commentField, &QPlainTextEdit::textChanged, [=] () {
        auto cmd = new OutputChangeCommentCommand( output(), ui->commentField->toPlainText() );
        output()->parent()->undoStack()->push( cmd );
    });
}

bool EditorOutput::canStartEditing(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    return dynamic_cast<Output*>(p) != nullptr;
}

Output * EditorOutput::output() const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( m_editedObject.internalPointer() );
    return dynamic_cast<Output*>(p);
}

void EditorOutput::updateUI()
{
    {
        QSignalBlocker _b(ui->nameField);
        ui->nameField->setText( output()->name() );
    }

    {
        QSignalBlocker _b(ui->commentField);
        ui->commentField->setPlainText( output()->comment() );
    }
}
