#ifndef TransitionTargetsModel_H
#define TransitionTargetsModel_H

#include <QScopedPointer>
#include "linearsubsetmodel.h"

class TransitionTargetsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    TransitionTargetsModel();
    ~TransitionTargetsModel();
    void setSourceModel(QAbstractItemModel *sourceModel);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    QScopedPointer<LinearSubsetModel> m_model;
};

#endif // TransitionTargetsModel_H
