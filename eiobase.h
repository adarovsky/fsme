#ifndef EIOBASE_H
#define EIOBASE_H
#include <QtDebug>
#include <QString>
#include <QVariant>
#include <QPoint>
#include <QDomDocument>
#include <QPair>
#include <QAbstractItemModel>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QEnableSharedFromThis>
#include <QUndoCommand>
#include "nameexception.h"

class StateMachine;
class EIOBase : public QEnableSharedFromThis<EIOBase>
{
protected:
    QString _name;
    QString _comment;
    QWeakPointer<StateMachine> _parent;
    friend class StateMachine;
public:
    enum AddUndoID {
        UndoIDEvent  = 100,
        UndoIDInput  = 101,
        UndoIDOutput = 102,
        UndoIDState = 103,
        UndoIDTransition = 104,
        UndoIDRoot = 105
    };

    EIOBase( QString name, QSharedPointer<StateMachine> parent ) : _name(name), _parent( parent ) {}
    virtual ~EIOBase();
    virtual bool rename( QString newName ) = 0;
    virtual bool canDelete() const = 0;
    virtual QString name() const            { return _name; }

    virtual QString comment() const;
    virtual void setComment(const QString& comment);

    virtual int index() const = 0;
    QSharedPointer<StateMachine> parent() const    { return _parent; }

    bool isValid() const            { return !name().isEmpty(); }

    virtual QDomNode save( QDomDocument ) = 0;

public:
    virtual QUndoCommand * renameCommand( const QString& newName ) = 0;
    virtual QUndoCommand * deleteCommand() = 0;

    static void renameInCondition(QSharedPointer<StateMachine> fsm, const QString& what, const QString& to);
};


#define RENAME_BOILERPLATE(ClassName, ObjectName)                                                   \
class Rename##ClassName##Command : public QUndoCommand {                                            \
    QString                    m_oldName;                                                           \
    QString                    m_newName;                                                           \
    QWeakPointer<StateMachine> m_document;                                                          \
    friend class ClassName##AddCommand;                                                             \
