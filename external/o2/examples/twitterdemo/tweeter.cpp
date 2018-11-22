#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QDebug>

#include "tweeter.h"
#include "o0globals.h"
#include "o1requestor.h"
#include "o0settingsstore.h"

const char O2_CONSUMER_KEY[] = "2vHeyIxjywIadjEhvbDpg";
const char O2_CONSUMER_SECRET[] = "Xfwe195Kp3ZpcCKgkYs7RKfugTm8EfpLkQvsKfX2vvs";

const int localPort = 8888;

Tweeter::Tweeter(QObject *parent) :
    QObject(parent) {
}

void Tweeter::doOAuth() {
    o1Twitter_ = new O1Twitter(this);
    o1Twitter_->setClientId(O2_CONSUMER_KEY);
    o1Twitter_->setClientSecret(O2_CONSUMER_SECRET);
    o1Twitter_->setLocalPort(localPort);

    // Create a store object for writing the received tokens
    O0SettingsStore *store = new O0SettingsStore(O2_ENCRYPTION_KEY);
    store->setGroupKey("twitter");
    o1Twitter_->setStore(store);

    // Connect signals
    connect(o1Twitter_, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o1Twitter_, SIGNAL(linkingFailed()), this, SIGNAL(linkingFailed()));
    connect(o1Twitter_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o1Twitter_, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(o1Twitter_, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));

    qDebug() << "Starting OAuth...";
    o1Twitter_->unlink();  // For the sake of this demo
    o1Twitter_->link();
}

void Tweeter::doXAuth(const QString &username, const QString &password) {
    oxTwitter_ = new OXTwitter(this);
    oxTwitter_->setClientId(O2_CONSUMER_KEY);
    oxTwitter_->setClientSecret(O2_CONSUMER_SECRET);
    oxTwitter_->setLocalPort(localPort);

    oxTwitter_->setUsername(username);
    oxTwitter_->setPassword(password);

    // Create a store object for writing the received tokens
    O0SettingsStore *store = new O0SettingsStore(O2_ENCRYPTION_KEY);
    store->setGroupKey("twitter");
    oxTwitter_->setStore(store);

    connect(oxTwitter_, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(oxTwitter_, SIGNAL(linkingFailed()), this, SIGNAL(linkingFailed()));
    connect(oxTwitter_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(oxTwitter_, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(oxTwitter_, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));

    qDebug() << "Starting XAuth...";
    qDebug() << "Username:" << username << "Password:" << password;
    oxTwitter_->unlink();  // For the sake of this demo
    oxTwitter_->link();
}

void Tweeter::postStatusUpdate(const QString &message) {
    if (!o1Twitter_->linked()) {
        qWarning() << "Application is not linked to Twitter!";
        emit statusPosted();
        return;
    }

    qDebug() << "Status update message:" << message.toLatin1().constData();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    O1Twitter* o1 = o1Twitter_;
    O1Requestor* requestor = new O1Requestor(manager, o1, this);

    QByteArray paramName("status");

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
    reqParams << O0RequestParameter(paramName, message.toLatin1());

    QByteArray postData = O1::createQueryParameters(reqParams);

    QUrl url = QUrl("https://api.twitter.com/1.1/statuses/update.json");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);

    QNetworkReply *reply = requestor->post(request, reqParams, postData);
    connect(reply, SIGNAL(finished()), this, SLOT(tweetReplyDone()));
}

void Tweeter::onOpenBrowser(const QUrl &url) {
    qDebug() << "Opening browser with URL" << url.toString();
    QDesktopServices::openUrl(url);
}

void Tweeter::onCloseBrowser() {
}

void Tweeter::onLinkedChanged() {
    qDebug() << "Linked changed!";
}

void Tweeter::onLinkingSucceeded() {
    O1Twitter *o1t = qobject_cast<O1Twitter *>(sender());
    if (!o1t->linked()) {
        return;
    }
    QVariantMap extraTokens = o1t->extraTokens();
    if (!extraTokens.isEmpty()) {
        emit extraTokensReady(extraTokens);
        qDebug() << "Extra tokens in response:";
        foreach (QString key, extraTokens.keys()) {
            qDebug() << "\t" << key << ":" << (extraTokens.value(key).toString().left(3) + "...");
        }
    }
    emit linkingSucceeded();
}

void Tweeter::tweetReplyDone() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ERROR:" << reply->errorString();
        qDebug() << "Content:" << reply->readAll();
    } else {
        qDebug() << "Tweet posted sucessfully!";
    }
    emit statusPosted();
}
