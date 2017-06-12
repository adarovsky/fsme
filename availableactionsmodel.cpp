#include <QIcon>
#include "availableactionsmodel.h"
#include "statemachine.h"

void AvailableActionsModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    LinearSubsetModel::setSourceModel( sourceModel );

    auto fsm = dynamic_cast<StateMachine*>( sourceModel );
    if (fsm) {
        setParents( QList<QPersistentModelIndex>() << fsm->outputsFolder() << fsm->eventsFolder() );
    }
}

QVariant AvailableActionsModel::data(const QModelIndex &index, int role) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( !fsm ) return QVariant();

    if (role == Qt::DecorationRole) {
        QModelIndex parent = mapToSource( index ).parent();
        if ( parent == fsm->eventsFolder() )
            return QIcon(":/images/event.png");
        else if ( parent == fsm->outputsFolder() )
            return QIcon(":/images/output.png");
        else
            return QVariant();
    }
    else
        return fsm->data( mapToSource(index), role );
}
