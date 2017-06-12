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

#ifndef __SEMANTIC_TYPE__
#define __SEMANTIC_TYPE__
class Semantic;
#include <qstring.h>

class ParType {
public:
    enum Builtin { Boolean, Int, Double, Char, String, Void };
    enum Access  { RValue, LValue };

public:
    ParType( Builtin type, const Semantic* s, Access acc = RValue );
    
    // check if a 'clean' cast available, i.e.
    // LInt -> RInt
    bool cleanCast( const ParType& type ) const;

    // returns cose of cast, i.e.
    // char -> short = 1
    // char -> int   = 2
    // char -> float = 3...
    int  widerCast( const ParType& type ) const;


    // check for cast function, i.e.
    // double -> int
    bool stdCast  ( const ParType& type ) const;

    Builtin type()   const { return (Builtin)type_; }
    Access  access() const { return access_; }
private:
    int    type_;
    Access access_;
    const Semantic* semantic;
    friend class Semantic;
};

struct par_name_t {
    QString prefix;
    par_name_t( int indent = 0 ) {
        for( int i = 0; i < indent; ++i )
            prefix += " ";
    }
    QString operator() (ParType pt) const;
    /* {
        switch( pt.type() ) {
        case ParType::Boolean:
            return pt.access() == ParType::RValue ? prefix+"bool"   : prefix+"bool&";
        case ParType::Int:
            return pt.access() == ParType::RValue ? prefix+"int"    : prefix+"int&";
        case ParType::Double:
            return pt.access() == ParType::RValue ? prefix+"double" : prefix+"double&";
        case ParType::String:
            return pt.access() == ParType::RValue ? prefix+"string" : prefix+"string&";
        case ParType::Char:
            return pt.access() == ParType::RValue ? prefix+"char"   : prefix+"char&";
        case ParType::Void:
            return prefix+"void";
        default:
            return "Unknown";
        }
    }*/
};

#endif
/// Local Variables: ***
/// mode: C++ ***
/// c-basic-offset: 4 ***
/// End: ***
