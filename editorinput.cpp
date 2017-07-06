#include "editorinput.h"
#include "statemachine.h"
#include "ui_editorbase.h"

EditorInput::EditorInput(QWidget *parent)
    : QWidget(parent),
      ui( new Ui::EditorBase )
{
    ui->setupUi( this );
    connect(ui->nameField, &QLineEdit::textChanged, [=] ( const QString &newValue ) {
        auto cmd = input()->renameCommand(newValue);
        if (cmd)
            input()->parent()->undoStack()->push(cmd);
    });
    connect(ui->commentField, &QPlainTextEdit::textChanged, [=] () {
        auto cmd = new InputChangeCommentCommand( input(), ui->commentField->toPlainText() );
        input()->parent()->undoStack()->push( cmd );
    });
}

bool EditorInput::canStartEditing(const QModelIndex &index) const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    return dynamic_cast<Input*>(p) != nullptr;
}

Input * EditorInput::input() const
{
    EIOBase * p = reinterpret_cast<EIOBase*>( m_editedObject.internalPointer() );
    return dynamic_cast<Input*>(p);
}

void EditorInput::updateUI()
{
    {
        QSignalBlocker _b(ui->nameField);
        ui->nameField->setText( input()->name() );
    }

    {
        QSignalBlocker _b(ui->commentField);
        ui->commentField->setPlainText( input()->comment() );
    }
}
