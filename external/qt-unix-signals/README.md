Unix Signal Watcher For Qt
==========================

Author: Simon Knopp  
Licence: [MIT](http://opensource.org/licenses/MIT)  


## Summary

When writing a Qt application, as with any application, it is sometimes useful
to handle Unix signals. Of course, Qt already incorporates the notion of
signals, so it would be nice if Unix signals could be mapped to Qt signals. Then
we could write handlers for Unix signals and connect them up in the same way as
normal Qt slots. 

The class described below does just this. It is heavily based on [this
example](http://qt-project.org/doc/qt-5.0/qtdoc/unix-signals.html) in the Qt
documentation, but it encapsulates that functionality in a generic re-usable
class.

## Interface

The interface is simple: you call `watchForSignal()` with the signals you're
interested in, and `connect()` your handlers to `SIGNAL(unixSignal(int))`.

``` c++
class UnixSignalWatcher : public QObject
{
    Q_OBJECT
public:
    explicit UnixSignalWatcher(QObject *parent = 0);
    ~UnixSignalWatcher();

    void watchForSignal(int signal);

signals:
    void unixSignal(int signal);
};
```

## Example usage

Let's look at an example program.

``` c++
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
```

This simply registers signal handlers for `SIGINT` and `SIGTERM` and then idles
forever. If you run it (`qmake && make && ./sigwatch-demo`) you'll see a
greeting and the pid of the process:

    Hello from process 6811

Press `^C` to send `SIGINT`. The `UnixSignalWatcher` will handle the signal,
which in turn is connected to `QCoreApplication::quit()`, so the event loop
exits and the farewell message is printed.

    ^CCaught signal: Interrupt 
    Goodbye

Similarly, you could use `kill` to send `SIGTERM`.

    $ ./sigwatch-demo &
    Hello from process 6848
    $ kill 6848
    Caught signal: Terminated
    Goodbye

If you send a signal that does not have a handler, though, you won't see the
farewell message. For instance:

    $ ./sigwatch-demo
    Hello from process 6906
    $ kill -SIGABRT 6906
    Aborted (core dumped)

## Compatibility

Tested with Qt 4.6 and 5.2 on Linux.

