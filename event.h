#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>
#include <QUndoCommand>
#include "eiobase.h"

class StateMachine;
class Event : public EIOBase {
    Event( QString name, QSharedPointer<StateMachine> parent );
    friend class StateMachine;
public:
    virtual int index() const override;
    virtual bool rename( QString newName ) override;
    virtual bool canDelete() const override;
    virtual QDomNode save( QDomDocument ) override;

public:
    QUndoCommand * renameCommand( const QString& newName ) override;
    QUndoCommand * deleteCommand() override;
};

UNDO_BASE_BOILERPLATE(Event)
#endif // EVENT_H
