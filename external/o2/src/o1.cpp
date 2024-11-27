#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>
#include <QStringList>
#include <algorithm>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,1,0)
#include <QMessageAuthenticationCode>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
#include <QRandomGenerator>
#endif


#include "o1.h"
#include "o2replyserver.h"
#include "o0globals.h"
#include "o0settingsstore.h"

O1::O1(QObject *parent, QNetworkAccessManager *manager, O0AbstractStore *store): O0BaseAuth(parent, store) {
    setSignatureMethod(O2_SIGNATURE_TYPE_HMAC_SHA256);
    manager_ = manager ? manager : new QNetworkAccessManager(this);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");

    setCallbackUrl(O2_CALLBACK_URL);
}

QByteArray O1::userAgent() const {
    return userAgent_;
}

void O1::setUserAgent(const QByteArray &value) {
    if (userAgent_ == value)
        return;

    userAgent_ = value;
    Q_EMIT userAgentChanged(userAgent_);
}

QUrl O1::requestTokenUrl() {
    return requestTokenUrl_;
}

void O1::setRequestTokenUrl(const QUrl &value) {
    requestTokenUrl_ = value;
    Q_EMIT requestTokenUrlChanged();
}

QList<O0RequestParameter> O1::requestParameters() {
    return requestParameters_;
}

void O1::setRequestParameters(const QList<O0RequestParameter> &value) {
    requestParameters_ = value;
    Q_EMIT requestParametersChanged();
}

QString O1::callbackUrl() {
    return callbackUrl_;
}

void O1::setCallbackUrl(const QString &value) {
    if ( callbackUrl_ == value )
        return;

    callbackUrl_ = value;
    Q_EMIT callbackUrlChanged(callbackUrl_);
}

QUrl O1::authorizeUrl() {
    return authorizeUrl_;
}

void O1::setAuthorizeUrl(const QUrl &value) {
    authorizeUrl_ = value;
    Q_EMIT authorizeUrlChanged();
}

QUrl O1::accessTokenUrl() {
    return accessTokenUrl_;
}

void O1::setAccessTokenUrl(const QUrl &value) {
    accessTokenUrl_ = value;
    Q_EMIT accessTokenUrlChanged();
}

QString O1::signatureMethod() {
    return signatureMethod_;
}

void O1::setSignatureMethod(const QString &value) {
    log( QStringLiteral( "O1::setSignatureMethod: %1" ).arg( value ) );
    signatureMethod_ = value;
}

void O1::unlink() {
    log( QStringLiteral( "O1::unlink" ) );
    setLinked(false);
    setToken("");
    setTokenSecret("");
    setExtraTokens(QVariantMap());
    Q_EMIT linkingSucceeded();
}

#if QT_VERSION < QT_VERSION_CHECK(5,1,0)
/// Calculate the HMAC variant of SHA1 hash.
/// @author     http://qt-project.org/wiki/HMAC-SHA1.
/// @copyright  Creative Commons Attribution-ShareAlike 2.5 Generic.
static QByteArray hmacSha1(QByteArray key, QByteArray baseString) {
    int blockSize = 64;
    if (key.length() > blockSize) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }
    QByteArray innerPadding(blockSize, char(0x36));
    QByteArray outerPadding(blockSize, char(0x5c));
    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i);
        outerPadding[i] = outerPadding[i] ^ key.at(i);
    }
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}
#endif

/// Get HTTP operation name.
static QString getOperationName(QNetworkAccessManager::Operation op) {
    switch (op) {
    case QNetworkAccessManager::GetOperation: return "GET";
    case QNetworkAccessManager::PostOperation: return "POST";
    case QNetworkAccessManager::PutOperation: return "PUT";
    case QNetworkAccessManager::DeleteOperation: return "DEL";
    default: return "";
    }
}

/// Build a concatenated/percent-encoded string from a list of headers.
QByteArray O1::encodeHeaders(const QList<O0RequestParameter> &headers) {
    return QUrl::toPercentEncoding(createQueryParameters(headers));
}

/// Build a base string for signing.
QByteArray O1::getRequestBase(const QList<O0RequestParameter> &oauthParams, const QList<O0RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op) {
    QByteArray base;

    // Initialize base string with the operation name (e.g. "GET") and the base URL
    base.append(getOperationName(op).toUtf8() + "&");
    base.append(QUrl::toPercentEncoding(url.toString(QUrl::RemoveQuery)) + "&");

    // Append a sorted+encoded list of all request parameters to the base string
    QList<O0RequestParameter> headers(oauthParams);
    headers.append(otherParams);
    std::sort(headers.begin(), headers.end());
    base.append(encodeHeaders(headers));

    return base;
}

QByteArray O1::sign(const QList<O0RequestParameter> &oauthParams, const QList<O0RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret) {
    QByteArray baseString = getRequestBase(oauthParams, otherParams, url, op);
    QByteArray secret = QUrl::toPercentEncoding(consumerSecret) + "&" + QUrl::toPercentEncoding(tokenSecret);
#if QT_VERSION >= QT_VERSION_CHECK(5,1,0)
    return QMessageAuthenticationCode::hash(baseString, secret, QCryptographicHash::Sha256).toBase64();
#else
    return hmacSha1(secret, baseString);
#endif
}

