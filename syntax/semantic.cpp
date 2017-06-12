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

#include "semantic.h"
#include <iostream>
#include <algorithm>
#include <iterator>

#include <functional>
using namespace std;

template <class _Operation1, class _Operation2, class _Operation3>
class binary_compose
  : public unary_function<typename _Operation2::argument_type,
                          typename _Operation1::result_type> {
protected:
  _Operation1 _M_fn1;
  _Operation2 _M_fn2;
  _Operation3 _M_fn3;
public:
  binary_compose(const _Operation1& __x, const _Operation2& __y,
                 const _Operation3& __z)
    : _M_fn1(__x), _M_fn2(__y), _M_fn3(__z) { }
  typename _Operation1::result_type
  operator()(const typename _Operation2::argument_type& __x) const {
    return _M_fn1(_M_fn2(__x), _M_fn3(__x));
  }
};
template <class _Operation1, class _Operation2, class _Operation3>
inline binary_compose<_Operation1, _Operation2, _Operation3>
compose2(const _Operation1& __fn1, const _Operation2& __fn2,
         const _Operation3& __fn3)
{
  return binary_compose<_Operation1,_Operation2,_Operation3>
    (__fn1, __fn2, __fn3);
}


using namespace std;

struct block_name_t {
    QString operator() (const BlockType& bt) const {
        switch( bt ) {
        case BlockCombined:
            return "Combined";
        case BlockFor:
            return "For";
        case BlockWhile:
            return "While";
        case BlockSwitch:
            return "Switch";
        case BlockIf:
            return "If";
        case BlockFunc:
            return "Func";
        default:
            return "Unknown";
        }
    }
};

struct par_castable_t : public binary_function<ParType, ParType, bool> {
    bool operator() ( ParType tfrom, ParType tto ) {
        bool ret = tfrom.cleanCast(tto) || (tfrom.widerCast(tto) >= 0) || tfrom.stdCast(tto);
        //        cout << "*** castable(" << pname(tfrom) << ", " << pname(tto) << ") = " << ret << endl;
        return ret;
    }
};

//======================== Block ============================
bool    Semantic::Block::isVarExists( QString name ) const
{
    return varList.find( Var(name, 0) ) != varList.end();
}

ParType Semantic::Block::varType    ( QString name ) const
{
    set<Var>::iterator i = varList.find( Var(name, semantic) );
    if( i != varList.end() )
        return i->type;
    else
        return ParType(ParType::Void, semantic);
}

bool    Semantic::Block::addVar     ( QString name, ParType type )
{
    if( !isVarExists(name) ) {
        varList.insert( Var(name, type) );
        return true;
    }
    return false;
}

//====================== BlockList ==========================



void Semantic::BlockList::addBlock( BlockType type, ParType ret )
{
    push_back( Block(type, semantic, ret) );
}

void Semantic::BlockList::delBlock()
{
    pop_back();
}

bool Semantic::BlockList::isVarConflicting( QString name ) const
{
    const_reverse_iterator i = rbegin();
    if( i->isVarExists(name) ) return true;
    if( ++i != rend() )
        return i->type() == BlockFunc && i->isVarExists( name );
    return false;
}

bool Semantic::BlockList::addVar( QString name, ParType type )
{
    return !isVarConflicting(name) && back().addVar( name, type );
}

ParType Semantic::BlockList::varType( QString name ) const
{
    const_reverse_iterator i = rbegin();
    for( ; i != rend(); ++i ) {
        ParType pt( ParType::Void, semantic);
        if( (pt = i->varType(name)), pt.type() != ParType::Void )
            return pt;
    }
    return ParType ( ParType::Void, semantic);               // there's no such a variable
}

bool Semantic::BlockList::canBreak() const
{
    return find_if( rbegin(), rend(), compose2( logical_or<bool>(), type_equal_t(BlockFor),
                                                compose2( logical_or<bool>(),
                                                          type_equal_t(BlockWhile),
                                                          type_equal_t(BlockSwitch) ) ) ) != rend();
}

bool Semantic::BlockList::canContinue() const
{
    return find_if( rbegin(), rend(), compose2( logical_or<bool>(),
                                                type_equal_t(BlockFor),
                                                type_equal_t(BlockWhile) ) ) != rend();
}

bool Semantic::BlockList::canReturn( ParType type ) const
{
    const_reverse_iterator i = find_if( rbegin(), rend(), type_equal_t(BlockFunc) );
    if( i == rend() ) return false;

    if( type.cleanCast(i->returnType()) || (type.widerCast(i->returnType()) > 0 ) || type.stdCast(i->returnType()) )
        return true;
    return false;
}

//====================== Semantic ===========================
Semantic::Semantic()
    : blockList( this ), funcList( this )
{
    initFunctions();
    blockList.addBlock( BlockFunc, ParType(ParType::Void, this) ); // global
    
}

void Semantic::enterBlock( BlockType bt, ParType pt )
{
//    cout << "Enter block " << block_name_t() ( bt ) << ", type = " << par_name_t()(pt) << endl;
    blockList.addBlock( bt, pt);
}

