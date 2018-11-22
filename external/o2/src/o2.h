#ifndef O2_H
#define O2_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPair>

#include "o0export.h"
#include "o0baseauth.h"
#include "o2reply.h"
#include "o0abstractstore.h"

class O2ReplyServer;

/// Simple OAuth2 authenticator.
class O0_EXPORT O2: public O0BaseAuth {
    Q_OBJECT
    Q_ENUMS(GrantFlow)

public:
    /// Authorization flow types.
    enum GrantFlow {
        GrantFlowAuthorizationCode, ///< @see http://tools.ietf.org/html/draft-ietf-oauth-v2-15#section-4.1
        GrantFlowImplicit, ///< @see http://tools.ietf.org/html/draft-ietf-oauth-v2-15#section-4.2
        GrantFlowResourceOwnerPasswordCredentials,
    };

    /// Authorization flow.
    Q_PROPERTY(GrantFlow grantFlow READ grantFlow WRITE setGrantFlow NOTIFY grantFlowChanged)
    GrantFlow grantFlow();
    void setGrantFlow(GrantFlow value);

    /// Resource owner username.
    /// O2 instances with the same (username, password) share the same "linked" and "token" properties.
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    QString username();
    void setUsername(const QString &value);

    /// Resource owner password.
    /// O2 instances with the same (username, password) share the same "linked" and "token" properties.
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    QString password();
    void setPassword(const QString &value);

    /// Scope of authentication.
    Q_PROPERTY(QString scope READ scope WRITE setScope NOTIFY scopeChanged)
    QString scope();
    void setScope(const QString &value);

    /// Localhost policy. By default it's value is http://127.0.0.1:%1/, however some services may
    /// require the use of http://localhost:%1/ or any other value.
    Q_PROPERTY(QString localhostPolicy READ localhostPolicy WRITE setLocalhostPolicy)
    QString localhostPolicy() const;
    void setLocalhostPolicy(const QString &value);

    /// API key.
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey)
    QString apiKey();
    void setApiKey(const QString &value);

    /// Page content on local host after successful oauth.
    /// Provide it in case you do not want to close the browser, but display something
    Q_PROPERTY(QByteArray replyContent READ replyContent WRITE setReplyContent)
    QByteArray replyContent();
    void setReplyContent(const QByteArray &value);

    /// Allow ignoring SSL errors?
    /// E.g. SurveyMonkey fails on Mac due to SSL error. Ignoring the error circumvents the problem
    Q_PROPERTY(bool ignoreSslErrors READ ignoreSslErrors WRITE setIgnoreSslErrors)
    bool ignoreSslErrors();
    void setIgnoreSslErrors(bool ignoreSslErrors);

    /// Request URL.
    Q_PROPERTY(QString requestUrl READ requestUrl WRITE setRequestUrl NOTIFY requestUrlChanged)
    QString requestUrl();
    void setRequestUrl(const QString &value);

    /// User-defined extra parameters to append to request URL
    Q_PROPERTY(QVariantMap extraRequestParams READ extraRequestParams WRITE setExtraRequestParams NOTIFY extraRequestParamsChanged)
    QVariantMap extraRequestParams();
    void setExtraRequestParams(const QVariantMap &value);

    /// Token request URL.
    Q_PROPERTY(QString tokenUrl READ tokenUrl WRITE setTokenUrl NOTIFY tokenUrlChanged)
    QString tokenUrl();
    void setTokenUrl(const QString &value);

    /// Token refresh URL.
    Q_PROPERTY(QString refreshTokenUrl READ refreshTokenUrl WRITE setRefreshTokenUrl NOTIFY refreshTokenUrlChanged)
    QString refreshTokenUrl();
    void setRefreshTokenUrl(const QString &value);

public:
    /// Constructor.
    /// @param  parent  Parent object.
    explicit O2(QObject *parent = 0, QNetworkAccessManager *manager = 0);

    /// Get authentication code.
    QString code();

    /// Get refresh token.
    QString refreshToken();

    /// Get token expiration time (seconds from Epoch).
    int expires();

public Q_SLOTS:
    /// Authenticate.
    Q_INVOKABLE virtual void link();

    /// De-authenticate.
    Q_INVOKABLE virtual void unlink();

    /// Refresh token.
    Q_INVOKABLE void refresh();

    /// Handle situation where reply server has opted to close its connection
    void serverHasClosed(bool paramsfound = false);

Q_SIGNALS:
    /// Emitted when a token refresh has been completed or failed.
    void refreshFinished(QNetworkReply::NetworkError error);

    // Property change signals
    void grantFlowChanged();
    void scopeChanged();
    void usernameChanged();
    void passwordChanged();
    void requestUrlChanged();
    void extraRequestParamsChanged();
    void refreshTokenUrlChanged();
    void tokenUrlChanged();

protected Q_SLOTS:
    /// Handle verification response.
    virtual void onVerificationReceived(QMap<QString, QString>);

    /// Handle completion of a token request.
    virtual void onTokenReplyFinished();

    /// Handle failure of a token request.
    virtual void onTokenReplyError(QNetworkReply::NetworkError error);

    /// Handle completion of a refresh request.
    virtual void onRefreshFinished();

    /// Handle failure of a refresh request.
    virtual void onRefreshError(QNetworkReply::NetworkError error);

protected:
    /// Build HTTP request body.
    QByteArray buildRequestBody(const QMap<QString, QString> &parameters);

    /// Set authentication code.
    void setCode(const QString &v);

    /// Set refresh token.
    void setRefreshToken(const QString &v);

    /// Set token expiration time.
    void setExpires(int v);

protected:
    QString username_;
    QString password_;
    QUrl requestUrl_;
    QVariantMap extraReqParams_;
    QUrl tokenUrl_;
    QUrl refreshTokenUrl_;
    QString scope_;
    QString code_;
    QString localhostPolicy_;
    QString apiKey_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
    O2ReplyList timedReplies_;
    GrantFlow grantFlow_;
};

#endif // O2_H
