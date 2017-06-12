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

#include "type.h"
#include "semantic.h"

QString par_name_t::operator() (ParType pt) const {
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
}



ParType::ParType( Builtin t, const Semantic* s, Access a )
    : type_( t ), access_( a ), semantic( s )
{
}

bool ParType::cleanCast( const ParType& t ) const
{
    if( type_ == Void || t.type_ == Void ) return false;
    return access_ == LValue ? type_ == t.type_ : (type_ == t.type_) && (access_ == t.access_);
}


int  ParType::widerCast( const ParType& t ) const 
{
    if( t.access_ == LValue )
        return type_ == t.type_ ? 0 : -1;
    switch( type_ ) {
    case Void:
        return -1;
    case Char:
        switch( t.type_ ) {
        case Char:
            return 0;
        case Int:
        case Boolean:
            return 1;
        case Double:
            return 2;
        default:
            return -1;
        }
    case Int:
        switch( t.type_ ) {
        case Int:
        case Boolean:
            return 0;
        case Double:
            return 1;
        default:
            return -1;
        }
        
    case Double:
        switch( t.type_ ) {
        case Double:
        case Boolean:
            return 0;
        default:
            return -1;
        }
    // custom types require manual casting
    default:
        return -1;
    }    
}
                                             
bool ParType::stdCast  ( const ParType& t ) const
{
    if( access_ == RValue && t.access_ == LValue )
        return false;
    par_name_t pname;
    Semantic::FuncDef fd( ParType(Void, semantic), "cast@"+pname(*this)+"@"+pname(t), *this );
    //    cout << "semantic->funcExists(" << fd << ") = " << semantic->funcExists( fd ) << endl;
    return semantic->funcExists( fd );
}
 