void Semantic::leaveBlock( BlockType bt )
{
//    cout << "Leave block " << block_name_t() ( bt ) << endl;
    blockList.delBlock();
}

void Semantic::gotVarDefinition ( QString name, ParType type )
{
//    cout << "Variable: " << par_name_t() (type) << " " << name << endl;
    if( !type.semantic )
        type.semantic = this;
    if( !blockList.addVar(name, type) )
        throw SemanticException( "Variable creation is not allowed: " + name );
}

void Semantic::gotVarUsage ( QString name )
{
//    cout << "Using variable: " << name << endl;
    ParType pt = blockList.varType(name);
    if( pt.type() == ParType::Void )           // none found
        throw SemanticException( "Unknown variable " + name );

    typeStack.push_back( pt );
    dumpStack();
}


void Semantic::gotConstant ( QString text, ParType type )
{
//    cout << "Using constant " << par_name_t() (type) << " " << text << endl;
    typeStack.push_back( type );
    dumpStack();
}

void Semantic::gotFuncDefinition( QString name, ParType ret, const VarList& parlist )
{
    ParList pl;
    for( VarList::const_iterator i = parlist.begin(); i != parlist.end(); ++i )
        pl.push_back( i->type );
    funcList.addFunc( FuncDef(ret, name, pl, pl.size()) ); // stub

    enterBlock( BlockFunc, ret );

    for( VarList::const_iterator i = parlist.begin(); i != parlist.end(); ++i ) {
        if( i->name != QString() )
            blockList.addVar( i->name, i->type );
    }
    cout << FuncDef(ret, name, pl, pl.size()) << endl;
}

void Semantic::gotFuncPrototype( QString , ParType , const VarList& parlist )
{
    //    cout << "Function prototype \"" << par_name_t() (ret) << " " << name << " ( ";
//    for( VarList::const_iterator i = parlist.begin(); i != parlist.end(); ++i ) {
//        cout << par_name_t()( i->type ) << " " << i->name << " ";
//        blockList.back().addVar( i->name, i->type );
//    }
    //    cout << ")\"" << endl;
}

void Semantic::gotFuncUsage     ( QString name, int npars )
{
    ParList l;
    for( int i = 0; i < npars; ++i ) {
        l.push_front( typeStack.back() );
        typeStack.pop_back();
    }
    FuncDef fd( ParType(ParType::Void, this), name, l, npars );
//    cout << "Using function \"" << fd << endl;
    typeStack.push_back( funcList.findCasting(fd) ); // stub
    dumpStack();
}

void Semantic::gotUnaryFuncUsage( QString name )
{
    gotFuncUsage( name, 1 );
}

void Semantic::gotBinaryFuncUsage( QString name )
{
    gotFuncUsage( name, 2 );
}

void Semantic::breakFound()
{
//    cout << "Break" << endl;
    if( !blockList.canBreak() )
        throw SemanticException( "Misplaced break" );
}

void Semantic::continueFound()
{
//    cout << "Continue" << endl;
    if( !blockList.canContinue() )
        throw SemanticException( "Misplaced continue" );

}

void Semantic::returnFound()
{
//    cout << "return" << endl;
    if( !blockList.canReturn(ParType(ParType::Void, this)) )
        throw SemanticException( "Misplaced returning void" );
    dumpStack();
}

void Semantic::returnValueFound()
{
//    cout << "return" << endl;
    ParType partype = typeStack.back();
    typeStack.pop_back();
    if( !blockList.canReturn(partype) )
        throw SemanticException( "Misplaced return or invalid return type" );
    dumpStack();
}

void Semantic::operatorFinished()
{
//    cout << "operator finished" << endl;
    typeStack.pop_back();
    dumpStack();
}

// ParType Semantic::topType( int ) const
// {
//     return RVoid;
// }

void Semantic::cast( ParType partype )
{
//    cout << "Trying to cast " << par_name_t()( typeStack.back() ) << " to " << par_name_t()( partype ) << endl;
    ParType type = typeStack.back();
    typeStack.pop_back();
    if( type.cleanCast(partype) || (type.widerCast(partype) > 0 ) || type.stdCast(partype) )
        typeStack.push_back( partype );
    else {
        par_name_t pname;
        throw SemanticException( "Cannot cast from "+pname(type)+" to "+pname(partype) );
    }
    dumpStack();
}

void Semantic::gotConditional()
{
    ParType second = typeStack.back();
    typeStack.pop_back();

    cast( second );                      // cast first argument to second
    typeStack.pop_back();
    
    cast( ParType(ParType::Boolean, this) ); // cast condition to int
    typeStack.pop_back();

    typeStack.push_back( second );
    dumpStack();
}

void Semantic::dumpStack() const
{
//     cout << "   Type Stack: " << endl;
//     transform( typeStack.begin(), typeStack.end(), ostream_iterator<string>(cout, "\n"), par_name_t(6) );
}
