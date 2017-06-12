//====================================================================
//
//    Copyright (C) 2001, 2004, Alexander Darovsky
//
// Filename:      syntax.cpp
// Description:
// Author:        Alexander Darovsky <adarovsky@users.sf.net>
// Created at:    Sat Nov 10 20:56:14 2001
// Modified at:   Wed Feb 25 16:36:17 2004
//
// Version:
// Update count:  53
// Status:        Experimental, do not distribute.
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
//
//====================================================================

#include "syntax.h"
#include <iomanip>
#include <qstring.h>
#include <qapplication.h>

#define SUCCESS_LOG(s) { return true; }

#define TR(what) (qApp->translate( "Exception", what ))

;
LexException::LexException( const QString& m, Lex l )
    : Exception( TR(QT_TRANSLATE_NOOP( "Exception", "%1 while reading token \"%2" )).arg(m).arg(l.lexText()), l.lexSource(), l.lexPos() )
{}

;
Syntax::Syntax( Scanner* s )
    : f( s ), error( TR(QT_TRANSLATE_NOOP( "Exception", "parse error" )), Lex() )
{
    debugLex = f->curLex();
}

QString Syntax::curSource() const
{
    return f->curLex().lexSource();
}

int    Syntax::curPos()   const
{
    return f->curLex().lexPos();
}


bool Syntax::match( LGroup g )
{
    debugLex = f->curLex();
    if( f->curLex().match( g ) ) {
        //        cerr << "** got " << f->curLex().lexText() << endl;
        lexBackup = f->curLex();
        f->popLex();
        //        return true;
        return true;
    }
    else return false;
}

bool Syntax::match( LType  t )
{
    debugLex = f->curLex();
    if( f->curLex().match( t ) ) {
        //        cerr << "** got " << f->curLex().lexText() << endl;
        lexBackup = f->curLex();
        f->popLex();
        return true;
    }
    else return false;
}

bool Syntax::match( LSubType t )
{
    debugLex = f->curLex();
    if( f->curLex().match( t ) ) {
        //        cerr << "** got " << f->curLex().lexText() << endl;
        lexBackup = f->curLex();
        f->popLex();
        return true;
    }
    else return false;
}

bool Syntax::parseExpr()
{
    if( parseAssign() ) {
        while( match(lkComma) ) {
            semantic.operatorFinished();
            if( !parseAssign() ) return false;
        }
        return true;
    }
    return false;
}

bool Syntax::parseAssign()
{
    if( parseConditional() ) {
        if( match(Assign) ) {
            QString name = lexBackup.lexText();
            if( !parseAssign() )
                return false;
            else {
                semantic.gotBinaryFuncUsage( "@"+name+"@" );
                return true;
            }
        }
        else return true;
    }
    return false;
}

bool Syntax::parseConditional()
{
    if( parseLogicOr() ) {
        if( match(lkQuestion) )
            if( parseExpr() && match(lkColon) && parseAssign() ) {
                semantic.gotConditional();
                return true;
            }
            else
                return false;
        else return true;
    }
    return false;
}

