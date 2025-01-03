#include <QList>
#include <QPair>
#include <QTcpServer>
#include <QMap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QCryptographicHash>
#include <QTimer>
#include <QVariantMap>
#include <QUuid>
#include <QUrlQuery>
#include <QRegularExpression>

#include "o2.h"
#include "o2pollserver.h"
#include "o2replyserver.h"
#include "o0globals.h"
#include "o0jsonresponse.h"
#include "o0settingsstore.h"

/// Add query parameters to a query
static void addQueryParametersToUrl(QUrl &url,  QList<QPair<QString, QString> > parameters) {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    url.setQueryItems(parameters);
#else
    QUrlQuery query(url);
    query.setQueryItems(parameters);
    url.setQuery(query);
#endif
}

// ref: https://tools.ietf.org/html/rfc8628#section-3.2
// Exception: Google sign-in uses "verification_url" instead of "*_uri" - we'll accept both.
static bool hasMandatoryDeviceAuthParams(const QVariantMap& params)
{
    if (!params.contains(O2_OAUTH2_DEVICE_CODE))
        return false;

    if (!params.contains(O2_OAUTH2_USER_CODE))
        return false;

    if (!(params.contains(O2_OAUTH2_VERIFICATION_URI) || params.contains(O2_OAUTH2_VERIFICATION_URL)))
        return false;

    if (!params.contains(O2_OAUTH2_EXPIRES_IN))
        return false;

    return true;
}

O2::O2(QObject *parent, QNetworkAccessManager *manager, O0AbstractStore *store): O0BaseAuth(parent, store) {
    manager_ = manager ? manager : new QNetworkAccessManager(this);
    grantFlow_ = GrantFlowAuthorizationCode;
    localhostPolicy_ = QString(O2_CALLBACK_URL);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
}

O2::GrantFlow O2::grantFlow() {
    return grantFlow_;
}

void O2::setGrantFlow(O2::GrantFlow value) {
    grantFlow_ = value;
    Q_EMIT grantFlowChanged();
}

QString O2::username() {
    return username_;
}

void O2::setUsername(const QString &value) {
    username_ = value;
    Q_EMIT usernameChanged();
}

QString O2::password() {
    return password_;
}

void O2::setPassword(const QString &value) {
    password_ = value;
    Q_EMIT passwordChanged();
}

QString O2::scope() {
    return scope_;
}

void O2::setScope(const QString &value) {
    scope_ = value;
    Q_EMIT scopeChanged();
}

QString O2::requestUrl() {
    return requestUrl_.toString();
}

void O2::setRequestUrl(const QString &value) {
    requestUrl_ = value;
    Q_EMIT requestUrlChanged();
}

QVariantMap O2::extraRequestParams()
{
  return extraReqParams_;
}

void O2::setExtraRequestParams(const QVariantMap &value)
{
  extraReqParams_ = value;
  Q_EMIT extraRequestParamsChanged();
}

QString O2::tokenUrl() {
    return tokenUrl_.toString();
}

void O2::setTokenUrl(const QString &value) {
    tokenUrl_= value;
    Q_EMIT tokenUrlChanged();
}

QString O2::refreshTokenUrl() {
    return refreshTokenUrl_.toString();
}

void O2::setRefreshTokenUrl(const QString &value) {
    refreshTokenUrl_ = value;
    Q_EMIT refreshTokenUrlChanged();
}

QString O2::grantType()
{
    if (!grantType_.isEmpty())
        return grantType_;

    switch (grantFlow_) {
    case GrantFlowAuthorizationCode:
        return O2_OAUTH2_GRANT_TYPE_CODE;
    case GrantFlowImplicit:
        return O2_OAUTH2_GRANT_TYPE_TOKEN;
    case GrantFlowResourceOwnerPasswordCredentials:
        return O2_OAUTH2_GRANT_TYPE_PASSWORD;
    case GrantFlowDevice:
        return O2_OAUTH2_GRANT_TYPE_DEVICE;
    case GrantFlowPkce:
        return "pkce";
    }

    return QString();
}

