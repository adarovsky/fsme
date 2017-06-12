#ifndef OUTPUT_H
#define OUTPUT_H

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>
#include "eiobase.h"

class Output : public EIOBase {
    Output(QString name, QSharedPointer<StateMachine> parent );
    friend class StateMachine;
public:
    virtual int index() const override;
    virtual bool rename( QString newName ) override;
    virtual QDomNode save( QDomDocument ) override;
    virtual bool canDelete() const override;

public:
    QUndoCommand * renameCommand( const QString& newName ) override;
    QUndoCommand * deleteCommand() override;
};

UNDO_BASE_BOILERPLATE(Output)

#endif // OUTPUT_H
