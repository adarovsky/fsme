#ifndef STATE_H
#define STATE_H

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>
#include <QEnableSharedFromThis>
#include <QPointF>
#include "eiobase.h"
#include "outputlistmodel.h"

class Transition;
class State : public EIOBase {
    State( QString name, QSharedPointer<StateMachine> parent );
    friend class StateMachine;
protected:
    void add( QSharedPointer<Transition> t );
    void insert( QSharedPointer<Transition> t, int pos );
    friend class Transition;
public:
    typedef QList< QSharedPointer<Transition> > TransitionList;
    QSharedPointer<Transition> createTransition(int pos, QSharedPointer<State> target = QSharedPointer<State>());
    void del( QSharedPointer<Transition> t );
protected:
    TransitionList  _transitions;
public:
    ~State();
    virtual int index() const override;
    virtual bool rename( QString newName ) override;
    virtual bool canDelete() const override;

    TransitionList transitionList() const;
    OutputListModel incomeActions;
    OutputListModel outcomeActions;

    bool moveTransition ( int i, int to );
    bool isValid() const { return !name().isEmpty(); }
    friend QDataStream& operator>>( QDataStream& s, State& st );
    virtual QDomNode save( QDomDocument ) override;

private:
    QPointF m_center;
public:
    QPointF center() const { return m_center; }
    void setCenter(const QPointF& c );

public:
    QUndoCommand * renameCommand( const QString& newName ) override;
    QUndoCommand * deleteCommand() override;
};

#endif // STATE_H
