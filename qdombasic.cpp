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
#include "qdombasic.h"

// bool --------------------------------------------------------------------
QDomNode& operator << ( QDomNode& n, const bool& v )
{
    QDomElement e = n.ownerDocument().createElement( "bool" );
    QDomText t = n.ownerDocument().createTextNode( v ? "true" : "false" );

    e.appendChild( t );
    n.appendChild( e );

    return n;
}


bool operator >> ( QDomNode& n, bool& v )
{
    QDomElement e = n.toElement();

    bool ret = true;
    if( e.tagName() == "bool" ) {
        QDomText data = e.firstChild().toText();

        if( data.data() == "true" )
            v = true;
        else if( data.data() == "false" )
            v = false;
        else {
            qWarning( "QDomNode& operator >> ( QDomNode& n, bool& v ): wrong data: %s",
                      (const char* )data.data().toLocal8Bit() );
            ret = false;
        }
    }
    else {
        qWarning( "QDomNode& operator >> ( QDomNode& n, bool& v ): wrong type tag: %s",
                  (const char*)e.tagName().toLocal8Bit() );
        ret = false;
    }

    n = n.nextSibling();
    return ret;
}


// int ------------------------------------------------------------------------------------------
QDomNode& operator << ( QDomNode& n, const int& v )
{
    QDomElement e = n.ownerDocument().createElement( "int" );
    QDomText t = n.ownerDocument().createTextNode( QString::number(v) );

    e.appendChild( t );
    n.appendChild( e );

    return n;
}


bool operator >> ( QDomNode& n, int& v )
{
    QDomElement e = n.toElement();

    bool ret = true;
    if( e.tagName() == "int" ) {
        QDomText data = e.firstChild().toText();
        
        v = data.data().toInt( &ret );
        
        if( !ret )
            qWarning( "QDomNode& operator >> ( QDomNode& n, int& v ): wrong data: %s",
                      (const char* )data.data().toLocal8Bit() );
    }
    else {
        qWarning( "QDomNode& operator >> ( QDomNode& n, int& v ): wrong type tag: %s",
                  (const char*)e.tagName().toLocal8Bit() );
        ret = false;
    }

    n = n.nextSibling();
    return ret;
}

// double ------------------------------------------------------------------------------------------
QDomNode& operator << ( QDomNode& n, const double& v )
{
    QDomElement e = n.ownerDocument().createElement( "double" );
    QDomText t = n.ownerDocument().createTextNode( QString::number(v) );

    e.appendChild( t );
    n.appendChild( e );

    return n;
}


bool operator >> ( QDomNode& n, double& v )
{
    QDomElement e = n.toElement();

    bool ret = true;
    if( e.tagName() == "double" ) {
        QDomText data = e.firstChild().toText();

        v = data.data().toDouble( &ret );

        if( !ret )
            qWarning( "QDomNode& operator >> ( QDomNode& n, double& v ): wrong data: %s",
                      (const char* )data.data().toLocal8Bit() );
    }
    else {
        qWarning( "QDomNode& operator >> ( QDomNode& n, double& v ): wrong type tag: %s",
                  (const char*)e.tagName().toLocal8Bit() );
        ret = false;
    }

    n = n.nextSibling();
    return ret;
}

// QString ------------------------------------------------------------------------------------------
QDomNode& operator << ( QDomNode& n, const QString& v )
{
    QDomElement e = n.ownerDocument().createElement( "qstring" );
    QDomText t = n.ownerDocument().createTextNode( v );

    e.appendChild( t );
    n.appendChild( e );

    return n;
}


bool operator >> ( QDomNode& n, QString& v )
{
    QDomElement e = n.toElement();

    bool ret = true;
    if( e.tagName() == "qstring" ) {
        QDomText data = e.firstChild().toText();

        v = data.data();
    }
    else {
        qWarning( "QDomNode& operator >> ( QDomNode& n, QString& v ): wrong type tag: %s",
                  (const char*)e.tagName().toLocal8Bit() );
        ret = false;
    }

    n = n.nextSibling();
    return ret;
}


// Enum types ----------------------------------------------------------------------------------------
QDomNode& writeEnum( QDomNode& n, const QString& typeName, const QString& v )
{
    QDomElement e = n.ownerDocument().createElement( typeName );
    QDomText t = n.ownerDocument().createTextNode( v );

    e.appendChild( t );
    n.appendChild( e );

    return n;
}


bool readEnum( QDomNode& n, const QString& typeName, QString& v )
{
    QDomElement e = n.toElement();

    bool ret = true;
    if( e.tagName() == typeName ) {
        QDomText data = e.firstChild().toText();

        v = data.data();
    }
    else {
        qWarning( "bool readEnum( QDomNode& n, '%s', QString& v ): wrong type tag: %s",
                  (const char*)typeName.toLocal8Bit(), (const char*)e.tagName().toLocal8Bit() );
        ret = false;
    }

    n = n.nextSibling();
    return ret;
}
