#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "o2pollserver.h"
#include "o0jsonresponse.h"
#include "o0baseauth.h"

static QMap<QString, QString> toVerificationParams(const QVariantMap &map)
{
    QMap<QString, QString> params;
    for (QVariantMap::const_iterator i = map.constBegin();
         i != map.constEnd(); ++i)
    {
        params[i.key()] = i.value().toString();
    }
    return params;
}

O2PollServer::O2PollServer(QNetworkAccessManager *manager, const QNetworkRequest &request, const QByteArray &payload, int expiresIn, QObject *parent)
    : QObject(parent)
    , manager_(manager)
    , request_(request)
    , payload_(payload)
    , expiresIn_(expiresIn)
{
    expirationTimer.setTimerType(Qt::VeryCoarseTimer);
    expirationTimer.setInterval(expiresIn * 1000);
    expirationTimer.setSingleShot(true);
    connect(&expirationTimer, &QTimer::timeout, this, &O2PollServer::onExpiration);
    expirationTimer.start();

    pollTimer.setTimerType(Qt::VeryCoarseTimer);
    pollTimer.setInterval(5 * 1000);
    pollTimer.setSingleShot(true);
    connect(&pollTimer, &QTimer::timeout, this, &O2PollServer::onPollTimeout);
}

int O2PollServer::interval() const
{
    return pollTimer.interval() / 1000;
}

void O2PollServer::setInterval(int interval)
{
    pollTimer.setInterval(interval * 1000);
    Q_EMIT intervalChanged(interval);
}

void O2PollServer::startPolling()
{
    if (expirationTimer.isActive()) {
        pollTimer.start();
    }
}

void O2PollServer::onPollTimeout()
{
    O0BaseAuth::log( QStringLiteral( "O2PollServer::onPollTimeout: retrying" ) );
    QNetworkReply * reply = manager_->post(request_, payload_);
    connect(reply, &QNetworkReply::finished, this, &O2PollServer::onReplyFinished);
}

void O2PollServer::onExpiration()
{
    pollTimer.stop();
    Q_EMIT serverClosed(false);
}

void O2PollServer::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if (!reply) {
        O0BaseAuth::log( QStringLiteral( "O2PollServer::onReplyFinished: reply is null" ) );
        return;
    }

    QByteArray replyData = reply->readAll();
    QMap<QString, QString> params = toVerificationParams(parseJsonResponse(replyData));

    // Dump replyData
    // SENSITIVE DATA in RelWithDebInfo or Debug builds
    // qDebug() << "O2PollServer::onReplyFinished: replyData\n";
    // qDebug() << QString( replyData );

    if (reply->error() == QNetworkReply::TimeoutError) {
        // rfc8628#section-3.2
        // "On encountering a connection timeout, clients MUST unilaterally
        // reduce their polling frequency before retrying.  The use of an
        // exponential backoff algorithm to achieve this, such as doubling the
        // polling interval on each such connection timeout, is RECOMMENDED."
        setInterval(interval() * 2);
        pollTimer.start();
    }
    else {
        QString error = params.value("error");
        if (error == "slow_down") {
            // rfc8628#section-3.2
            // "A variant of 'authorization_pending', the authorization request is
            // still pending and polling should continue, but the interval MUST
            // be increased by 5 seconds for this and all subsequent requests."
            setInterval(interval() + 5);
            pollTimer.start();
        }
        else if (error == "authorization_pending") {
            // keep trying - rfc8628#section-3.2
            // "The authorization request is still pending as the end user hasn't
            // yet completed the user-interaction steps (Section 3.3)."
            pollTimer.start();
        }
        else {
            expirationTimer.stop();
            Q_EMIT serverClosed(true);
            // let O2 handle the other cases
            Q_EMIT verificationReceived(params);
        }
    }
    reply->deleteLater();
}