bool Syntax::parseLogicOr()
{
    if( !parseLogicAnd() ) {
        return false;
    }
    while( match(Conjunction) ) {
        QString name = lexBackup.lexText();
        if( !parseLogicAnd() ) {
            throw LexException( TR(QT_TRANSLATE_NOOP( "Exception", "Valid 'logical and' expression expected" )), f->curLex() );
        }
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseLogicAnd()
{
    if( !parseOr() ) {
        return false;
    }
    while( match(Disjunction) ) {
        QString name = lexBackup.lexText();
        if( !parseOr() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid 'or' expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseOr()
{
    if( !parseXor() ) {
        return false;
    }
    while( match(lkStroke) ) {
        QString name = lexBackup.lexText();
        if( !parseXor() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid 'xor' expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseXor()
{
    if( !parseAnd() ) {
        return false;
    }
    while( match(lkCap) ) {
        QString name = lexBackup.lexText();
        if( !parseAnd() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid 'and' expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseAnd()
{
    if( !parseEqual() ) {
        return false;
    }
    while( match(lkAmp) ) {
        QString name = lexBackup.lexText();
        if( !parseEqual() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid equality expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseEqual()
{
    if( !parseCompare() ) {
        return false;
    }
    while( match(Equality) ) {
        QString name = lexBackup.lexText();
        if( !parseCompare() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid shift expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseCompare()
{
    if( !parseShift() ) {
        return false;
    }
    while( match(Compare) ) {
        QString name = lexBackup.lexText();
        if( !parseShift() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid shift expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseShift()
{
    if( !parsePlus() ) {
        return false;
    }
    while( match( Shift ) ) {
        QString name = lexBackup.lexText();
        if( !parsePlus() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid summation expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}


bool Syntax::parsePlus()
{
    if( !parseMul() ) {
        return false;
    }
    while( match( PlsMinus ) ) {
        QString name = lexBackup.lexText();
        if( !parseMul() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid multiplication expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseMul()
{
    if( !parseCast() ) {
        return false;
    }
    while( match( MulDiv ) ) {
        QString name = lexBackup.lexText();
        if( !parseCast() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid cast expression expected")), lexBackup );
        else
            semantic.gotBinaryFuncUsage( "@"+name+"@" );
    }
    return true;
}

bool Syntax::parseCast()
{
    if( match(lkLbracket) ) {
        if( !match(Type) )  {
            f->pushLex( lexBackup );
            if( parseUnary() )
                return true;
            else
                return false;
        }
        else {
            ParType partype = lexToRValue( lexBackup.lexType() );
            if( !match(lkRbracket) )
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "')' expected")), lexBackup );
            if( parseExpr() )
                semantic.cast( partype );
            else
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Valid expression expected")), lexBackup );
        }
        return true;
    }
    if( parseUnary() )
        return true;
    else
        return false;
}

bool Syntax::parseUnary()
{
    if( parsePostfix() )  return true;

    if( (match(lkExclamation) || match(lkTilde) || match(PlsMinus) || match(Unary)) ) {
        QString name = lexBackup.lexText();
        if( parseCast() ) {
            semantic.gotUnaryFuncUsage( name+"@" );
            return true;
        }
        else
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Cast expression required")), lexBackup );
    }
    //    error =  LexException( "error while reading unary expression", f->curLex() ) ;
    return false;
}

bool Syntax::parsePostfix()
{
    if( match(Indent) ) {
        QString name = lexBackup.lexText();
        if( match(lkLbracket) ) { // function call

            if( match(lkRbracket) ) {
                semantic.gotFuncUsage( name, 0 );
                return true;
            }
            if( !parseAssign() ) {
                throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "Invalid function call before %1")), curSource(), curPos() ) ;
            }
            int npars = 1;
            while( match(lkComma) ) {
                if( !parseAssign() ) {
                    throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "Invalid function call before ")), lexBackup.lexSource(), curPos() ) ;
                }
                npars++;
            }
            if( match(lkRbracket) ) {
                semantic.gotFuncUsage( name, npars );
                return true;
            }
            error = Exception( TR( QT_TRANSLATE_NOOP( "Exception", "Invalid function call before %1")), curSource(), curPos() ) ;
            return false;
        }
        else {
            semantic.gotVarUsage( name );
            while( match(Unary) ) {
                semantic.gotFuncUsage( "@"+lexBackup.lexText(), 1 );
            }
        }
        return true;
    } else if( parseToken() ) {
        while( match(Unary) ) {
            semantic.gotFuncUsage( "@"+lexBackup.lexText(), 1 );
        }
        return true;
    }
    return false;
}

// bool Syntax::parsePM()          // to remove
// {
//     if( match(lkLbracket) )
//         return parsePM() && match(lkRbracket);
//     return match(Indent);
// }


bool Syntax::parseToken()
{
    if( match(Indent) ) {
        semantic.gotVarUsage( lexBackup.lexText() );
        return true;
    }
    if( match(Constant) ) {
        semantic.gotConstant( lexBackup.lexText(), lexToRValue(lexBackup.lexType()) );
        return true;
    }
    if( match(lkLbracket) && parseExpr() && match(lkRbracket) ) return true;
    if( match(Eof) )
        throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "unexpected end of file")), curSource(), curPos() ) ;
    else {
        error = Exception( TR( QT_TRANSLATE_NOOP( "Exception", "parse error before ")), curSource(), curPos() ) ;
        return false;
    }
}

bool Syntax::parseInstruction()
{
    return parseMarked() ||
        parseInstructExpr() ||
        parseCombined() ||
        parseSelect() ||
        parseIter() ||
        parseTransition() ||
        parseDefinition();
}


bool Syntax::parseMarked()
{
    if( match(lkCase) && parseConditional() && match(lkColon) && parseInstruction() ) return true;
    if( match(lkDefault) && match(lkColon) && parseInstruction() ) return true;
    return false;
}

bool Syntax::parseInstructExpr()
{
    if( match(lkSemicolon) )
        return true;
    if( (parseExpr() && match(lkSemicolon)) ) {
        semantic.operatorFinished();
        return true;
    }
    return false;
}

bool Syntax::parseCombined()
{
    if( match(lkLCbracket) ) {
        semantic.enterBlock( BlockCombined );
        if( match(lkRCbracket) ) {
            semantic.leaveBlock( BlockCombined );
            return true;
        }
    if( !parseInstruction() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected")), lexBackup );
        while( parseInstruction() );
        if( !match(lkRCbracket) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "'}' expected")), lexBackup );
        semantic.leaveBlock( BlockCombined );
        return true;
    }
    return false;
}

bool Syntax::parseSelect()
{
    if( match(lkIf) ) {
        semantic.enterBlock( BlockIf );
        if( !match( lkLbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "'(' expected")), lexBackup );
        if( !parseExpr() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Expression expected")), lexBackup );
    else {
            semantic.cast( ParType(ParType::Boolean, &semantic) );
            semantic.operatorFinished();
    }
        if( !match( lkRbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "')' expected")), lexBackup );
        if( !parseInstruction() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected")), lexBackup );
        if( match(lkElse) ) {
            semantic.leaveBlock( BlockIf );
            semantic.enterBlock( BlockElse );
            if( !parseInstruction() )
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected" )), lexBackup );
            semantic.leaveBlock( BlockElse );
        }
        else
            semantic.leaveBlock( BlockIf );
        return true;
    }
    if( match(lkSwitch) ) {
        semantic.enterBlock( BlockSwitch );
        if( !match( lkLbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "'(' expected" )), lexBackup );
        if( !parseExpr() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Expression expected" )), lexBackup );
        if( !match( lkRbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "')' expected" )), lexBackup );
        if( !parseInstruction() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected" )), lexBackup );
        semantic.leaveBlock( BlockSwitch );
        return true;
    }
    return false;
}

bool Syntax::parseIter()
{
    if( match(lkWhile) ) {
        semantic.enterBlock( BlockWhile );
        if( !match( lkLbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "'(' expected" )), lexBackup );
        if( !parseExpr() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Expression expected" )), lexBackup );
        else {
            semantic.cast( ParType(ParType::Boolean, &semantic) );
            semantic.operatorFinished();
        }
        if( !match( lkRbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "')' expected" )), lexBackup );
        if( !parseInstruction() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected" )), lexBackup );
        semantic.leaveBlock( BlockWhile );
        return true;
    }
    if( match(lkDo) ) {
        semantic.enterBlock( BlockWhile );
        if( !parseInstruction() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected" )), lexBackup );
        if( !match(lkWhile) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "While expected" )), lexBackup );
        if( !match( lkLbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "'(' expected" )), lexBackup );
        if( !parseExpr() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Expression expected" )), lexBackup );
        else {
            semantic.cast( ParType(ParType::Boolean, &semantic) );
            semantic.operatorFinished();
        }
        if( !match( lkRbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "')' expected" )), lexBackup );
        if( !match( lkSemicolon ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "';' expected" )), lexBackup );
        semantic.leaveBlock( BlockWhile );
        return true;
    }
    if( match(lkFor) ) {
        semantic.enterBlock( BlockFor );
        if( !match( lkLbracket ) )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "'(' expected" )), lexBackup );
        if( !match(lkSemicolon) && ! parseDefinition() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Incorrect definition part of 'for'" )), lexBackup );

        if( !match(lkSemicolon) ) {
            if( !(parseExpr() && match(lkSemicolon)) ) {
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Incorrect check part of 'for'" )), lexBackup );
            }
            else {
                semantic.cast( ParType(ParType::Boolean, &semantic) );
                semantic.operatorFinished();
            }
        }
        if( !match(lkRbracket) ) {
            if( !(parseExpr() && match(lkRbracket)) ) {
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Incorrect execution part of 'for'" )), lexBackup );
            }
            else {
                semantic.operatorFinished();
            }
        }
        if( !parseInstruction() )
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Instruction expected after 'for'" )), lexBackup );

        semantic.leaveBlock( BlockFor );
        return true;
    }
    return false;
}

bool Syntax::parseTransition()
{
    if( match(lkBreak) ) {
        if( match(lkSemicolon) ) {
            semantic.breakFound();
            return true;
        }
        else
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "';' expected" )), lexBackup );
    }
    if( match(lkContinue) ) {
        if( match(lkSemicolon) ) {
            semantic.continueFound();
            return true;
        }
        else
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "';' expected" )), lexBackup );
    }
    if( match(lkReturn) ) {
        if( match(lkSemicolon) ) {
            semantic.returnFound();
            return true;
        }
        else if( parseExpr() && match(lkSemicolon) ) {
            semantic.returnValueFound();
            return true;
        }
        else
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "';' expected" )), lexBackup );
    }
    return false;
}

