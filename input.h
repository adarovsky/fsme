#ifndef INPUT_H
#define INPUT_H

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>
#include "eiobase.h"

class Input : public EIOBase {
    Input( QString name, QSharedPointer<StateMachine> parent );
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

UNDO_BASE_BOILERPLATE(Input)
#endif // INPUT_H