void O2::setGrantType(const QString &value)
{
    if (grantType_ == value)
        return;
    grantType_ = value;
    Q_EMIT grantTypeChanged(grantType_);
}

void O2::link() {
    log( QStringLiteral( "O2::link" ) );

    // Create the reply server if it doesn't exist
    // and we don't use an external web interceptor
    if(!useExternalWebInterceptor_) {
        if(replyServer() == nullptr) {
            O2ReplyServer * replyServer = new O2ReplyServer(this);
            connect(replyServer, &O2ReplyServer::verificationReceived, this, &O2::onVerificationReceived);
            connect(replyServer, &O2ReplyServer::serverClosed, this, &O2::serverHasClosed);
            setReplyServer(replyServer);
        }
    }

    if (linked()) {
        log( QStringLiteral( "O2::link: Linked already" ) );
        Q_EMIT linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken("");
    setTokenSecret("");
    setExtraTokens(QVariantMap());
    setRefreshToken(QString());
    setExpires(0);

    if (grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowImplicit || grantFlow_ == GrantFlowPkce) {

        const thread_local QRegularExpression rx("([^a-zA-Z0-9]|[-])");
        QString uniqueState = QUuid::createUuid().toString().remove(rx);
        if (useExternalWebInterceptor_) {
            // Save redirect URI, as we have to reuse it when requesting the access token
            redirectUri_ = localhostPolicy_.arg(localPort());
        } else {
            // Start listening to authentication replies
            if (!replyServer()->isListening()) {
                if (replyServer()->listen(QHostAddress::Any, localPort_)) {
                    log( QStringLiteral( "O2::link: Reply server listening on port %1" ).arg( localPort() ) );
                } else {
                    log( QStringLiteral("O2::link: Reply server failed to start listening on port %1").arg( localPort() ), O0BaseAuth::LogLevel::Warning );
                    Q_EMIT linkingFailed();
                    return;
                }
            }

            // Save redirect URI, as we have to reuse it when requesting the access token
            redirectUri_ = localhostPolicy_.arg(replyServer()->serverPort());
            replyServer()->setUniqueState(uniqueState);
        }

        // Assemble intial authentication URL
        QList<QPair<QString, QString> > parameters;
        parameters.append(qMakePair(QString(O2_OAUTH2_RESPONSE_TYPE),
                                    (grantFlow_ == GrantFlowAuthorizationCode)? QString(O2_OAUTH2_GRANT_TYPE_CODE): QString(O2_OAUTH2_GRANT_TYPE_TOKEN)));
        parameters.append(qMakePair(QString(O2_OAUTH2_CLIENT_ID), clientId_));
        if ( !redirectUri_.isEmpty() )
            parameters.append(qMakePair(QString(O2_OAUTH2_REDIRECT_URI), redirectUri_));
        if ( !scope_.isEmpty() )
            parameters.append(qMakePair(QString(O2_OAUTH2_SCOPE), scope_.replace( " ", "+" )));
        parameters.append(qMakePair(QString(O2_OAUTH2_STATE), uniqueState));
        if ( !apiKey_.isEmpty() )
            parameters.append(qMakePair(QString(O2_OAUTH2_API_KEY), apiKey_));

        if ( grantFlow_ == GrantFlowPkce )
        {
            pkceCodeVerifier_ = ( QUuid::createUuid().toString( QUuid::WithoutBraces ) +
                                 QUuid::createUuid().toString( QUuid::WithoutBraces ) ).toLatin1();
            pkceCodeChallenge_ = QCryptographicHash::hash( pkceCodeVerifier_, QCryptographicHash::Sha256 ).toBase64(
                QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals );
            parameters.append( qMakePair( QString( O2_OAUTH2_PKCE_CODE_CHALLENGE_PARAM ), pkceCodeChallenge_ ) );
            parameters.append( qMakePair( QString( O2_OAUTH2_PKCE_CODE_CHALLENGE_METHOD_PARAM ), QString( O2_OAUTH2_PKCE_CODE_CHALLENGE_METHOD_S256 ) ) );
        }

        const QVariantMap extraParams = extraRequestParams();
        for (auto it = extraParams.constBegin(); it != extraParams.constEnd(); ++it) {
            parameters.append(qMakePair(it.key(), it.value().toString()));
        }
        // Show authentication URL with a web browser
        QUrl url(requestUrl_);
        addQueryParametersToUrl(url, parameters);
        log( QStringLiteral( "O2::link: Emit openBrowser %1" ).arg( url.toString() ) );
        Q_EMIT openBrowser(url);
    } else if (grantFlow_ == GrantFlowResourceOwnerPasswordCredentials) {
        QList<O0RequestParameter> parameters;
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_ID, clientId_.toUtf8()));
        if ( !clientSecret_.isEmpty() )
            parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_SECRET, clientSecret_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_USERNAME, username_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_PASSWORD, password_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_GRANT_TYPE_PASSWORD));
        parameters.append(O0RequestParameter(O2_OAUTH2_SCOPE, scope_.toUtf8()));
        if ( !apiKey_.isEmpty() )
            parameters.append(O0RequestParameter(O2_OAUTH2_API_KEY, apiKey_.toUtf8()));

        const QVariantMap extraParams = extraRequestParams();
        for (auto it = extraParams.constBegin(); it != extraParams.constEnd(); ++it) {
            parameters.append(O0RequestParameter(it.key().toUtf8(), it.value().toByteArray()));
        }
        QByteArray payload = O0BaseAuth::createQueryParameters(parameters);

        log( QStringLiteral( "O2::link: Sending token request for resource owner flow" ) );
        QUrl url(tokenUrl_);
        QNetworkRequest tokenRequest(url);
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *tokenReply = getManager()->post(tokenRequest, payload);

        connect(tokenReply,
                &QNetworkReply::finished,
                this,
                &O2::onTokenReplyFinished,
                Qt::QueuedConnection);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
        connect(tokenReply, &QNetworkReply::errorOccurred, this, &O2::onTokenReplyError, Qt::QueuedConnection);
