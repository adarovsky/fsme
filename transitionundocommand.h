#ifndef TRANSITIONUNDOCOMMAND_H
#define TRANSITIONUNDOCOMMAND_H

#include <QUndoCommand>
#include <QSharedPointer>

#include "eiobase.h"
#include "statemachine.h"

class Transition;
class StateMachine;
class TransitionUndoCommand : public QUndoCommand {
protected:
    QString m_stateName;
    int     m_pos;
    QWeakPointer<StateMachine> m_document;
protected:
    QSharedPointer<State> state() const;
    QSharedPointer<Transition> transition() const;
public:
    TransitionUndoCommand(Transition * e, const QString& text, QUndoCommand * parent = nullptr);
};

class TransitionAddCommand : public QUndoCommand {
    QWeakPointer<StateMachine> m_document;
    QString                    m_stateName;
    QString                    m_destName;
    QString                    m_newName;
    QVector<QPointF>           m_initialCP;
    int                        m_pos;
public:
    TransitionAddCommand(QSharedPointer<State> source, QSharedPointer<State> dest, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
    virtual int id() const Q_DECL_OVERRIDE { return EIOBase::UndoIDTransition; }
    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    QSharedPointer<Transition> transition() const;
};

class RenameTransitionCommand : public TransitionUndoCommand {
    QString m_oldName;
    QString m_newName;
    friend class TransitionAddCommand;
public:
    RenameTransitionCommand(Transition * e, const QString& newName, QUndoCommand * parent = nullptr);
    virtual void undo() override;
    virtual void redo() override;

    virtual int id() const Q_DECL_OVERRIDE { return EIOBase::UndoIDTransition; }
    bool mergeWith(const QUndoCommand *) Q_DECL_OVERRIDE { return false; }
};

class TransitionChangeCommentCommand : public TransitionUndoCommand {
    QString m_newComment;
    QString m_oldComment;
public:
    TransitionChangeCommentCommand(Transition * e, const QString& newComment, QUndoCommand * parent = nullptr);
    virtual void undo() override;
    virtual void redo() override;

    virtual int id() const Q_DECL_OVERRIDE { return 100 + EIOBase::UndoIDTransition; }
    bool mergeWith(const QUndoCommand *) Q_DECL_OVERRIDE;
};

class TransitionChangeDestinationCommand : public TransitionUndoCommand {
    QString          m_newDestination;
    QString          m_oldDestination;
public:
    TransitionChangeDestinationCommand(Transition * e, const QString& newDestination, QUndoCommand * parent = nullptr);
    virtual void undo() override;
    virtual void redo() override;
};

class TransitionChangeConditionCommand : public TransitionUndoCommand {
    QString m_newCondition;
    QString m_oldCondition;
public:
    TransitionChangeConditionCommand(Transition * e, const QString& newCondition, QUndoCommand * parent = nullptr);

    virtual int id() const Q_DECL_OVERRIDE { return 101 + EIOBase::UndoIDTransition; }
    bool mergeWith(const QUndoCommand *) Q_DECL_OVERRIDE;

    virtual void undo() override;
    virtual void redo() override;
};

class TransitionTransformCommand : public TransitionUndoCommand {
    QVector<QPointF> m_newControlPoints;
    QVector<QPointF> m_oldControlPoints;
public:
    TransitionTransformCommand(Transition * e, const QVector<QPointF>& controlPoints, QUndoCommand * parent = nullptr);
    virtual int id() const override { return 10002; }
    bool mergeWith(const QUndoCommand *command) override;
    virtual void undo() override;
    virtual void redo() override;
};

class TransitionActionCommand : public TransitionUndoCommand {
protected:
    QPersistentModelIndex   m_folder;
    int                     m_index;

    QSharedPointer<EIOBase> target() const;
    OutputListModel * targetList() const;

    static QSharedPointer<EIOBase> itemAt(Transition * s, int pos);
public:
    TransitionActionCommand(Transition * s, EIOBase * target, const QString& text, QUndoCommand * parent = nullptr);
};

class TransitionAddActionCommand : public TransitionActionCommand {
    int                     m_pos;
public:
    TransitionAddActionCommand(Transition * s, EIOBase * target, int pos, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class TransitionRemoveActionCommand : public TransitionActionCommand {
    int                     m_pos;
public:
    TransitionRemoveActionCommand(Transition * s, int pos, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class TransitionMoveActionCommand : public TransitionActionCommand {
    int                     m_oldPos;
    int                     m_newPos;
public:
    TransitionMoveActionCommand(Transition * s, int oldPos, int newPos, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class TransitionDeleteCommand : public TransitionUndoCommand {
public:
    TransitionDeleteCommand(Transition * t, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

#endif // TRANSITIONUNDOCOMMAND_H
