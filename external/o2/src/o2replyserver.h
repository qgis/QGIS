#ifndef O2REPLYSERVER_H
#define O2REPLYSERVER_H

#include <QTcpServer>
#include <QMap>
#include <QByteArray>
#include <QString>

#include "o0export.h"

/// HTTP server to process authentication response.
class O0_EXPORT O2ReplyServer: public QTcpServer {
    Q_OBJECT

public:
    explicit O2ReplyServer(QObject *parent = nullptr);

    /// Page content on local host after successful oauth - in case you do not want to close the browser, but display something
    Q_PROPERTY(QByteArray replyContent READ replyContent WRITE setReplyContent NOTIFY replyContentChanged)
    QByteArray replyContent();
    void setReplyContent(const QByteArray &value);

    /// Seconds to keep listening *after* first response for a callback with token content
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout NOTIFY timeoutChanged)
    int timeout();
    void setTimeout(int timeout);

    /// Maximum number of callback tries to accept, in case some don't have token content (favicons, etc.)
    Q_PROPERTY(int callbackTries READ callbackTries WRITE setCallbackTries NOTIFY callbackTriesChanged)
    int callbackTries();
    void setCallbackTries(int maxtries);

    QString uniqueState();
    void setUniqueState(const QString &state);

Q_SIGNALS:
    void verificationReceived(QMap<QString, QString>);
    void serverClosed(bool); // whether it has found parameters
    void replyContentChanged();
    void timeoutChanged(int timeout);
    void callbackTriesChanged(int maxtries);

public Q_SLOTS:
    void onIncomingConnection();
    void onBytesReady();
    QMap<QString, QString> parseQueryParams(QByteArray *data);
    void closeServer();
    void closeServer(QTcpSocket *socket, bool hasparameters = false);

protected:
    QByteArray replyContent_;
    int timeout_{15};
    int maxtries_{3};
    int tries_{0};
    QString uniqueState_;
};

#endif // O2REPLYSERVER_H
