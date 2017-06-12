#include <QLineEdit>
#include <QPlainTextEdit>

#include "editorbase.h"
#include "eiobase.h"
#include "ui_editorbase.h"

EditorBase::EditorBase()
    : m_editing( false )
{
}

QSharedPointer<StateMachine> EditorBase::fsm() const
{
    return m_document.toStrongRef();
}

void EditorBase::setStateMachine(QSharedPointer<StateMachine> fsm)
{
    m_document = fsm;
}

bool EditorBase::edit(const QModelIndex &index)
{
    if ( canStartEditing(index) ) {
        m_editedObject = QPersistentModelIndex(index);
        updateUI();
        return true;
    }

    return false;
}

void EditorBase::endEditing()
{
    m_editedObject = QPersistentModelIndex();
}