bool Syntax::parseDefinition()
{
    if( !match(Type) ) return false; // it's not a definition
    LexType justType = lexBackup.lexType();
    ParType partype( ParType::Void, &semantic );
    switch( lexBackup.lexType().type ) {
    case Double:
        partype = ParType( ParType::Double, &semantic );
        break;
    case Int:
        partype = ParType( ParType::Int, &semantic );
        break;
    case Boolean:
        partype = ParType( ParType::Boolean, &semantic );
        break;
    case Char:
        partype = ParType( ParType::Char, &semantic );
        break;
    case Void:
        //        partype = ParType( ParType::Void, &semantic );
        break;
    default:
        throw LexException(TR( QT_TRANSLATE_NOOP( "Exception", "Unknown type" )), lexBackup );
    }

    if( !match(Indent) )        // invalid definition
        throw LexException(TR( QT_TRANSLATE_NOOP( "Exception", "Invalid definition encountered" )), lexBackup );

    QString name = lexBackup.lexText();
    if( match(lkLbracket) ) {   // it's a function
        enum State { sUnknown, sType, sIndent, sComma, sDefPar };
        State s = sUnknown;
        bool b_break = false;
        bool defPars = false;

        VarList parlist;

        while( !b_break )
            switch( s ) {
            case sComma:
                if( match(Type) ) {
                    s = sType;
                    parlist.push_back( Var("", lexToRValue(lexBackup.lexType())) );
                    break;
                }
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Type expected" )), lexBackup );
            case sUnknown:
                if( match(Type) ) {
                    s = sType;
                    parlist.push_back( Var("", lexToRValue(lexBackup.lexType())) );
                    break;
                }
                if( match(lkRbracket) ) {
                    b_break = true;
                    break;
                }
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Type or ')' expected" )), lexBackup );
            case sType:
                if( match(Indent) ) {
                    s = sIndent;
                    break;
                }
                if( match(lkComma) ) {
                    s = sComma;
                    break;
                }
                if( match(lkAssign) ) {
                    s = sDefPar;
                    break;
                }
                if( match(lkRbracket) ) {
                    b_break = true;
                    break;
                }
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Parameter name, ')' or ',' expected" )), lexBackup );
            case sIndent:
                parlist.back().name = lexBackup.lexText();
                if( match(lkComma) ) {
                    if( !defPars ) {
                        s = sComma;
                        break;
                    }
                    else
                        throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Default parameter expected " )), lexBackup );
                }

                if( match(lkAssign) ) {
                    s = sDefPar;
                    break;
                }
                if( match(lkRbracket) ) {
                    if( !defPars ) {
                        b_break = true;
                        break;
                    }
                    else
                        throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Default parameter expected " )), lexBackup );
                }
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "',' or ')' expected" )), lexBackup );
            case sDefPar:
                defPars = true;
                if( match(Constant) ) {
                    if( match(lkComma) ) {
                        s = sComma;
                        break;
                    }
                    if( match(lkRbracket) ) {
                        b_break = true;
                        break;
                    }
                    throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "',' or ')' expected" )), lexBackup );
                }
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Constant expression expected" )), lexBackup );
            default :
                throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "Unpredictable state" )) , curSource(), curPos() ) ;
            }

        if( match(lkSemicolon) ) { // prototype
            semantic.gotFuncPrototype( name, partype, parlist );
            return true;
        }
        if( match(lkLCbracket) ) {
            f->pushLex( lexBackup );
            semantic.gotFuncDefinition( name, partype, parlist );
//            semantic.enterBlock( BlockFunc );
            if( parseCombined() ) // function body
                return true;
            semantic.leaveBlock( BlockFunc );
        }
        else
            throw error;            // incorrect function body
    }

    // it's variable
    enum State { sType, sIndent, sDefPar };
    State s = sIndent;
    bool b_break = false;
    QString cur_var;
    while( !b_break )
        switch( s ) {
        case sType:
            if( match(Indent) ) {
                s = sIndent;
                break;
            }
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Indentifier expected" )), lexBackup );
        case sIndent:
            // input action
            cur_var = lexBackup.lexText();
            semantic.gotVarDefinition( cur_var, lexToLValue(justType) );

            if( match(lkComma) ) {
                s = sType;
                break;
            }
            if( match(lkAssign) ) {
                s = sDefPar;
                break;
            }
            if( match(lkSemicolon) ) {
                b_break = true;
                break;
            }
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "',', '=' or ';' expected" )), lexBackup );
        case sDefPar:
            semantic.gotVarUsage( cur_var );
            if( parseExpr() ) {
                semantic.gotBinaryFuncUsage( "@=@" ); // initialization
                semantic.operatorFinished();
                if( match(lkComma) ) {
                    s = sType;
                    break;
                }
                if( match(lkSemicolon) ) {
                    b_break = true;
                    break;
                }
                throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "',' or ';' expected" )), f->curLex() );
            }
            throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Constant expression expected" )), f->curLex() );
        default :
            throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "Unpredictable state" )) , curSource(), curPos() ) ;
        }

    //    semantic.gotVarDefinition( name, partype );
    return true;
    throw LexException( TR( QT_TRANSLATE_NOOP( "Exception", "Invalid definition encountered" )), f->curLex() );
}

