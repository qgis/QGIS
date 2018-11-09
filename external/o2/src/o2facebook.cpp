#include <QDebug>
#include <QMap>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2facebook.h"
#include "o0globals.h"

static const char *FbEndpoint = "https://graph.facebook.com/oauth/authorize?display=touch";
static const char *FbTokenUrl = "https://graph.facebook.com/oauth/access_token";
static const char *FbExpiresKey = "expires";

O2Facebook::O2Facebook(QObject *parent): O2(parent) {
    setRequestUrl(FbEndpoint);
    setTokenUrl(FbTokenUrl);
}

void O2Facebook::onVerificationReceived(const QMap<QString, QString> response) {
    qDebug() << "O2Facebook::onVerificationReceived: Emitting closeBrowser()";
    Q_EMIT closeBrowser();

    if (response.contains("error")) {
        qWarning() << "O2Facebook::onVerificationReceived: Verification failed";
        foreach (QString key, response.keys()) {
            qWarning() << "O2Facebook::onVerificationReceived:" << key << response.value(key);
        }
        Q_EMIT linkingFailed();
        return;
    }

    // Save access code
    setCode(response.value(O2_OAUTH2_GRANT_TYPE_CODE));

    // Exchange access code for access/refresh tokens
    QUrl url(tokenUrl_);
#if QT_VERSION < 0x050000
    url.addQueryItem(O2_OAUTH2_CLIENT_ID, clientId_);
    url.addQueryItem(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    url.addQueryItem(O2_OAUTH2_SCOPE, scope_);
    url.addQueryItem(O2_OAUTH2_GRANT_TYPE_CODE, code());
    url.addQueryItem(O2_OAUTH2_REDIRECT_URI, redirectUri_);
#else
    QUrlQuery query(url);
    query.addQueryItem(O2_OAUTH2_CLIENT_ID, clientId_);
    query.addQueryItem(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    query.addQueryItem(O2_OAUTH2_SCOPE, scope_);
    query.addQueryItem(O2_OAUTH2_GRANT_TYPE_CODE, code());
    query.addQueryItem(O2_OAUTH2_REDIRECT_URI, redirectUri_);
    url.setQuery(query);
#endif

    QNetworkRequest tokenRequest(url);
    QNetworkReply *tokenReply = manager_->get(tokenRequest);
    timedReplies_.add(tokenReply);
    connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
    connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void O2Facebook::onTokenReplyFinished() {
    qDebug() << "O2Facebook::onTokenReplyFinished";

    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        // Process reply
        QByteArray replyData = tokenReply->readAll();
        QVariantMap reply;
        foreach (QString pair, QString(replyData).split("&")) {
            QStringList kv = pair.split("=");
            if (kv.length() == 2) {
                reply.insert(kv[0], kv[1]);
            }
        }

        // Interpret reply
        setToken(reply.value(O2_OAUTH2_ACCESS_TOKEN, QString()).toString());
        setExpires(reply.value(FbExpiresKey).toInt());
        setRefreshToken(reply.value(O2_OAUTH2_REFRESH_TOKEN, QString()).toString());
        setExtraTokens(reply);
        timedReplies_.remove(tokenReply);
        setLinked(true);
        Q_EMIT linkingSucceeded();
    } else {
        qWarning() << "O2Facebook::onTokenReplyFinished:" << tokenReply->errorString();
    }
}
