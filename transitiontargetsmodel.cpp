#include <QIcon>
#include "transitiontargetsmodel.h"
#include "statemachine.h"

TransitionTargetsModel::TransitionTargetsModel()
    : m_model( new LinearSubsetModel )
{
}

TransitionTargetsModel::~TransitionTargetsModel()
{

    connect(m_model.data(), &QAbstractItemModel::rowsAboutToBeInserted, this, [=](const QModelIndex &, int start, int end){
        beginInsertRows(QModelIndex(), start+1, end+1);
    });

    connect(m_model.data(), &QAbstractItemModel::rowsInserted, this, [=](){
        endInsertRows();
    });

    connect(m_model.data(), &QAbstractItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex &, int start, int end){
        beginRemoveRows(QModelIndex(), start + 1, end + 1);
    });

    connect(m_model.data(), &QAbstractItemModel::rowsRemoved, this, [=](){
        endRemoveRows();
    });

    connect(m_model.data(), &QAbstractItemModel::dataChanged, this, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        QModelIndex tl = createIndex(topLeft.row()+1, topLeft.column(), topLeft.internalPointer());
        QModelIndex br = createIndex(bottomRight.row()+1, bottomRight.column(), bottomRight.internalPointer());
        emit dataChanged(tl, br, roles);
    });

    connect(m_model.data(), &QAbstractItemModel::modelAboutToBeReset, this, &QAbstractItemModel::modelAboutToBeReset);
    connect(m_model.data(), &QAbstractItemModel::modelReset, this, &QAbstractItemModel::modelReset);
    connect(m_model.data(), &QAbstractItemModel::layoutAboutToBeChanged, this, &QAbstractItemModel::layoutAboutToBeChanged);
    connect(m_model.data(), &QAbstractItemModel::layoutChanged, this, &QAbstractItemModel::layoutChanged);
}

void TransitionTargetsModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_model->setSourceModel( sourceModel );
    auto fsm = dynamic_cast<StateMachine*>( sourceModel );
    if (fsm)
        m_model->setParents( QList<QPersistentModelIndex>() << fsm->statesFolder() );
}

QModelIndex TransitionTargetsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= rowCount(parent))
        return QModelIndex();

    if (row == 0) return createIndex(0, 0, nullptr);

    QModelIndex idx = m_model->index(row - 1, column);
    return createIndex(row, column, idx.internalPointer());
}

QModelIndex TransitionTargetsModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int TransitionTargetsModel::rowCount(const QModelIndex &parent) const
{
    return m_model->rowCount() + 1;
}

int TransitionTargetsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

Qt::ItemFlags TransitionTargetsModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;

    return Qt::NoItemFlags;
}

QVariant TransitionTargetsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() == 0) {
        return role == Qt::DisplayRole ? QString() : QVariant();
    }

    return m_model->data( createIndex(index.row() - 1, index.column(), index.internalPointer()), role );
}
