#ifndef ROOTUNDOCOMMAND_H
#define ROOTUNDOCOMMAND_H

#include "statemachine.h"

class RootUndoCommand : public QUndoCommand {
    QWeakPointer<StateMachine> m_document;
protected:
    QSharedPointer<Root> root() const;
public:
    RootUndoCommand(Root * e, const QString & text, QUndoCommand * parent = nullptr);
};

class RenameRootCommand : public RootUndoCommand {
    QString m_oldName;
    QString m_newName;
    QWeakPointer<StateMachine> m_document;
public:
    RenameRootCommand(Root * e, const QString& newName, QUndoCommand * parent = nullptr);
    virtual void undo() override;
    virtual void redo() override;
};

class RootChangeCommentCommand : public RootUndoCommand {
    QString m_oldComment;
    QString m_newComment;
public:
    RootChangeCommentCommand(Root * e, const QString& newName, QUndoCommand * parent = nullptr);
    virtual int id() const Q_DECL_OVERRIDE { return 100 + EIOBase::UndoIDRoot; }
    bool mergeWith(const QUndoCommand *) Q_DECL_OVERRIDE;

    virtual void undo() override;
    virtual void redo() override;
};

class RootChangeBaseClassCommand : public RootUndoCommand {
    QString m_oldBaseClass;
    QString m_newBaseClass;
public:
    RootChangeBaseClassCommand(Root * e, const QString& newName, QUndoCommand * parent = nullptr);
    virtual int id() const Q_DECL_OVERRIDE { return 101 + EIOBase::UndoIDRoot; }
    bool mergeWith(const QUndoCommand *) Q_DECL_OVERRIDE;

    virtual void undo() override;
    virtual void redo() override;
};

class RootChangeInitialStateCommand : public RootUndoCommand {
    QString m_oldInitialState;
    QString m_newInitialState;
public:
    RootChangeInitialStateCommand(Root * e, const QString& newName, QUndoCommand * parent = nullptr);
    ~RootChangeInitialStateCommand();

    virtual void undo() override;
    virtual void redo() override;
};
#endif // ROOTUNDOCOMMAND_H
