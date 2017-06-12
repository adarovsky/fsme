#include <QUndoCommand>
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