#endif
    }
    else if (grantFlow_ == GrantFlowDevice) {
        QList<O0RequestParameter> parameters;
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_ID, clientId_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_SCOPE, scope_.toUtf8()));
        QByteArray payload = O0BaseAuth::createQueryParameters(parameters);

        QUrl url(requestUrl_);
        QNetworkRequest deviceRequest(url);
        deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *tokenReply = getManager()->post(deviceRequest, payload);

        connect(tokenReply,
                &QNetworkReply::finished,
                this,
                &O2::onDeviceAuthReplyFinished,
                Qt::QueuedConnection);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
        connect(tokenReply, &QNetworkReply::errorOccurred, this, &O2::onTokenReplyError, Qt::QueuedConnection);
#endif
    }
}

void O2::unlink() {
    log( QStringLiteral( "O2::unlink" ) );
    setLinked(false);
    setToken(QString());
    setRefreshToken(QString());
    setExpires(0);
    setExtraTokens(QVariantMap());
    Q_EMIT linkingSucceeded();
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    log( QStringLiteral( "O2::onVerificationReceived: Emitting closeBrowser()" ) );
    Q_EMIT closeBrowser();

    if (response.contains("error")) {
        log( QStringLiteral("O2::onVerificationReceived: Verification failed: %1").arg( response.value( "error") ), O0BaseAuth::LogLevel::Warning );
        Q_EMIT linkingFailed();
        return;
    }

    if (grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowPkce ) {
        // Save access code
        setCode(response.value(QString(O2_OAUTH2_GRANT_TYPE_CODE)));

        // Exchange access code for access/refresh tokens
        QString query;
        if(!apiKey_.isEmpty())
            query = QString("?" + QString(O2_OAUTH2_API_KEY) + "=" + apiKey_);
        QNetworkRequest tokenRequest(QUrl(tokenUrl_.toString() + query));
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
        tokenRequest.setRawHeader("Accept", O2_MIME_TYPE_JSON);
        QMap<QString, QString> parameters;
        parameters.insert(O2_OAUTH2_GRANT_TYPE_CODE, code());
        parameters.insert(O2_OAUTH2_CLIENT_ID, clientId_);
        //No client secret with PKCE
        if ( grantFlow_ != GrantFlowPkce )
        {
            parameters.insert(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
        }
        parameters.insert(O2_OAUTH2_REDIRECT_URI, redirectUri_);
        parameters.insert(O2_OAUTH2_GRANT_TYPE, O2_AUTHORIZATION_CODE);
        if ( grantFlow() == GrantFlowPkce )
        {
            parameters.insert( O2_OAUTH2_PKCE_CODE_VERIFIER_PARAM, pkceCodeVerifier_ );
        }
        QByteArray data = buildRequestBody(parameters);

        log( QStringLiteral("O2::onVerificationReceived: Exchange access code data:\n%1").arg(QString(data)) );

        QNetworkReply *tokenReply = getManager()->post(tokenRequest, data);
        timedReplies_.add(tokenReply);
        connect(tokenReply,
                &QNetworkReply::finished,
                this,
                &O2::onTokenReplyFinished,
                Qt::QueuedConnection);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
        connect(tokenReply, &QNetworkReply::errorOccurred, this, &O2::onTokenReplyError, Qt::QueuedConnection);
#endif
    } else if (grantFlow_ == GrantFlowImplicit || grantFlow_ == GrantFlowDevice) {
      // Check for mandatory tokens
      if (response.contains(O2_OAUTH2_ACCESS_TOKEN)) {
          log( QStringLiteral("O2::onVerificationReceived: Access token returned for implicit or device flow") );
          setToken(response.value(O2_OAUTH2_ACCESS_TOKEN));
          if (response.contains(O2_OAUTH2_EXPIRES_IN)) {
            bool ok = false;
            const int expiresIn = response.value(O2_OAUTH2_EXPIRES_IN).toInt(&ok);
            if (ok) {
                log( QStringLiteral("O2::onVerificationReceived: Token expires in %1 seconds" ).arg( expiresIn ) );
                setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + static_cast< qint64 >( expiresIn ));
            }
          }
          if (response.contains(O2_OAUTH2_REFRESH_TOKEN)) {
              setRefreshToken(response.value(O2_OAUTH2_REFRESH_TOKEN));
          }
          setLinked(true);
          Q_EMIT linkingSucceeded();
      } else {
          log( QStringLiteral("O2::onVerificationReceived: Access token missing from response for implicit or device flow"), O0BaseAuth::LogLevel::Warning );
          Q_EMIT linkingFailed();
      }
    } else {
        setToken(response.value(O2_OAUTH2_ACCESS_TOKEN));
        setRefreshToken(response.value(O2_OAUTH2_REFRESH_TOKEN));
    }
}

