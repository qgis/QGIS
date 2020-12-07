#ifndef TWEETMODEL_H
#define TWEETMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

/// List of tweets, suitable as a ListView model
class TweetModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        RoleText = Qt::UserRole + 1,
    };

    TweetModel(QObject *parent = 0);

    /// Clear all tweets
    void clearTweets();

    /// Add a tweet
    void addTweet(QVariantMap tweet);

    /// Get number of tweets
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /// Access a tweet
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    /// Get role names
    QHash<int, QByteArray>roleNames() const;

protected:
    QList<QVariantMap> tweets;
};

#endif // TWEETMODEL_H
