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
#include "scanner.h"
#include <string>
using namespace std;
typedef pair<const QString, LexType> LexPair; 
LexPair static_lexes[] = {
    LexPair( "<",   LexType(Compare,  TUnknown, lkLess)      ),
    LexPair( ">",   LexType(Compare,  TUnknown, lkMore)      ),
    LexPair( "<=",  LexType(Compare,  TUnknown, lkLessEq)    ),
    LexPair( ">=",  LexType(Compare,  TUnknown, lkMoreEq)    ),
    LexPair( "<<",  LexType(Shift  ,  TUnknown, lkLeftShift) ),
    LexPair( ">>",  LexType(Shift  ,  TUnknown, lkRightShift)),
    LexPair( "<<=", LexType(Assign,   TUnknown, lkLeftShiftAssign) ),
    LexPair( ">>=", LexType(Assign,   TUnknown, lkRightShiftAssign) ),
    LexPair( "!",   LexType(Unary,    TUnknown, lkExclamation)),
    LexPair( "!=",  LexType(Equality, TUnknown, lkNotEqual)  ),
    LexPair( "==",  LexType(Equality, TUnknown, lkEqual)     ),
    LexPair( "%",   LexType(MulDiv,   TUnknown, lkModule)    ),
    LexPair( "%=",  LexType(Assign,   TUnknown, lkModuleAssign)   ),
    LexPair( "^",   LexType(Bitwise,  TUnknown, lkCap)       ), 
    LexPair( "^=",  LexType(Assign,   TUnknown, lkCapAssign) ), 
    LexPair( "&",   LexType(Bitwise,  TUnknown, lkAmp)       ),
    LexPair( "&&",  LexType(Disjunction,  TUnknown, SUnknown)),
    LexPair( "&=",  LexType(Assign,   TUnknown, lkAmpAssign) ),
    LexPair( "*",   LexType(MulDiv,   TUnknown, lkMul)       ),
    LexPair( "*=",  LexType(Assign,   TUnknown, lkMulAssign) ),
    LexPair( "(",   LexType(Brackets, TUnknown, lkLbracket)  ),
    LexPair( ")",   LexType(Brackets, TUnknown, lkRbracket)  ),
    LexPair( "+",   LexType(PlsMinus, TUnknown, lkPlus)      ),
    LexPair( "+=",  LexType(Assign,   TUnknown, lkPlusAssign)),
    LexPair( "-",   LexType(PlsMinus, TUnknown, lkMinus)     ),
    LexPair( "-=",  LexType(Assign,   TUnknown, lkMinusAssign) ),
    LexPair( "=",   LexType(Assign,   TUnknown, lkAssign)    ),
    LexPair( "{",   LexType(Brackets, TUnknown, lkLCbracket) ),
    LexPair( "}",   LexType(Brackets, TUnknown, lkRCbracket) ),
    LexPair( "|",   LexType(Bitwise,  TUnknown, lkStroke)    ),
    LexPair( "||",  LexType(Conjunction,  TUnknown, SUnknown)),
    LexPair( "|=",  LexType(Bitwise,  TUnknown, lkStrokeAssign)),
    LexPair( "~",   LexType(Bitwise,  TUnknown, lkTilde)     ),
    LexPair( "~=",  LexType(Assign,   TUnknown, lkTildeAssign) ),
    LexPair( "[",   LexType(Brackets, TUnknown, lkLSbracket) ),
    LexPair( "]",   LexType(Brackets, TUnknown, lkRSbracket) ),
    LexPair( "\\",  LexType(GUnknown, TUnknown, lkBackslash) ), 
    LexPair( ";",   LexType(Terminal, TUnknown, lkSemicolon) ),
    LexPair( ":",   LexType(Terminal, TUnknown, lkColon)     ),
    LexPair( "?",   LexType(Terminal, TUnknown, lkQuestion)  ), 
    LexPair( ",",   LexType(Terminal, TUnknown, lkComma)     ), 
    LexPair( ".",   LexType(Terminal, TUnknown, lkPoint)     ), 
    LexPair( "/",   LexType(MulDiv,   TUnknown, lkSlash)     ),
    LexPair( "/=",  LexType(Assign,   TUnknown, lkSlashAssign) ),
    LexPair( "++",  LexType(Unary,    TUnknown, lkIncrement) ),
    LexPair( "--",  LexType(Unary,    TUnknown, lkDecrement) ),
    LexPair("continue", LexType(Keyword, TUnknown, lkContinue)),
    LexPair("default",  LexType(Keyword, TUnknown, lkDefault)),
    LexPair("for",      LexType(Keyword, TUnknown, lkFor)    ),
    LexPair("break",    LexType(Keyword, TUnknown, lkBreak)  ),
    LexPair("case",     LexType(Keyword, TUnknown, lkCase)   ),
    LexPair("do",       LexType(Keyword, TUnknown, lkDo)     ),

    LexPair("if",       LexType(Keyword, TUnknown, lkIf)     ),
    LexPair("switch",   LexType(Keyword, TUnknown, lkSwitch) ),
    LexPair("else",     LexType(Keyword, TUnknown, lkElse)   ),
    LexPair("return",   LexType(Keyword, TUnknown, lkReturn) ),
    LexPair("while",    LexType(Keyword, TUnknown, lkWhile)  ),

    LexPair("void",     LexType(Type,    Void,     lkVoid)   ),
    LexPair("int",      LexType(Type,    Int,      lkInt)    ),
    LexPair("double",   LexType(Type,    Double,   lkDouble) ),

};
const map<QString, LexType>
Scanner::keywords( static_lexes, static_lexes + sizeof(static_lexes)/sizeof(static_lexes[0]) );
