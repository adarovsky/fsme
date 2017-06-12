#ifndef STATEUNDOCOMMAND_H
#define STATEUNDOCOMMAND_H
#include <QUndoCommand>
#include <QSharedPointer>
#include "statemachine.h"

class StateUndoCommand : public QUndoCommand
{
protected:
    QString m_stateName;
    QWeakPointer<StateMachine> m_document;
protected:
    QSharedPointer<State> state() const;
public:
    StateUndoCommand(State * s, const QString& text, QUndoCommand * parent = nullptr);
};

class StateAddCommand: public QUndoCommand {
    QString m_stateName;
    QPointF m_pos;
    QWeakPointer<StateMachine> m_document;
protected:
    QSharedPointer<State> state() const;
public:
    StateAddCommand(const QString& stateName, const QPointF& pos, QSharedPointer<StateMachine> document, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class StateActionCommand : public StateUndoCommand {
public:
    enum ActionType {In, Out};
protected:
    ActionType              m_type;
    QPersistentModelIndex   m_folder;
    int                     m_index;

    QSharedPointer<EIOBase> target() const;
    OutputListModel * targetList() const;

    static QSharedPointer<EIOBase> itemAt(State * s, int pos, ActionType type);
public:
    StateActionCommand(State * s, EIOBase * target, const QString& text, ActionType type, QUndoCommand * parent = nullptr);
};

class StateAddActionCommand : public StateActionCommand {
    int                     m_pos;
public:
    StateAddActionCommand(State * s, EIOBase * target, int pos, ActionType type, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class StateRemoveActionCommand : public StateActionCommand {
    int                     m_pos;
public:
    StateRemoveActionCommand(State * s, int pos, ActionType type, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class StateMoveActionCommand : public StateActionCommand {
    int                     m_oldPos;
    int                     m_newPos;
public:
    StateMoveActionCommand(State * s, int oldPos, int newPos, ActionType type, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class StateChangeCommentCommand : public StateUndoCommand {
    QString m_newComment;
    QString m_oldComment;
public:
    StateChangeCommentCommand(State * e, const QString& newComment, QUndoCommand * parent = nullptr);
    virtual void undo() override;
    virtual void redo() override;

    virtual int id() const Q_DECL_OVERRIDE { return 100 + EIOBase::UndoIDState; }
    bool mergeWith(const QUndoCommand *) Q_DECL_OVERRIDE;
};

class TransitionMoveCommand : public StateUndoCommand {
    int                     m_oldPos;
    int                     m_newPos;
public:
    TransitionMoveCommand(Transition * s, int newPos, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

class StateDeleteCommand : public StateUndoCommand {
    QPointF m_pos;
public:
    StateDeleteCommand(State * s, QUndoCommand * parent = nullptr);
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
};

#endif // STATEUNDOCOMMAND_H
