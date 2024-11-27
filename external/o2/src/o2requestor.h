#ifndef O2REQUESTOR_H
#define O2REQUESTOR_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QByteArray>
#include <QHttpMultiPart>

#include "o0export.h"
#include "o2reply.h"

class O2;

/// Makes authenticated requests.
class O0_EXPORT O2Requestor: public QObject {
    Q_OBJECT

public:
    explicit O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent = nullptr);
    ~O2Requestor() override;
    
    
    /// Some services require the access token to be sent as a Authentication HTTP header
    /// and refuse requests with the access token in the query.
    /// This function allows to use or ignore the access token in the query.
    /// The default value of `true` means that the query will contain the access token.
    /// By setting the value to false, the query will not contain the access token.
    /// See:
    /// https://tools.ietf.org/html/draft-ietf-oauth-v2-bearer-16#section-4.3
    /// https://tools.ietf.org/html/rfc6750#section-2.3
    
    void setAddAccessTokenInQuery(bool value);

    /// Some services require the access token to be sent as a Authentication HTTP header.
    /// This is the case for Twitch and Mixer.
    /// When the access token expires and is refreshed, O2Requestor::retry() needs to update the Authentication HTTP header.
    /// In order to do so, O2Requestor needs to know the format of the Authentication HTTP header.
    void setAccessTokenInAuthenticationHTTPHeaderFormat(const QString &value);

public Q_SLOTS:
    /// Make a GET request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int get(const QNetworkRequest &req, int timeout = 60*1000);

    /// Make a POST request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int post(const QNetworkRequest &req, const QByteArray &data, int timeout = 60*1000);
    int post(const QNetworkRequest &req, QHttpMultiPart* data, int timeout = 60*1000);

    /// Make a PUT request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int put(const QNetworkRequest &req, const QByteArray &data, int timeout = 60*1000);
    int put(const QNetworkRequest &req, QHttpMultiPart* data, int timeout = 60*1000);

    /// Make a DELETE request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int deleteResource(const QNetworkRequest &req, int timeout = 60*1000);

    /// Make a HEAD request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int head(const QNetworkRequest &req, int timeout = 60*1000);

    /// Make a custom request.
    /// @return Request ID or -1 if there are too many requests in the queue.
    int customRequest(const QNetworkRequest &req, const QByteArray &verb, const QByteArray &data, int timeout = 60*1000);

Q_SIGNALS:

    /// Emitted when a request has been completed or failed.
    void finished(int id, QNetworkReply::NetworkError error, QByteArray data);

    /// Emitted when a request has been completed or failed.
    void finished(int id, QNetworkReply::NetworkError error, QString errorText, QByteArray data);

    /// Emitted when a request has been completed or failed. Also reply headers will be provided.
    void finished(int id, QNetworkReply::NetworkError error, QByteArray data, QList<QNetworkReply::RawHeaderPair> headers);

    /// Emitted when a request has been completed or failed. Also reply headers will be provided.
    void finished(int id, QNetworkReply::NetworkError error, QString errorText, QByteArray data, QList<QNetworkReply::RawHeaderPair> headers);

    /// Emitted when an upload has progressed.
    void uploadProgress(int id, qint64 bytesSent, qint64 bytesTotal);

protected Q_SLOTS:
    /// Handle refresh completion.
    void onRefreshFinished(QNetworkReply::NetworkError error);

    /// Handle request finished.
    void onRequestFinished();

    /// Handle request error.
    void onRequestError(QNetworkReply::NetworkError error);

    /// Re-try request (after successful token refresh).
    void retry();

    /// Finish the request, Q_EMIT finished() signal.
    void finish();

    /// Handle upload progress.
    void onUploadProgress(qint64 uploaded, qint64 total);

protected:
    int setup(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &verb = QByteArray());

    enum Status {
        Idle, Requesting, ReRequesting
    };

    QNetworkAccessManager *manager_{nullptr};
    O2 *authenticator_{nullptr};
    QNetworkRequest request_;
    QByteArray data_;
    QHttpMultiPart* multipartData_{nullptr};
    QNetworkReply *reply_{nullptr};
    Status status_{Idle};
    int id_{1};
    QNetworkAccessManager::Operation operation_{QNetworkAccessManager::GetOperation};
    QUrl url_;
    O2ReplyList timedReplies_;
    QNetworkReply::NetworkError error_{QNetworkReply::NoError};
    bool addAccessTokenInQuery_{true};
    QString accessTokenInAuthenticationHTTPHeaderFormat_;
    bool rawData_{false};
};

#endif // O2REQUESTOR_H