QString O2::code() {
    QString key = QString(O2_KEY_CODE).arg(clientId_);
    return store_->value(key);
}

void O2::setCode(const QString &c) {
    QString key = QString(O2_KEY_CODE).arg(clientId_);
    store_->setValue(key, c);
}

void O2::onTokenReplyFinished() {
    log( QStringLiteral("O2::onTokenReplyFinished") );
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (!tokenReply)
    {
        log( QStringLiteral("O2::onTokenReplyFinished: reply is null") );
        return;
    }
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();

        // Dump replyData
        // SENSITIVE DATA in RelWithDebInfo or Debug builds
        //qDebug() << "O2::onTokenReplyFinished: replyData\n";
        //qDebug() << QString( replyData );

        QVariantMap tokens = parseJsonResponse(replyData);

        // Dump tokens
        log( QStringLiteral("O2::onTokenReplyFinished: Tokens returned:\n") );
        for (auto it = tokens.constBegin(); it != tokens.constEnd(); ++it) {
            // SENSITIVE DATA in RelWithDebInfo or Debug builds, so it is truncated first
            log( QStringLiteral("%1: %2...").arg( it.key(), it.value().toString().left( 3 ) ) );
        }

        // Check for mandatory tokens
        if (tokens.contains(O2_OAUTH2_ACCESS_TOKEN)) {
            log( QStringLiteral("O2::onTokenReplyFinished: Access token returned") );
            setToken(tokens.take(O2_OAUTH2_ACCESS_TOKEN).toString());
            bool ok = false;
            const int expiresIn = tokens.take(O2_OAUTH2_EXPIRES_IN).toInt(&ok);
            if (ok) {
                log( QStringLiteral("O2::onTokenReplyFinished: Token expires in %1 seconds").arg( expiresIn ) );
                setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + static_cast< qint64 >( expiresIn ));
            }
            setRefreshToken(tokens.take(O2_OAUTH2_REFRESH_TOKEN).toString());
            setExtraTokens(tokens);
            timedReplies_.remove(tokenReply);
            setLinked(true);
            Q_EMIT linkingSucceeded();
        } else {
            log( QStringLiteral("O2::onTokenReplyFinished: Access token missing from response"), O0BaseAuth::LogLevel::Warning );
            Q_EMIT linkingFailed();
        }
    }
    tokenReply->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (!tokenReply)
    {
      log( QStringLiteral("O2::onTokenReplyError: reply is null") );
    } else {
      log( QStringLiteral("O2::onTokenReplyError: %1: %2").arg( error ).arg( tokenReply->errorString() ), O0BaseAuth::LogLevel::Warning );
      log( QStringLiteral("O2::onTokenReplyError: %1 ").arg( QString( tokenReply->readAll() ) ) );
      timedReplies_.remove(tokenReply);
    }
	
    setToken(QString());
    setRefreshToken(QString());
    Q_EMIT linkingFailed();
}

