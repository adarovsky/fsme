#ifndef ROOT_H
#define ROOT_H

#include <QString>
#include <QDomDocument>
#include <QSharedPointer>
#include "eiobase.h"

class Root : public EIOBase {
    Root( QSharedPointer<StateMachine> parent );
    friend class StateMachine;
public:
    virtual int index() const override;
    virtual QString name() const Q_DECL_OVERRIDE;
    virtual bool rename( QString newName ) Q_DECL_OVERRIDE;
    virtual bool canDelete() const override;
    virtual QDomNode save( QDomDocument ) Q_DECL_OVERRIDE;

    virtual QString comment() const Q_DECL_OVERRIDE;
    virtual void setComment(const QString& comment) Q_DECL_OVERRIDE;

    virtual QString baseClass() const;
    virtual void setBaseClass(const QString& baseClass);

    virtual QString initialState() const;
    virtual void setInitialState(const QString& initialState);

public:
    QUndoCommand * renameCommand( const QString& newName ) override;
    QUndoCommand * deleteCommand() override;
};


#endif // ROOT_H
