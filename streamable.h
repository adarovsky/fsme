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
#ifndef STREAMABLE_H
#define STREAMABLE_H

#include <qobject.h>
#include <qdom.h>
#include <qmap.h>
#include <qstringlist.h>

#ifdef USE_SHARED_PTR
    #include <qsharedpointer.h>
#endif
//#include <qdict.h>

/** \file streamable.h
 *  \brief Dynamic streaming library
 *
 *  Defines Streamable class providing dynamic streamable
 *  capabilities for QT library
 */
#ifndef Q_ASSERT
#   define Q_ASSERT ASSERT
#endif

class StreamableInit;
class StreamableSpawner;
class Streamable {
protected:
    friend class StreamableInit;
    static const QString nameTag;
    static const QString dataTag;
    static const QString metaTag;

    virtual const char* __className() const = 0;

    virtual void __prepareToWrite() const = 0;
    virtual void __write( QDomElement n ) const;
    virtual bool __read( QDomElement n );

    void write( QDomElement ) const {}
    bool read( QDomElement )        { return true; }

    static QMap<QString, StreamableSpawner*> *_Streamables;
public:

#ifdef USE_SHARED_PTR
    static QSharedPointer<Streamable> spawn( QString className );
    static QSharedPointer<Streamable> load( QDomElement classNode );
#else
    static Streamable* spawn( QString className );
    static Streamable* load( QDomElement classNode );
#endif
    void save( QDomNode parent ) const;
    QDomElement classElement( QDomDocument doc ) const;

    class Bool {
        bool b;
    public:
        Bool() : b(false) {}
        Bool(bool _b) : b(_b) {}
        Bool( const Bool& _b ) { *this = _b; }
        operator bool() const { return b; }
        operator bool()       { return b; }

        bool operator=( bool _b ) { b = _b; return b; }
    };

    Streamable() { }
    Streamable( const Streamable& ) { }
    virtual ~Streamable() {}

    static QStringList registeredStreamables() { return _Streamables->keys(); }
};


/** \brief Instantiator class
 * It spawns new class with virtual makeInstance() const
 */
class StreamableSpawner {
public:
#ifdef USE_SHARED_PTR
    virtual QSharedPointer<Streamable> makeInstance() const = 0;
#else
    virtual Streamable* makeInstance() const = 0;
#endif
};


/** \brief Constructor and destructor of global map
 *
 * A class providing initialization of global Streamable::_Streamables
 * map.
 * \internal
 */
class StreamableInit {
    static int n;
public:

    /** make global map instance at startup, do nothing otherwise
     * \internal
     */
    StreamableInit() {
        if( n++ == 0 )
            Streamable::_Streamables = new QMap<QString, StreamableSpawner*>;
    }

    /** delete global map at the end of execution, do nothing otherwise
     *  \internal
     */
    ~StreamableInit() {
        if( --n == 0 )
            delete Streamable::_Streamables;
    }
};

static StreamableInit __streamableInit;

/** \internal Instantiator class definition
 * It spawns new class with virtual makeInstance() const
 *
 */
#ifdef USE_SHARED_PTR
#define DECLARE_SPAWNER( cls )                                          \
    class cls##Spawner : virtual StreamableSpawner {                    \
    public:                                                             \
        cls##Spawner();                                                 \
        virtual QSharedPointer<Streamable> makeInstance() const;     \
    };
#else
#define DECLARE_SPAWNER( cls )                          \
    class cls##Spawner : virtual StreamableSpawner {    \
    public:                                             \
        cls##Spawner();                                 \
        virtual Streamable* makeInstance() const;       \
    };
#endif

/** \internal Instantiator class implementation
 * It spawns new class with virtual makeInstance() const
 *
 */
