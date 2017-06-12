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
#include "streamable.h"
using namespace std;

/** \class Streamable
 *  \brief A base class for all classes dynamically stored to data stream
 *
 *  Derive classes you want to dynamically save to stream from Streamable,
 *  fill the DECLARE_STREAMABLE( cls ) and IMPLEMENT_STREAMABLE( cls ) macros
 *  and be happy :-)
 */

/**  \example test.cpp
     This is an example of using this library. Note that type of objects
     are automatically deduced before read.
 */

/** \brief A global storage of object instances
 *
 *  All objects are registered here with IMPLEMENT_STREAMABLE(N) macros
 *  when an object needs to be loaded, this map is asked for a class with
 *  corresponding name, then it is clone()'d and read from stream
 */
QMap<QString, StreamableSpawner*> *Streamable::_Streamables;

/** \internal A var indicating when it is time to create or destroy
 * the global map. It is used as workaround of different compilers to
 * endure that map is created before first static instance is made
 */
int StreamableInit::n = 0;

const QString Streamable::nameTag = "classname";
const QString Streamable::dataTag = "data";
const QString Streamable::metaTag = "meta";

/** \brief Get an instance for class \a className
 *
 * Create an instance for the object of class \a className.
 * It is used with macro IMPLEMENT_STREAMABLE( cls )
 * for specified class name. This function is used inside stream engine, so you
 * wouldn't need to call it directly.
 * \retval 0 if the specified object couldn't be instantiated (there's no such a class).
 * \retval pointer to the created instance. You get an instance created with default
 * constructor. You must delete it when you finish with it.
 */
#ifdef USE_SHARED_PTR
QSharedPointer<Streamable> Streamable::spawn( QString className )
{
    QMap<QString, StreamableSpawner*>::ConstIterator i = Streamable::_Streamables->find( className );
    if( i == _Streamables->end() ) {
        qWarning( "Streamable::spawn('%s'): Class is not in streamable database", (const char*)className.toLocal8Bit().data() );
        return QSharedPointer<Streamable>();
    }
    return (*i)->makeInstance();
}
#else
Streamable* Streamable::spawn( QString className )
{
    QMap<QString, StreamableSpawner*>::ConstIterator i = Streamable::_Streamables->find( className );
    if( i == _Streamables->end() ) {
        qWarning( "Streamable::spawn('%s'): Class is not in streamable database", (const char*)className.toLocal8Bit().data() );
        return 0;
    }
    return (*i)->makeInstance();
}
#endif

/** \brief Load object from stream
 *
 * This virtual function is created inside IMPLEMENT_STREAMABLE macro.
 * You probably wouldn't call it directly. \internal
 */
bool Streamable::__read( QDomElement )
{
    return true;
}

/** \brief Save object to stream
 *
 * This virtual function is created inside IMPLEMENT_STREAMABLE macro.
 * You probably wouldn't call it directly. \internal
 */
void Streamable::__write( QDomElement ) const
{

}

/** \brief Save the object to XML tree
 *
 *  Save object to \a parent subtree.
 *  \param parent a parent of the created XML packet
 */

void Streamable::save( QDomNode parent ) const
{
    QDomElement element = parent.ownerDocument().createElement( "class" );
    element.setAttribute( nameTag, __className() );
    parent.appendChild( element );
    __prepareToWrite();
    __write( element );
}


/** \brief Get an XML element representing this object
 *
 *  \param doc a document which creates the returned element
 */
QDomElement Streamable::classElement( QDomDocument doc ) const
{
    QDomElement element = doc.createElement( "class" );
    element.setAttribute( nameTag, __className() );
    __prepareToWrite();
    __write( element );
    return element;
}


/** \brief Instantiate and read an object from given XML packet
 *
 * Call this function to get an instance of the saved object
 * \param classNode a packet to read the object from.
 * \retval 0 if the specified object couldn't be instantiated or read properly.
 * \retval pointer to the created instance if all was OK. In this case, You get
 * an instance created with default constructor. You must delete it when you finish with it.
 */
#ifdef USE_SHARED_PTR
QSharedPointer<Streamable> Streamable::load( QDomElement classNode )
{
    Q_ASSERT( classNode.tagName() == "class" );
    QString name = classNode.attribute(nameTag);
    QSharedPointer<Streamable> obj;
    if( !(obj = spawn(name)) )
        return QSharedPointer<Streamable>();
    if( !obj->__read(classNode) ) {
        qWarning( "Streamable::load( '%s' ): object read failed", (const char*)name.toLocal8Bit().data() );
        return QSharedPointer<Streamable>();
    }
    return obj;
}
#else
Streamable* Streamable::load( QDomElement classNode )
{
    Q_ASSERT( classNode.tagName() == "class" );
    QString name = classNode.attribute(nameTag);
    Streamable* obj;
    if( !(obj = spawn(name)) )
        return 0;
    if( !obj->__read(classNode) ) {
        qWarning( "Streamable::load( '%s' ): object read failed", (const char*)name.toLocal8Bit().data() );
        delete obj;
        return 0;
    }
    return obj;
}
#endif

/** \fn Streamable::__className() const
 * \brief Get class name of the specified class
 * \internal
 */

/** \fn Streamable::read( QDomElement element )
 * \brief Read user data from XML tree.
 *
 * Reimplement this function (default just returns true)
 * if you want to load
 * custom data from XML stream. StoredVal seems to be
 * simpler approach.
 * \param element a data element to save your data to
 * \retval true if reading was successful
 * \retval false otherwise
 */

/** \fn Streamable::write( QDomElement parent ) const
 * \brief Save your data to XML tree
 *
 * Reimplement this function (default one does nothing)
 * if you want to save custom data to the stream. StoredVal
 * seems to be simpler approach.
 */

