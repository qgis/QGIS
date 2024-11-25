#include <cassert>

#include <QTimer>
#include <QBuffer>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#include "o2requestor.h"
#include "o2.h"
#include "o0globals.h"

O2Requestor::O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent): QObject(parent) {
    manager_ = manager;
    authenticator_ = authenticator;
    if (authenticator) {
        timedReplies_.setIgnoreSslErrors(authenticator->ignoreSslErrors());
    }
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(authenticator,
            &O2::refreshFinished,
            this,
            &O2Requestor::onRefreshFinished,
            Qt::QueuedConnection);
}

O2Requestor::~O2Requestor() = default;

void O2Requestor::setAddAccessTokenInQuery(bool value) {
    addAccessTokenInQuery_ = value;
}

void O2Requestor::setAccessTokenInAuthenticationHTTPHeaderFormat(const QString &value) {
    accessTokenInAuthenticationHTTPHeaderFormat_ = value;
}

int O2Requestor::get(const QNetworkRequest &req, int timeout/* = 60*1000*/) {
    if (-1 == setup(req, QNetworkAccessManager::GetOperation)) {
        return -1;
    }
    reply_ = manager_->get(request_);
    timedReplies_.add(new O2Reply(reply_, timeout));

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    return id_;
}

int O2Requestor::post(const QNetworkRequest &req, const QByteArray &data, int timeout/* = 60*1000*/) {
    if (-1 == setup(req, QNetworkAccessManager::PostOperation)) {
        return -1;
    }
    rawData_ = true;
    data_ = data;
    reply_ = manager_->post(request_, data_);
    timedReplies_.add(new O2Reply(reply_, timeout));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

int O2Requestor::post(const QNetworkRequest & req, QHttpMultiPart* data, int timeout/* = 60*1000*/)
{
    if (-1 == setup(req, QNetworkAccessManager::PostOperation)) {
        return -1;
    }
    rawData_ = false;
    multipartData_ = data;
    reply_ = manager_->post(request_, multipartData_);
    multipartData_->setParent(reply_);
    timedReplies_.add(new O2Reply(reply_, timeout));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

int O2Requestor::put(const QNetworkRequest &req, const QByteArray &data, int timeout/* = 60*1000*/) {
    if (-1 == setup(req, QNetworkAccessManager::PutOperation)) {
        return -1;
    }
    rawData_ = true;
    data_ = data;
    reply_ = manager_->put(request_, data_);
    timedReplies_.add(new O2Reply(reply_, timeout));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

int O2Requestor::put(const QNetworkRequest & req, QHttpMultiPart* data, int timeout/* = 60*1000*/)
{
    if (-1 == setup(req, QNetworkAccessManager::PutOperation)) {
        return -1;
    }
    rawData_ = false;
    multipartData_ = data;
    reply_ = manager_->put(request_, multipartData_);
    multipartData_->setParent(reply_);
    timedReplies_.add(new O2Reply(reply_, timeout));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

int O2Requestor::deleteResource(const QNetworkRequest & req, int timeout/* = 60*1000*/)
{
    if (-1 == setup(req, QNetworkAccessManager::DeleteOperation)) {
        return -1;
    }
    rawData_ = false;
    reply_ = manager_->deleteResource(request_);
    timedReplies_.add(new O2Reply(reply_, timeout));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    return id_;
}

int O2Requestor::customRequest(const QNetworkRequest &req, const QByteArray &verb, const QByteArray &data, int timeout/* = 60*1000*/)
{
    (void)timeout;

    if (-1 == setup(req, QNetworkAccessManager::CustomOperation, verb)) {
        return -1;
    }
    data_ = data;
    QBuffer * buffer = new QBuffer;
    buffer->setData(data_);
    reply_ = manager_->sendCustomRequest(request_, verb, buffer);
    buffer->setParent(reply_);
    timedReplies_.add(new O2Reply(reply_));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
    return id_;
}

int O2Requestor::head(const QNetworkRequest &req, int timeout/* = 60*1000*/)
{
    if (-1 == setup(req, QNetworkAccessManager::HeadOperation)) {
        return -1;
    }
    reply_ = manager_->head(request_);
    timedReplies_.add(new O2Reply(reply_, timeout));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    return id_;
}

void O2Requestor::onRefreshFinished(QNetworkReply::NetworkError error) {
    if (status_ != Requesting) {
        O0BaseAuth::log( QStringLiteral("O2Requestor::onRefreshFinished: No pending request"), O0BaseAuth::LogLevel::Warning );
        return;
    }
// Suppress warning: Potential leak of memory in qtimer.h [clang-analyzer-cplusplus.NewDeleteLeaks]
#ifndef __clang_analyzer__
    if (QNetworkReply::NoError == error) {
        QTimer::singleShot(100, this, &O2Requestor::retry);
    } else {
        error_ = error;
        QTimer::singleShot(10, this, &O2Requestor::finish);
    }
#endif
}

void O2Requestor::onRequestFinished() {
    if (status_ == Idle) {
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        return;
    }
// Suppress warning: Potential leak of memory in qtimer.h [clang-analyzer-cplusplus.NewDeleteLeaks]
#ifndef __clang_analyzer__
    if (reply_->error() == QNetworkReply::NoError) {
        QTimer::singleShot(10, this, &O2Requestor::finish);
    }
#endif
}

void O2Requestor::onRequestError(QNetworkReply::NetworkError error) {
    O0BaseAuth::log( QStringLiteral("O2Requestor::onRequestError: Error %1").arg( error ), O0BaseAuth::LogLevel::Warning );
    if (status_ == Idle) {
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        return;
    }
    int httpStatus = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    O0BaseAuth::log( QStringLiteral("O2Requestor::onRequestError: HTTP status %1 %2").arg( httpStatus ).arg( reply_->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()), O0BaseAuth::LogLevel::Warning );
    if ((status_ == Requesting) && (httpStatus == 401)) {
        // Call O2::refresh. Note the O2 instance might live in a different thread
        if (QMetaObject::invokeMethod(authenticator_, "refresh")) {
            return;
        }
        O0BaseAuth::log( QStringLiteral( "O2Requestor::onRequestError: Invoking remote refresh failed" ), O0BaseAuth::LogLevel::Critical );
    }
    error_ = error;
// Suppress warning: Potential leak of memory in qtimer.h [clang-analyzer-cplusplus.NewDeleteLeaks]
#ifndef __clang_analyzer__
    QTimer::singleShot(10, this, &O2Requestor::finish);
#endif
}

void O2Requestor::onUploadProgress(qint64 uploaded, qint64 total) {
    if (status_ == Idle) {
        O0BaseAuth::log( QStringLiteral("O2Requestor::onUploadProgress: No pending request"), O0BaseAuth::LogLevel::Warning );
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        return;
    }
    // Restart timeout because request in progress
    O2Reply *o2Reply = timedReplies_.find(reply_);
    if(o2Reply)
        o2Reply->start();
    Q_EMIT uploadProgress(id_, uploaded, total);
}

int O2Requestor::setup(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &verb) {
    static int currentId = 1;

    if (status_ != Idle) {
        O0BaseAuth::log( QStringLiteral("O2Requestor::setup: Another request pending"), O0BaseAuth::LogLevel::Warning );
        return -1;
    }

    request_ = request;
    operation_ = operation;
    id_ = currentId++;
    url_ = request.url();

    QUrl url = url_;
    if (addAccessTokenInQuery_) {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        url.addQueryItem(O2_OAUTH2_ACCESS_TOKEN, authenticator_->token());
#else
        QUrlQuery query(url);
        query.addQueryItem(O2_OAUTH2_ACCESS_TOKEN, authenticator_->token());
        url.setQuery(query);
#endif
    }

    request_.setUrl(url);

    // If the service require the access token to be sent as a Authentication HTTP header, we add the access token.
    if (!accessTokenInAuthenticationHTTPHeaderFormat_.isEmpty()) {
        request_.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, accessTokenInAuthenticationHTTPHeaderFormat_.arg(authenticator_->token()).toLatin1());
    }

    if (!verb.isEmpty()) {
        request_.setRawHeader(O2_HTTP_HTTP_HEADER, verb);
    }

    status_ = Requesting;
    error_ = QNetworkReply::NoError;
    return id_;
}

void O2Requestor::finish() {
    QByteArray data;
    if (status_ == Idle) {
        O0BaseAuth::log( QStringLiteral("O2Requestor::finish: No pending request"), O0BaseAuth::LogLevel::Warning );
        return;
    }
    data = reply_->readAll();
    status_ = Idle;
    timedReplies_.remove(reply_);
    reply_->disconnect(this);
    reply_->deleteLater();
    QList<QNetworkReply::RawHeaderPair> headers = reply_->rawHeaderPairs();
    Q_EMIT finished(id_, error_, data);
    Q_EMIT finished(id_, error_, reply_->errorString(), data);
    Q_EMIT finished(id_, error_, data, headers);
    Q_EMIT finished(id_, error_, reply_->errorString(), data, headers);
}

void O2Requestor::retry() {
    if (status_ != Requesting) {
        O0BaseAuth::log( QStringLiteral("O2Requestor::retry: No pending request"), O0BaseAuth::LogLevel::Warning );
        return;
    }
    timedReplies_.remove(reply_);
    reply_->disconnect(this);
    reply_->deleteLater();
    QUrl url = url_;
    if (addAccessTokenInQuery_) {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        url.addQueryItem(O2_OAUTH2_ACCESS_TOKEN, authenticator_->token());
#else
        QUrlQuery query(url);
        query.addQueryItem(O2_OAUTH2_ACCESS_TOKEN, authenticator_->token());
        url.setQuery(query);
#endif
    }
    request_.setUrl(url);

    // If the service require the access token to be sent as a Authentication HTTP header,
    // we update the access token when retrying.
    if(!accessTokenInAuthenticationHTTPHeaderFormat_.isEmpty()) {
        request_.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, accessTokenInAuthenticationHTTPHeaderFormat_.arg(authenticator_->token()).toLatin1());
    }

    status_ = ReRequesting;
    switch (operation_) {
    case QNetworkAccessManager::GetOperation:
        reply_ = manager_->get(request_);
        break;
    case QNetworkAccessManager::PostOperation:
        reply_ = rawData_ ? manager_->post(request_, data_) : manager_->post(request_, multipartData_);
        break;
    case QNetworkAccessManager::CustomOperation:
    {
        QBuffer * buffer = new QBuffer;
        buffer->setData(data_);
        reply_ = manager_->sendCustomRequest(request_, request_.rawHeader(O2_HTTP_HTTP_HEADER), buffer);
        buffer->setParent(reply_);
    }
        break;
    case QNetworkAccessManager::PutOperation:
        reply_ = rawData_ ? manager_->put(request_, data_) : manager_->put(request_, multipartData_);
        break;
    case QNetworkAccessManager::HeadOperation:
        reply_ = manager_->head(request_);
        break;
    default:
        assert(!"Unspecified operation for request");
        reply_ = manager_->get(request_);
        break;
    }
    timedReplies_.add(reply_);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(reply_,
            &QNetworkReply::errorOccurred,
            this,
            &O2Requestor::onRequestError,
            Qt::QueuedConnection);
#endif
    connect(reply_,
            &QNetworkReply::finished,
            this,
            &O2Requestor::onRequestFinished,
            Qt::QueuedConnection);
    connect(reply_, &QNetworkReply::uploadProgress, this, &O2Requestor::onUploadProgress);
}