#ifdef USE_SHARED_PTR
#define IMPLEMENT_SPAWNER( cls )                                                                        \
    cls::cls##Spawner::cls##Spawner()                                                                   \
    {                                                                                                   \
        (*_Streamables)[ #cls ] = this;                                                                 \
    }                                                                                                   \
    QSharedPointer<Streamable> cls::cls##Spawner::makeInstance() const                               \
    {                                                                                                   \
        return QSharedPointer<Streamable>( new cls( /*Streamable::streamableConstructor*/ ) );       \
    }                                                                                                   \
    cls::cls##Spawner cls##streamable_object;
#else
#define IMPLEMENT_SPAWNER( cls )                                        \
    cls::cls##Spawner::cls##Spawner()                                   \
    {                                                                   \
        (*_Streamables)[ #cls ] = this;                                 \
    }                                                                   \
    Streamable* cls::cls##Spawner::makeInstance() const                 \
    {                                                                   \
        return new cls( /*Streamable::streamableConstructor*/ );        \
    }                                                                   \
    cls::cls##Spawner cls##streamable_object;
#endif


/** \def DECLARE_CUSTOM_STREAMABLE( cls )
 * \brief Declare needed functions for streaming.
 *
 * Insert this to the public part of your class to make it streamable.
 * \param cls name of you class
 */
#define DECLARE_CUSTOM_STREAMABLE( cls )                        \
public:                                                         \
    DECLARE_SPAWNER( cls )                                      \
                                                                \
    friend QDomNode& operator << ( QDomNode& n, const cls& v ); \
    friend bool operator >> ( QDomNode& n, cls& v );            \
                                                                \
    void __registerStoredVar( StoredVarBase* v )                \
    { __storedVars.append( v ); }                               \
    void __unregisterStoredVar( StoredVarBase* v )              \
    { __storedVars.removeAll( v ); }                            \
    StoredVars __storedVars;                                    \
protected:                                                      \
    mutable Streamable::Bool __##cls##loaded;                   \
    mutable Streamable::Bool __##cls##written;                  \
    virtual void __write( QDomElement n ) const;                \
    virtual bool __read( QDomElement n );                       \
    virtual void __prepareToWrite() const;                      \
    virtual const char* __className() const {                   \
        return #cls;                                            \
    }                                                           \
private:                                                        \
    void write( QDomElement ) const;                            \
    bool read( QDomElement );

// Just copy of above #define.
#define DECLARE_STREAMABLE( cls )                       \
public:                                                         \
    DECLARE_SPAWNER( cls )                                      \
                                                                \
    friend QDomNode& operator << ( QDomNode& n, const cls& v ); \
    friend bool operator >> ( QDomNode& n, cls& v );            \
                                                                \
    void __registerStoredVar( StoredVarBase* v )                \
    { __storedVars.append( v ); }                               \
    void __unregisterStoredVar( StoredVarBase* v )              \
    { __storedVars.removeAll( v ); }                            \
    StoredVars __storedVars;                                    \
protected:                                                      \
    mutable Streamable::Bool __##cls##loaded;                   \
    mutable Streamable::Bool __##cls##written;                  \
    virtual void __write( QDomElement n ) const;                \
    virtual bool __read( QDomElement n );                       \
    virtual void __prepareToWrite() const;                      \
    virtual const char* __className() const {                   \
        return #cls;                                            \
    }                                                           \
private:                                                        \
    void write( QDomElement ) const { }                         \
    bool read( QDomElement ) { return true; }



#define __SWRITELINE( v )                                       \
    e << v;                                                     \
    e.lastChild().toElement().setAttribute( "name", #v );

#define __SREADLINE( cls, v )                                                           \
    if( d.attribute("name") != #v ) {                                                   \
        qWarning( "bool "#cls"::read( QDomeElement e ): "                               \
                  "Unexpected name attribute: %s, while reading %s.",                   \
                  (const char* )d.attribute("name").toLocal8Bit().data(),                        \
                  #v );                                                                 \
        return false;                                                                   \
    }                                                                                   \
                                                                                        \
    if( !(d >> v) ) {                                                                   \
        qWarning( "bool "#cls"::read( QDomeElement e ): Error reading '"#v"'." );       \
        return false;                                                                   \
    }

#define IMPLEMENT_STORED_VARS1( cls, v1 )       \
void cls::write( QDomElement e ) const          \
{                                               \
    __SWRITELINE( v1 )                          \
}                                               \
                                                \
bool cls::read( QDomElement e )                 \
{                                               \
    Q_ASSERT( e.firstChild().isElement() );     \
    QDomElement d = e.firstChild().toElement(); \
                                                \
     __SREADLINE( cls, v1 )                     \
                                                \
    return true;                                \
}


#define IMPLEMENT_STORED_VARS2( cls, v1, v2 )   \
void cls::write( QDomElement e ) const          \
{                                               \
    __SWRITELINE( v1 )                          \
    __SWRITELINE( v2 )                          \
}                                               \
                                                \
bool cls::read( QDomElement e )                 \
{                                               \
    Q_ASSERT( e.firstChild().isElement() );     \
    QDomElement d = e.firstChild().toElement(); \
                                                \
     __SREADLINE( cls, v1 )                     \
     __SREADLINE( cls, v2 )                     \
                                                \
    return true;                                \
}


#define IMPLEMENT_STORED_VARS3( cls, v1, v2, v3 )               \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
                                                                \
    return true;                                                \
}


#define IMPLEMENT_STORED_VARS4( cls, v1, v2, v3, v4 )           \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
                                                                \
    return true;                                                \
}


#define IMPLEMENT_STORED_VARS5( cls, v1, v2, v3, v4, v5 )       \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
                                                                \
    return true;                                                \
}


#define IMPLEMENT_STORED_VARS6( cls, v1, v2, v3, v4, v5, v6 )   \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
    __SWRITELINE( v6 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
     __SREADLINE( cls, v6 )                                     \
                                                                \
    return true;                                                \
}

#define IMPLEMENT_STORED_VARS7( cls, v1, v2, v3, v4, v5, v6, v7 )   \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
    __SWRITELINE( v6 )                                          \
    __SWRITELINE( v7 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
     __SREADLINE( cls, v6 )                                     \
     __SREADLINE( cls, v7 )                                     \
                                                                \
    return true;                                                \
}

#define IMPLEMENT_STORED_VARS8( cls, v1, v2, v3, v4, v5, v6, v7, v8 )   \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
    __SWRITELINE( v6 )                                          \
    __SWRITELINE( v7 )                                          \
    __SWRITELINE( v8 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
     __SREADLINE( cls, v6 )                                     \
     __SREADLINE( cls, v7 )                                     \
     __SREADLINE( cls, v8 )                                     \
                                                                \
    return true;                                                \
}

#define IMPLEMENT_STORED_VARS9( cls, v1, v2, v3, v4, v5, v6, v7, v8, v9 )   \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
    __SWRITELINE( v6 )                                          \
    __SWRITELINE( v7 )                                          \
    __SWRITELINE( v8 )                                          \
    __SWRITELINE( v9 )                                          \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
     __SREADLINE( cls, v6 )                                     \
     __SREADLINE( cls, v7 )                                     \
     __SREADLINE( cls, v8 )                                     \
     __SREADLINE( cls, v9 )                                     \
                                                                \
    return true;                                                \
}

#define IMPLEMENT_STORED_VARS10( cls, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 )   \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
    __SWRITELINE( v6 )                                          \
    __SWRITELINE( v7 )                                          \
    __SWRITELINE( v8 )                                          \
    __SWRITELINE( v9 )                                          \
    __SWRITELINE( v10 )                                         \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
     __SREADLINE( cls, v6 )                                     \
     __SREADLINE( cls, v7 )                                     \
     __SREADLINE( cls, v8 )                                     \
     __SREADLINE( cls, v9 )                                     \
     __SREADLINE( cls, v10 )                                    \
                                                                \
    return true;                                                \
}

