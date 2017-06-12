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
/*
 * Header for QDomNode stream operators << and >> with the basic types.
 *
 *
 * Kir Kostuchenko --- 19-Feb-2003 11:44 <kir@users.sf.net> ---
 *
 */

#ifndef QDOMSTREAM_H
#define QDOMSTREAM_H

#include <qdom.h>
#include <qstring.h>

// bool
QDomNode& operator << ( QDomNode& n, const bool& v );
bool operator >> ( QDomNode& n, bool& v );

// int
QDomNode& operator << ( QDomNode& n, const int& v );
bool operator >> ( QDomNode& n, int& v );

// double
QDomNode& operator << ( QDomNode& n, const double& v );
bool operator >> ( QDomNode& n, double& v );

// QString
QDomNode& operator << ( QDomNode& n, const QString& v );
bool operator >> ( QDomNode& n, QString& v );

// char* -> QString conversion.
inline QDomNode& operator << ( QDomNode& n, const char* v )
{
    return (n << QString(v));
}

// Just help for writing enum types operators.
QDomNode& writeEnum( QDomNode& n, const QString& typeName, const QString& v );
bool readEnum( QDomNode& n, const QString& typeName, QString& v );

#define IMPLEMENT_QDOM_ENUM2( BaseType, EnumType, v1, v2 )             \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}


#define IMPLEMENT_QDOM_ENUM3( BaseType, EnumType, v1, v2, v3 )         \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    case BaseType::v3:                                                 \
        return writeEnum( n, typeName, #v3 );                          \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v3 ) {                                             \
        v = BaseType::v3;                                              \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}

