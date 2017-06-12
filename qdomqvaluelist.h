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
 * Header for QDomNode stream operators << and >> with the QValueList type.
 *
 *
 * Kir Kostuchenko --- 19-Feb-2003 11:44 <kir@users.sf.net> ---
 *
 */

#ifndef QDOMQVALUELIST_H
#define QDOMQVALUELIST_H

#include <qdom.h>

template<class Val>
QDomNode& operator << ( QDomNode& n, const QList<Val>& v )
{
    QDomElement e = n.ownerDocument().createElement( "qvaluelist" );

    for( typename QList<Val>::ConstIterator i = v.begin(); i != v.end(); ++i )
        e << *i;

    n.appendChild( e );
    return n;
}


template<class Val>
bool operator >> ( QDomNode& n, QList<Val>& v )
{
    v.clear();

    QDomElement e = n.toElement();

    if( e.tagName() != "qvaluelist" ) {
        qWarning( "QDomNode& operator >> ( QDomNode& n, QValueList<Val>& v ): wrong type tag: %s",
                  (const char*)e.tagName().toLocal8Bit().data() );
        n = n.nextSibling();
        return false;
    }

    bool ret = true;
    Val val;
    QDomNode fc = e.firstChild();

    while( !fc.isNull() ) {
        ret = fc >> val;

        if( !ret )
            break;

        v.append( val );
    }

    n = n.nextSibling();
    return ret;
}

#endif

// Local Variables:
// mode: C++
// c-basic-offset: 4
// End:
