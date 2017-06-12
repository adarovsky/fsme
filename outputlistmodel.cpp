#include <QIcon>

#include "outputlistmodel.h"
#include "statemachine.h"
#include "scopeguard.h"

OutputListModel::OutputListModel(EIOBase * target)
    : QAbstractListModel(), m_target( target ), m_updating( false )
{
    auto fsm = m_target->parent();
    if (fsm) {
        connect(fsm.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(olm_dataChanged(QModelIndex,QModelIndex,QVector<int>)));
    }
}

QModelIndex OutputListModel::mapToSource(const QModelIndex &index) const
{
    auto fsm = m_target->parent();
    if ( !fsm ) return QModelIndex();

    if ( index.row() < 0 || index.row() >= m_collection.count() )
        return QModelIndex();

    auto eio = m_collection.at( index.row() ).toStrongRef();
    if ( !eio ) return QModelIndex();

    return fsm->indexForItem( eio.data() );
}

int OutputListModel::rowCount(const QModelIndex &) const
{
    return m_collection.count();
}

QVariant OutputListModel::data(const QModelIndex &index, int role) const
{
    auto fsm = m_target->parent();
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

bool OutputListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    int from = sourceRow;
    int to = destinationChild;

    if (sourceRow < destinationChild)
        destinationChild++;

    if (beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild)) {

        for(int i = 0; i < count; ++i) {
            m_collection.move(from, to);

            if (sourceRow > destinationChild) {
                from++;
                to++;
            }
        }

        endMoveRows();
        return true;
    }
    else {
        return false;
    }
}

void OutputListModel::append( QSharedPointer<EIOBase> item )
{
    beginInsertRows( QModelIndex(), m_collection.count(), m_collection.count() + 1 );
    m_collection.append( item );
    endInsertRows();
}

void OutputListModel::insert( int pos, QSharedPointer<EIOBase> item )
{
    beginInsertRows( QModelIndex(), pos, pos + 1 );
    m_collection.insert( pos, item );
    endInsertRows();
}

void OutputListModel::removeAt( int pos )
{
    beginRemoveRows( QModelIndex(), pos, pos );
    m_collection.removeAt( pos );
    endRemoveRows();
}

QSharedPointer<EIOBase> OutputListModel::at( int pos )
{
    return m_collection.at( pos );
}


void OutputListModel::olm_modelAboutToBeReset()
{
}

void OutputListModel::olm_modelReset()
{
}

void OutputListModel::olm_rowsInserted(const QModelIndex &parent, int first, int last)
{
}

void OutputListModel::olm_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    SCOPE_GUARD(m_updating);

    auto fsm = m_target->parent();
    if ( !fsm ) return;

    int bottom = m_collection.count() - 1;
    int top = -10;
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        EIOBase * p = reinterpret_cast<EIOBase*>( topLeft.sibling(i, topLeft.column()).internalPointer() );
        int pos = 0;
        for( const_iterator j = begin(); j != end(); ++j, ++pos ) {
            auto x = j->toStrongRef();
            if ( p == x.data() ) {
                bottom = std::min(bottom, pos);
                top = std::max(top, pos);
            }
        }
    }

    if ( bottom >= top ) {
        emit dataChanged( index(bottom, 0), index(top, 0), roles );
    }
}

void OutputListModel::olm_rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
}