QByteArray O1::buildAuthorizationHeader(const QList<O0RequestParameter> &oauthParams) {
    bool first = true;
    QByteArray ret("OAuth ");
    QList<O0RequestParameter> headers(oauthParams);
    std::sort(headers.begin(), headers.end());
    for (const O0RequestParameter &h: headers) {
        if (first) {
            first = false;
        } else {
            ret.append(",");
        }
        ret.append(h.name);
        ret.append("=\"");
        ret.append(QUrl::toPercentEncoding(h.value));
        ret.append("\"");
    }
    return ret;
}

void O1::decorateRequest(QNetworkRequest &req, const QList<O0RequestParameter> &oauthParams) {
    req.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(oauthParams));
    if (!userAgent_.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        req.setHeader(QNetworkRequest::UserAgentHeader, userAgent_);
#else
        req.setRawHeader("User-Agent", userAgent_);
#endif
    }
}

QByteArray O1::generateSignature(const QList<O0RequestParameter> headers, const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation) {
    QByteArray signature;
    if (signatureMethod() == O2_SIGNATURE_TYPE_HMAC_SHA256) {
        signature = sign(headers, signingParameters, req.url(), operation, clientSecret(), tokenSecret());
    } else if (signatureMethod() == O2_SIGNATURE_TYPE_PLAINTEXT) {
        signature = clientSecret().toLatin1() + "&" + tokenSecret().toLatin1();
    }
    return signature;
}

void O1::link() {
    log( QStringLiteral( "O1::link" ) );

    // Create the reply server if it doesn't exist
    // and we don't use an external web interceptor
    if(!useExternalWebInterceptor_) {
        if(replyServer() == nullptr) {
            O2ReplyServer * replyServer = new O2ReplyServer(this);
            connect(replyServer, &O2ReplyServer::verificationReceived,
                    this, &O1::onVerificationReceived
                    );
            setReplyServer(replyServer);
        }
    }

    if (linked()) {
        qDebug() << "O1::link: Linked already";
        Q_EMIT linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken("");
    setTokenSecret("");
    setExtraTokens(QVariantMap());

    if (!useExternalWebInterceptor_) {
        // Start reply server
        if (!replyServer()->isListening())
            replyServer()->listen(QHostAddress::Any, localPort());
    }

    // Get any query parameters for the request
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery requestData;
#else
    QUrl requestData = requestTokenUrl();
#endif
    const QList<O0RequestParameter> parameters = requestParameters();
    for(const O0RequestParameter& param :parameters)
      requestData.addQueryItem(QString(param.name), QUrl::toPercentEncoding(QString(param.value)));

    // Get the request url and add parameters
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrl requestUrl = requestTokenUrl();
    requestUrl.setQuery(requestData);
    // Create request
    QNetworkRequest request(requestUrl);
#else
    // Create request
    QNetworkRequest request(requestData);
#endif

    // Create initial token request
    QList<O0RequestParameter> headers;
    headers.append(O0RequestParameter(O2_OAUTH_CALLBACK, callbackUrl().arg(localPort()).toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_NONCE, nonce()));
#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
    headers.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentSecsSinceEpoch()).toLatin1()));
#else
    headers.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
#endif
    headers.append(O0RequestParameter(O2_OAUTH_VERSION, "1.0"));
    headers.append(O0RequestParameter(O2_OAUTH_SIGNATURE_METHOD, signatureMethod().toLatin1()));
    headers.append(O0RequestParameter(O2_OAUTH_SIGNATURE, generateSignature(headers, request, requestParameters(), QNetworkAccessManager::PostOperation)));
    log( QStringLiteral( "O1:link: Token request headers:" ) );
    for(const O0RequestParameter &param: qAsConst(headers)) {
        log( QStringLiteral( "  %1=%2" ).arg( QString( param.name ), QString( param.value ) ) );
    }

    // Clear request token
    requestToken_.clear();
    requestTokenSecret_.clear();

    // Post request
    decorateRequest(request, headers);
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QNetworkReply *reply = manager_->post(request, QByteArray());
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenRequestError(QNetworkReply::NetworkError)));
#else
    connect(reply, &QNetworkReply::errorOccurred, this, &O1::onTokenRequestError);
#endif
    connect(reply, &QNetworkReply::finished, this, &O1::onTokenRequestFinished);
}

void O1::onTokenRequestError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O1::onTokenRequestError:" << (int)error << reply->errorString() << reply->readAll();
    Q_EMIT linkingFailed();
}

