#ifndef O2TIMEDREPLYLIST_H
#define O2TIMEDREPLYLIST_H

#include <QList>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QByteArray>

#include "o0export.h"

/// A network request/reply pair that can time out.
class O0_EXPORT O2Reply: public QTimer {
    Q_OBJECT

public:
    explicit O2Reply(QNetworkReply *reply, int timeOut = 60 * 1000, QObject *parent = nullptr);

Q_SIGNALS:
    void error(QNetworkReply::NetworkError);

public Q_SLOTS:
    /// When time out occurs, the QNetworkReply's error() signal is triggered.
    void onTimeOut();

public:
    QNetworkReply *reply;
};

/// List of O2Replies.
class O0_EXPORT O2ReplyList {
public:
    O2ReplyList() { ignoreSslErrors_ = false; }

    /// Destructor.
    /// Deletes all O2Reply instances in the list.
    virtual ~O2ReplyList();

    /// Create a new O2Reply from a QNetworkReply, and add it to this list.
    void add(QNetworkReply *reply);

    /// Add an O2Reply to the list, while taking ownership of it.
    void add(O2Reply *reply);

    /// Remove item from the list that corresponds to a QNetworkReply.
    void remove(QNetworkReply *reply);

    /// Find an O2Reply in the list, corresponding to a QNetworkReply.
    /// @return Matching O2Reply or NULL.
    O2Reply *find(const QNetworkReply* reply);

    bool ignoreSslErrors() const;
    void setIgnoreSslErrors(bool ignoreSslErrors);

protected:
    QList<O2Reply *> replies_;
    bool ignoreSslErrors_;
};

#endif // O2TIMEDREPLYLIST_H
