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
#ifndef _MY_SYNTAX_H_
#define _MY_SYNTAX_H_

#include <QTextDocument>
#include <QSyntaxHighlighter>
#include <QColor>
#include <QSharedPointer>
#include <QWeakPointer>

class StateMachine;
class EIOHighlighter : public QSyntaxHighlighter {
    QWeakPointer<StateMachine> _sm;
public:
    EIOHighlighter( QTextDocument * textEdit );
    void setStateMachine( QSharedPointer<StateMachine> sm ) { _sm = sm; }
    QSharedPointer<StateMachine> stateMachine() const { return _sm.toStrongRef(); }
    static QColor eventColor, inputColor, constColor, errorColor, punctColor;

    virtual void  highlightBlock(const QString &text) Q_DECL_OVERRIDE;
};

#endif

/// Local Variables:
/// mode: C++
/// c-basic-offset: 4
/// End:
