#include <QTimer>
#include <QNetworkReply>

#include "o1timedreply.h"

O1TimedReply::O1TimedReply(QNetworkReply *parent, int pTimeout): QTimer(parent) {
    setSingleShot(true);
    setInterval(pTimeout);
    connect(this, &QTimer::timeout, this, &O1TimedReply::onTimeout);
    connect(parent, &QNetworkReply::finished, this, &O1TimedReply::onFinished);
}

void O1TimedReply::onFinished() {
    stop();
    Q_EMIT finished();
}

void O1TimedReply::onTimeout() {
    Q_EMIT error(QNetworkReply::TimeoutError);
}