QByteArray O2::buildRequestBody(const QMap<QString, QString> &parameters) {
    QByteArray body;
    bool first = true;
    for (auto it = parameters.constBegin(); it != parameters.constEnd(); ++it) {
        if (first) {
            first = false;
        } else {
            body.append("&");
        }
        body.append(QUrl::toPercentEncoding(it.key()) + QString("=").toUtf8() + QUrl::toPercentEncoding(it.value()));
    }
    return body;
}

qint64 O2::expires() {
    QString key = QString(O2_KEY_EXPIRES).arg(clientId_);
    return store_->value(key).toLongLong();
}

void O2::setExpires(qint64 v) {
    QString key = QString(O2_KEY_EXPIRES).arg(clientId_);
    store_->setValue(key, QString::number(v));
}

QNetworkAccessManager *O2::getManager()
{
    return manager_;
}

void O2::startPollServer(const QVariantMap &params)
{
    bool ok = false;
    const int expiresIn = params[O2_OAUTH2_EXPIRES_IN].toInt(&ok);
    if (!ok) {
        log( QStringLiteral("O2::startPollServer: No expired_in parameter"), O0BaseAuth::LogLevel::Warning );
        Q_EMIT linkingFailed();
        return;
    }

    log( QStringLiteral("O2::startPollServer: device_ and user_code expires in %1 seconds").arg( expiresIn ) );

    QUrl url(tokenUrl_);
    QNetworkRequest authRequest(url);
    authRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    const QString deviceCode = params[O2_OAUTH2_DEVICE_CODE].toString();
    const QString grantType = grantType_.isEmpty() ? O2_OAUTH2_GRANT_TYPE_DEVICE : grantType_;

    QList<O0RequestParameter> parameters;
    parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_ID, clientId_.toUtf8()));
    if ( !clientSecret_.isEmpty() )
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_SECRET, clientSecret_.toUtf8()));
    parameters.append(O0RequestParameter(O2_OAUTH2_CODE, deviceCode.toUtf8()));
    parameters.append(O0RequestParameter(O2_OAUTH2_GRANT_TYPE, grantType.toUtf8()));
    QByteArray payload = O0BaseAuth::createQueryParameters(parameters);

    O2PollServer * pollServer = new O2PollServer(getManager(), authRequest, payload, expiresIn, this);
    if (params.contains(O2_OAUTH2_INTERVAL)) {
        int interval = params[O2_OAUTH2_INTERVAL].toInt(&ok);
        if (ok)
            pollServer->setInterval(interval);
    }
    connect(pollServer, &O2PollServer::verificationReceived, this, &O2::onVerificationReceived);
    connect(pollServer, &O2PollServer::serverClosed, this, &O2::serverHasClosed);
    setPollServer(pollServer);
    pollServer->startPolling();
}

