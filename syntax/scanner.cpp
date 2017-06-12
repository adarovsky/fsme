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
#include <ctype.h>
#include <stdlib.h>
#include <iostream>             // for debug
#include <cstdio>
#include <iterator>
#include <string>

#include <qstring.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

/*inline bool isalpha( const QChar& c ) { return c.isLetter(); }
inline bool isdigit( const QChar& c ) { return c.isDigit();  }
inline bool ispunct( const QChar& c ) { return c.isPunct();  }
inline bool isspace( const QChar& c ) { return c.isSpace();  }
*/

Scanner::Scanner()
    : pos( 0 )
{}

inline bool isodigit( char c ) { return '0' <= c && c <= '7'; }

string expand_esc( string src )
{
   enum { plain, escape } state = plain;
   string buf;
   back_insert_iterator<string> w (buf);
   string::iterator             r = src.begin();

   for ( ; r!= src.end(); ++r )
      switch ( state ) {
      case plain:
         if ( *r == '\\' ) state = escape; else *w++ = *r;
         break;
      case escape:
         switch ( *r ) {
         case 'a': *w++ = '\007'; state = plain; break;
         case 'b': *w++ = '\010'; state = plain; break;
         case 'f': *w++ = '\014'; state = plain; break;
         case 'n': *w++ = '\012'; state = plain; break;
         case 'r': *w++ = '\015'; state = plain; break;
         case 't': *w++ = '\011'; state = plain; break;
         case 'v': *w++ = '\013'; state = plain; break;
         case 'x':
         case 'X': {
            char buf[3] = { 0, 0, 0 };
            if ( isxdigit(r[1]) ) {
               buf[0] = *(++r);
               if ( isxdigit( r[1] ) ) buf[1] = *(++r);
            }
            *w++ = *buf ? (char) strtol( buf, (char**) NULL, 16 ) : *r;
            state = plain;
         } break;
         default: {
            char buf[4] = { 0, 0, 0, 0 };
            if ( isodigit(*r) ) {
               buf[0] = *r;
               if ( isodigit( r[1] ) ) {
                  buf[1] = *(++r);
                  if ( isodigit( r[1] ) ) buf[2] = *(++r);
               }
            }
            *w++ = *buf ? (char) strtol( buf, (char**) NULL, 8 ) : *r;
            state = plain;
         }
         } break;
      }
   //      *w = '\0';
   //   buf
   return buf;
}


Exception::Exception( QString m_, QString s_, int l_ )
    : m( m_ ), s(s_), l( l_ )
{
// //     char buf[17];
// //     snprintf( buf, 17, "%d", l_ );
// //     m = m_ + " at " + s_ + ":" + buf;
//     m = m;
//     l = l;
}


bool Scanner::isAlpha() const
{
    return cur_char.isLetter() || cur_char == '_';
}
bool Scanner::isDoubleQuote() const
{
    return cur_char == '"';
}
bool Scanner::isEqual() const
{
    return cur_char == '=';
}
bool Scanner::isEscape() const
{
    return cur_char == '\\';
}
bool Scanner::isKeyword() const
{
    return keywords.find( cur_token ) != keywords.end();
}
bool Scanner::isLess() const
{
    return cur_char == '<';
}
bool Scanner::isMore() const
{
    return cur_char == '>';
}
bool Scanner::isNumber() const
{
    return cur_char.isNumber();
}
bool Scanner::isPunct() const
{
    return cur_char.isPunct();
}
bool Scanner::isQuote() const
{
    return cur_char == '\'';
}
bool Scanner::isSimpleDelim() const
{
    return cur_char == ';' ||
        cur_char == '{' ||
        cur_char == '}' ||
        cur_char == '(' ||
        cur_char == ')' ||
        cur_char == '[' ||
        cur_char == ']' ||
        cur_char == '.' ||
        cur_char == ',' ||
        cur_char == ':';
}
bool Scanner::isSpace() const
{
    return cur_char.isSpace();
}
bool Scanner::isMinus() const
{
    return cur_char == '-';
}
bool Scanner::isPlus() const
{
    return cur_char == '+';
}
bool Scanner::isDot() const
{
    return cur_char == '.';
}

bool Scanner::isE() const
{
    return cur_char == 'e' || cur_char == 'E';
}

