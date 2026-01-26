#ifndef O0BASEAUTH_H
#define O0BASEAUTH_H

#include <QByteArray>
#include <QObject>
#include <QMap>
#include <QString>
#include <QUrl>
#include <QVariantMap>

#include "o0export.h"
#include "o0abstractstore.h"
#include "o0requestparameter.h"

class O2ReplyServer;
class O2PollServer;

/// Base class of OAuth authenticators
class O0_EXPORT O0BaseAuth : public QObject
{
    Q_OBJECT
public:
    enum class LogLevel
    {
        Debug,
        Warning,
        Critical
    };
    explicit O0BaseAuth(QObject *parent = nullptr, O0AbstractStore *store = nullptr);

public:
    /// Are we authenticated?
    Q_PROPERTY(bool linked READ linked WRITE setLinked NOTIFY linkedChanged)
    bool linked();

    /// Authentication token.
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)
    QString token();

    /// Authentication token secret.
    Q_PROPERTY(QString tokenSecret READ tokenSecret NOTIFY tokenSecretChanged)
    QString tokenSecret();

    /// Provider-specific extra tokens, available after a successful authentication
    Q_PROPERTY(QVariantMap extraTokens READ extraTokens NOTIFY extraTokensChanged)
    QVariantMap extraTokens();

    /// Client application ID.
    /// O1 instances with the same (client ID, client secret) share the same "linked", "token" and "tokenSecret" properties.
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
    QString clientId();
    void setClientId(const QString &value);

    /// Client application secret.
    /// O1 instances with the same (client ID, client secret) share the same "linked", "token" and "tokenSecret" properties.
    Q_PROPERTY(QString clientSecret READ clientSecret WRITE setClientSecret NOTIFY clientSecretChanged)
    QString clientSecret();
    void setClientSecret(const QString &value);

    /// Should we use a reply server (default) or an external web interceptor?
    Q_PROPERTY(bool useExternalWebInterceptor READ useExternalWebInterceptor WRITE setUseExternalWebInterceptor NOTIFY useExternalWebInterceptorChanged)
    bool useExternalWebInterceptor();
    void setUseExternalWebInterceptor(bool useExternalWebInterceptor);

    /// Page content on local host after successful oauth.
    /// Provide it in case you do not want to close the browser, but display something
    Q_PROPERTY(QByteArray replyContent READ replyContent WRITE setReplyContent NOTIFY replyContentChanged)
    QByteArray replyContent() const;
    void setReplyContent(const QByteArray &value);

    /// TCP port number to use in local redirections.
    /// The OAuth "redirect_uri" will be set to "http://localhost:<localPort>/".
    /// If localPort is set to 0 (default), O2 will replace it with a free one.
    Q_PROPERTY(int localPort READ localPort WRITE setLocalPort NOTIFY localPortChanged)
    int localPort();
    void setLocalPort(int value);

    /// Sets the storage object to use for storing the OAuth tokens on a peristent medium
    void setStore(O0AbstractStore *store);

    /// Construct query string from list of headers
    static QByteArray createQueryParameters(const QList<O0RequestParameter> &parameters);

    /// Sets a custom logging function to use instead of the default qDebug()/qWarning() mechanism
    static void setLoggingFunction( std::function<void( const QString&, LogLevel level ) > function );

    /// Logs a message
    ///
    /// This will default to using qDebug/qWarning, unless a custom logger function has been registered
    static void log( const QString& message, LogLevel level = LogLevel::Debug );

public Q_SLOTS:
    /// Authenticate.
    Q_INVOKABLE virtual void link() = 0;

    /// De-authenticate.
    Q_INVOKABLE virtual void unlink() = 0;

Q_SIGNALS:
    /// Emitted when client needs to open a web browser window, with the given URL.
    void openBrowser(const QUrl &url);

    /// Emitted when client can close the browser window.
    void closeBrowser();

    /// Emitted when client needs to show a verification uri and user code
    void showVerificationUriAndCode(const QUrl &uri, const QString &code);

    /// Emitted when authentication/deauthentication succeeded.
    void linkingSucceeded();

    /// Emitted when authentication/deauthentication failed.
    void linkingFailed();

    // Property change signals

    void linkedChanged();
    void clientIdChanged();
    void clientSecretChanged();
    void localPortChanged();
    void tokenChanged();
    void tokenSecretChanged();
    void extraTokensChanged();
    void useExternalWebInterceptorChanged( bool enabled );
    void replyContentChanged();

protected:
    /// Set authentication token.
    void setToken(const QString &v);

    /// Set authentication token secret.
    void setTokenSecret(const QString &v);

    /// Set the linked state
    void setLinked(bool v);

    /// Set extra tokens found in OAuth response
    void setExtraTokens(QVariantMap extraTokens);

    /// Set local reply server
    void setReplyServer(O2ReplyServer *server);

    O2ReplyServer * replyServer() const;

    /// Set local poll server
    void setPollServer(O2PollServer *server);

    O2PollServer * pollServer() const;

protected:
    QString clientId_;
    QString clientSecret_;
    QString redirectUri_;
    QString requestToken_;
    QString requestTokenSecret_;
    QUrl requestTokenUrl_;
    QUrl authorizeUrl_;
    QUrl accessTokenUrl_;
    quint16 localPort_;
    O0AbstractStore *store_{nullptr};
    QVariantMap extraTokens_;
    QByteArray pkceCodeVerifier_;
    QString pkceCodeChallenge_;
    bool useExternalWebInterceptor_{false};
    QByteArray replyContent_;
    static std::function<void( const QString&, LogLevel level ) > sLoggingFunction;

private:
    O2ReplyServer *replyServer_{nullptr};
    O2PollServer *pollServer_{nullptr};

    friend class TestBaseAuth;
    friend class TestO2;
};

#endif // O0BASEAUTH
