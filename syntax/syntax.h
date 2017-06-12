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

#ifndef __SYNTAX__
#define __SYNTAX__
#include "scanner.h"
#include "semantic.h"
#include <memory>

class LexException : public Exception {
public:
    LexException( const QString& m, Lex l );
    LexException& operator=(const Exception& e ) { *this = LexException(e.message(), Lex()); return *this;}
};

class Syntax {
    Lex debugLex;
    Lex lexBackup;
    Semantic          semantic;
    auto_ptr<Scanner> f;

    bool match( LGroup g );
    bool match( LType  t );
    bool match( LSubType t );

    bool parseAssign();
    bool parseConditional();
    bool parseExpr();
    bool parseLogicOr();
    bool parseLogicAnd();
    bool parseOr();
    bool parseXor();
    bool parseAnd();
    bool parseEqual();
    bool parseCompare();
    bool parseShift();
    bool parsePlus();
    bool parseMul();
    bool parseToken();
    bool parseUnary();
    bool parseCast();
    //    bool parsePM();
    bool parsePostfix();

    bool parseInstruction();
    bool parseMarked();
    bool parseInstructExpr();
    bool parseCombined();
    bool parseSelect();
    bool parseIter();
    bool parseTransition();
    bool parseDefinition();
    bool parseDefVar();
    bool parseDefFunc();

    ParType lexToRValue( LexType t ) const;
    ParType lexToLValue( LexType t ) const;

    LexException error;
public:
    Syntax( Scanner* s );
    void    check();
    QString curSource() const;
    int    curPos()   const;
    void   registerVariable ( QString name, ParType type );
};

#endif
/// Local Variables: ***
/// mode: C++ ***
/// c-basic-offset: 4 ***
/// End: ***
