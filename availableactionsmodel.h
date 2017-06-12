#ifndef AVAILABLEACTIONSMODEL_H
#define AVAILABLEACTIONSMODEL_H

#include "linearsubsetmodel.h"

class AvailableActionsModel : public LinearSubsetModel
{
    Q_OBJECT
public:
    virtual void setSourceModel(QAbstractItemModel *sourceModel) Q_DECL_OVERRIDE;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
};

#endif // AVAILABLEACTIONSMODEL_H
