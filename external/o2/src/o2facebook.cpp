#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#include "o2facebook.h"
#include "o0globals.h"

static const char *FbEndpoint = "https://graph.facebook.com/oauth/authorize?display=touch";
static const char *FbTokenUrl = "https://graph.facebook.com/oauth/access_token";
static const char *FbExpiresKey = "expires_in";

O2Facebook::O2Facebook(QObject *parent): O2(parent) {
    setRequestUrl(FbEndpoint);
    setTokenUrl(FbTokenUrl);
}

void O2Facebook::onVerificationReceived(const QMap<QString, QString> response) {
    qDebug() << "O2Facebook::onVerificationReceived: Emitting closeBrowser()";
    Q_EMIT closeBrowser();

    if (response.contains("error")) {
        qWarning() << "O2Facebook::onVerificationReceived: Verification failed";
        for (auto it = response.constBegin(); it != response.constEnd(); ++it) {
            qWarning() << "O2Facebook::onVerificationReceived:" << it.key() << it.value();
        }
        Q_EMIT linkingFailed();
        return;
    }

    // Save access code
    setCode(response.value(O2_OAUTH2_GRANT_TYPE_CODE));

    // Exchange access code for access/refresh tokens
    QUrl url(tokenUrl_);
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
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
    connect(tokenReply, &QNetworkReply::finished, this, &O2Facebook::onTokenReplyFinished, Qt::QueuedConnection);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(tokenReply, &QNetworkReply::errorOccurred, this, &O2Facebook::onTokenReplyError, Qt::QueuedConnection);
#endif
}

void O2Facebook::onTokenReplyFinished() {
    qDebug() << "O2Facebook::onTokenReplyFinished";

    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        // Process reply
        QByteArray replyData = tokenReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(replyData);
        const QJsonObject rootObject = doc.object();

        QVariantMap reply;
        const QStringList keys = rootObject.keys();
        for (const QString &key : keys) {
            reply.insert(key, rootObject[key].toVariant());
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
