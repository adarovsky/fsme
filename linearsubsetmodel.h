#ifndef LINEARSUBSETMODEL_H
#define LINEARSUBSETMODEL_H

#include <QList>
#include <QAbstractProxyModel>

class LinearSubsetModel  : public QAbstractProxyModel
{
    Q_OBJECT
public:
    void setParents(const QList<QPersistentModelIndex>& parents);
    virtual void setSourceModel(QAbstractItemModel *sourceModel) Q_DECL_OVERRIDE;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const Q_DECL_OVERRIDE;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const Q_DECL_OVERRIDE;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
private:
    QList<QPersistentModelIndex> m_parents;
    QList<QMetaObject::Connection> m_connections;
};

#endif // LINEARSUBSETMODEL_H