void Syntax::check()
{
//     if( !parseDefinition() )
//         throw error;
//     while( parseDefinition() );
//     if( !match(Eof) )
//          throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "expression parse error on " )) + lexBackup.lexText() + " (it's not Eof)" , lexBackup.lexSource(), lexBackup.lexPos() ) ;
//     return true;
    try {
        if( !parseExpr() )
            throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "expression parse error on %1" )).arg(f->curLex().lexText()), curSource(), curPos() );
        if( !match(Eof) )
            throw Exception( TR( QT_TRANSLATE_NOOP( "Exception", "expression parse error on %1 (it's not Eof)" )).arg(f->curLex().lexText()), curSource(), curPos() );
    }
    catch( SemanticException& e ) {
        throw Exception( e.message(), curSource(), curPos() );
    }
}

ParType Syntax::lexToRValue( LexType t ) const
{
    switch( t.type ) {
    case Double:
        return ParType( ParType::Double, &semantic );
        break;
    case Int:
        return ParType( ParType::Int, &semantic );
        break;
    case Boolean:
        return ParType( ParType::Boolean, &semantic );
        break;
    case Char:
        return ParType( ParType::Char, &semantic );
        break;
    case String:
        return ParType( ParType::String, &semantic );
        break;
    default:
        throw LexException(TR( QT_TRANSLATE_NOOP( "Exception", "Unknown type" )), f->curLex() );
    }
}

ParType Syntax::lexToLValue( LexType t ) const
{
    switch( t.type ) {
    case Double:
        return ParType( ParType::Double, &semantic, ParType::LValue );
        break;
    case Int:
        return ParType( ParType::Int, &semantic, ParType::LValue );
        break;
    case Boolean:
        return ParType( ParType::Boolean, &semantic, ParType::LValue );
        break;
    case Char:
        return ParType( ParType::Char, &semantic, ParType::LValue );
        break;
    case String:
        return ParType( ParType::String, &semantic, ParType::LValue );
        break;
    default:
        throw LexException(TR( QT_TRANSLATE_NOOP( "Exception", "Unknown type" )), f->curLex() );
    }
}

void Syntax::registerVariable ( QString name, ParType type )
{
    semantic.gotVarDefinition( name, type );
}

