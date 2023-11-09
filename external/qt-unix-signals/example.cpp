#include <QCoreApplication>
#include <QDebug>
#include "sigwatch.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qDebug() << "Hello from process" << QCoreApplication::applicationPid();

    UnixSignalWatcher sigwatch;
    sigwatch.watchForSignal(SIGINT);
    sigwatch.watchForSignal(SIGTERM);
    QObject::connect(&sigwatch, SIGNAL(unixSignal(int)), &app, SLOT(quit()));

    int exitcode = app.exec();
    qDebug() << "Goodbye";
    return exitcode;
}

