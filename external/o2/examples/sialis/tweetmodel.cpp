#include <QDebug>

#include "tweetmodel.h"

TweetModel::TweetModel(QObject *parent): QAbstractListModel(parent) {
}

QHash<int, QByteArray> TweetModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[RoleText] = "rawText";
    return roles;
}

void TweetModel::addTweet(QVariantMap tweet) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    tweets.append(tweet);
    endInsertRows();
}

void TweetModel::clearTweets() {
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    tweets.clear();
    endRemoveRows();
}

int TweetModel::rowCount(const QModelIndex &) const {
    return tweets.count();
}

QVariant TweetModel::data(const QModelIndex &index, int role) const {
    QVariant result;
    QVariantMap tweet = tweets[index.row()];
    switch (role) {
    case TweetModel::RoleText:
        result = tweet["text"];
    default:
        result = tweet["text"];
    }
    return result;
}
