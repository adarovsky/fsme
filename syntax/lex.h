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
// -*- C++ -*-
#ifndef __LEX_DEFINITIONS
#define __LEX_DEFINITIONS
#include <qstring.h>

enum LGroup   { GUnknown,
                Keyword,
                Type,
                Constant,
                Indent,
                Unary,
                Compare,
                Equality,
                Assign,
                MulDiv,
                PlsMinus,
                Bitwise,
                Brackets,
                Terminal,
                Shift,
                Conjunction, 
                Disjunction,
                Eof
};
enum LType    { TUnknown,
                Double,
                Int,
		Boolean,
                String,
                Char,
                Void
};

enum LSubType { SUnknown,
                lkLess,
                lkMore,
                lkLessEq,
                lkMoreEq,
                lkLeftShiftAssign,
                lkRightShiftAssign,
                lkLeftShift,
                lkRightShift,
                lkExclamation,
                lkModule,
                lkModuleAssign,
                lkCap,
                lkCapAssign,
                lkAmp,
                lkAmpAssign,
                lkMul,
                lkMulAssign,
                lkLbracket,
                lkRbracket,
                lkPlus,
                lkPlusAssign,
                lkMinus,
                lkMinusAssign,
                lkAssign,
                lkLCbracket,
                lkRCbracket,
                lkStroke,
                lkStrokeAssign,
                lkTilde,
                lkTildeAssign,
                lkLSbracket,
                lkRSbracket,
                lkBackslash,
                lkSemicolon,
                lkColon,
                lkQuestion,
                lkComma,
                lkPoint,
                lkEqual,
                lkNotEqual,
                lkSlash,
                lkSlashAssign,
                lkIncrement,
                lkDecrement,

                lkContinue,
                lkDefault,
                lkFor,
                lkBreak,
                lkCase,
                lkDo,
                lkDouble,
                lkIf,
                lkSwitch,
                lkElse,
                lkVoid,
                lkInt,
                lkReturn,
                lkWhile
};

struct LexType {
    LGroup   group;
    LType    type;
    LSubType subtype;
    LexType( LGroup g = GUnknown, LType t = TUnknown, LSubType s = SUnknown )
        : group( g ), type( t ), subtype( s ) {}
};

class Lex {
    LexType type;
    QString  text;
    QString  source;
    int     position;
public:
    Lex() : position(0) { type.group = GUnknown; type.type = TUnknown; type.subtype = SUnknown; }
    Lex( LexType t, QString txt, QString src, int pos ) : type(t), text(txt), source(src), position(pos) {}
    bool match( LGroup g )    { return type.group == g;    }
    bool match( LType  t )    { return type.type  == t;    }
    bool match( LSubType st ) { return type.subtype == st; }
    QString lexText() const    { return text;   }
    int lexPos() const        { return position;   }
    QString lexSource() const  { return source; }
    LexType lexType() const   { return type;   }
};
#endif
