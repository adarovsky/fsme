//====================================================================
// Filename:      semantic.h
// 
//                
// Description:   A semantic implementation
// Author:        Alexander Darovsky <adarovsky@sourceforge.net>
//                
// Created at:    Tue Nov 13 11:08:42 2001
// Modified at:   Tue Feb 24 22:32:23 2004
// Modified by:   Alexander Darovsky <adarovsky@sourceforge.net>
//                
// Version:       
// Update count:  35
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
//====================================================================
#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__
#include <qstring.h>
#include <list>
//#include <vector>
#include <set>
#include <functional>
#include "type.h"
#include "exception.h"


typedef std::list<ParType> ParList;


enum BlockType { BlockFor,
                 BlockWhile,
                 BlockSwitch,
                 BlockIf,
                 BlockElse,
                 BlockCombined,
                 BlockFunc };

/// An exception special for semantic errors
class SemanticException : public Exception {
public:
    SemanticException( const QString& message ) : Exception( message, "", 0 )  { setPlain( message ); }
};

/// Just a variable. Only type and name are significant for us
struct Var {
    QString name;
    ParType type;
    Var(const Semantic* sem) : type(ParType::Void, sem ) {}
    Var(const QString& n, const Semantic* sem) : name(n), type(ParType::Void, sem) {}
    Var( QString n, ParType t) : name(n), type(t) {}
    bool operator==( const Var& v ) const { return v.name == name; }
    bool operator< ( const Var& v ) const { return v.name < name; }
    bool isValid() const { return type.type() != ParType::Void; }
};
typedef std::list<Var> VarList;

class Semantic {
    // ========================= Block ==========================
    // Block is a set of variables, which can answer the following questions:
    // - is there a variable with given name          - isVarExists
    // - give me the type of the var with given name  - varType
    // - add a variable with given name and type,
    //   and return true, if it's possible,
    //   false otherwise                              - addVar
    class Block {
        BlockType type_;
        ParType   ret_type; // for func block
        const Semantic* semantic;
    public:
        Block( BlockType t, const Semantic* sem, ParType rt = ParType( ParType::Void, 0 ) ) : type_( t ), ret_type( rt ), semantic(sem) {}
        BlockType type() const { return type_; }
    private:
        std::set<Var> varList;
    public:
        bool    isVarExists( QString name ) const;
        ParType varType    ( QString name ) const;
        bool    addVar     ( QString name, ParType type );
        ParType returnType() const { return ret_type; }
    };

    struct type_equal_t : public std::unary_function<Block, bool> {
        BlockType type;
        type_equal_t( BlockType t ) : type( t ) {}
        bool operator() ( const Block& b ) const { return b.type() == type; }
    };

    // ========================= Block List ==========================
    // Block list manages stack of blocks, and can answer the following questions:
    class BlockList : private std::list<Block> {
        const Semantic* semantic;
    public:
        BlockList( const Semantic* sem ) : semantic( sem ) {}
        void addBlock( BlockType type, ParType ret );
        void delBlock();
        bool addVar( QString name, ParType type );
        //        bool delVar( QString name );
        bool isVarConflicting( QString name ) const; // if there's a variable with this name in the current block
        ParType varType( QString name ) const; // get type of the first var in all blocks, void if not found

        bool canBreak()  const;       // is there a block of type for, while or switch
        bool canReturn( ParType type ) const;       // is there a block of type func
        bool canContinue() const;     // is there a block of type for or while
    } blockList;


public:
    // ========================= Func Definition ==========================
    class FuncDef {
        ParType ret_;            // return type
        QString  name_;
        ParList params_;
        size_t  mandatory_;      // number of mandatory parameters
    public:
        FuncDef( ParType r, const QString& n, const ParList& p, size_t m );
        FuncDef( ParType r, const QString& n, ParType first );
        FuncDef( ParType r, const QString& n, ParType first, ParType second );

        QString  name()       const { return name_; }
        size_t  mandatory()  const { return mandatory_; }
        ParType returnType() const { return ret_; }
        ParList allParams()  const;
        ParList mandatoryParams () const;
        bool    cleanCast  ( const FuncDef& d ) const;
        int     widerCast  ( const FuncDef& d ) const;
        bool    stdCast    ( const FuncDef& d ) const;

        QString toString() const;
    };




    // ========================= Func List ================================
    class FuncList : private std::list<FuncDef> {
        const Semantic* semantic;
    public:
        FuncList( const Semantic* sem) : semantic(sem){}
        FuncList( FuncDef* start, FuncDef* end ) : std::list<FuncDef>( start, end ) {}
        void addFunc( const FuncDef& fd );        // throw an exception if there's a conflict
        ParType findCertain( const FuncDef& fd, bool* found = 0 ) const; // find a func with matching params without any casting
        ParType findCasting( const FuncDef& fd ) const; // find a func with cast enabled
                                                  // note that these two functions throw exception if
                                                  // any ambiguity definitions found
        bool    funcExists( const FuncDef& fd ) const;
    };


private:
    // Stack of a program, executing with type sensivity
    ParList  typeStack;

    // List of known functions
    FuncList funcList;

    // Fill builtin functions
    void     initFunctions();

    // debugging
    void     dumpStack() const;
public:
    Semantic();
    void enterBlock( BlockType bt, ParType ret = ParType(ParType::Void, 0) ); // {
    void leaveBlock( BlockType bt );                        // }

    void breakFound();                                      // break;
    void continueFound();                                   // continue;
    void returnFound();                                     // return;
    void returnValueFound();                                // return a;
    void operatorFinished();                                // ;

    void gotVarDefinition ( QString name, ParType type );                  // int a
    void gotVarUsage      ( QString name /*ParType type*/ );               // a = 10
    void gotConstant      ( QString text, ParType type );                  // 10
    void gotFuncPrototype ( QString name, ParType ret, const VarList& l ); // void func();
    void gotFuncDefinition( QString name, ParType ret, const VarList& l ); // void func(){}
    void gotFuncUsage     ( QString name, int npars );                     // func();
    void gotUnaryFuncUsage( QString name );                                // func( a );
    void gotBinaryFuncUsage( QString name );                               // func( a,b );

    void gotConditional();

    void cast( ParType partype );

public:
    bool funcExists( const FuncDef& def ) const { return funcList.funcExists( def ); }
};

std::ostream& operator<<( std::ostream& os, const Semantic::FuncDef& d );

#endif
/// Local Variables: ***
/// mode: C++ ***
/// c-basic-offset: 4 ***
/// End: ***

