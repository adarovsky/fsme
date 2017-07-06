#include <QUndoCommand>
#include <QRegularExpression>
#include "statemachine.h"

EIOBase::~EIOBase()
{
}

QString EIOBase::comment() const
{
    return _comment;
}

void EIOBase::setComment(const QString &comment)
{
    _comment = comment;
    parent()->informItemChanged(this, Qt::WhatsThisRole);
}

void EIOBase::renameInCondition(QSharedPointer<StateMachine> fsm, const QString &what, const QString &to)
{
    if (fsm && !what.isEmpty()) {
        QRegularExpression rx("\\b" + QRegularExpression::escape(what) + "\\b");
        for( auto s = fsm->statesBegin(); s != fsm->statesEnd(); ++s ) {
            auto tl = (*s)->transitionList();
            for ( auto t = tl.begin(); t != tl.end(); ++ t ) {
                auto s = (*t)->condition();
                auto s1 = QString(s).replace(rx, to);
                if (s != s1) {
                    (*t)->setCondition( s1 );
                }
            }
        }
    }
}
