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

/// Simple OAuth2 authenticator.
class O0_EXPORT O2: public O0BaseAuth
{
    Q_OBJECT
public:

    /// Authorization flow types.
    enum GrantFlow {
        GrantFlowAuthorizationCode, ///< @see http://tools.ietf.org/html/draft-ietf-oauth-v2-15#section-4.1
        GrantFlowImplicit, ///< @see http://tools.ietf.org/html/draft-ietf-oauth-v2-15#section-4.2
        GrantFlowResourceOwnerPasswordCredentials,
        GrantFlowPkce, ///< @see https://www.rfc-editor.org/rfc/rfc7636
        GrantFlowDevice ///< @see https://tools.ietf.org/html/rfc8628#section-1
    };
    Q_ENUM(GrantFlow)

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
    Q_PROPERTY(QString localhostPolicy READ localhostPolicy WRITE setLocalhostPolicy NOTIFY localHostPolicyChanged)
    QString localhostPolicy() const;
    void setLocalhostPolicy(const QString &value);

    /// API key.
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    QString apiKey();
    void setApiKey(const QString &value);

    /// Allow ignoring SSL errors?
    /// E.g. SurveyMonkey fails on Mac due to SSL error. Ignoring the error circumvents the problem
    Q_PROPERTY(bool ignoreSslErrors READ ignoreSslErrors WRITE setIgnoreSslErrors NOTIFY ignoreSslErrorsChanged)
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

    /// Grant type (if non-standard)
    Q_PROPERTY(QString grantType READ grantType WRITE setGrantType NOTIFY grantTypeChanged)
    QString grantType();
    void setGrantType(const QString &value);

public:
    /// Constructor.
    /// @param  parent  Parent object.
    explicit O2(QObject *parent = nullptr, QNetworkAccessManager *manager = nullptr, O0AbstractStore *store = nullptr);

    /// Get authentication code.
    QString code();

    /// Get refresh token.
    QString refreshToken();

    /// Get token expiration time (seconds from Epoch).
    qint64 expires();

public Q_SLOTS:
    /// Authenticate.
    Q_INVOKABLE void link() override;

    /// De-authenticate.
    Q_INVOKABLE void unlink() override;

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
    void localHostPolicyChanged(const QString& policy);
    void apiKeyChanged(const QString& key);
    void ignoreSslErrorsChanged(bool ignore);
    void grantTypeChanged(const QString& type);

public Q_SLOTS:
    /// Handle verification response.
    virtual void onVerificationReceived(QMap<QString, QString>);

protected Q_SLOTS:
    /// Handle completion of a token request.
    virtual void onTokenReplyFinished();

    /// Handle failure of a token request.
    virtual void onTokenReplyError(QNetworkReply::NetworkError error);

    /// Handle completion of a refresh request.
    virtual void onRefreshFinished();

    /// Handle failure of a refresh request.
    virtual void onRefreshError(QNetworkReply::NetworkError error);

    /// Handle completion of a Device Authorization Request
    virtual void onDeviceAuthReplyFinished();

protected:
    /// Build HTTP request body.
    QByteArray buildRequestBody(const QMap<QString, QString> &parameters);

    /// Set authentication code.
    void setCode(const QString &c);

    /// Set refresh token.
    void setRefreshToken(const QString &v);

    /// Set token expiration time.
    void setExpires(qint64 v);

    /// Returns the QNetworkAccessManager instance to use for network access
    virtual QNetworkAccessManager *getManager();

    /// Start polling authorization server
    void startPollServer(const QVariantMap &params);

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
    O2ReplyList timedReplies_;
    GrantFlow grantFlow_;
    QString grantType_;

    friend class TestO2;
};

#endif // O2_H
