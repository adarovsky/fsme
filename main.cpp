#include <os/log.h>

#include "mainwindow.h"
#include "application.h"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    MainWindow w;
    w.show();

    QObject::connect(&a, SIGNAL(openFile(QString)), &w, SLOT(fileOpen(QString)));

    return a.exec();
}
