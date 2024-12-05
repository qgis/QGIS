#ifndef O2POLLSERVER_H
#define O2POLLSERVER_H

#include <QByteArray>
#include <QMap>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QTimer>

#include "o0export.h"

class QNetworkAccessManager;

/// Poll an authorization server for token
class O0_EXPORT O2PollServer : public QObject
{
    Q_OBJECT

public:
    explicit O2PollServer(QNetworkAccessManager * manager, const QNetworkRequest &request, const QByteArray & payload, int expiresIn, QObject *parent = nullptr);

    /// Seconds to wait between polling requests
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    int interval() const;
    void setInterval(int interval);

Q_SIGNALS:
    void verificationReceived(QMap<QString, QString>);
    void serverClosed(bool); // whether it has found parameters
    void intervalChanged(int interval);

public Q_SLOTS:
    void startPolling();

protected Q_SLOTS:
    void onPollTimeout();
    void onExpiration();
    void onReplyFinished();

protected:
    QNetworkAccessManager *manager_;
    const QNetworkRequest request_;
    const QByteArray payload_;
    const int expiresIn_;
    QTimer expirationTimer;
    QTimer pollTimer;
};

#endif // O2POLLSERVER_H
