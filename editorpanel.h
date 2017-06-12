#ifndef EDITORPANEL_H
#define EDITORPANEL_H

#include <QStackedWidget>
#include <QSharedPointer>
#include "statemachine.h"

class QItemSelection;
class EditorPanel : public QStackedWidget
{
    Q_OBJECT
    QWeakPointer<StateMachine> m_document;
public:
    explicit EditorPanel(QWidget *parent = 0);

signals:
public slots:
    void setStateMachine( QSharedPointer<StateMachine> fsm );
    void modelAboutToBeReset();
    void modelReset();
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void layoutChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex> (), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);

    void rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

    // originated from selection model
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};

#endif // EDITORPANEL_H