void O1::onTokenRequestFinished() {
    qDebug() << "O1::onTokenRequestFinished";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << QString( "Request: %1" ).arg(reply->request().url().toString());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "O1::onTokenRequestFinished: " << reply->errorString();
        return;
    }

    // Get request token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    requestToken_ = response.value(O2_OAUTH_TOKEN, "");
    requestTokenSecret_ = response.value(O2_OAUTH_TOKEN_SECRET, "");
    setToken(requestToken_);
    setTokenSecret(requestTokenSecret_);

    // Checking for "oauth_callback_confirmed" is present and set to true
    QString oAuthCbConfirmed = response.value(O2_OAUTH_CALLBACK_CONFIRMED, "false");
    if (requestToken_.isEmpty() || requestTokenSecret_.isEmpty() || (oAuthCbConfirmed == "false")) {
        qWarning() << "O1::onTokenRequestFinished: No oauth_token, oauth_token_secret or oauth_callback_confirmed in response :" << data;
        Q_EMIT linkingFailed();
        return;
    }

    // Continue authorization flow in the browser
    QUrl url(authorizeUrl());
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    url.addQueryItem(O2_OAUTH_TOKEN, requestToken_);
    url.addQueryItem(O2_OAUTH_CALLBACK, callbackUrl().arg(localPort()).toLatin1());
#else
    QUrlQuery query(url);
    query.addQueryItem(O2_OAUTH_TOKEN, requestToken_);
    query.addQueryItem(O2_OAUTH_CALLBACK, callbackUrl().arg(localPort()).toLatin1());
    url.setQuery(query);
#endif
    Q_EMIT openBrowser(url);
}

void O1::onVerificationReceived(QMap<QString, QString> params) {
    qDebug() << "O1::onVerificationReceived";
    Q_EMIT closeBrowser();
    verifier_ = params.value(O2_OAUTH_VERFIER, "");
    if (params.value(O2_OAUTH_TOKEN) == requestToken_) {
        // Exchange request token for access token
        exchangeToken();
    } else {
        qWarning() << "O1::onVerificationReceived: oauth_token missing or doesn't match";
        Q_EMIT linkingFailed();
    }
}

void O1::exchangeToken() {
    qDebug() << "O1::exchangeToken";

    // Create token exchange request
    QNetworkRequest request(accessTokenUrl());
    QList<O0RequestParameter> oauthParams;
    oauthParams.append(O0RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERSION, "1.0"));
#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
    oauthParams.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentSecsSinceEpoch()).toLatin1()));
#else
    oauthParams.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
#endif
    oauthParams.append(O0RequestParameter(O2_OAUTH_NONCE, nonce()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_TOKEN, requestToken_.toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERFIER, verifier_.toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE_METHOD, signatureMethod().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE, generateSignature(oauthParams, request, QList<O0RequestParameter>(), QNetworkAccessManager::PostOperation)));

    // Post request
    decorateRequest(request, oauthParams);
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QNetworkReply *reply = manager_->post(request, QByteArray());
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenExchangeError(QNetworkReply::NetworkError)));
#else
    connect(reply, &QNetworkReply::errorOccurred, this, &O1::onTokenExchangeError);
#endif
    connect(reply, &QNetworkReply::finished, this, &O1::onTokenExchangeFinished);
}

void O1::onTokenExchangeError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O1::onTokenExchangeError:" << (int)error << reply->errorString() << reply->readAll();
    Q_EMIT linkingFailed();
}

void O1::onTokenExchangeFinished() {
    qDebug() << "O1::onTokenExchangeFinished";

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "O1::onTokenExchangeFinished: " << reply->errorString();
        return;
    }

    // Get access token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    if (response.contains(O2_OAUTH_TOKEN) && response.contains(O2_OAUTH_TOKEN_SECRET)) {
        setToken(response.take(O2_OAUTH_TOKEN));
        setTokenSecret(response.take(O2_OAUTH_TOKEN_SECRET));
        // Set extra tokens if any
        if (!response.isEmpty()) {
            QVariantMap extraTokens;
            for (auto it = response.constBegin(); it != response.constEnd(); ++it) {
                extraTokens.insert(it.key(), it.value());
            }
            setExtraTokens(extraTokens);
        }
        setLinked(true);
        Q_EMIT linkingSucceeded();
    } else {
        qWarning() << "O1::onTokenExchangeFinished: oauth_token or oauth_token_secret missing from response" << data;
        Q_EMIT linkingFailed();
    }
}

QMap<QString, QString> O1::parseResponse(const QByteArray &response) {
    QMap<QString, QString> ret;
    const QList<QByteArray> params = response.split('&');
    for (const QByteArray &param: params) {
        QList<QByteArray> kv = param.split('=');
        if (kv.length() == 2) {
            ret.insert(QUrl::fromPercentEncoding(kv[0]), QUrl::fromPercentEncoding(kv[1]));
        }
    }
    return ret;
}

QByteArray O1::nonce() {
#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
    QString u = QString::number(QDateTime::currentSecsSinceEpoch()).toLatin1();
#else
    QString u = QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1();
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    u.append(QString::number(QRandomGenerator::global()->generate()));
#else
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        qsrand(QTime::currentTime().msec());
    }
    u.append(QString::number(qrand()));
#endif
    return u.toLatin1();
}
