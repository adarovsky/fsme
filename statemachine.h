/*
FSME - A State Machine Editor
Copyright (c) 2003-2005 Alexander Darovsky

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//====================================================================
//
//    Copyright (C) 2001, 2003-2004, Alexander Darovsky
//
// Filename:      state.h
// Description:   basic state machine data description
// Author:        Alexander Darovsky <adarovsky@users.sf.net>
// Created at:    Sun Nov 11 10:32:20 2001
// Modified at:   Thu Apr  8 11:27:15 2004
//
// Version:
// Update count:  5
// Status:        Experimental, do not distribute.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//====================================================================
#ifndef __STATE_H__
#define __STATE_H__

#include "nameexception.h"
#include "event.h"

#include <QString>
#include <QVariant>
#include <QPoint>
#include <QDomDocument>
#include <QPair>
#include <QAbstractItemModel>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QEnableSharedFromThis>
#include "root.h"
#include "event.h"
#include "input.h"
#include "output.h"
#include "state.h"
#include "transition.h"

class StateMachine : public QAbstractItemModel,
                     public QEnableSharedFromThis<StateMachine> {
    Q_OBJECT
    QList<QSharedPointer<Event> >       events;
    QList<QSharedPointer<State> >       states;
    QList<QSharedPointer<Input> >       inputs;
    QList<QSharedPointer<Output> >      outputs;

    QSharedPointer<Root>        _root;
    QUndoStack * m_undoStack;

    void add( QSharedPointer<Event> e );
    void add( QSharedPointer<Input> i );
    void add( QSharedPointer<Output> o );
    void add( QSharedPointer<State>  s );
public:

    enum { GraphicPositionRole = Qt::UserRole + 100, ContentChangedRole = Qt::UserRole + 101 };

    QString       name;
    QString       comment;
    QString       baseClass;
    QString       initialState;

    QSharedPointer<Event>   addEvent ( const QString& name );
    QSharedPointer<Input>   addInput ( const QString& name );
    QSharedPointer<Output>  addOutput( const QString& name );
    QSharedPointer<State>   addState ( const QString& name, const QPointF& pos );

    void del( QSharedPointer<Event>  e );
    void del( QSharedPointer<Input>  i );
    void del( QSharedPointer<Output> o );
    void del( QSharedPointer<State>  s );

    QSharedPointer<Event>   findEvent ( const QString& name ) const;
    QSharedPointer<Input>   findInput ( const QString& name ) const;
    QSharedPointer<Output>  findOutput( const QString& name ) const;
    QSharedPointer<State>   findState ( const QString& name ) const;

    QModelIndex eventsFolder() const;
    QModelIndex inputsFolder() const;
    QModelIndex outputsFolder() const;
    QModelIndex statesFolder() const;
    QModelIndex indexForItem( EIOBase * item ) const;

    QSharedPointer<Root>    root() const { return _root; }

    void ensureEventsSorted();
    void ensureInputsSorted();
    void ensureOutputsSorted();
    void ensureStatesSorted();

    QList<QSharedPointer<Event> >::ConstIterator   eventsBegin() const { return events.constBegin(); }
    QList<QSharedPointer<Event> >::ConstIterator   eventsEnd() const   { return events.constEnd(); }
    QList<QSharedPointer<State> >::ConstIterator   statesBegin() const { return states.constBegin(); }
    QList<QSharedPointer<State> >::ConstIterator   statesEnd() const   { return states.constEnd(); }
    QList<QSharedPointer<Input> >::ConstIterator   inputsBegin() const { return inputs.constBegin(); }
    QList<QSharedPointer<Input> >::ConstIterator   inputsEnd() const   { return inputs.constEnd(); }
    QList<QSharedPointer<Output> >::ConstIterator  outputsBegin() const { return outputs.constBegin(); }
    QList<QSharedPointer<Output> >::ConstIterator  outputsEnd() const  { return outputs.constEnd(); }

    QUndoStack * undoStack() const { return m_undoStack; }

public:

    StateMachine();
    virtual ~StateMachine();
    void clear();

    virtual bool     load( const QDomDocument& from );
    virtual void save(QDomDocument doc, QDomElement root);
    virtual void saveCore(QDomDocument doc, QDomElement root );
    virtual void saveGraphics(QDomDocument doc, QDomElement root );

public:
    QSharedPointer<EIOBase> item(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column,
                                  const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
private:
    void informItemChanged(EIOBase * item, int role = Qt::EditRole);

    friend class EIOBase;
    friend class Root;
    friend class Event;
    friend class Input;
    friend class Output;
    friend class State;
    friend class Transition;

    bool loadCore( const QDomElement & sm );
    bool loadGraphics (const QDomElement & graphics);

    bool readInput( const QDomElement& e );
    bool readEvent( const QDomElement& e );
    bool readOutput( const QDomElement& e );
    bool readState( const QDomElement& e );
    bool fillState( const QDomElement& e );
    bool readTransitions( const QDomElement& e, State& state );
    bool readTransition (const QDomElement& e, State& state , int pos);
    bool readIncomeActions( const QDomElement& e, State& state );
    bool readOutcomeActions( const QDomElement& e, State& state );
};

#endif // __STATE_HH__

/// Local Variables: ***
/// mode: C++ ***
/// c-basic-offset: 4 ***
/// End: ***