/** \class Streamable::Bool
 * Boolean variable used to store
 * default value (false). Used for initial construction
 * of an object to be read properly
 */

/// Get bool DOM element
QDomElement element( QDomDocument doc, const bool& value, QString name )
{
    QDomElement ret = doc.createElement( "bool" );
    if( !name.isEmpty() )
        ret.setAttribute( "name", name );
    QDomText txt = doc.createTextNode( value ? "true" : "false" );
    ret.appendChild( txt );
    return ret;
}

/// Get int DOM element
QDomElement element( QDomDocument doc, const int& value, QString name )
{
    QDomElement ret = doc.createElement( "int" );
    if( !name.isEmpty() )
        ret.setAttribute( "name", name );
    QDomText txt = doc.createTextNode( QString::number(value) );
    ret.appendChild( txt );
    return ret;
}

/// Get double DOM element
QDomElement element( QDomDocument doc, const double& value, QString name  )
{
    QDomElement ret = doc.createElement( "double" );
    if( !name.isEmpty() )
        ret.setAttribute( "name", name );
    QDomText txt = doc.createTextNode( QString::number(value) );
    ret.appendChild( txt );
    return ret;
}

/// Get string DOM element
QDomElement element( QDomDocument doc, const QString& value, QString name )
{
    QDomElement ret = doc.createElement( "string" );
    if( !name.isEmpty() )
        ret.setAttribute( "name", name );
    QDomText txt = doc.createTextNode( value );
    ret.appendChild( txt );
    return ret;
}

/// Read int \a value from \a element
bool value( QDomNode element, bool& value )
{
    QDomText data = element.firstChild().toText();
    Q_ASSERT( !data.isNull() );
    if( data.isNull() )
        return false;
    if( data.data() == "true" ) {
        value = true;
        return true;
    }
    else if (data.data() == "false") {
        value = false;
        return true;
    }
    else
        return false;
}

/// Read double \a value from \a element
bool value( QDomNode element, int& value )
{
    QDomText data = element.firstChild().toText();
    Q_ASSERT( !data.isNull() );
    if( data.isNull() )
        return false;
    bool ok;
    value = data.data().toInt( &ok );
    return ok;
}


/// Read double \a value from \a element
bool value( QDomNode element, double& value )
{
    QDomText data = element.firstChild().toText();
    Q_ASSERT( !data.isNull() );
    if( data.isNull() )
        return false;
    bool ok;
    value = data.data().toDouble( &ok );
    return ok;
}

/// Read string \a value from \a element
bool value( QDomNode element, QString& value )
{
    QDomText data = element.firstChild().toText();
    Q_ASSERT( !data.isNull() );
    if( data.isNull() )
        return false;
    value = data.data();
    return true;
}

/** \class StoredValBase
 *  \internal A base class for stored variables
 */

/** \class StoredVal
 * \brief A value stored in XML
 *
 * This is a convenient way to store your simple data
 * in XML. Just use StoredVal instead of your type and
 * it will be automatically saved/restored to XML
 * For example:
 * \code
 * class Sample : public Streamable {
 *     DECLARE_STREAMABLE( Sample )
 * protected:
 *     virtual void write( QDomElement ) const;
 *     virtual bool read( QDomElement );
 *     int var_to_store;
 * };
 * IMPLEMENT_STREAMABLE( Sample );
 * ...
 * \endcode
 * you write
 * \code
 * class Sample : public Streamable {
 *     DECLARE_STREAMABLE( Sample )
 * protected:
 *     StoredVal<int> var_to_store;
 * public:
 *     Sample();
 * };
 * IMPLEMENT_STREAMABLE( Sample );
 * ...
 * Sample::Sample()
 *     : var_to_store( this, "Sample", "var_to_store" )
 * {
 * ...
 * }
 * \endcode
 * and var_to_store will be saved and loaded automatically
 */


/** \fn StoredVal::StoredVal( Streamable* parent, const Val& v, QString typeName, QString name )
 *  \brief Constructs StoredVal with value \a v
 *
 *  \param parent a Streamable to which this variable belongs
 *  \param v an initial value
 *  \param typeName a name of the class this variable defined in.
 *         One must provide this name, because I cannot imagine the
 *         way to provide it implicitly (virtual function will give the name
 *         of the last child, and non-virtual will always be called from
 *         Streamable itself)
 *  \param name optional variable name
 */

/** \fn StoredVal::StoredVal( Streamable* parent, QString typeName, QString name )
 *  \brief Constructs StoredVal with value \a v
 *
 *  \param parent a Streamable to which this variable belongs
 *  \param typeName a name of the class this variable defined in.
 *         One must provide this name, because I cannot imagine the
 *         way to provide it implicitly (virtual function will give the name
 *         of the last child, and non-virtual will always be called from
 *         Streamable itself)
 *  \param name optional variable name
 */

/** \fn StoredVal::write (QDomNode parent) const
 * \brief Write value to XML element and append it to \a parent
 */

/** \fn StoredVal::read (QDomNode from)
 * \brief Read value from node \a from
 */

/** \fn StoredVal::operator Val & ()
 * \brief Get a reference to real value for manipulation
 */

/** \fn StoredVal::operator= (const Val &v)
 * \brief Simple assign operator
 */

#ifdef USE_SHARED_PTR
QDomNode& operator << ( QDomNode& n, QSharedPointer<const Streamable> v )
{
    if( v )
        v->save( n.toElement() );
    return n;
}
#else
QDomNode& operator << ( QDomNode& n, const Streamable* v )
{
    if( v )
        v->save( n.toElement() );
    return n;
}
#endif
