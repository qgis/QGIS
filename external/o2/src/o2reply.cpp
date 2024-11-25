#include <QTimer>
#include <QNetworkReply>

#include "o2reply.h"

O2Reply::O2Reply(QNetworkReply *reply, int timeOut, QObject *parent): QTimer(parent), reply(reply) {
    setSingleShot(true);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(this, SIGNAL(error(QNetworkReply::NetworkError)), reply, SIGNAL(error(QNetworkReply::NetworkError)), Qt::QueuedConnection);
#else
    connect(this, &O2Reply::error, reply, &QNetworkReply::errorOccurred, Qt::QueuedConnection);
#endif
    connect(this, &QTimer::timeout, this, &O2Reply::onTimeOut, Qt::QueuedConnection);
    start(timeOut);
}

void O2Reply::onTimeOut() {
    Q_EMIT error(QNetworkReply::TimeoutError);
}

O2ReplyList::~O2ReplyList() {
    for (O2Reply *timedReply: qAsConst(replies_)) {
        delete timedReply;
    }
}

void O2ReplyList::add(QNetworkReply *reply) {
    if (reply && ignoreSslErrors())
            reply->ignoreSslErrors();
    add(new O2Reply(reply));
}

void O2ReplyList::add(O2Reply *reply) {
    replies_.append(reply);
}

void O2ReplyList::remove(QNetworkReply *reply) {
    O2Reply *o2Reply = find(reply);
    if (o2Reply) {
        o2Reply->stop();
        (void)replies_.removeOne(o2Reply);
    }
}

O2Reply *O2ReplyList::find(const QNetworkReply *reply) {
    for (O2Reply *timedReply: qAsConst(replies_)) {
        if (timedReply->reply == reply) {
            return timedReply;
        }
    }
    return nullptr;
}

bool O2ReplyList::ignoreSslErrors() const
{
    return ignoreSslErrors_;
}

void O2ReplyList::setIgnoreSslErrors(bool ignoreSslErrors)
{
    ignoreSslErrors_ = ignoreSslErrors;
}
