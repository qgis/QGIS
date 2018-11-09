#ifndef O1REQUESTOR_H
#define O1REQUESTOR_H

#include <QObject>
#include <QNetworkRequest>
#include <QByteArray>

#include "o0export.h"
#include "o1.h"

class QNetworkAccessManager;
class QNetworkReply;
class O1;

/// Makes authenticated requests using OAuth 1.0.
class O0_EXPORT O1Requestor: public QObject {
    Q_OBJECT

public:
    explicit O1Requestor(QNetworkAccessManager *manager, O1 *authenticator, QObject *parent = 0);

public Q_SLOTS:
    /// Make a GET request.
    /// @param  req                 Network request.
    /// @param  signingParameters   Extra (non-OAuth) parameters participating in signing.
    /// @return Reply.
    QNetworkReply *get(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters);

    /// Make a POST request.
    /// @param  req                 Network request.
    /// @param  signingParameters   Extra (non-OAuth) parameters participating in signing.
    /// @param  data                Request payload.
    /// @return Reply.
    QNetworkReply *post(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, const QByteArray &data);

    /// Make a POST request.
    /// @param  req                 Network request.
    /// @param  signingParameters   Extra (non-OAuth) parameters participating in signing.
    /// @param  multiPart           HTTPMultiPart.
    /// @return Reply.
    QNetworkReply *post(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, QHttpMultiPart *multiPart);

    /// Make a PUT request.
    /// @param  req                 Network request.
    /// @param  signingParameters   Extra (non-OAuth) parameters participating in signing.
    /// @param  data                Request payload.
    /// @return Reply.
    QNetworkReply *put(const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, const QByteArray &data);

protected:
    /// Return new request based on the original, with the "Authentication:" header added.
    QNetworkRequest setup(const QNetworkRequest &request, const QList<O0RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation);

    /// Augment reply with a timer.
    QNetworkReply *addTimer(QNetworkReply *reply);

    QNetworkAccessManager *manager_;
    O1 *authenticator_;
};


#endif // O1REQUESTOR_H
