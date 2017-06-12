#ifndef STATEADDPLACEHOLDER_H
#define STATEADDPLACEHOLDER_H
#include "QGraphicsRectItem"
#include "QGraphicsTextItem"

class StateAddPlaceholder : public QGraphicsTextItem
{
    Q_OBJECT
signals:
    void editFinished( const QString& text );
    void editCanceled();
public:
    StateAddPlaceholder();
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
};

#endif // STATEADDPLACEHOLDER_H