#define IMPLEMENT_QDOM_ENUM4( BaseType, EnumType, v1, v2, v3, v4 )     \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    case BaseType::v3:                                                 \
        return writeEnum( n, typeName, #v3 );                          \
    case BaseType::v4:                                                 \
        return writeEnum( n, typeName, #v4 );                          \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v3 ) {                                             \
        v = BaseType::v3;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v4 ) {                                             \
        v = BaseType::v4;                                              \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}

#define IMPLEMENT_QDOM_ENUM5( BaseType, EnumType, v1, v2, v3, v4, v5 ) \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    case BaseType::v3:                                                 \
        return writeEnum( n, typeName, #v3 );                          \
    case BaseType::v4:                                                 \
        return writeEnum( n, typeName, #v4 );                          \
    case BaseType::v5:                                                 \
        return writeEnum( n, typeName, #v5 );                          \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v3 ) {                                             \
        v = BaseType::v3;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v4 ) {                                             \
        v = BaseType::v4;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v5 ) {                                             \
        v = BaseType::v5;                                              \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}


#define IMPLEMENT_QDOM_ENUM8( BaseType, EnumType, v1, v2, v3, v4, v5, v6, v7, v8 ) \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    case BaseType::v3:                                                 \
        return writeEnum( n, typeName, #v3 );                          \
    case BaseType::v4:                                                 \
        return writeEnum( n, typeName, #v4 );                          \
    case BaseType::v5:                                                 \
        return writeEnum( n, typeName, #v5 );                          \
    case BaseType::v6:                                                 \
        return writeEnum( n, typeName, #v6 );                          \
    case BaseType::v7:                                                 \
        return writeEnum( n, typeName, #v7 );                          \
    case BaseType::v8:                                                 \
        return writeEnum( n, typeName, #v8 );                          \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v3 ) {                                             \
        v = BaseType::v3;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v4 ) {                                             \
        v = BaseType::v4;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v5 ) {                                             \
        v = BaseType::v5;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v6 ) {                                             \
        v = BaseType::v6;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v7 ) {                                             \
        v = BaseType::v7;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v8 ) {                                             \
        v = BaseType::v8;                                              \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}


#define IMPLEMENT_QDOM_ENUM9( BaseType, EnumType, v1, v2, v3, v4, v5, v6, v7, v8, v9 ) \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    case BaseType::v3:                                                 \
        return writeEnum( n, typeName, #v3 );                          \
    case BaseType::v4:                                                 \
        return writeEnum( n, typeName, #v4 );                          \
    case BaseType::v5:                                                 \
        return writeEnum( n, typeName, #v5 );                          \
    case BaseType::v6:                                                 \
        return writeEnum( n, typeName, #v6 );                          \
    case BaseType::v7:                                                 \
        return writeEnum( n, typeName, #v7 );                          \
    case BaseType::v8:                                                 \
        return writeEnum( n, typeName, #v8 );                          \
    case BaseType::v9:                                                 \
        return writeEnum( n, typeName, #v9 );                          \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v3 ) {                                             \
        v = BaseType::v3;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v4 ) {                                             \
        v = BaseType::v4;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v5 ) {                                             \
        v = BaseType::v5;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v6 ) {                                             \
        v = BaseType::v6;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v7 ) {                                             \
        v = BaseType::v7;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v8 ) {                                             \
        v = BaseType::v8;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v9 ) {                                             \
        v = BaseType::v9;                                              \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}


#define IMPLEMENT_QDOM_ENUM15( BaseType, EnumType, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15 ) \
QDomNode& operator << ( QDomNode& n, const BaseType::EnumType& v )     \
{                                                                      \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
                                                                       \
    switch( v ) {                                                      \
    case BaseType::v1:                                                 \
        return writeEnum( n, typeName, #v1 );                          \
    case BaseType::v2:                                                 \
        return writeEnum( n, typeName, #v2 );                          \
    case BaseType::v3:                                                 \
        return writeEnum( n, typeName, #v3 );                          \
    case BaseType::v4:                                                 \
        return writeEnum( n, typeName, #v4 );                          \
    case BaseType::v5:                                                 \
        return writeEnum( n, typeName, #v5 );                          \
    case BaseType::v6:                                                 \
        return writeEnum( n, typeName, #v6 );                          \
    case BaseType::v7:                                                 \
        return writeEnum( n, typeName, #v7 );                          \
    case BaseType::v8:                                                 \
        return writeEnum( n, typeName, #v8 );                          \
    case BaseType::v9:                                                 \
        return writeEnum( n, typeName, #v9 );                          \
    case BaseType::v10:                                                \
        return writeEnum( n, typeName, #v10 );                         \
    case BaseType::v11:                                                \
        return writeEnum( n, typeName, #v11 );                         \
    case BaseType::v12:                                                \
        return writeEnum( n, typeName, #v12 );                         \
    case BaseType::v13:                                                \
        return writeEnum( n, typeName, #v13 );                         \
    case BaseType::v14:                                                \
        return writeEnum( n, typeName, #v14 );                         \
    case BaseType::v15:                                                \
        return writeEnum( n, typeName, #v15 );                         \
    default:                                                           \
        return writeEnum( n, typeName, "Unknown enum value" );         \
    }                                                                  \
}                                                                      \
                                                                       \
                                                                       \
bool operator >> ( QDomNode& n, BaseType::EnumType& v )                \
{                                                                      \
    QString sv;                                                        \
    QString typeName = QString( #BaseType#EnumType ).lower();          \
    if( !readEnum(n, typeName, sv) )                                   \
        return false;                                                  \
                                                                       \
    if( sv == #v1 ) {                                                  \
        v = BaseType::v1;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v2 ) {                                             \
        v = BaseType::v2;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v3 ) {                                             \
        v = BaseType::v3;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v4 ) {                                             \
        v = BaseType::v4;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v5 ) {                                             \
        v = BaseType::v5;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v6 ) {                                             \
        v = BaseType::v6;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v7 ) {                                             \
        v = BaseType::v7;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v8 ) {                                             \
        v = BaseType::v8;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v9 ) {                                             \
        v = BaseType::v9;                                              \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v10 ) {                                            \
        v = BaseType::v10;                                             \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v11 ) {                                            \
        v = BaseType::v11;                                             \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v12 ) {                                            \
        v = BaseType::v12;                                             \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v13 ) {                                            \
        v = BaseType::v13;                                             \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v14 ) {                                            \
        v = BaseType::v14;                                             \
        return true;                                                   \
    }                                                                  \
    else if( sv == #v15 ) {                                            \
        v = BaseType::v15;                                             \
        return true;                                                   \
    }                                                                  \
    else                                                               \
        return false;                                                  \
}


#endif

// Local Variables:
// mode: C++
// c-basic-offset: 4
// End:
