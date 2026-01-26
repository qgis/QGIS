#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QPair>
#include <QTimer>
#include <QStringList>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#include "o0globals.h"
#include "o2replyserver.h"
#include "o0baseauth.h"

O2ReplyServer::O2ReplyServer(QObject *parent): QTcpServer(parent) {
    O0BaseAuth::log( QStringLiteral( "O2ReplyServer: Starting" ) );
    connect(this, &QTcpServer::newConnection, this, &O2ReplyServer::onIncomingConnection);
    replyContent_ = "<HTML></HTML>";
}

void O2ReplyServer::onIncomingConnection() {
    O0BaseAuth::log( QStringLiteral( "O2ReplyServer::onIncomingConnection: Receiving..." ) );
    QTcpSocket *socket = nextPendingConnection();
    connect(socket, &QIODevice::readyRead, this, &O2ReplyServer::onBytesReady, Qt::UniqueConnection);
    connect(socket, &QAbstractSocket::disconnected, socket, &QObject::deleteLater);

    // Wait for a bit *after* first response, then close server if no useable data has arrived
    // Helps with implicit flow, where a URL fragment may need processed by local user-agent and
    // sent as secondary query string callback, or additional requests make it through first,
    // like for favicons, etc., before such secondary callbacks are fired
    QTimer *timer = new QTimer(socket);
    timer->setObjectName("timeoutTimer");
    connect(timer, &QTimer::timeout, this, qOverload<>(&O2ReplyServer::closeServer));
    timer->setSingleShot(true);
    timer->setInterval(timeout() * 1000);
    connect(socket, &QTcpSocket::readyRead, timer, qOverload<>(&QTimer::start));
}

void O2ReplyServer::onBytesReady() {
    if (!isListening()) {
        // server has been closed, stop processing queued connections
        return;
    }
    O0BaseAuth::log( QStringLiteral( "O2ReplyServer::onBytesReady: Processing request" ) );
    // NOTE: on first call, the timeout timer is started
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        O0BaseAuth::log( QStringLiteral("O2ReplyServer::onBytesReady: No socket available"), O0BaseAuth::LogLevel::Warning );
        return;
    }
    QByteArray reply;
    reply.append("HTTP/1.0 200 OK \r\n");
    reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
    reply.append(QString("Content-Length: %1\r\n\r\n").arg(replyContent_.size()).toLatin1());
    reply.append(replyContent_);
    socket->write(reply);
    O0BaseAuth::log( QStringLiteral( "O2ReplyServer::onBytesReady: Sent reply" ) );

    QByteArray data = socket->readAll();
    QMap<QString, QString> queryParams = parseQueryParams(&data);
    if (queryParams.isEmpty()) {
        if (tries_ < maxtries_ ) {
            O0BaseAuth::log( QStringLiteral( "O2ReplyServer::onBytesReady: No query params found, waiting for more callbacks" ) );
            ++tries_;
            return;
        } else {
            tries_ = 0;
            O0BaseAuth::log( QStringLiteral("O2ReplyServer::onBytesReady: No query params found, maximum callbacks received"), O0BaseAuth::LogLevel::Warning );
            closeServer(socket, false);
            return;
        }
    }
    if (!uniqueState_.isEmpty() && !queryParams.contains(QString(O2_OAUTH2_STATE))) {
        O0BaseAuth::log( QStringLiteral( "O2ReplyServer::onBytesReady: Malicious or service request" ) );
        closeServer(socket, true);
        return; // Malicious or service (e.g. favicon.ico) request
    }
    O0BaseAuth::log( QStringLiteral( "O2ReplyServer::onBytesReady: Query params found, closing server" ) );
    closeServer(socket, true);
    Q_EMIT verificationReceived(queryParams);
}

QMap<QString, QString> O2ReplyServer::parseQueryParams(QByteArray *data) {
    O0BaseAuth::log( QStringLiteral( "O2ReplyServer::parseQueryParams" ) );
    O0BaseAuth::log( QStringLiteral("O2ReplyServer::parseQueryParams data:\n%1").arg(QString(*data)) );

    QString splitGetLine = QString(*data).split("\r\n").first();
    splitGetLine.remove("GET ");
    splitGetLine.remove("HTTP/1.1");
    splitGetLine.remove("\r\n");
    splitGetLine.prepend("http://localhost");
    QUrl getTokenUrl(splitGetLine);

    QList< QPair<QString, QString> > tokens;
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    tokens = getTokenUrl.queryItems();
#else
    QUrlQuery query(getTokenUrl);
    tokens = query.queryItems();
#endif
    QMap<QString, QString> queryParams;
    for (const QPair<QString, QString> &tokenPair: qAsConst(tokens)) {
        // FIXME: We are decoding key and value again. This helps with Google OAuth, but is it mandated by the standard?
        QString key = QUrl::fromPercentEncoding(QByteArray().append(tokenPair.first.trimmed().toLatin1()));
        QString value = QUrl::fromPercentEncoding(QByteArray().append(tokenPair.second.trimmed().toLatin1()));
        queryParams.insert(key, value);
    }
    return queryParams;
}

void O2ReplyServer::closeServer()
{
  closeServer( nullptr );
}

void O2ReplyServer::closeServer(QTcpSocket *socket, bool hasparameters)
{
  if (!isListening()) {
      return;
  }

  O0BaseAuth::log( QStringLiteral( "O2ReplyServer::closeServer: Initiating" ) );
  int port = serverPort();

  if (!socket && sender()) {
      QTimer *timer = qobject_cast<QTimer*>(sender());
      if (timer) {
          O0BaseAuth::log( QStringLiteral("O2ReplyServer::closeServer: Closing due to timeout"), O0BaseAuth::LogLevel::Warning );
          timer->stop();
          socket = qobject_cast<QTcpSocket *>(timer->parent());
          timer->deleteLater();
      }
  }
  if (socket) {
      QTimer *timer = socket->findChild<QTimer*>("timeoutTimer");
      if (timer) {
          O0BaseAuth::log( QStringLiteral( "O2ReplyServer::closeServer: Stopping socket's timeout timer" ) );
          timer->stop();
      }
      socket->disconnectFromHost();
  }
  close();
  O0BaseAuth::log( QStringLiteral( "O2ReplyServer::closeServer: Closed, no longer listening on port %1" ).arg( port ) );
  Q_EMIT serverClosed(hasparameters);
}

QByteArray O2ReplyServer::replyContent() {
    return replyContent_;
}

void O2ReplyServer::setReplyContent(const QByteArray &value) {
    if (replyContent_ == value)
        return;
    replyContent_ = value;
    Q_EMIT replyContentChanged();
}

int O2ReplyServer::timeout()
{
  return timeout_;
}

void O2ReplyServer::setTimeout(int timeout)
{
    if ( timeout_ == timeout )
        return;

    timeout_ = timeout;
    Q_EMIT timeoutChanged(timeout_);
}

int O2ReplyServer::callbackTries()
{
  return maxtries_;
}

void O2ReplyServer::setCallbackTries(int maxtries)
{
    if (maxtries_ == maxtries)
        return;

    maxtries_ = maxtries;
    Q_EMIT callbackTriesChanged(maxtries_);
}

QString O2ReplyServer::uniqueState()
{
    return uniqueState_;
}

void O2ReplyServer::setUniqueState(const QString &state)
{
    uniqueState_ = state;
}
