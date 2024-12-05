#include <QDebug>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "o1requestor.h"
#include "o1timedreply.h"
#include "o0globals.h"

O1Requestor::O1Requestor(QNetworkAccessManager *manager, O1 *authenticator, QObject *parent): QObject(parent) {
    manager_ = manager;
    authenticator_ = authenticator;
}

QNetworkReply *O1Requestor::get(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::GetOperation);
    return addTimer(manager_->get(request));
}

QNetworkReply *O1Requestor::post(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, const QByteArray &data) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::PostOperation);
    return addTimer(manager_->post(request, data));
}

QNetworkReply *O1Requestor::post(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, QHttpMultiPart * multiPart) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::PostOperation);
    return addTimer(manager_->post(request, multiPart));
}

QNetworkReply *O1Requestor::put(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, const QByteArray &data) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::PutOperation);
    return addTimer(manager_->put(request, data));
}

QNetworkReply *O1Requestor::addTimer(QNetworkReply *reply) {
    (void)new O1TimedReply(reply);
    return reply;
}

QNetworkRequest O1Requestor::setup(const QNetworkRequest &request, const QList<O0RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation) {
    // Collect OAuth parameters
    QList<O0RequestParameter> oauthParams;
    oauthParams.append(O0RequestParameter(O2_OAUTH_CONSUMER_KEY, authenticator_->clientId().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_VERSION, "1.0"));
    oauthParams.append(O0RequestParameter(O2_OAUTH_TOKEN, authenticator_->token().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE_METHOD, authenticator_->signatureMethod().toLatin1()));
    oauthParams.append(O0RequestParameter(O2_OAUTH_NONCE, O1::nonce()));
#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
    oauthParams.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentSecsSinceEpoch()).toLatin1()));
#else
    oauthParams.append(O0RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
#endif

    // Add signature parameter
    oauthParams.append(O0RequestParameter(O2_OAUTH_SIGNATURE, authenticator_->generateSignature(oauthParams, request, signingParameters, operation)));

    // Return a copy of the original request with authorization header set
    QNetworkRequest req(request);
    authenticator_->decorateRequest(req, oauthParams);
    return req;
}