#define IMPLEMENT_STORED_VARS11( cls, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 )   \
void cls::write( QDomElement e ) const                          \
{                                                               \
    __SWRITELINE( v1 )                                          \
    __SWRITELINE( v2 )                                          \
    __SWRITELINE( v3 )                                          \
    __SWRITELINE( v4 )                                          \
    __SWRITELINE( v5 )                                          \
    __SWRITELINE( v6 )                                          \
    __SWRITELINE( v7 )                                          \
    __SWRITELINE( v8 )                                          \
    __SWRITELINE( v9 )                                          \
    __SWRITELINE( v10 )                                         \
    __SWRITELINE( v11 )                                         \
}                                                               \
                                                                \
bool cls::read( QDomElement e )                                 \
{                                                               \
    Q_ASSERT( e.firstChild().isElement() );                     \
    QDomElement d = e.firstChild().toElement();                 \
                                                                \
     __SREADLINE( cls, v1 )                                     \
     __SREADLINE( cls, v2 )                                     \
     __SREADLINE( cls, v3 )                                     \
     __SREADLINE( cls, v4 )                                     \
     __SREADLINE( cls, v5 )                                     \
     __SREADLINE( cls, v6 )                                     \
     __SREADLINE( cls, v7 )                                     \
     __SREADLINE( cls, v8 )                                     \
     __SREADLINE( cls, v9 )                                     \
     __SREADLINE( cls, v10 )                                    \
     __SREADLINE( cls, v11 )                                    \
                                                                \
    return true;                                                \
}


