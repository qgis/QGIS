#ifndef O1_H
#define O1_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>

#include "o0export.h"
#include "o0baseauth.h"

class O2ReplyServer;

/// Simple OAuth 1.0 authenticator.
class O0_EXPORT O1: public O0BaseAuth {
    Q_OBJECT

public:
    /// Signature method
    Q_PROPERTY(QString signatureMethod READ signatureMethod WRITE setSignatureMethod NOTIFY signatureMethodChanged)
    QString signatureMethod();
    void setSignatureMethod(const QString &value);

    /// Token request URL.
    Q_PROPERTY(QUrl requestTokenUrl READ requestTokenUrl WRITE setRequestTokenUrl NOTIFY requestTokenUrlChanged)
    QUrl requestTokenUrl();
    void setRequestTokenUrl(const QUrl &value);

    /// Parameters to pass with request URL.
    Q_PROPERTY(QList<O0RequestParameter> requestParameters READ requestParameters WRITE setRequestParameters);
    QList<O0RequestParameter> requestParameters();
    void setRequestParameters(const QList<O0RequestParameter> &value);

    /// Callback URL.
    /// It should contain a `%1` place marker, to be replaced by `O0BaseAuth::localPort()`.
    /// Defaults to `O2_CALLBACK_URL`.
    Q_PROPERTY(QString callbackUrl READ callbackUrl WRITE setCallbackUrl)
    QString callbackUrl();
    void setCallbackUrl(const QString &value);

    /// Authorization URL.
    Q_PROPERTY(QUrl authorizeUrl READ authorizeUrl WRITE setAuthorizeUrl NOTIFY authorizeUrlChanged)
    QUrl authorizeUrl();
    void setAuthorizeUrl(const QUrl &value);

    /// Access token URL.
    Q_PROPERTY(QUrl accessTokenUrl READ accessTokenUrl WRITE setAccessTokenUrl NOTIFY accessTokenUrlChanged)
    QUrl accessTokenUrl();
    void setAccessTokenUrl(const QUrl &value);

    /// Constructor.
    explicit O1(QObject *parent = 0, QNetworkAccessManager *manager = 0);

    /// Parse a URL-encoded response string.
    static QMap<QString, QString> parseResponse(const QByteArray &response);

    /// Build the value of the "Authorization:" header.
    static QByteArray buildAuthorizationHeader(const QList<O0RequestParameter> &oauthParams);

    /// Create unique bytes to prevent replay attacks.
    static QByteArray nonce();

    /// Generate signature string depending on signature method type
    QByteArray generateSignature(const QList<O0RequestParameter> headers, const QNetworkRequest &req, const QList<O0RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation);

    /// Calculate the HMAC-SHA1 signature of a request.
    /// @param  oauthParams     OAuth parameters.
    /// @param  otherParams     Other parameters participating in signing.
    /// @param  URL             Request URL. May contain query parameters, but they will not be used for signing.
    /// @param  op              HTTP operation.
    /// @param  consumerSecret  Consumer (application) secret.
    /// @param  tokenSecret     Authorization token secret (empty if not yet available).
    /// @return Signature that can be used as the value of the "oauth_signature" parameter.
    static QByteArray sign(const QList<O0RequestParameter> &oauthParams, const QList<O0RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret);

    /// Build a base string for signing.
    static QByteArray getRequestBase(const QList<O0RequestParameter> &oauthParams, const QList<O0RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op);

    /// Build a concatenated/percent-encoded string from a list of headers.
    static QByteArray encodeHeaders(const QList<O0RequestParameter> &headers);

public Q_SLOTS:
    /// Authenticate.
    Q_INVOKABLE virtual void link();

    /// De-authenticate.
    Q_INVOKABLE virtual void unlink();

Q_SIGNALS:
    void requestTokenUrlChanged();
    void authorizeUrlChanged();
    void accessTokenUrlChanged();
    void signatureMethodChanged();

protected Q_SLOTS:
    /// Handle verification received from the reply server.
    virtual void onVerificationReceived(QMap<QString,QString> params);

    /// Handle token request error.
    virtual void onTokenRequestError(QNetworkReply::NetworkError error);

    /// Handle token request finished.
    virtual void onTokenRequestFinished();

    /// Handle token exchange error.
    void onTokenExchangeError(QNetworkReply::NetworkError error);

    /// Handle token exchange finished.
    void onTokenExchangeFinished();

protected:
    /// Exchange temporary token to authentication token
    void exchangeToken();

    QUrl requestUrl_;
    QList<O0RequestParameter> requestParameters_;
    QString callbackUrl_;
    QUrl tokenUrl_;
    QUrl refreshTokenUrl_;
    QString verifier_;
    QString signatureMethod_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
};

#endif // O1_H
