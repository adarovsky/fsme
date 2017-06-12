#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QFileOpenEvent>

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv, int = ApplicationFlags);

    bool event(QEvent *event) {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            emit openFile( openEvent->file() );
        }

        return QApplication::event(event);
    }
signals:
    void openFile( const QString& file );
};

#endif // APPLICATION_H