/** \brief Implement streaming functions
 *
 * Write this in the implementation place of your class after definition
 * of streaming operators.
 * \param cls name of you class
 */
#define IMPLEMENT_STREAMABLE( cls )             \
    IMPLEMENT_SPAWNER( cls )                    \
    IMPLEMENT_ABSTRACT_STREAMABLE( cls )

/// The same as above, but without instantiator object
#define IMPLEMENT_ABSTRACT_STREAMABLE( cls )                                                             \
    void cls::__write( QDomElement s ) const {                                                  \
        Streamable::__write( s );                                                               \
        s.setAttribute( nameTag, #cls );                                                        \
        if( ! __##cls##written ) {                                                              \
            QDomElement data = s.ownerDocument().createElement( dataTag );                      \
            data.setAttribute( nameTag, #cls );                                                 \
            s.appendChild( data );                                                              \
            const StoredVars& v = __storedVars;                                                \
            for( StoredVars::ConstIterator i = v.begin(); i != v.end(); ++i )                   \
                (*i)->write( data );                                                            \
            cls::write( data );                                                                 \
        }                                                                                       \
        __##cls##written = true;                                                                \
    }                                                                                           \
    bool cls::__read( QDomElement s ) {                                                         \
        if( !Streamable::__read( s ) )                                                          \
            return false;                                                                       \
        if( !__##cls##loaded ) {                                                                \
            QDomElement data = s.firstChild().toElement();                                      \
            Q_ASSERT( !data.isNull() );                                                         \
            Q_ASSERT( data.tagName() == dataTag && data.attribute( nameTag ) == #cls );         \
                                                                                                \
            QDomNode n = data.firstChild();                                                     \
            StoredVars& v = __storedVars;                                                      \
            for( StoredVars::Iterator i = v.begin(); i != v.end(); ++i, n = n.nextSibling() )   \
                if( !(*i)->read( n ) )                                                          \
                    return false;                                                               \
                                                                                                \
            if( ! cls::read(data) )                                                             \
                return false;                                                                   \
            __##cls##loaded = true;                                                             \
        }                                                                                       \
        return true;                                                                            \
    }                                                                                           \
    void cls::__prepareToWrite() const {                                                        \
        __##cls##written = false;                                                               \
    }                                                                                           \
    QDomNode& operator << ( QDomNode& n, const cls& v )                                         \
    {                                                                                           \
        v.save( n.toElement() );                                                                \
        return n;                                                                               \
    }                                                                                           \
    bool operator >> ( QDomNode& n, cls& v )                                                    \
    {                                                                                           \
        bool ret = v.__read( n.toElement() );                                                   \
        n = n.nextSibling();                                                                    \
        return ret;                                                                             \
    }


/** \brief Implement streaming functions
 *
 * Write this in the implementation place of your class after definition
 * of streaming operators. If your class is derived from atreamed class,
 * use this macro instead of IMPLEMENT_STREAMABLE( cls )
 * \param cls name of you class
 * \param base1 is a parent of your class
 */
#define IMPLEMENT_STREAMABLE1( cls, base1 )             \
    IMPLEMENT_SPAWNER( cls )                            \
    IMPLEMENT_ABSTRACT_STREAMABLE1( cls, base1 )

/// The same as above, but without instantiator object
#define IMPLEMENT_ABSTRACT_STREAMABLE1( cls, base1 )                                                     \
    void cls::__write( QDomElement s ) const {                                                  \
        s.setAttribute( nameTag, #cls );                                                        \
        if( ! __##base1##written ) {                                                            \
            QDomElement _base1 = s.ownerDocument().createElement( metaTag );                    \
            s.appendChild( _base1 );                                                            \
            base1::__write( _base1 );                                                           \
        }                                                                                       \
        if( ! __##cls##written ) {                                                              \
            QDomElement data = s.ownerDocument().createElement( dataTag );                      \
            data.setAttribute( nameTag, #cls );                                                 \
            __##cls##written = true;                                                            \
            s.appendChild( data );                                                              \
            const StoredVars& v = __storedVars;                                                \
            for( StoredVars::ConstIterator i = v.begin(); i != v.end(); ++i )                   \
                (*i)->write( data );                                                            \
            cls::write( data );                                                                 \
        }                                                                                       \
    }                                                                                           \
    bool cls::__read( QDomElement s ) {                                                         \
        QDomElement el = s.firstChild().toElement();                                            \
                                                                                                \
        if( !__##base1##loaded ) {                                                              \
            Q_ASSERT( !el.isNull() );                                                           \
            Q_ASSERT( el.tagName() == metaTag );                                                \
            if( !base1::__read( el ) )                                                          \
                return false;                                                                   \
            el = el.nextSibling().toElement();                                                  \
        }                                                                                       \
        if( !__##cls##loaded ) {                                                                \
            QDomElement data = el;                                                              \
            Q_ASSERT( !data.isNull() );                                                         \
            Q_ASSERT( data.tagName() == dataTag && data.attribute( nameTag ) == #cls );         \
                                                                                                \
            QDomNode n = data.firstChild();                                                     \
            StoredVars& v = __storedVars;                                                      \
            for( StoredVars::Iterator i = v.begin(); i != v.end(); ++i, n = n.nextSibling() )   \
                if( !(*i)->read( n ) )                                                          \
                    return false;                                                               \
                                                                                                \
            if( !cls::read( data ) )                                                            \
                return false;                                                                   \
                                                                                                \
            __##cls##loaded = true;                                                             \
        }                                                                                       \
        return true;                                                                            \
    }                                                                                           \
    void cls::__prepareToWrite() const {                                                        \
        base1::__prepareToWrite();                                                              \
        __##cls##written = false;                                                               \
    }                                                                                           \
    QDomNode& operator << ( QDomNode& n, const cls& v )                                         \
    {                                                                                           \
        v.save( n.toElement() );                                                                \
        return n;                                                                               \
    }                                                                                           \
    bool operator >> ( QDomNode& n, cls& v )                                                    \
    {                                                                                           \
        bool ret = v.__read( n.toElement() );                                                   \
        n = n.nextSibling();                                                                    \
        return ret;                                                                             \
    }

/** \brief Implement streaming functions
 *
 * Write this in the implementation place of your class after definition
 * of streaming operators. If your class is derived from two atreamed classes,
 * use this macro instead of IMPLEMENT_STREAMABLE( cls )
 * \param cls name of you class
 * \param base1 is the first parent of you class
 * \param base2 is the second parent of your class
 */
#define IMPLEMENT_STREAMABLE2( cls, base1, base2 )      \
    IMPLEMENT_SPAWNER( cls )                            \
    IMPLEMENT_ABSTRACT_STREAMABLE2( cls, base1, base2 )

/// The same as above, but without instantiator object
#define IMPLEMENT_ABSTRACT_STREAMABLE2( cls, base1, base2 )                                              \
    void cls::__write( QDomElement s ) const {                                                  \
        s.setAttribute( nameTag, #cls );                                                        \
                                                                                                \
        QDomElement _base;                                                                      \
                                                                                                \
        if( ! __##base1##written ) {                                                            \
            _base = s.ownerDocument().createElement( metaTag );                                 \
            s.appendChild( _base );                                                             \
            base1::__write( _base );                                                            \
        }                                                                                       \
        if( ! __##base2##written ) {                                                            \
            _base = s.ownerDocument().createElement( metaTag );                                 \
            s.appendChild( _base );                                                             \
            base2::__write( _base );                                                            \
        }                                                                                       \
                                                                                                \
        if( ! __##cls##written ) {                                                              \
            QDomElement data = s.ownerDocument().createElement( dataTag );                      \
            data.setAttribute( nameTag, #cls );                                                 \
            __##cls##written = true;                                                            \
            s.appendChild( data );                                                              \
            const StoredVars& v = __storedVars;                                                \
            for( StoredVars::ConstIterator i = v.begin(); i != v.end(); ++i )                   \
                (*i)->write( data );                                                            \
            cls::write( data );                                                                 \
        }                                                                                       \
    }                                                                                           \
    bool cls::__read( QDomElement s ) {                                                         \
        QDomElement el = s.firstChild().toElement();                                            \
                                                                                                \
        if( ! __##base1##loaded ) {                                                             \
            Q_ASSERT( !el.isNull() );                                                           \
            Q_ASSERT( el.tagName() == metaTag );                                                \
            if( !base1::__read( el ) )                                                          \
                return false;                                                                   \
            el = el.nextSibling().toElement();                                                  \
        }                                                                                       \
                                                                                                \
        if( ! __##base2##loaded ) {                                                             \
            Q_ASSERT( !el.isNull() );                                                           \
            Q_ASSERT( el.tagName() == metaTag );                                                \
            if( !base2::__read( el ) )                                                          \
                return false;                                                                   \
            el = el.nextSibling().toElement();                                                  \
        }                                                                                       \
                                                                                                \
        if( !__##cls##loaded ) {                                                                \
            QDomElement data = el.toElement();                                                  \
            Q_ASSERT( !data.isNull() );                                                         \
            Q_ASSERT( data.tagName() == dataTag && data.attribute( nameTag ) == #cls );         \
                                                                                                \
            QDomNode n = data.firstChild();                                                     \
            StoredVars& v = __storedVars;                                                      \
            for( StoredVars::Iterator i = v.begin(); i != v.end(); ++i, n = n.nextSibling() )   \
                if( !(*i)->read( n ) )                                                          \
                    return false;                                                               \
                                                                                                \
            if( !cls::read( data ) )                                                            \
                return false;                                                                   \
                                                                                                \
            __##cls##loaded = true;                                                             \
        }                                                                                       \
        return true;                                                                            \
    }                                                                                           \
    void cls::__prepareToWrite() const {                                                        \
        base1::__prepareToWrite();                                                              \
        base2::__prepareToWrite();                                                              \
        __##cls##written = false;                                                               \
    }                                                                                           \
    QDomNode& operator << ( QDomNode& n, const cls& v )                                         \
    {                                                                                           \
        v.save( n.toElement() );                                                                \
        return n;                                                                               \
    }                                                                                           \
    bool operator >> ( QDomNode& n, cls& v )                                                    \
    {                                                                                           \
        bool ret = v.__read( n.toElement() );                                                   \
        n = n.nextSibling();                                                                    \
        return ret;                                                                             \
    }


/** \brief Implement streaming functions
 *
 * Write this in the implementation place of your class after definition
 * of streaming operators. If your class is derived from three atreamed classes,
 * use this macro instead of IMPLEMENT_STREAMABLE( cls )
 * \param cls name of you class
 * \param base1 is the first parent of you class
 * \param base2 is the second parent of your class
 * \param base3 is the third parent of your class
 */
#define IMPLEMENT_STREAMABLE3( cls, base1, base2, base3 )       \
    IMPLEMENT_SPAWNER( cls )                                    \
    IMPLEMENT_ABSTRACT_STREAMABLE3( cls, base1, base2, base3 )

/// The same as above, but without instantiator object
#define IMPLEMENT_ABSTRACT_STREAMABLE3( cls, base1, base2, base3 )                                       \
    void cls::__write( QDomElement s ) const {                                                  \
        s.setAttribute( nameTag, #cls );                                                        \
                                                                                                \
        QDomElement _base;                                                                      \
                                                                                                \
        if( ! __##base1##written ) {                                                            \
            _base = s.ownerDocument().createElement( metaTag );                                 \
            s.appendChild( _base );                                                             \
            base1::__write( _base );                                                            \
        }                                                                                       \
        if( ! __##base2##written ) {                                                            \
            _base = s.ownerDocument().createElement( metaTag );                                 \
            s.appendChild( _base );                                                             \
            base2::__write( _base );                                                            \
        }                                                                                       \
        if( ! __##base3##written ) {                                                            \
            _base = s.ownerDocument().createElement( metaTag );                                 \
            s.appendChild( _base );                                                             \
            base3::__write( _base );                                                            \
        }                                                                                       \
                                                                                                \
        if( ! __##cls##written ) {                                                              \
            QDomElement data = s.ownerDocument().createElement( dataTag );                      \
            data.setAttribute( nameTag, #cls );                                                 \
            __##cls##written = true;                                                            \
            s.appendChild( data );                                                              \
            const StoredVars& v = __storedVars;                                                \
            for( StoredVars::ConstIterator i = v.begin(); i != v.end(); ++i )                   \
                (*i)->write( data );                                                            \
            cls::write( data );                                                                 \
        }                                                                                       \
    }                                                                                           \
    bool cls::__read( QDomElement s ) {                                                         \
        QDomElement el = s.firstChild().toElement();                                            \
                                                                                                \
        if( ! __##base1##loaded ) {                                                             \
            Q_ASSERT( !el.isNull() );                                                           \
            Q_ASSERT( el.tagName() == metaTag );                                                \
            if( !base1::__read( el ) )                                                          \
                return false;                                                                   \
            el = el.nextSibling().toElement();                                                  \
        }                                                                                       \
                                                                                                \
        if( ! __##base2##loaded ) {                                                             \
            Q_ASSERT( !el.isNull() );                                                           \
            Q_ASSERT( el.tagName() == metaTag );                                                \
            if( !base2::__read( el ) )                                                          \
                return false;                                                                   \
            el = el.nextSibling().toElement();                                                  \
        }                                                                                       \
                                                                                                \
        if( ! __##base3##loaded ) {                                                             \
            Q_ASSERT( !el.isNull() );                                                           \
            Q_ASSERT( el.tagName() == metaTag );                                                \
            if( !base3::__read( el ) )                                                          \
                return false;                                                                   \
            el = el.nextSibling().toElement();                                                  \
        }                                                                                       \
                                                                                                \
        if( !__##cls##loaded ) {                                                                \
            QDomElement data = el.toElement();                                                  \
            Q_ASSERT( !data.isNull() );                                                         \
            Q_ASSERT( data.tagName() == dataTag && data.attribute( nameTag ) == #cls );         \
                                                                                                \
            QDomNode n = data.firstChild();                                                     \
            StoredVars& v = __storedVars;                                                      \
            for( StoredVars::Iterator i = v.begin(); i != v.end(); ++i, n = n.nextSibling() )   \
                if( !(*i)->read( n ) )                                                          \
                    return false;                                                               \
                                                                                                \
            if( !cls::read( data ) )                                                            \
                return false;                                                                   \
                                                                                                \
            __##cls##loaded = true;                                                             \
        }                                                                                       \
        return true;                                                                            \
    }                                                                                           \
    void cls::__prepareToWrite() const {                                                        \
        base1::__prepareToWrite();                                                              \
        base2::__prepareToWrite();                                                              \
        base3::__prepareToWrite();                                                              \
        __##cls##written = false;                                                               \
    }                                                                                           \
    QDomNode& operator << ( QDomNode& n, const cls& v )                                         \
    {                                                                                           \
        v.save( n.toElement() );                                                                \
        return n;                                                                               \
    }                                                                                           \
    bool operator >> ( QDomNode& n, cls& v )                                                    \
    {                                                                                           \
        bool ret = v.__read( n.toElement() );                                                   \
        n = n.nextSibling();                                                                    \
        return ret;                                                                             \
    }


#ifdef USE_SHARED_PTR
QDomNode& operator << ( QDomNode& n, QSharedPointer<const Streamable> v );
#else
QDomNode& operator << ( QDomNode& n, const Streamable* v );
#endif

#ifdef USE_SHARED_PTR
template<class T>
bool operator >> ( QDomNode& n, QSharedPointer<T>& v )
{
    QSharedPointer<Streamable> s = Streamable::load( n.toElement() );
    v = dynamic_cast<T>( s );
    if( !v ) {
        n = n.nextSibling();
        return false;
    }
    return true;
}
#else
template<class T>
bool operator >> ( QDomNode& n, T*& v )
{
    Streamable* s = Streamable::load( n.toElement() );
    v = dynamic_cast<T*>( s );
    if( !v ) {
        delete s;
        n = n.nextSibling();
        return false;
    }
    return true;
}
#endif



class StoredVarBase {
public:
    virtual void write( QDomNode parent ) const = 0;
    virtual bool read( QDomNode from ) = 0;
    virtual ~StoredVarBase(){}
};
typedef QList<StoredVarBase*> StoredVars;

template <class Base, class Val>
class StoredVar : public StoredVarBase {
    Base*       _parent;
    Val         _val;
    QString     _name;
public:
    StoredVar( Base* parent, const Val& v, QString name = QString::null );
    StoredVar( Base* parent, QString name = QString::null );
    void write( QDomNode parent ) const;
    bool read( QDomNode from );

    operator Val&()       { return _val; }
    operator Val&() const { return _val; }
    const Val& constRef() const    { return _val; }
    Val& operator=( const Val& v ) { _val = v; return *this; }
};

template <class Base, class Val>
StoredVar<Base, Val>::StoredVar( Base* parent, const Val& v, QString name )
    : _parent(parent), _val( v ), _name( name )
{
    _parent->__registerStoredVar( this );
}

template <class Base, class Val>
StoredVar<Base, Val>::StoredVar( Base* parent, QString name )
    : _parent(parent),_val( Val() ), _name( name )
{
    _parent->__registerStoredVar( this );
}

template <class Base, class Val>
void StoredVar<Base, Val>::write( QDomNode parent ) const
{
    parent << _val;

    // Save name attribute, if exists.
    if( _name )
        parent.lastChild().toElement().setAttribute( "name", _name );
}

template <class Base, class Val>
bool StoredVar<Base, Val>::read( QDomNode from )
{
    Q_ASSERT( from.toElement().attribute( "name" ) == _name );
    return (from >> _val);
}

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
