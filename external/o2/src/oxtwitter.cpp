#include <QDateTime>
#include <QDebug>

#include "oxtwitter.h"
#include "o0globals.h"

const char XAUTH_USERNAME[] = "x_auth_username";
const char XAUTH_PASSWORD[] = "x_auth_password";
const char XAUTH_MODE[] = "x_auth_mode";
const char XAUTH_MODE_VALUE[] = "client_auth";

OXTwitter::OXTwitter(QObject *parent): O1Twitter(parent) {
}

QString OXTwitter::username() {
    return username_;
}

void OXTwitter::setUsername(const QString &username) {
    username_ = username;
    Q_EMIT usernameChanged();
}

QString OXTwitter::password() {
    return password_;
}

void OXTwitter::setPassword(const QString &password) {
    password_ = password;
    Q_EMIT passwordChanged();
}

void OXTwitter::link() {
    qDebug() << "OXTwitter::link";
    if (linked()) {
        qDebug() << "Linked already";
        return;
    }

    if (username_.isEmpty() || password_.isEmpty()) {
        qWarning() << "Error: XAuth parameters not set. Aborting!";
        return;
    }

    // prepare XAuth parameters
    xAuthParams_.append(O0RequestParameter(QByteArray(XAUTH_USERNAME), username_.toLatin1()));
    xAuthParams_.append(O0RequestParameter(QByteArray(XAUTH_PASSWORD), password_.toLatin1()));
    xAuthParams_.append(O0RequestParameter(QByteArray(XAUTH_MODE), QByteArray(XAUTH_MODE_VALUE)));

    QList<O0RequestParameter> oauthParams;
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE_METHOD, O2_SIGNATURE_TYPE_HMAC_SHA1));
    oauthParams.append(O0RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERSION, "1.0"));
    oauthParams.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_NONCE, nonce()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_TOKEN, QByteArray("")));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERFIER, QByteArray("")));

    QByteArray signature = sign(oauthParams, xAuthParams_, accessTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE, signature));

    // Post request
    QNetworkRequest request(accessTokenUrl());
    request.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(oauthParams));
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QNetworkReply *reply = manager_->post(request, createQueryParameters(xAuthParams_));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenExchangeError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onTokenExchangeFinished()));
}
