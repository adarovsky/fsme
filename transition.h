#ifndef TRANSITION_H
#define TRANSITION_H

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>
#include "eiobase.h"
#include "state.h"
#include "outputlistmodel.h"

/** Transition */
class Transition : public EIOBase {
    QWeakPointer<State> _parent;
public:
    /// Destination
    QWeakPointer<State> target;
    /// Transite target, @see Transition::TransitionType
    QWeakPointer<State> transiteTarget;
    /// Actions to perform on transition
    OutputListModel  actions;

    /** Transition type
     */
    enum TransitionType { Simple,     /**< Just step into another state*/
                          SaveSource, /**< Step to another state, saving a state
                                           where we exited from. This is an analog
                                           to function call */
                          Transite,   /**< Step to the state \a target, saving \a transiteTarget.
                                         This is useful when you pass from states \f$A_1\f$, \f$A_2\f$
                                         to \f$B_1\f$, \f$B_2\f$ through state \f$C\f$. Normally, you should double it to
                                         \f$C_1\f$ and \f$C_2\f$, but soon your graph will become quite
                                         a mess. Therefore in states \f$A_n\f$ you can set transitionType to
                                         \a Transite, and set transiteTarget to \f$B_n\f$.
                                       */
                          Return     /**< Return from saved state. If there are no
                                        saved states, go to \a target*/} transitionType;
    Transition(QSharedPointer<State> parent);

    virtual int index() const override;

    virtual bool rename( QString newName ) override;
    virtual bool canDelete() const override;
    QSharedPointer<State> parentState() const { return _parent; }
    virtual QDomNode save( QDomDocument ) override;

    static TransitionType parseTransitionType( const QString& tt );
    static QString unparseTransitionType( TransitionType tt );

private:
    QVector<QPointF> m_controlPoints;
    QString     m_condition;
public:
    QVector<QPointF> controlPoints() const { return m_controlPoints; }
    void setControlPoints(const QVector<QPointF>& co);

    void setCondition(const QString& condition );
    QString condition() const { return m_condition; }
public:
    QUndoCommand * renameCommand( const QString& newName ) override;
    QUndoCommand * deleteCommand() override;
};


#endif // TRANSITION_H
