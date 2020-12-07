#ifndef OXTWITTER_H
#define OXTWITTER_H

#include "o0export.h"
#include "o1twitter.h"

/// Twitter authenticator using Twitter XAuth
class O0_EXPORT OXTwitter: public O1Twitter {
    Q_OBJECT

public:
    explicit OXTwitter(QObject *parent = 0);

    /// XAuth Username
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    QString username();
    void setUsername(const QString &username);

    /// XAuth Password
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    QString password();
    void setPassword(const QString &password);

public Q_SLOTS:
    /// Authenticate.
    Q_INVOKABLE virtual void link();

Q_SIGNALS:
    void usernameChanged();
    void passwordChanged();

private:
    QList<O0RequestParameter> xAuthParams_;
    QString username_;
    QString password_;
};

#endif // OXTWITTER_H
