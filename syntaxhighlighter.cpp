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
#include "syntaxhighlighter.h"
#include "syntax/syntax.h"
#include "statemachine.h"

#include <qapplication.h>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(EIOHighlighterLogging)
Q_LOGGING_CATEGORY(EIOHighlighterLogging, "fsm.highlighter")

QColor EIOHighlighter::eventColor = Qt::darkGreen;
QColor EIOHighlighter::inputColor = Qt::blue;
QColor EIOHighlighter::constColor = Qt::darkBlue;
QColor EIOHighlighter::errorColor = Qt::red;
QColor EIOHighlighter::punctColor = Qt::black;

EIOHighlighter::EIOHighlighter(QTextDocument * e )
    : QSyntaxHighlighter( e ), _sm( 0 )
{
}


void EIOHighlighter::highlightBlock(const QString &text)
{
    auto fsm = stateMachine();
    if( !fsm )
        return;

    StringScanner* s = new StringScanner(text);
    Syntax syn( s );

    QStringList l;
    ParType t( ParType::Boolean, 0 );

    int errorPos = -1;
    try {
        QModelIndex folder = fsm->eventsFolder();
        for(int i = 0; i < fsm->rowCount(folder); ++i ) {
            syn.registerVariable( folder.child(i, 0).data().toString(), t );
        }

        folder = fsm->inputsFolder();
        for(int i = 0; i < fsm->rowCount(folder); ++i ) {
            syn.registerVariable( folder.child(i, 0).data().toString(), t );
        }

        syn.registerVariable( "true", t );
        syn.registerVariable( "false", t );

        syn.check();
    }
//     catch( SemanticException& e ) {
//         qWarning( "%s", (const char*)e.message().local8Bit() );
//         emit parseError( e.message(), e.pos() );
//     }
    catch( Exception& e ) {
        qCWarning(EIOHighlighterLogging).noquote() << "Error:" << e.message() << ": " << e.pos();
//        qApp->postEvent( qApp->mainWidget(), new StatusMessageEvent( e.message() ) );
        errorPos = e.pos();
    }

    s->reset();
    while( !s->curLex().match(Eof) ) {
        Lex l = s->curLex();
        if( l.lexPos() == errorPos ) {
            setFormat( l.lexPos(), l.lexText().length(), errorColor );
        }
        else {
            if( l.match(Indent) ) {
                if( stateMachine()->findEvent( l.lexText() ) ) {
                    setFormat( l.lexPos(), l.lexText().length(), eventColor );
                }
                else if( stateMachine()->findInput( l.lexText() ) ) {
                    setFormat( l.lexPos(), l.lexText().length(), inputColor );
                }
                else if( l.lexText() == "true" || l.lexText() == "false" ) {
                    setFormat( l.lexPos(), l.lexText().length(), constColor );
                }
                else {
                    setFormat( l.lexPos(), l.lexText().length(), errorColor );
                }
            }
            else if( l.match(lkLbracket) ||
                     l.match(lkRbracket) ||
                     l.match(Disjunction)||
                     l.match(Conjunction)||
                     l.match(lkExclamation) ) {
                setFormat( l.lexPos(), l.lexText().length(), punctColor );
            }
            else {
                setFormat( l.lexPos(), l.lexText().length(), errorColor );
            }
        }
        s->popLex();
    }
}

