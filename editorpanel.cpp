#include <QItemSelection>
#include <QtDebug>
#include "statemachine.h"
#include "editorpanel.h"
#include "editorbase.h"

EditorPanel::EditorPanel(QWidget *parent)
    : QStackedWidget(parent)
{
}

void EditorPanel::setStateMachine( QSharedPointer<StateMachine> fsm )
{
    m_document = fsm;
    connect( fsm.data(), SIGNAL(modelAboutToBeReset()), SLOT(modelAboutToBeReset()) );
    connect( fsm.data(), SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)) );
    connect( fsm.data(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );
    connect( fsm.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)) );
    connect( fsm.data(), &QAbstractItemModel::layoutChanged, this, &EditorPanel::layoutChanged );
    connect( fsm.data(), &QAbstractItemModel::rowsMoved, this, &EditorPanel::rowsMoved );

    modelAboutToBeReset();
    modelReset();
}

void EditorPanel::modelAboutToBeReset()
{
}

void EditorPanel::modelReset()
{
}

void EditorPanel::rowsInserted(const QModelIndex &parent, int first, int last)
{
}

void EditorPanel::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    EditorBase * editor = dynamic_cast<EditorBase*>( currentWidget() );
    if (!editor) return;

    for( int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        if (topLeft.sibling(i, 0) == editor->editedIndex())
            editor->updateUI();
    }
}

void EditorPanel::rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
}

void EditorPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    setCurrentIndex( 0 );

    if (selected.count() > 1)
        setCurrentIndex( 1 );
    else if (selected.count() == 1){
        EditorBase * editor = nullptr;
        for( int i = count() - 1; i >= 2; --i ) {
            editor = dynamic_cast<EditorBase*>( widget(i) );
            if ( editor && editor->edit( selected.indexes().first() ) ) {
                setCurrentIndex( i );
                break;
            }
        }
    }
}

void EditorPanel::layoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    EditorBase * editor = dynamic_cast<EditorBase*>( currentWidget() );
    if (!editor) return;

    if (parents.contains(editor->editedIndex().parent())) {
        editor->updateUI();
    }
}

void EditorPanel::rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    EditorBase * editor = dynamic_cast<EditorBase*>( currentWidget() );
    if (!editor) return;

    if ( parent == editor->editedIndex().parent() ) {
            editor->updateUI();
    }
}
