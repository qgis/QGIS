#include <QTimer>
#include <QNetworkReply>

#include "o2reply.h"

O2Reply::O2Reply(QNetworkReply *r, int timeOut, QObject *parent): QTimer(parent), reply(r) {
    setSingleShot(true);
    connect(this, SIGNAL(error(QNetworkReply::NetworkError)), reply, SIGNAL(error(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(this, SIGNAL(timeout()), this, SLOT(onTimeOut()), Qt::QueuedConnection);
    start(timeOut);
}

void O2Reply::onTimeOut() {
    Q_EMIT error(QNetworkReply::TimeoutError);
}

O2ReplyList::~O2ReplyList() {
    foreach (O2Reply *timedReply, replies_) {
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

O2Reply *O2ReplyList::find(QNetworkReply *reply) {
    foreach (O2Reply *timedReply, replies_) {
        if (timedReply->reply == reply) {
            return timedReply;
        }
    }
    return 0;
}

bool O2ReplyList::ignoreSslErrors()
{
    return ignoreSslErrors_;
}

void O2ReplyList::setIgnoreSslErrors(bool ignoreSslErrors)
{
    ignoreSslErrors_ = ignoreSslErrors;
}
