#ifndef O1TIMEDREPLY_H
#define O1TIMEDREPLY_H

#include <QNetworkReply>
#include <QTimer>

#include "o0export.h"

/// A timer connected to a network reply.
class O0_EXPORT O1TimedReply: public QTimer {
    Q_OBJECT

public:
    explicit O1TimedReply(QNetworkReply *parent, int pTimeout=60*1000);

Q_SIGNALS:
    /// Emitted when we have timed out waiting for the network reply.
    void error(QNetworkReply::NetworkError);
    /// Emitted when the network reply has responded.
    void finished();

private Q_SLOTS:
    void onFinished();
    void onTimeout();
};

#endif
