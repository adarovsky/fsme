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
#ifndef FSMScanner2033121743
#define FSMScanner2033121743
#include <queue> // for event queue

class FSMScanner {
public:
  enum Event { eEnd, eSymbol };
protected:
   virtual bool isAlpha() const = 0; 
   virtual bool isAmp() const = 0; 
   virtual bool isDiv() const = 0; 
   virtual bool isDot() const = 0; 
   virtual bool isDoubleQuote() const = 0; 
   virtual bool isE() const = 0; 
   virtual bool isEqual() const = 0; 
   virtual bool isEscape() const = 0; 
   virtual bool isHash() const = 0; 
   virtual bool isInclude() const = 0; 
   virtual bool isKeyword() const = 0; 
   virtual bool isLess() const = 0; 
   virtual bool isMinus() const = 0; 
   virtual bool isMore() const = 0; 
   virtual bool isMul() const = 0; 
   virtual bool isNewLine() const = 0; 
   virtual bool isNumber() const = 0; 
   virtual bool isPlus() const = 0; 
   virtual bool isPunct() const = 0; 
   virtual bool isQuote() const = 0; 
   virtual bool isSimpleDelim() const = 0; 
   virtual bool isSpace() const = 0; 
   virtual bool isStroke() const = 0; 
   virtual bool isX() const = 0; 


   virtual void zAddCharToken() = 0; 
   virtual void zAddDoubleToken() = 0; 
   virtual void zAddIndentToken() = 0; 
   virtual void zAddIntToken() = 0; 
   virtual void zAddKeywordToken() = 0; 
   virtual void zAddStringToken() = 0; 
   virtual void zAddSymbolToken() = 0; 
   virtual void zAppendToCur() = 0; 
   virtual void zBackupPos() = 0; 
   virtual void zClearToken() = 0; 
   virtual void zIncludeDefaultFile() = 0; 
   virtual void zIncludeLocalFile() = 0; 
   virtual void zPushBack() = 0; 
   virtual void zReportError() = 0; 
   virtual void zReportInvalidDouble() = 0; 
   virtual void zReportInvalidInclude() = 0; 
   virtual void zReportUnexpectedEndOfFile() = 0; 

public:
  enum States { yAmp, yEatWS, yEatWhitespace, yError, yFinishMultiLineComment, yMayBeComment, yMinus, yPlus, yProcessDefaultInclude, yProcessLocalInclude, yReadChar, yReadComplexOperator, yReadDot, yReadDoubleEq, yReadIndent, yReadLeftShift, yReadLess, yReadMacro, yReadMantissa, yReadMantissaFirst, yReadMantissaSign, yReadMore, yReadMultiLineComment, yReadNumber, yReadReal, yReadRightShift, yReadSingleLineComment, yReadString, yStroke, yUnknown }; // states

private:
   States __Y;
   bool __recurseLock;
   std::queue<Event> __events; 
   void __processEvent( Event e );
public:
   FSMScanner() : __events() { __Y = yUnknown; __recurseLock = false; }
   virtual ~FSMScanner() {}
   States currentState() const { return __Y; }
   static const char* eventName( Event e );
   static const char* stateName( States s );
   void A( Event e );
};

#endif
