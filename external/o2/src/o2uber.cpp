#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2uber.h"
#include "o0globals.h"

static const char *UberEndpoint = "https://login.uber.com/oauth/v2/authorize";
static const char *UberTokenUrl = "https://login.uber.com/oauth/v2/token";
static const char *UberExpiresIn = "expires_in";
static const char *UberGrantType = "authorization_code";

O2Uber::O2Uber(QObject *parent): O2(parent)
{
    setRequestUrl(UberEndpoint);
    setTokenUrl(UberTokenUrl);
}

void O2Uber::onVerificationReceived(const QMap<QString, QString> response){

    qDebug() << "O2Uber::onVerificationReceived: Emitting closeBrowser()";
    Q_EMIT closeBrowser();

    if (response.contains("error")) {
        qWarning() << "O2Uber::onVerificationReceived: Verification failed";
        for (auto it=response.constBegin(); it!=response.constEnd(); ++it) {
            qWarning() << "O2Uber::onVerificationReceived:" << it.key() << it.value();
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
    url.addQueryItem(O2_OAUTH2_GRANT_TYPE, UberGrantType);
    url.addQueryItem(O2_OAUTH2_REDIRECT_URI, redirectUri_);
    url.addQueryItem(O2_OAUTH2_GRANT_TYPE_CODE, code());
    url.addQueryItem(O2_OAUTH2_SCOPE, scope_);
#else
    QUrlQuery query(url);
    query.addQueryItem(O2_OAUTH2_CLIENT_ID, clientId_);
    query.addQueryItem(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    query.addQueryItem(O2_OAUTH2_GRANT_TYPE, UberGrantType);
    query.addQueryItem(O2_OAUTH2_REDIRECT_URI, redirectUri_);
    query.addQueryItem(O2_OAUTH2_GRANT_TYPE_CODE, code());
    query.addQueryItem(O2_OAUTH2_SCOPE, scope_);
    url.setQuery(query);
#endif

    QNetworkRequest tokenRequest(url);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *tokenReply = manager_->post(tokenRequest, QByteArray());
    timedReplies_.add(tokenReply);
    connect(tokenReply, &QNetworkReply::finished, this, &O2Uber::onTokenReplyFinished, Qt::QueuedConnection);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(tokenReply, &QNetworkReply::errorOccurred, this, &O2Uber::onTokenReplyError, Qt::QueuedConnection);
#endif
}

void O2Uber::onTokenReplyFinished(){
    qDebug() << "O2Uber::onTokenReplyFinished";

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
        setExpires(reply.value(UberExpiresIn).toInt());
        setRefreshToken(reply.value(O2_OAUTH2_REFRESH_TOKEN, QString()).toString());
        setExtraTokens(reply);
        timedReplies_.remove(tokenReply);
        setLinked(true);
        Q_EMIT linkingSucceeded();
    } else {
        qWarning() << "O2Uber::onTokenReplyFinished:" << tokenReply->errorString();
    }
}
