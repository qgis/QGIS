#include <QList>
#include <QPair>
#include <QDebug>
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

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

#include "o2.h"
#include "o2pollserver.h"
#include "o2replyserver.h"
#include "o0globals.h"
#include "o0jsonresponse.h"
#include "o0settingsstore.h"

/// Add query parameters to a query
static void addQueryParametersToUrl(QUrl &url,  QList<QPair<QString, QString> > parameters) {
#if QT_VERSION < 0x050000
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
    grantType_ = value;
}

void O2::link() {
    //qDebug() << "O2::link";

    // Create the reply server if it doesn't exist
    // and we don't use an external web interceptor
    if(!useExternalWebInterceptor_) {
        if(replyServer() == NULL) {
            O2ReplyServer * replyServer = new O2ReplyServer(this);
            connect(replyServer, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
            connect(replyServer, SIGNAL(serverClosed(bool)), this, SLOT(serverHasClosed(bool)));
            setReplyServer(replyServer);
        }
    }

    if (linked()) {
        //qDebug() << "O2::link: Linked already";
        Q_EMIT linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken("");
    setTokenSecret("");
    setExtraTokens(QVariantMap());
    setRefreshToken(QString());
    setExpires(0);

    if (grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowImplicit) {

#if QT_VERSION >= 0x050000
        QString uniqueState = QUuid::createUuid().toString().remove(QRegularExpression("([^a-zA-Z0-9]|[-])"));
#else
        QString uniqueState = QUuid::createUuid().toString().remove(QRegExp("([^a-zA-Z0-9]|[-])"));
#endif
        if (useExternalWebInterceptor_) {
            // Save redirect URI, as we have to reuse it when requesting the access token
            redirectUri_ = localhostPolicy_.arg(localPort());
        } else {
            // Start listening to authentication replies
            if (!replyServer()->isListening()) {
                if (replyServer()->listen(QHostAddress::Any, localPort_)) {
                    //qDebug() << "O2::link: Reply server listening on port" << localPort();
                } else {
                    qWarning() << "O2::link: Reply server failed to start listening on port" << localPort();
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
        parameters.append(qMakePair(QString(O2_OAUTH2_REDIRECT_URI), redirectUri_));
        parameters.append(qMakePair(QString(O2_OAUTH2_SCOPE), scope_.replace( " ", "+" )));
        parameters.append(qMakePair(QString(O2_OAUTH2_STATE), uniqueState));
        if ( !apiKey_.isEmpty() )
            parameters.append(qMakePair(QString(O2_OAUTH2_API_KEY), apiKey_));
        foreach (QString key, extraRequestParams().keys()) {
            parameters.append(qMakePair(key, extraRequestParams().value(key).toString()));
        }
        // Show authentication URL with a web browser
        QUrl url(requestUrl_);
        addQueryParametersToUrl(url, parameters);
        //qDebug() << "O2::link: Emit openBrowser" << url.toString();
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
        foreach (QString key, extraRequestParams().keys()) {
            parameters.append(O0RequestParameter(key.toUtf8(), extraRequestParams().value(key).toByteArray()));
        }
        QByteArray payload = O0BaseAuth::createQueryParameters(parameters);

        //qDebug() << "O2::link: Sending token request for resource owner flow";
        QUrl url(tokenUrl_);
        QNetworkRequest tokenRequest(url);
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *tokenReply = getManager()->post(tokenRequest, payload);

        connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
#if QT_VERSION < 0x051500
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
        connect(tokenReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
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

        connect(tokenReply, SIGNAL(finished()), this, SLOT(onDeviceAuthReplyFinished()), Qt::QueuedConnection);
#if QT_VERSION < 0x051500
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
        connect(tokenReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#endif
    }
}

void O2::unlink() {
    //qDebug() << "O2::unlink";
    setLinked(false);
    setToken(QString());
    setRefreshToken(QString());
    setExpires(0);
    setExtraTokens(QVariantMap());
    Q_EMIT linkingSucceeded();
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    //qDebug() << "O2::onVerificationReceived: Emitting closeBrowser()";
    Q_EMIT closeBrowser();

    if (response.contains("error")) {
        qWarning() << "O2::onVerificationReceived: Verification failed:" << response;
        Q_EMIT linkingFailed();
        return;
    }

    if (grantFlow_ == GrantFlowAuthorizationCode) {
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
        parameters.insert(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
        parameters.insert(O2_OAUTH2_REDIRECT_URI, redirectUri_);
        parameters.insert(O2_OAUTH2_GRANT_TYPE, O2_AUTHORIZATION_CODE);
        QByteArray data = buildRequestBody(parameters);

        //qDebug() << QString("O2::onVerificationReceived: Exchange access code data:\n%1").arg(QString(data));

        QNetworkReply *tokenReply = getManager()->post(tokenRequest, data);
        timedReplies_.add(tokenReply);
        connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
#if QT_VERSION < 0x051500
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
        connect(tokenReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#endif
    } else if (grantFlow_ == GrantFlowImplicit || grantFlow_ == GrantFlowDevice) {
      // Check for mandatory tokens
      if (response.contains(O2_OAUTH2_ACCESS_TOKEN)) {
          //qDebug() << "O2::onVerificationReceived: Access token returned for implicit or device flow";
          setToken(response.value(O2_OAUTH2_ACCESS_TOKEN));
          if (response.contains(O2_OAUTH2_EXPIRES_IN)) {
            bool ok = false;
            int expiresIn = response.value(O2_OAUTH2_EXPIRES_IN).toInt(&ok);
            if (ok) {
                //qDebug() << "O2::onVerificationReceived: Token expires in" << expiresIn << "seconds";
                setExpires((int)(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn));
            }
          }
          if (response.contains(O2_OAUTH2_REFRESH_TOKEN)) {
              setRefreshToken(response.value(O2_OAUTH2_REFRESH_TOKEN));
          }
          setLinked(true);
          Q_EMIT linkingSucceeded();
      } else {
          qWarning() << "O2::onVerificationReceived: Access token missing from response for implicit or device flow";
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
    //qDebug() << "O2::onTokenReplyFinished";
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (!tokenReply)
    {
      qDebug() << "O2::onTokenReplyFinished: reply is null";
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
        //qDebug() << "O2::onTokenReplyFinished: Tokens returned:\n";
        //foreach (QString key, tokens.keys()) {
            // SENSITIVE DATA in RelWithDebInfo or Debug builds, so it is truncated first
            //qDebug() << key << ": "<< tokens.value( key ).toString().left( 3 ) << "...";
        //}

        // Check for mandatory tokens
        if (tokens.contains(O2_OAUTH2_ACCESS_TOKEN)) {
            // qDebug() << "O2::onTokenReplyFinished: Access token returned";
            setToken(tokens.take(O2_OAUTH2_ACCESS_TOKEN).toString());
            bool ok = false;
            int expiresIn = tokens.take(O2_OAUTH2_EXPIRES_IN).toInt(&ok);
            if (ok) {
                // qDebug() << "O2::onTokenReplyFinished: Token expires in" << expiresIn << "seconds";
                setExpires((int)(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn));
            }
            setRefreshToken(tokens.take(O2_OAUTH2_REFRESH_TOKEN).toString());
            setExtraTokens(tokens);
            timedReplies_.remove(tokenReply);
            setLinked(true);
            Q_EMIT linkingSucceeded();
        } else {
            qWarning() << "O2::onTokenReplyFinished: Access token missing from response";
            Q_EMIT linkingFailed();
        }
    }
    tokenReply->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (!tokenReply)
    {
      //qDebug() << "O2::onTokenReplyError: reply is null";
    } else {
      qWarning() << "O2::onTokenReplyError: " << error << ": " << tokenReply->errorString();
      //qDebug() << "O2::onTokenReplyError: " << tokenReply->readAll();
      timedReplies_.remove(tokenReply);
    }
	
    setToken(QString());
    setRefreshToken(QString());
    Q_EMIT linkingFailed();
}

QByteArray O2::buildRequestBody(const QMap<QString, QString> &parameters) {
    QByteArray body;
    bool first = true;
    foreach (QString key, parameters.keys()) {
        if (first) {
            first = false;
        } else {
            body.append("&");
        }
        QString value = parameters.value(key);
        body.append(QUrl::toPercentEncoding(key) + QString("=").toUtf8() + QUrl::toPercentEncoding(value));
    }
    return body;
}

int O2::expires() {
    QString key = QString(O2_KEY_EXPIRES).arg(clientId_);
    return store_->value(key).toInt();
}

void O2::setExpires(int v) {
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
    int expiresIn = params[O2_OAUTH2_EXPIRES_IN].toInt(&ok);
    if (!ok) {
        qWarning() << "O2::startPollServer: No expired_in parameter";
        Q_EMIT linkingFailed();
        return;
    }

    //qDebug() << "O2::startPollServer: device_ and user_code expires in" << expiresIn << "seconds";

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
    connect(pollServer, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
    connect(pollServer, SIGNAL(serverClosed(bool)), this, SLOT(serverHasClosed(bool)));
    setPollServer(pollServer);
    pollServer->startPolling();
}

QString O2::refreshToken() {
    QString key = QString(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O2::setRefreshToken(const QString &v) {
    //qDebug() << "O2::setRefreshToken" << v.left(4) << "...";
    QString key = QString(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    store_->setValue(key, v);
}

void O2::refresh() {
    // qDebug() << "O2::refresh: Token: ..." << refreshToken().right(7);

    if (refreshToken().isEmpty()) {
        qWarning() << "O2::refresh: No refresh token";
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }
    if (refreshTokenUrl_.isEmpty()) {
        qWarning() << "O2::refresh: Refresh token URL not set";
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
    connect(refreshReply, SIGNAL(finished()), this, SLOT(onRefreshFinished()), Qt::QueuedConnection);
#if QT_VERSION < 0x051500
    connect(refreshReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRefreshError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(refreshReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(onRefreshError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#endif
}

void O2::onRefreshFinished() {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());

    if (refreshReply->error() == QNetworkReply::NoError) {
        QByteArray reply = refreshReply->readAll();
        QVariantMap tokens = parseJsonResponse(reply);
        if ( tokens.contains(QStringLiteral("error")) ) {
          qDebug() << " Error refreshing token" << tokens.value(QStringLiteral("error")).toMap().value(QStringLiteral("message")).toString().toLocal8Bit().constData();
          unlink();
          timedReplies_.remove(refreshReply);
          Q_EMIT refreshFinished(QNetworkReply::NoError);
        }
        else
        {
          setToken(tokens.value(O2_OAUTH2_ACCESS_TOKEN).toString());
          setExpires((int)(QDateTime::currentMSecsSinceEpoch() / 1000 + tokens.value(O2_OAUTH2_EXPIRES_IN).toInt()));
          QString refreshToken = tokens.value(O2_OAUTH2_REFRESH_TOKEN).toString();
          if(!refreshToken.isEmpty()) {
              setRefreshToken(refreshToken);
          }
          else {
              //qDebug() << "No new refresh token. Keep the old one.";
          }
          timedReplies_.remove(refreshReply);
          setLinked(true);
          Q_EMIT linkingSucceeded();
          Q_EMIT refreshFinished(QNetworkReply::NoError);
          //qDebug() << " New token expires in" << expires() << "seconds";
        }
    } else {
        //qDebug() << "O2::onRefreshFinished: Error" << (int)refreshReply->error() << refreshReply->errorString();
    }
    refreshReply->deleteLater();
}

void O2::onRefreshError(QNetworkReply::NetworkError error) {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O2::onRefreshError: " << error;
    unlink();
    timedReplies_.remove(refreshReply);
    Q_EMIT refreshFinished(error);
}

void O2::onDeviceAuthReplyFinished()
{
    qDebug() << "O2::onDeviceAuthReplyFinished";
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (!tokenReply)
    {
      qDebug() << "O2::onDeviceAuthReplyFinished: reply is null";
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
        qDebug() << "O2::onDeviceAuthReplyFinished: Tokens returned:\n";
        foreach (QString key, params.keys()) {
            // SENSITIVE DATA in RelWithDebInfo or Debug builds, so it is truncated first
            qDebug() << key << ": "<< params.value( key ).toString().left( 3 ) << "...";
        }

        // Check for mandatory parameters
        if (hasMandatoryDeviceAuthParams(params)) {
            qDebug() << "O2::onDeviceAuthReplyFinished: Device auth request response";

            const QString userCode = params.take(O2_OAUTH2_USER_CODE).toString();
            QUrl uri = params.take(O2_OAUTH2_VERIFICATION_URI).toUrl();
            if (uri.isEmpty())
                uri = params.take(O2_OAUTH2_VERIFICATION_URL).toUrl();

            if (params.contains(O2_OAUTH2_VERIFICATION_URI_COMPLETE))
                Q_EMIT openBrowser(params.take(O2_OAUTH2_VERIFICATION_URI_COMPLETE).toUrl());

            Q_EMIT showVerificationUriAndCode(uri, userCode);

            startPollServer(params);
        } else {
            qWarning() << "O2::onDeviceAuthReplyFinished: Mandatory parameters missing from response";
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
    setPollServer(NULL);
}

QString O2::localhostPolicy() const {
    return localhostPolicy_;
}

void O2::setLocalhostPolicy(const QString &value) {
    localhostPolicy_ = value;
}

QString O2::apiKey() {
    return apiKey_;
}

void O2::setApiKey(const QString &value) {
    apiKey_ = value;
}

bool O2::ignoreSslErrors() {
    return timedReplies_.ignoreSslErrors();
}

void O2::setIgnoreSslErrors(bool ignoreSslErrors) {
    timedReplies_.setIgnoreSslErrors(ignoreSslErrors);
}
