#include <QIcon>
#include "eventsinputsmodel.h"
#include "statemachine.h"

void EventsInputsModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QAbstractProxyModel::setSourceModel( sourceModel );

    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeInserted, this, [=](const QModelIndex &parent, int start, int end){
        auto fsm = dynamic_cast<StateMachine*>(sourceModel);
        if (fsm) {
            int offset = 0;
            if (parent == fsm->eventsFolder()) {
                offset += fsm->rowCount(fsm->inputsFolder());
            }

            if (parent == fsm->eventsFolder() || parent == fsm->inputsFolder()) {
                beginInsertRows(QModelIndex(), start + offset, end + offset);
            }
        }
    });

    connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex &parent, int, int ){
        auto fsm = dynamic_cast<const StateMachine*>(sourceModel);
        if (fsm) {
            if (parent == fsm->eventsFolder() || parent == fsm->inputsFolder()) {
                endInsertRows();
            }
        }
    });

    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex &parent, int start, int end){
        auto fsm = dynamic_cast<const StateMachine*>(sourceModel);
        if (fsm) {
            int offset = 0;
            if (parent == fsm->eventsFolder()) {
                offset += fsm->rowCount(fsm->inputsFolder());
            }
            if (parent == fsm->eventsFolder() || parent == fsm->inputsFolder()) {
                beginRemoveRows(QModelIndex(), start + offset, end + offset);
            }
        }
    });

    connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [=](const QModelIndex &parent, int, int ){
        auto fsm = dynamic_cast<const StateMachine*>(sourceModel);
        if (fsm) {
            if (parent == fsm->eventsFolder() || parent == fsm->inputsFolder()) {
                endRemoveRows();
            }
        }
    });

    connect(sourceModel, &QAbstractItemModel::dataChanged, this, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        auto fsm = dynamic_cast<StateMachine*>(sourceModel);
        if (fsm && topLeft.parent() == fsm->eventsFolder() || topLeft.parent() == fsm->inputsFolder()) {
            emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
        }
    });
}

QModelIndex EventsInputsModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {
        if (sourceIndex.parent() == fsm->inputsFolder()) {
            return createIndex(sourceIndex.row(), sourceIndex.column(), fsm->inputsFolder().child(sourceIndex.row(), sourceIndex.column()).internalPointer());
        }
        else if ( sourceIndex.parent() == fsm->eventsFolder() ) {
            int inputs = fsm->rowCount( fsm->inputsFolder() );

            return createIndex(sourceIndex.row() + inputs, sourceIndex.column(), fsm->eventsFolder().child(sourceIndex.row(), sourceIndex.column()).internalPointer());
        }
    }

    return QModelIndex();
}

QModelIndex EventsInputsModel::mapToSource(const QModelIndex &proxyIndex) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {
        EIOBase * obj = reinterpret_cast<EIOBase*>( proxyIndex.internalPointer() );
        return obj ? fsm->indexForItem( obj ) : QModelIndex();
    }

    return QModelIndex();
}

QModelIndex EventsInputsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= rowCount(parent))
        return QModelIndex();

    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {
        int inputs = fsm->rowCount( fsm->inputsFolder() );
        if (row < inputs)
            return createIndex(row, column, fsm->inputsFolder().child(row, column).internalPointer());
        else
            return createIndex(row, column, fsm->eventsFolder().child(row - inputs, column).internalPointer());
    }

    return QModelIndex();
}

QModelIndex EventsInputsModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int EventsInputsModel::rowCount(const QModelIndex &parent) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {
        int inputs = fsm->rowCount( fsm->inputsFolder() );
        int events = fsm->rowCount( fsm->eventsFolder() );

        return inputs + events;
    }

    return 0;
}

int EventsInputsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant EventsInputsModel::data(const QModelIndex &index, int role) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( !fsm ) return QVariant();

    if (role == Qt::DecorationRole) {
        QModelIndex parent = mapToSource( index ).parent();
        if ( parent == fsm->eventsFolder() )
            return QIcon(":/images/event.png");
        else if ( parent == fsm->inputsFolder() )
            return QIcon(":/images/input.png");
        else
            return QVariant();
    }
    else
        return fsm->data( mapToSource(index), role );
}
