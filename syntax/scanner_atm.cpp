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
#include "scanner_atm.h"

void FSMScanner::__processEvent( Event e )
{
   States yOld = __Y;
   bool pass = false;
   switch( __Y ) { //transitions 
   case yAmp:
      if( e == eSymbol && isAmp() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol && !isAmp() && !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yEatWS:
      if( e == eSymbol && isSpace() ) {
         //outcome actions
         __Y = yEatWS;
         pass = true;
      }
      else if( e == eSymbol && isDoubleQuote() ) {
         //outcome actions
         __Y = yProcessLocalInclude;
         pass = true;
      }
      else if( e == eSymbol && isLess() ) {
         //outcome actions
         __Y = yProcessDefaultInclude;
         pass = true;
      }
      break;
   case yEatWhitespace:
      if( e == eSymbol && isSpace() ) {
         //outcome actions
         __Y = yEatWhitespace;
         pass = true;
      }
      else if( !isSpace() ) {
         //outcome actions
         zPushBack();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yError:
      break;
   case yFinishMultiLineComment:
      if( e == eSymbol && isDiv() ) {
         //outcome actions
         zClearToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol && !isDiv() ) {
         //outcome actions
         zPushBack();
         __Y = yReadMultiLineComment;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         zReportUnexpectedEndOfFile();
         __Y = yError;
         pass = true;
      }
      break;
   case yMayBeComment:
      if( e == eSymbol && isMul() ) {
         //outcome actions
         zClearToken();
         __Y = yReadMultiLineComment;
         pass = true;
      }
      else if( e == eSymbol && isDiv() ) {
         //outcome actions
         zClearToken();
         __Y = yReadSingleLineComment;
         pass = true;
      }
      else if( e == eSymbol || e == eEnd ) {
         //outcome actions
         zAddKeywordToken();
         zPushBack();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yMinus:
      if( isMinus() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( !isMinus() && !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yPlus:
      if( isPlus() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( !isPlus() && !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yProcessDefaultInclude:
      if( e == eSymbol && isNewLine() ) {
         //outcome actions
         zReportInvalidInclude();
         __Y = yError;
         pass = true;
      }
      else if( e == eSymbol && !isMore() ) {
         //outcome actions
         zAppendToCur();
         __Y = yProcessDefaultInclude;
         pass = true;
      }
      else if( e == eSymbol && isMore() ) {
         //outcome actions
         zIncludeDefaultFile();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yProcessLocalInclude:
      if( e == eSymbol && isNewLine() ) {
         //outcome actions
         zReportInvalidInclude();
         __Y = yError;
         pass = true;
      }
      else if( e == eSymbol && !isDoubleQuote() ) {
         //outcome actions
         zAppendToCur();
         __Y = yProcessLocalInclude;
         pass = true;
      }
      else if( e == eSymbol && isDoubleQuote() ) {
         //outcome actions
         zIncludeLocalFile();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadChar:
      if( e == eSymbol && isQuote() ) {
         //outcome actions
         zAddCharToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol ) {
         //outcome actions
         zAppendToCur();
         __Y = yReadChar;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         __Y = yError;
         pass = true;
      }
      break;
   case yReadComplexOperator:
      if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol ) {
         //outcome actions
         zPushBack();
         zAddSymbolToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadDot:
      if( isNumber() ) {
         //outcome actions
         __Y = yReadReal;
         pass = true;
      }
      else if( !isNumber() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadDoubleEq:
      if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadIndent:
      if( e == eSymbol && ( isAlpha() || isNumber() ) ) {
         //outcome actions
         __Y = yReadIndent;
         pass = true;
      }
      else if( e == eSymbol && isKeyword() ) {
         //outcome actions
         zAddKeywordToken();
         zPushBack();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol || e == eEnd ) {
         //outcome actions
         zAddIndentToken();
         zPushBack();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadLeftShift:
      if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadLess:
      if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( isLess() ) {
         //outcome actions
         zAppendToCur();
         __Y = yReadLeftShift;
         pass = true;
      }
      else if( !isLess() && !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadMacro:
      if( e == eSymbol && isAlpha() ) {
         //outcome actions
         zAppendToCur();
         __Y = yReadMacro;
         pass = true;
      }
      else if( e == eSymbol && !isAlpha() && isInclude() ) {
         //outcome actions
         zClearToken();
         __Y = yEatWS;
         pass = true;
      }
      break;
   case yReadMantissa:
      if( e == eSymbol && isNumber() ) {
         //outcome actions
         zAppendToCur();
         __Y = yReadMantissa;
         pass = true;
      }
      else if( e == eSymbol && !isNumber() ) {
         //outcome actions
         zPushBack();
         zAddDoubleToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         zAddDoubleToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadMantissaFirst:
      if( e == eSymbol && isNumber() ) {
         //outcome actions
         __Y = yReadMantissa;
         pass = true;
      }
      else if( e == eSymbol && !isNumber() ) {
         //outcome actions
         zReportInvalidDouble();
         __Y = yError;
         pass = true;
      }
      break;
   case yReadMantissaSign:
      if( e == eSymbol && (isPlus() || isMinus()) ) {
         //outcome actions
         __Y = yReadMantissaFirst;
         pass = true;
      }
      else if( e == eSymbol && isNumber() ) {
         //outcome actions
         __Y = yReadMantissa;
         pass = true;
      }
      else if( e == eSymbol && !isNumber() ) {
         //outcome actions
         zReportInvalidDouble();
         __Y = yError;
         pass = true;
      }
      break;
   case yReadMore:
      if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( !isMore() && !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( isMore() ) {
         //outcome actions
         zAppendToCur();
         __Y = yReadRightShift;
         pass = true;
      }
      break;
   case yReadMultiLineComment:
      if( e == eSymbol && isMul() ) {
         //outcome actions
         __Y = yFinishMultiLineComment;
         pass = true;
      }
      else if( e == eSymbol ) {
         //outcome actions
         __Y = yReadMultiLineComment;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         zReportUnexpectedEndOfFile();
         __Y = yError;
         pass = true;
      }
      break;
   case yReadNumber:
      if( e == eSymbol && isNumber() ) {
         //outcome actions
         __Y = yReadNumber;
         pass = true;
      }
      else if( e == eSymbol && isDot() ) {
         //outcome actions
         __Y = yReadReal;
         pass = true;
      }
      else if( e == eSymbol && isE() ) {
         //outcome actions
         __Y = yReadMantissaSign;
         pass = true;
      }
      else if( e == eSymbol && isAlpha() ) {
         //outcome actions
         zReportInvalidDouble();
         __Y = yError;
         pass = true;
      }
      else if( e == eSymbol && !isNumber() ) {
         //outcome actions
         zPushBack();
         zAddIntToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         zAddIntToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadReal:
      if( e == eSymbol && isNumber() ) {
         //outcome actions
         __Y = yReadReal;
         pass = true;
      }
      else if( e == eSymbol && isE() ) {
         //outcome actions
         __Y = yReadMantissaSign;
         pass = true;
      }
      else if( e == eSymbol && !isNumber() ) {
         //outcome actions
         zPushBack();
         zAddDoubleToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         zAddDoubleToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadRightShift:
      if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yReadSingleLineComment:
      if( e == eSymbol && isNewLine() ) {
         //outcome actions
         zClearToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol && !isNewLine() ) {
         //outcome actions
         __Y = yReadSingleLineComment;
         pass = true;
      }
      break;
   case yReadString:
      if( e == eSymbol && isDoubleQuote() ) {
         //outcome actions
         zAddStringToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol ) {
         //outcome actions
         zAppendToCur();
         __Y = yReadString;
         pass = true;
      }
      else if( e == eEnd ) {
         //outcome actions
         zReportUnexpectedEndOfFile();
         __Y = yError;
         pass = true;
      }
      break;
   case yStroke:
      if( e == eSymbol && isStroke() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( isEqual() ) {
         //outcome actions
         zAppendToCur();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol && !isStroke() && !isEqual() ) {
         //outcome actions
         zPushBack();
         zAddKeywordToken();
         __Y = yUnknown;
         pass = true;
      }
      break;
   case yUnknown:
      if( e == eSymbol && isDiv() ) {
         //outcome actions
         __Y = yMayBeComment;
         pass = true;
      }
      else if( e == eSymbol && isHash() ) {
         //outcome actions
         __Y = yEatWS;
         pass = true;
      }
      else if( e == eSymbol && isSpace() ) {
         //outcome actions
         __Y = yEatWhitespace;
         pass = true;
      }
      else if( e == eSymbol && isAlpha() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadIndent;
         pass = true;
      }
      else if( e == eSymbol && isQuote() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadChar;
         pass = true;
      }
      else if( e == eSymbol && isDoubleQuote() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadString;
         pass = true;
      }
      else if( e == eSymbol && isNumber() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadNumber;
         pass = true;
      }
      else if( e == eSymbol && isDot() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadDot;
         pass = true;
      }
      else if( e == eSymbol && isSimpleDelim() ) {
         //outcome actions
         zBackupPos();
         zAppendToCur();
         zAddSymbolToken();
         __Y = yUnknown;
         pass = true;
      }
      else if( e == eSymbol && isLess() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadLess;
         pass = true;
      }
      else if( e == eSymbol && isMore() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadMore;
         pass = true;
      }
      else if( e == eSymbol && isEqual() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadDoubleEq;
         pass = true;
      }
      else if( e == eSymbol && isPlus() ) {
         //outcome actions
         zBackupPos();
         __Y = yPlus;
         pass = true;
      }
      else if( e == eSymbol && isMinus() ) {
         //outcome actions
         zBackupPos();
         __Y = yMinus;
         pass = true;
      }
      else if( e == eSymbol && isStroke() ) {
         //outcome actions
         zBackupPos();
         __Y = yStroke;
         pass = true;
      }
      else if( e == eSymbol && isAmp() ) {
         //outcome actions
         zBackupPos();
         __Y = yAmp;
         pass = true;
      }
      else if( e == eSymbol && isPunct() ) {
         //outcome actions
         zBackupPos();
         __Y = yReadComplexOperator;
         pass = true;
      }
      break;
   }

   if( yOld == __Y && !pass ) { /*__recurseLock = false;*/ return; }

   switch( __Y ) { // income actions
   case yAmp:
      zAppendToCur();
      break;
   case yEatWS:
      break;
   case yEatWhitespace:
      break;
   case yError:
      zReportError();
      break;
   case yFinishMultiLineComment:
      break;
   case yMayBeComment:
      zAppendToCur();
      zBackupPos();
      break;
   case yMinus:
      zAppendToCur();
      break;
   case yPlus:
      zAppendToCur();
      break;
   case yProcessDefaultInclude:
      break;
   case yProcessLocalInclude:
      break;
   case yReadChar:
      break;
   case yReadComplexOperator:
      zAppendToCur();
      break;
   case yReadDot:
      zAppendToCur();
      break;
   case yReadDoubleEq:
      zAppendToCur();
      break;
   case yReadIndent:
      zAppendToCur();
      break;
   case yReadLeftShift:
      break;
   case yReadLess:
      zAppendToCur();
      break;
   case yReadMacro:
      break;
   case yReadMantissa:
      zAppendToCur();
      break;
   case yReadMantissaFirst:
      zAppendToCur();
      break;
   case yReadMantissaSign:
      zAppendToCur();
      break;
   case yReadMore:
      zAppendToCur();
      break;
   case yReadMultiLineComment:
      break;
   case yReadNumber:
      zAppendToCur();
      break;
   case yReadReal:
      zAppendToCur();
      break;
   case yReadRightShift:
      break;
   case yReadSingleLineComment:
      break;
   case yReadString:
      break;
   case yStroke:
      zAppendToCur();
      break;
   case yUnknown:
      zBackupPos();
      break;
   }
}

void FSMScanner::A( Event e )
{
   __events.push( e );
   if( !__recurseLock ) {
      __recurseLock = true;
      while( !__events.empty() ) {
         __processEvent( __events.front() );
         __events.pop();
      }
      __recurseLock = false;
   }
}

const char* FSMScanner::eventName( Event e )
{
   switch(e) {
   case eEnd:
      return "eEnd";
   case eSymbol:
      return "eSymbol";
   default:
      return "Unknown event";
   }
}

const char* FSMScanner::stateName( States s )
{
   switch(s) {
   case yAmp:
      return "yAmp";
   case yEatWS:
      return "yEatWS";
   case yEatWhitespace:
      return "yEatWhitespace";
   case yError:
      return "yError";
   case yFinishMultiLineComment:
      return "yFinishMultiLineComment";
   case yMayBeComment:
      return "yMayBeComment";
   case yMinus:
      return "yMinus";
   case yPlus:
      return "yPlus";
   case yProcessDefaultInclude:
      return "yProcessDefaultInclude";
   case yProcessLocalInclude:
      return "yProcessLocalInclude";
   case yReadChar:
      return "yReadChar";
   case yReadComplexOperator:
      return "yReadComplexOperator";
   case yReadDot:
      return "yReadDot";
   case yReadDoubleEq:
      return "yReadDoubleEq";
   case yReadIndent:
      return "yReadIndent";
   case yReadLeftShift:
      return "yReadLeftShift";
   case yReadLess:
      return "yReadLess";
   case yReadMacro:
      return "yReadMacro";
   case yReadMantissa:
      return "yReadMantissa";
   case yReadMantissaFirst:
      return "yReadMantissaFirst";
   case yReadMantissaSign:
      return "yReadMantissaSign";
   case yReadMore:
      return "yReadMore";
   case yReadMultiLineComment:
      return "yReadMultiLineComment";
   case yReadNumber:
      return "yReadNumber";
   case yReadReal:
      return "yReadReal";
   case yReadRightShift:
      return "yReadRightShift";
   case yReadSingleLineComment:
      return "yReadSingleLineComment";
   case yReadString:
      return "yReadString";
   case yStroke:
      return "yStroke";
   case yUnknown:
      return "yUnknown";
   default:
      return "Unknown state";
   }
}
