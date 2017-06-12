#include <algorithm>

#include "linearsubsetmodel.h"
#include "statemachine.h"
#include "scopeguard.h"

void LinearSubsetModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    beginResetModel();
    scope_guard endReset([&]{
        endResetModel();
    }); Q_UNUSED(endReset);

    foreach (auto c, m_connections) {
        QObject::disconnect(c);
    }
    m_connections.clear();

    m_parents.clear();
    QAbstractProxyModel::setSourceModel( sourceModel );

    m_connections << connect(sourceModel, &QAbstractItemModel::rowsAboutToBeInserted, this, [=](const QModelIndex &parent, int start, int end){
        auto fsm = dynamic_cast<StateMachine*>(sourceModel);
        if (fsm) {
            int offset = 0;
            bool found = false;
            foreach (auto idx, m_parents) {
                if (idx == parent) {
                    found = true;
                    break;
                }
                offset += fsm->rowCount(idx);
            }

            if (found) {
                beginInsertRows(QModelIndex(), start + offset, end + offset);
            }
        }
    });

    m_connections << connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex &parent, int, int ){
        auto fsm = dynamic_cast<const StateMachine*>(sourceModel);
        if (fsm) {
            bool found = false;
            foreach (auto idx, m_parents) {
                if (idx == parent) {
                    found = true;
                    break;
                }
            }

            if (found) {
                endInsertRows();
            }
        }
    });

    m_connections << connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex &parent, int start, int end){
        auto fsm = dynamic_cast<const StateMachine*>(sourceModel);
        if (fsm) {
            int offset = 0;
            bool found = false;
            foreach (auto idx, m_parents) {
                if (idx == parent) {
                    found = true;
                    break;
                }
                offset += fsm->rowCount(idx);
            }

            if (found) {
                beginRemoveRows(QModelIndex(), start + offset, end + offset);
            }
        }
    });

    m_connections << connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [=](const QModelIndex &parent, int, int ){
        auto fsm = dynamic_cast<const StateMachine*>(sourceModel);
        if (fsm) {
            bool found = false;
            foreach (auto idx, m_parents) {
                if (idx == parent) {
                    found = true;
                    break;
                }
            }

            if (found) {
                endRemoveRows();
            }
        }
    });

    m_connections << connect(sourceModel, &QAbstractItemModel::dataChanged, this, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        auto fsm = dynamic_cast<StateMachine*>(sourceModel);
        if (fsm && m_parents.contains( topLeft.parent() ) ) {
            emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
        }
    });

    m_connections << connect(sourceModel, &QAbstractItemModel::layoutAboutToBeChanged, this, [=](const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint) {
        bool found = false;
        foreach (auto idx, m_parents) {
            if (parents.contains(idx)) {
                found = true;
                break;
            }
        }

        if (found)
            emit layoutAboutToBeChanged(QList<QPersistentModelIndex>() << QModelIndex(), hint);
    });
    m_connections << connect(sourceModel, &QAbstractItemModel::layoutChanged, this, [=](const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint) {
        bool found = false;
        foreach (auto idx, m_parents) {
            if (parents.contains(idx)) {
                found = true;
                break;
            }
        }

        if (found)
            emit layoutChanged(QList<QPersistentModelIndex>() << QModelIndex(), hint);
    });

    m_connections << connect(sourceModel, &QAbstractItemModel::modelAboutToBeReset, this, &QAbstractItemModel::modelAboutToBeReset);
    m_connections << connect(sourceModel, &QAbstractItemModel::modelReset, this, &QAbstractItemModel::modelReset);
}

void LinearSubsetModel::setParents(const QList<QPersistentModelIndex>& parents)
{
    beginResetModel();
    m_parents = parents;
    endResetModel();
}

QModelIndex LinearSubsetModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {
        int offset = 0;
        bool found = false;
        foreach (auto idx, m_parents) {
            if (idx == sourceIndex.parent()) {
                found = true;
                break;
            }
            offset += fsm->rowCount(idx);
        }

        if (found) {
            return createIndex(sourceIndex.row() + offset, sourceIndex.column(), fsm->eventsFolder().child(sourceIndex.row(), sourceIndex.column()).internalPointer());
        }
    }

    return QModelIndex();
}

QModelIndex LinearSubsetModel::mapToSource(const QModelIndex &proxyIndex) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {
        EIOBase * obj = reinterpret_cast<EIOBase*>( proxyIndex.internalPointer() );
        return obj ? fsm->indexForItem( obj ) : QModelIndex();
    }

    return QModelIndex();
}

QModelIndex LinearSubsetModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= rowCount(parent))
        return QModelIndex();

    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );

    if ( fsm ) {
        auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
        if ( fsm ) {
            int offset = 0;
            QModelIndex sourceIndex;
            foreach (auto idx, m_parents) {
                int count = fsm->rowCount(idx);
                if (offset + count > row) {
                    sourceIndex = idx.child(row - offset, column);
                    break;
                }

                offset += count;
            }

            if (sourceIndex.isValid()) {
                return createIndex(row, column, sourceIndex.internalPointer());
            }
        }
    }

    return QModelIndex();
}

QModelIndex LinearSubsetModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int LinearSubsetModel::rowCount(const QModelIndex &) const
{
    auto fsm = dynamic_cast<StateMachine*>( sourceModel() );
    if ( fsm ) {

        return std::accumulate(m_parents.begin(), m_parents.end(), 0, [=](int rows, const QPersistentModelIndex& idx) {
            return rows + fsm->rowCount( idx );
        });
    }

    return 0;
}

int LinearSubsetModel::columnCount(const QModelIndex &) const
{
    return 1;
}
