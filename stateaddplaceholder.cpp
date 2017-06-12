#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QKeyEvent>

#include "stateaddplaceholder.h"

StateAddPlaceholder::StateAddPlaceholder()
    : QGraphicsTextItem()
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFont( QFont("helvetica", 14, QFont::Medium) );
}

QRectF StateAddPlaceholder::boundingRect() const
{
    return QGraphicsTextItem::boundingRect().adjusted( -10, -10, 10, 10 );
}

void StateAddPlaceholder::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setPen( QPen( QBrush( Qt::black ), 1 ) );
    painter->setBrush( QBrush( Qt::lightGray ) );

    painter->drawRect( boundingRect() );
    painter->restore();

    QGraphicsTextItem::paint( painter, option, widget );
}

void StateAddPlaceholder::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Return) {
        QString text = toPlainText().simplified();
        if ( text.isEmpty() )
            emit editCanceled();
        else
            emit editFinished( text );
        event->accept();
        return;
    }

    QGraphicsTextItem::keyPressEvent( event );
}

void StateAddPlaceholder::focusOutEvent(QFocusEvent *event)
{
    QString text = toPlainText().simplified();
    if ( text.isEmpty() )
        emit editCanceled();
    else
        emit editFinished( text );
    QGraphicsTextItem::focusOutEvent(event);
}

QVariant StateAddPlaceholder::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged && !value.toBool())
        emit editCanceled();
    return value;
}
