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
    explicit O2ReplyServer(QObject *parent = 0);

    /// Page content on local host after successful oauth - in case you do not want to close the browser, but display something
    Q_PROPERTY(QByteArray replyContent READ replyContent WRITE setReplyContent)
    QByteArray replyContent();
    void setReplyContent(const QByteArray &value);

    /// Seconds to keep listening *after* first response for a callback with token content
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    int timeout();
    void setTimeout(int timeout);

    /// Maximum number of callback tries to accept, in case some don't have token content (favicons, etc.)
    Q_PROPERTY(int callbackTries READ callbackTries WRITE setCallbackTries)
    int callbackTries();
    void setCallbackTries(int maxtries);

Q_SIGNALS:
    void verificationReceived(QMap<QString, QString>);
    void serverClosed(bool); // whether it has found parameters

public Q_SLOTS:
    void onIncomingConnection();
    void onBytesReady();
    QMap<QString, QString> parseQueryParams(QByteArray *data);
    void closeServer(QTcpSocket *socket = 0, bool hasparameters = false);

protected:
    QByteArray replyContent_;
    int timeout_;
    int maxtries_;
    int tries_;
};

#endif // O2REPLYSERVER_H
