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
//====================================================================
// Filename:      scanner.h
//
//
// Description:   A scanner implementation
// Author:        Alexander Darovsky <adarovsky@sourceforge.net>
//
// Created at:    Wed Nov 28 18:14:24 2001
// Modified at:   Tue Feb 24 18:55:37 2004
// Modified by:   Alexander Darovsky <adarovsky@sourceforge.net>
//
// Version:
// Update count:  1
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

#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "scanner_atm.h"
#include "lex.h"
#include <list>
#include <queue>
#include <set>
#include <map>
#include <qstring.h>

#include "exception.h"
using namespace std;

class Scanner : public FSMScanner {
    QString       cur_token;
    QChar         cur_char;
    int           cur_pos;
    deque<Lex>    lex, lex_backup;
public:
    static const map<QString, LexType> keywords;
protected:
    int           pos;
    QString       source;
    set<QString>  used_includes;
    virtual bool isAlpha() const;
    virtual bool isDoubleQuote() const;
    virtual bool isEqual() const;
    virtual bool isEscape() const;
    virtual bool isKeyword() const;
    virtual bool isLess() const;
    virtual bool isMore() const;
    virtual bool isNumber() const;
    virtual bool isPunct() const;
    virtual bool isQuote() const;
    virtual bool isSimpleDelim() const;
    virtual bool isSpace() const;
    virtual bool isMinus() const;
    virtual bool isPlus() const;
    virtual bool isDot() const;
    virtual bool isE() const;
    virtual bool isHash() const;
    virtual bool isInclude() const;
    virtual bool isNewLine() const;
    virtual bool isDiv() const;
    virtual bool isMul() const;
    virtual bool isX() const;
    virtual bool isStroke() const;
    virtual bool isAmp() const;

    virtual void zAddCharToken();
    virtual void zAddDoubleToken();
    virtual void zAddIndentToken();
    virtual void zAddIntToken();
    virtual void zAddKeywordToken();
    virtual void zAddStringToken();
    virtual void zAddSymbolToken();
    virtual void zAppendToCur();
    virtual void zBackupPos();
    virtual void zPushBack();
    virtual void zReportError();
    virtual void zReportInvalidDouble();
    virtual bool get_char( QChar& c ) = 0;
    virtual void push_back( QChar c ) = 0;
    virtual void zClearToken();
    virtual void zIncludeDefaultFile();
    virtual void zIncludeLocalFile();
    virtual void zReportInvalidInclude();
    virtual void zReportUnexpectedEndOfFile();
    void build();
public:
    Scanner();
    Lex  curLex();
    void popLex() { if( !lex.empty() ) lex.pop_back(); }
    void pushLex( Lex l ) { lex.push_back(l); }
    void reset() { lex = lex_backup; }
};

class StringScanner : public Scanner {
    QString src;
    //    int pos;
protected:
    virtual bool get_char( QChar& c );
    virtual void push_back( QChar c );
    friend class Scanner;
public:
    StringScanner( QString source );
    virtual ~StringScanner() {}
};

#endif
/// Local Variables: ***
/// mode: C++ ***
/// c-basic-offset: 4 ***
/// End: ***