public:                                                                                             \
    Rename##ClassName##Command(ClassName * e, const QString& newName, QUndoCommand * parent = nullptr);\
    virtual int id() const Q_DECL_OVERRIDE { return EIOBase::UndoID##ClassName; }                   \
    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;                                    \
    virtual void undo() override;                                                                   \
    virtual void redo() override;                                                                   \
};                                                                                                  \
Rename##ClassName##Command::Rename##ClassName##Command(ClassName *e, const QString &newName, QUndoCommand * parent)\
    : QUndoCommand(QObject::tr("Rename %1 %2 to %3\nrename").arg(QObject::tr(#ObjectName), e->name(), newName), parent),\
      m_oldName(e->name()), m_newName(newName), m_document(e->parent())                             \
{                                                                                                   \
}                                                                                                   \
bool Rename##ClassName##Command::mergeWith(const QUndoCommand * command)                            \
{                                                                                                   \
    auto other = dynamic_cast<const Rename##ClassName##Command*>( command );                        \
    if (other && m_newName == other->m_oldName) {                                                   \
        m_newName = other->m_newName;                                                               \
        return true;                                                                                \
    }                                                                                               \
    return false;                                                                                   \
}                                                                                                   \
QUndoCommand * ClassName::renameCommand( const QString& newName )                                   \
{                                                                                                   \
    if( parent()->find##ClassName(_name) != this || parent()->find##ClassName(newName) )            \
        return nullptr;                                                                             \
    return new Rename##ClassName##Command(this, newName);                                           \
}                                                                                                   \
bool ClassName::canDelete() const                                                                   \
{                                                                                                   \
    return true;                                                                                    \
}                                                                                                   \
QUndoCommand * ClassName::deleteCommand()                                                           \
{                                                                                                   \
    return new ClassName##DeleteCommand(this);                                                      \
}


#define UNDO_BASE_BOILERPLATE(ClassName)                                                            \
class ClassName##UndoCommand : public QUndoCommand                                                  \
{                                                                                                   \
    QString m_name;                                                                                 \
    QWeakPointer<StateMachine> m_document;                                                          \
protected:                                                                                          \
    QSharedPointer<ClassName> object() const;                                                       \
public:                                                                                             \
    ClassName##UndoCommand(ClassName * s, const QString& text, QUndoCommand * parent = nullptr);    \
};                                                                                                  \
class ClassName##ChangeCommentCommand : public ClassName##UndoCommand {                             \
    QString m_oldComment;                                                                           \
    QString m_newComment;                                                                           \
public:                                                                                             \
    ClassName##ChangeCommentCommand(ClassName * s, const QString& comment, QUndoCommand * parent = nullptr);\
    virtual int id() const Q_DECL_OVERRIDE { return 100 + EIOBase::UndoID##ClassName; }             \
    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;                                    \
    virtual void undo() Q_DECL_OVERRIDE;                                                            \
    virtual void redo() Q_DECL_OVERRIDE;                                                            \
};                                                                                                  \
class ClassName##AddCommand : public QUndoCommand {                                                 \
    QString                    m_newName;                                                           \
    QSharedPointer<ClassName>  m_object;                                                            \
    QWeakPointer<StateMachine> m_document;                                                          \
public:                                                                                             \
    ClassName##AddCommand(QSharedPointer<StateMachine> document, QUndoCommand * parent = nullptr);  \
    QSharedPointer<ClassName> object() const { return m_object; }                                   \
    virtual int id() const Q_DECL_OVERRIDE { return EIOBase::UndoID##ClassName; }                   \
    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;                                    \
    virtual void undo() Q_DECL_OVERRIDE;                                                            \
    virtual void redo() Q_DECL_OVERRIDE;                                                            \
};                                                                                                  \
class ClassName##DeleteCommand : public QUndoCommand {                                              \
    QString                    m_name;                                                              \
    QWeakPointer<StateMachine> m_document;                                                          \
public:                                                                                             \
    ClassName##DeleteCommand(ClassName * obj, QUndoCommand * parent = nullptr);                     \
    virtual void undo() Q_DECL_OVERRIDE;                                                            \
    virtual void redo() Q_DECL_OVERRIDE;                                                            \
};

#define UNDO_BASE_BOILERPLATE_IMPL(ClassName, ObjectName)                                           \
ClassName##UndoCommand::ClassName##UndoCommand(ClassName * s, const QString& text, QUndoCommand * parent)\
    : QUndoCommand(text, parent), m_name(s->name()), m_document( s->parent() )                      \
{                                                                                                   \
}                                                                                                   \
QSharedPointer<ClassName> ClassName##UndoCommand::object() const                                    \
{                                                                                                   \
    auto fsm = m_document.toStrongRef();                                                            \
    return fsm ? fsm->find##ClassName( m_name ) : QSharedPointer<ClassName>();                      \
}                                                                                                   \
ClassName##ChangeCommentCommand::ClassName##ChangeCommentCommand(ClassName * s, const QString& comment, QUndoCommand * parent)\
    : ClassName##UndoCommand(s, QObject::tr("Change comment"), parent), m_oldComment( s->comment() ), m_newComment( comment )\
{}                                                                                                  \
void ClassName##ChangeCommentCommand::undo()                                                        \
{                                                                                                   \
    ClassName##UndoCommand::undo();                                                                 \
    object()->setComment( m_oldComment );                                                           \
}                                                                                                   \
void ClassName##ChangeCommentCommand::redo()                                                        \
{                                                                                                   \
    object()->setComment( m_newComment );                                                           \
    ClassName##UndoCommand::redo();                                                                 \
}                                                                                                   \
bool ClassName##ChangeCommentCommand::mergeWith(const QUndoCommand * command)                       \
{                                                                                                   \
    auto other = dynamic_cast<const ClassName##ChangeCommentCommand*>( command );                   \
    if (other && object() == other->object()) {                                                     \
        m_newComment = other->m_newComment;                                                         \
        return true;                                                                                \
    }                                                                                               \
    return false;                                                                                   \
}                                                                                                   \
ClassName##AddCommand::ClassName##AddCommand(QSharedPointer<StateMachine> document, QUndoCommand * parent)\
    : QUndoCommand(QObject::tr("Add %1").arg(#ObjectName), parent), m_document( document )          \
{}                                                                                                  \
void ClassName##AddCommand::undo()                                                                  \
{                                                                                                   \
    QUndoCommand::undo();                                                                           \
    auto fsm = m_document.toStrongRef();                                                            \
    if (fsm && m_object) {                                                                          \
        fsm->del( m_object );                                                                       \
    }                                                                                               \
}                                                                                                   \
void ClassName##AddCommand::redo()                                                                  \
{                                                                                                   \
    auto fsm = m_document.toStrongRef();                                                            \
    if (fsm) {                                                                                      \
        m_object = fsm->add##ClassName(m_newName);                                                  \
    }                                                                                               \
    QUndoCommand::redo();                                                                           \
}                                                                                                   \
bool ClassName##AddCommand::mergeWith(const QUndoCommand *command)                                  \
{                                                                                                   \
    auto other = dynamic_cast<const Rename##ClassName##Command*>( command );                        \
    if ( other ) {                                                                                  \
        m_newName = other->m_newName;                                                               \
        return true;                                                                                \
    }                                                                                               \
    return false;                                                                                   \
}                                                                                                   \
void ClassName##DeleteCommand::undo()                                                               \
{                                                                                                   \
    auto fsm = m_document.toStrongRef();                                                            \
    if (fsm) {                                                                                      \
        fsm->add##ClassName(m_name);                                                                \
    }                                                                                               \
    QUndoCommand::undo();                                                                           \
}                                                                                                   \
void ClassName##DeleteCommand::redo()                                                               \
{                                                                                                   \
    QUndoCommand::redo();                                                                           \
    auto fsm = m_document.toStrongRef();                                                            \
    if (fsm) {                                                                                      \
        fsm->del(fsm->find##ClassName(m_name));                                                     \
    }                                                                                               \
}

#endif // EIOBASE_H
