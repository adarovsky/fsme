#ifndef OUTPUTLISTMODEL_H
#define OUTPUTLISTMODEL_H

#include <QWeakPointer>
#include <QAbstractListModel>

class State;
class EIOBase;
class OutputListModel : public QAbstractListModel
{
    Q_OBJECT
    EIOBase*                       m_target;
    QList< QWeakPointer<EIOBase> > m_collection;
    bool                           m_updating;
protected:
    // Get model index in the source model (fsm) obtained from m_target->parent()
    QModelIndex mapToSource( const QModelIndex& index ) const;
public:
    explicit OutputListModel(EIOBase * target);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) Q_DECL_OVERRIDE;

    void append( QSharedPointer<EIOBase> item );
    void insert( int pos, QSharedPointer<EIOBase> item );
    void removeAt( int pos );
    QSharedPointer<EIOBase> at( int pos );

    typedef QList< QWeakPointer<EIOBase> >::const_iterator const_iterator;

    const_iterator begin() const { return m_collection.begin(); }
    const_iterator end() const { return m_collection.end(); }
signals:

protected slots:
    // used to handle output and event changes in FSM
    void olm_modelAboutToBeReset();
    void olm_modelReset();
    void olm_rowsInserted(const QModelIndex &parent, int first, int last);
    void olm_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void olm_rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
};

#endif // OUTPUTLISTMODEL_H
