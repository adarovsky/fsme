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
#include <QObject>

#include "semantic.h"
#include "type.h"

#include <functional>
#include <climits>
#include <iostream>
#include <sstream>
#include <map>
#include <iterator>

using namespace std;

#include <algorithm>

struct par_clean_cast_t : public binary_function<ParType, ParType, bool> {
    bool operator() ( ParType tfrom, ParType tto ) {
        return tfrom.cleanCast(tto);
    }
};

struct par_wider_cast_t : public binary_function<ParType, ParType, bool> {
    int operator() ( ParType tfrom, ParType tto ) {
        return tfrom.widerCast(tto);
    }
};

struct par_std_cast_t : public binary_function<ParType, ParType, bool> {
    bool operator() ( ParType tfrom, ParType tto ) {
        return tfrom.stdCast(tto);
    }
};

struct par_castable_t : public binary_function<ParType, ParType, bool> {
    bool operator() ( ParType tfrom, ParType tto ) {
        bool ret = tfrom.cleanCast(tto) || (tfrom.widerCast(tto) >= 0) || tfrom.stdCast(tto);
        //        cout << "*** castable(" << pname(tfrom) << ", " << pname(tto) << ") = " << ret << endl;
        return ret;
    }
};


// ========================= Func Definition ==========================
Semantic::FuncDef::FuncDef( ParType r, const QString& n, const ParList& p, size_t m )
    : ret_( r ), name_( n ), params_( p ), mandatory_( m )
{
}

Semantic::FuncDef::FuncDef( ParType r, const QString& n, ParType first )
    : ret_( r ), name_( n )
{
    params_.push_back( first );
    mandatory_ = 1;
}

Semantic::FuncDef::FuncDef( ParType r, const QString& n, ParType first, ParType second )
    : ret_( r ), name_( n )
{
    params_.push_back( first );
    params_.push_back( second );
    mandatory_ = 2;
}

ParList Semantic::FuncDef::allParams() const
{
    return params_;
}

ParList Semantic::FuncDef::mandatoryParams () const
{
    ParList pars;
    copy_n( params_.begin(), mandatory_, back_insert_iterator<ParList>(pars) );
    return pars;
}

bool Semantic::FuncDef::cleanCast( const FuncDef& d ) const
{
    if( d.name() != name() ) return false;
    ParList pars = d.allParams();
    if( pars.size() < mandatory_ || pars.size() > params_.size() ) return false;
    bool ret = equal( pars.begin(), pars.end(), params_.begin(), par_clean_cast_t() );

    return ret;
}

int  Semantic::FuncDef::widerCast( const FuncDef& d ) const
{
    //    cout << "*** Casting " << *this << " to " << d << endl;
    if( d.name() != name() ) return -1;
    ParList pars = d.allParams();
    if( pars.size() < mandatory_ || pars.size() > params_.size() ) {
//         cout << "***   pars.size() = " << pars.size() << ", mandatory_ = "
//              << mandatory_ << ", params_.size() = " << params_.size() << endl;
        return -1;
    }
    ParList::const_iterator i, j;
    par_wider_cast_t wcast;
    int  cost = 0;
    for( i = pars.begin(), j = params_.begin(); i != pars.end(); ++i, ++j ) {
        int c = wcast(*i, *j);
        //        cout << "***   wcast(" << par_name_t()(*i) << ", " << par_name_t()(*j) << ") = " << c << endl;
        if( c < 0 ) return -1;  // not castable
        cost += c;
    }

    return cost;
}

bool Semantic::FuncDef::stdCast( const FuncDef& d ) const
{
    if( d.name() != name() ) return false;
    ParList pars = d.allParams();
    if( pars.size() < mandatory_ || pars.size() > params_.size() )
        return false;

    bool ret = equal( pars.begin(), pars.end(), params_.begin(), par_castable_t() );
    return ret;
}

ostream& operator<<( ostream& os, const Semantic::FuncDef& d )
{
//    os << par_name_t()( d.returnType() ) << " " << d.name() << " (";
//    ParList l = d.allParams();
//    ParList::iterator i = l.end(); --i;
//    transform( l.begin(), i, ostream_iterator<QString>(os, ", "), par_name_t() );
//    if( l.empty() )
//        os << ")";
//    else
//        os << par_name_t() (*i) << ")";
    return os;
}

QString Semantic::FuncDef::toString() const
{
    ostringstream os;
    os << *this;
    return QString::fromLocal8Bit( os.str().c_str() );
}


// ========================= Func List ================================
void Semantic::FuncList::addFunc( const FuncDef& fd )
{
    for( iterator i = begin(); i != end(); ++i )
        if( i->cleanCast( fd ) )
            throw SemanticException( "Duplicate function definition" );
    push_back( fd );
}

ParType Semantic::FuncList::findCertain( const FuncDef& fd, bool* found ) const
{
    for( const_iterator i = begin(); i != end(); ++i ) {
        //        cout << "Comparing   " << *i << " and " << fd << endl;
        if( i->cleanCast( fd ) ) {
            if( found ) *found = true;
            return i->returnType();
        }
    }
    if( found ) *found = false;
    return ParType(ParType::Void, semantic);
}


ParType Semantic::FuncList::findCasting( const FuncDef& fd ) const
{
    bool ok;
    // ---------- exact match ----------
    ParType type = findCertain( fd, &ok );
    if( ok ) return type;

    // ---------- match by wider cast -----------
    const_iterator i;
    map<int, const FuncDef*> funcCosts;
    for( i = begin(); i != end(); ++i ) {
        int weight = i->widerCast( fd );
        if( weight < 0 ) continue;
        map<int, const FuncDef*>::const_iterator amb;
        if( (amb = funcCosts.find(weight)) != funcCosts.end() ) {
            const char* message = QT_TRANSLATE_NOOP( "SemanticException", "Ambiguity found between '%1' and '%2'" );
            throw SemanticException( QObject::tr( message ).arg(i->toString()).arg(amb->second->toString()) );
        }
        funcCosts.insert( make_pair(weight, &(*i)) );
    }
    if( !funcCosts.empty() )
        return funcCosts.begin()->second->returnType();


    // find with standard cast
    for( const_iterator i = begin(); i != end(); ++i ) {
        //        cout << "Comparing   " << *i << " and " << fd << endl;
        if( i->stdCast( fd ) )
            return i->returnType();

    }

    const char* message = QT_TRANSLATE_NOOP( "SemanticException", "Function '%1' not found" );
    throw SemanticException( QObject::tr(message).arg( fd.toString() ) );
}

bool Semantic::FuncList::funcExists( const FuncDef& fd ) const
{
    for( const_iterator i = begin(); i != end(); ++i ) {
        //        cout << "Comparing   " << *i << " and " << fd << endl;
        if( i->cleanCast( fd ) )
            return true;
    }
    return false;
}

// bool find_cast_t::operator() (ParType from, ParType to) const
// {
//     funcList.
// }