bool Scanner::isHash() const
{
    return cur_char == '#';
}

bool Scanner::isInclude() const
{
    return cur_token == "include";
}

bool Scanner::isNewLine() const
{
    return cur_char == '\n';
}

bool Scanner::isMul() const
{
    return cur_char == '*';
}

bool Scanner::isDiv() const
{
    return cur_char == '/';
}

bool Scanner::isX() const
{
    return cur_char == 'x' || cur_char == 'X';
}

bool Scanner::isStroke() const
{
    return cur_char == '|';
}

bool Scanner::isAmp() const
{
    return cur_char == '&';
}


void Scanner::zBackupPos()
{
    cur_pos = pos - 1;
}

void Scanner::zAddCharToken()
{
    lex.push_front( Lex( LexType(Constant, Char, SUnknown), cur_token, source, cur_pos) );
    cur_token = QString::null;
}
void Scanner::zAddDoubleToken()
{
    lex.push_front( Lex( LexType(Constant, Double, SUnknown), cur_token, source, cur_pos) );
    cur_token = QString::null;
}
void Scanner::zAddIntToken()
{
    lex.push_front( Lex( LexType(Constant, Int, SUnknown), cur_token, source, cur_pos) );
    cur_token = QString::null;
}
void Scanner::zAddStringToken()
{
    cur_token = expand_esc( (const char*)cur_token.toLocal8Bit() ).c_str();
    if( !lex.empty() && lex.back().match(String) )
        lex.front() = Lex( LexType(Constant, String, SUnknown), lex.back().lexText()+cur_token, source, lex.back().lexPos() );
    else
        lex.push_front( Lex( LexType(Constant, String, SUnknown), cur_token, source, cur_pos) );

    cur_token = QString::null;
}
void Scanner::zAddIndentToken()
{
    lex.push_front( Lex( LexType(Indent, TUnknown, SUnknown), cur_token, source, cur_pos) );
    cur_token = QString::null;
}
void Scanner::zAddKeywordToken()
{
    lex.push_front( Lex( keywords.find(cur_token)->second, cur_token, source, cur_pos) );
    cur_token = QString::null;
}
void Scanner::zAddSymbolToken()
{
    lex.push_front( Lex( keywords.find(cur_token)->second, cur_token, source, cur_pos) );
    cur_token = QString::null;
}
void Scanner::zAppendToCur()
{
    cur_token += cur_char;
}
void Scanner::zPushBack()
{
    push_back( cur_char );
}
void Scanner::zReportError()
{
    throw Exception( "A scan error occured while reading token " + cur_token, source, cur_pos );
}

void Scanner::zReportInvalidDouble()
{
    throw Exception( "Invalid double " + cur_token, source, cur_pos );
}

void Scanner::zClearToken()
{
    cur_token = QString::null;
}

void Scanner::zIncludeDefaultFile()
{
//      if( used_includes.find( cur_token ) == used_includes.end() ) {
//          FileScanner f( cur_token, used_includes );
//          while( !f.curLex().match(Eof) ) {
//              lex.push_front( f.curLex() );
//              f.popLex();
//          }
//      }
//      else {
//          throw Exception("recurse include of " + cur_token, source, line+1 );
//      }
    cur_token = QString::null;
}

void Scanner::zIncludeLocalFile()
{
    cur_token = QString::null;
}

void Scanner::zReportInvalidInclude()
{
    throw Exception( "unexpected end of line in macro directive", source, cur_pos);
}

void Scanner::zReportUnexpectedEndOfFile()
{
    throw Exception( "Unexpected end of file at ", source, cur_pos );
}

void Scanner::build()
{
    pos = 0;
    while( get_char(cur_char) ) {
        A( eSymbol );
    }
    A( eEnd );
    lex_backup = lex;
}

Lex Scanner::curLex()
{
    if( !lex.empty() ) {
        Lex ret = lex.back();
        return ret;
    }
    else return Lex( LexType(Eof), QString::null, source, cur_pos );
}

StringScanner::StringScanner( QString source )
    : src( source )
{
    build();
}

bool StringScanner::get_char( QChar& c )
{
    if (pos >= src.length())
        return false;
    c = src.at(pos);
    return pos++ < src.length();
}
void StringScanner::push_back( QChar /*c*/ )
{
    pos--;
}
