#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "o1requestor.h"

#include "twitterapi.h"

TwitterApi::TwitterApi(QObject *parent): QObject(parent), authenticator_(0) {
    manager_ = new QNetworkAccessManager(this);
    tweetModel_ = new TweetModel(this);
}

TwitterApi::~TwitterApi() {
}

TweetModel *TwitterApi::tweetModel() {
    return tweetModel_;
}

O1Twitter *TwitterApi::authenticator() const {
    return authenticator_;
}
void TwitterApi::setAuthenticator(O1Twitter *v) {
    authenticator_ = v;
}

void TwitterApi::requestTweets() {
    if (!authenticator_ || !authenticator_->linked()) {
        tweetModel_->clearTweets();
        emit tweetModelChanged();
        return;
    }
    O1Requestor *requestor = new O1Requestor(manager_, authenticator_, this);
    QUrl url = QUrl("https://api.twitter.com/1.1/statuses/home_timeline.json");
    QNetworkRequest request(url);
    QNetworkReply *reply = requestor->get(request, QList<O0RequestParameter>());
    connect(reply, SIGNAL(finished()), this, SLOT(tweetsReceived()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestFailed(QNetworkReply::NetworkError)));
}

void TwitterApi::tweetsReceived() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
    tweetModel_->clearTweets();
    QJsonArray jsonArray = jsonResponse.array();
    foreach (const QJsonValue &v, jsonArray) {
        QVariantMap item = v.toObject().toVariantMap();
        tweetModel_->addTweet(item);
    }
    emit tweetModelChanged();
}

void TwitterApi::requestFailed(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "TwitterApi::requestFailed:" << (int)error << reply->errorString() << reply->readAll();
}
