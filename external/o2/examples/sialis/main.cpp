#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtWebView/QtWebView>

#include "o1.h"
#include "o1twitter.h"
#include "o1requestor.h"
#include "twitterapi.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QtWebView::initialize();
    qmlRegisterType<O1Twitter>("com.pipacs.o2", 1, 0, "O1Twitter");
    qmlRegisterType<TwitterApi>("com.pipacs.o2", 1, 0, "TwitterApi");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