QString O2::refreshToken() {
    QString key = QString(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O2::setRefreshToken(const QString &v) {
    log( QStringLiteral("O2::setRefreshToken %1...").arg( v.left(4) ));
    QString key = QString(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    store_->setValue(key, v);
}

void O2::refresh() {
    log( QStringLiteral("O2::refresh: Token: ...%1").arg( refreshToken().right(7) ));

    if (refreshToken().isEmpty()) {
        log( QStringLiteral("O2::refresh: No refresh token"), O0BaseAuth::LogLevel::Warning );
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }
    if (refreshTokenUrl_.isEmpty()) {
        log( QStringLiteral("O2::refresh: Refresh token URL not set"), O0BaseAuth::LogLevel::Warning );
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }

    QNetworkRequest refreshRequest(refreshTokenUrl_);
    refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QMap<QString, QString> parameters;
    parameters.insert(O2_OAUTH2_CLIENT_ID, clientId_);
    parameters.insert(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    parameters.insert(O2_OAUTH2_REFRESH_TOKEN, refreshToken());
    parameters.insert(O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_REFRESH_TOKEN);

    QByteArray data = buildRequestBody(parameters);
    QNetworkReply *refreshReply = getManager()->post(refreshRequest, data);
    timedReplies_.add(refreshReply);
    connect(refreshReply,
            &QNetworkReply::finished,
            this,
            &O2::onRefreshFinished,
            Qt::QueuedConnection);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(refreshReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRefreshError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(refreshReply, &QNetworkReply::errorOccurred, this, &O2::onRefreshError, Qt::QueuedConnection);
#endif
}

void O2::onRefreshFinished() {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());

    if (refreshReply->error() == QNetworkReply::NoError) {
        QByteArray reply = refreshReply->readAll();
        QVariantMap tokens = parseJsonResponse(reply);
        if ( tokens.contains(QStringLiteral("error")) ) {
          log( QStringLiteral(" Error refreshing token %1" ).arg( tokens.value(QStringLiteral("error")).toMap().value(QStringLiteral("message")).toString().toLocal8Bit().constData() ) );
          unlink();
          timedReplies_.remove(refreshReply);
          Q_EMIT refreshFinished(QNetworkReply::NoError);
        }
        else
        {
          setToken(tokens.value(O2_OAUTH2_ACCESS_TOKEN).toString());
          setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + static_cast<qint64>(tokens.value(O2_OAUTH2_EXPIRES_IN).toInt()));
          QString refreshToken = tokens.value(O2_OAUTH2_REFRESH_TOKEN).toString();
          if(!refreshToken.isEmpty()) {
              setRefreshToken(refreshToken);
          }
          else {
              log( QStringLiteral("No new refresh token. Keep the old one.") );
          }
          timedReplies_.remove(refreshReply);
          setLinked(true);
          Q_EMIT linkingSucceeded();
          Q_EMIT refreshFinished(QNetworkReply::NoError);
          log( QStringLiteral(" New token expires in %1 seconds").arg( expires() ) );
        }
    } else {
        log( QStringLiteral( "O2::onRefreshFinished: Error %1 %2" ).arg( (int)refreshReply->error() ).arg( refreshReply->errorString() ) );
    }
    refreshReply->deleteLater();
}

void O2::onRefreshError(QNetworkReply::NetworkError error) {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    log( QStringLiteral("O2::onRefreshError: %1").arg( error ), O0BaseAuth::LogLevel::Warning );
    unlink();
    timedReplies_.remove(refreshReply);
    Q_EMIT refreshFinished(error);
}

void O2::onDeviceAuthReplyFinished()
{
    log( QStringLiteral("O2::onDeviceAuthReplyFinished") );
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (!tokenReply)
    {
      log( QStringLiteral("O2::onDeviceAuthReplyFinished: reply is null") );
      return;
    }
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();

        // Dump replyData
        // SENSITIVE DATA in RelWithDebInfo or Debug builds
        //qDebug() << "O2::onDeviceAuthReplyFinished: replyData\n";
        //qDebug() << QString( replyData );

        QVariantMap params = parseJsonResponse(replyData);

        // Dump tokens
        log( QStringLiteral("O2::onDeviceAuthReplyFinished: Tokens returned:\n") );
        for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
            // SENSITIVE DATA in RelWithDebInfo or Debug builds, so it is truncated first
            log( QStringLiteral("%1: %2...").arg( it.key(), it.value().toString().left( 3 ) ) );
        }

        // Check for mandatory parameters
        if (hasMandatoryDeviceAuthParams(params)) {
            log( QStringLiteral("O2::onDeviceAuthReplyFinished: Device auth request response") );

            const QString userCode = params.take(O2_OAUTH2_USER_CODE).toString();
            QUrl uri = params.take(O2_OAUTH2_VERIFICATION_URI).toUrl();
            if (uri.isEmpty())
                uri = params.take(O2_OAUTH2_VERIFICATION_URL).toUrl();

            if (params.contains(O2_OAUTH2_VERIFICATION_URI_COMPLETE))
                Q_EMIT openBrowser(params.take(O2_OAUTH2_VERIFICATION_URI_COMPLETE).toUrl());

            Q_EMIT showVerificationUriAndCode(uri, userCode);

            startPollServer(params);
        } else {
            log( QStringLiteral("O2::onDeviceAuthReplyFinished: Mandatory parameters missing from response"), O0BaseAuth::LogLevel::Warning );
            Q_EMIT linkingFailed();
        }
    }
    tokenReply->deleteLater();
}

void O2::serverHasClosed(bool paramsfound)
{
    if ( !paramsfound ) {
        // server has probably timed out after receiving first response
        Q_EMIT linkingFailed();
    }
    // poll server is not re-used for later auth requests
    setPollServer(nullptr);
}

QString O2::localhostPolicy() const {
    return localhostPolicy_;
}

void O2::setLocalhostPolicy(const QString &value) {
    if (localhostPolicy_ == value)
        return;

    localhostPolicy_ = value;
    Q_EMIT localHostPolicyChanged(localhostPolicy_);
}

QString O2::apiKey() {
    return apiKey_;
}

void O2::setApiKey(const QString &value) {
    if (apiKey_ == value)
        return;

    apiKey_ = value;
    Q_EMIT apiKeyChanged(apiKey_);
}

bool O2::ignoreSslErrors() {
    return timedReplies_.ignoreSslErrors();
}

void O2::setIgnoreSslErrors(bool ignoreSslErrors) {
    timedReplies_.setIgnoreSslErrors(ignoreSslErrors);
    Q_EMIT ignoreSslErrorsChanged(ignoreSslErrors);
}
