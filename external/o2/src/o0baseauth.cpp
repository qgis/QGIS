#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <QUrlQuery>

#include "o0baseauth.h"
#include "o0globals.h"
#include "o0settingsstore.h"
#include "o2replyserver.h"
#include "o2pollserver.h"

static const quint16 DefaultLocalPort = 1965;

std::function<void( const QString&, O0BaseAuth::LogLevel level ) > O0BaseAuth::sLoggingFunction;

O0BaseAuth::O0BaseAuth(QObject *parent, O0AbstractStore *store): QObject(parent), localPort_( DefaultLocalPort ) {
    setStore(store);
}

void O0BaseAuth::setStore(O0AbstractStore *store) {
    if (store_) {
        store_->deleteLater();
    }
    if (store) {
        store_ = store;
        store_->setParent(this);
    } else {
        store_ = new O0SettingsStore(O2_ENCRYPTION_KEY, this);
        return;
    }
}

bool O0BaseAuth::linked() {
    QString key = QString(O2_KEY_LINKED).arg(clientId_);
    bool result = !store_->value(key).isEmpty();
    log( QStringLiteral( "O0BaseAuth::linked: %1 " ).arg( result? "Yes": "No" ) );
    return result;
}

void O0BaseAuth::setLinked(bool v) {
    log( QStringLiteral( "O0BaseAuth::setLinked: %1 " ).arg( v? "true": "false" ) );
    bool oldValue = linked();
    QString key = QString(O2_KEY_LINKED).arg(clientId_);
    store_->setValue(key, v? "1": "");
    if (oldValue != v) {
        Q_EMIT linkedChanged();
    }
}

QString O0BaseAuth::tokenSecret() {
    QString key = QString(O2_KEY_TOKEN_SECRET).arg(clientId_);
    return store_->value(key);
}

void O0BaseAuth::setTokenSecret(const QString &v) {
    QString key = QString(O2_KEY_TOKEN_SECRET).arg(clientId_);
    store_->setValue(key, v);
    Q_EMIT tokenSecretChanged();
}

QString O0BaseAuth::token() {
    QString key = QString(O2_KEY_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O0BaseAuth::setToken(const QString &v) {
    QString key = QString(O2_KEY_TOKEN).arg(clientId_);
    store_->setValue(key, v);
    Q_EMIT tokenChanged();
}

QString O0BaseAuth::clientId() {
    return clientId_;
}

void O0BaseAuth::setClientId(const QString &value) {
    clientId_ = value;
    Q_EMIT clientIdChanged();
}

QString O0BaseAuth::clientSecret() {
    return clientSecret_;
}

void O0BaseAuth::setClientSecret(const QString &value) {
    clientSecret_ = value;
    Q_EMIT clientSecretChanged();
}

bool O0BaseAuth::useExternalWebInterceptor() {
    return useExternalWebInterceptor_;
}

void O0BaseAuth::setUseExternalWebInterceptor(bool useExternalWebInterceptor) {
    if ( useExternalWebInterceptor_ == useExternalWebInterceptor )
        return;
    useExternalWebInterceptor_ = useExternalWebInterceptor;
    Q_EMIT useExternalWebInterceptorChanged(useExternalWebInterceptor_);
}

QByteArray O0BaseAuth::replyContent() const {
    return replyContent_;
}

void O0BaseAuth::setReplyContent(const QByteArray &value) {
    bool changed = replyContent_ != value;
    replyContent_ = value;
    if (replyServer_) {
        replyServer_->setReplyContent(replyContent_);
    }
    if ( changed )
    {
        Q_EMIT replyContentChanged();
    }
}

int O0BaseAuth::localPort() {
    return localPort_;
}

void O0BaseAuth::setLocalPort(int value) {
    log( QStringLiteral( "O0BaseAuth::setLocalPort:%1" ).arg( value ) );
    localPort_ = static_cast<quint16>(value);
    Q_EMIT localPortChanged();
}

QVariantMap O0BaseAuth::extraTokens() {
    const QString key = QString(O2_KEY_EXTRA_TOKENS).arg(clientId_);
    QByteArray bytes = QByteArray::fromBase64(store_->value(key).toLatin1());
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    stream >> extraTokens_;
    return extraTokens_;
}

void O0BaseAuth::setExtraTokens(QVariantMap extraTokens) {
    extraTokens_ = extraTokens;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << extraTokens;
    QString key = QString(O2_KEY_EXTRA_TOKENS).arg(clientId_);
    store_->setValue(key, bytes.toBase64());
    Q_EMIT extraTokensChanged();
}

void O0BaseAuth::setReplyServer(O2ReplyServer * server)
{
    delete replyServer_;

    replyServer_ = server;
    replyServer_->setReplyContent(replyContent_);
}

O2ReplyServer * O0BaseAuth::replyServer() const
{
    return replyServer_;
}

void O0BaseAuth::setPollServer(O2PollServer *server)
{
    if (pollServer_)
        pollServer_->deleteLater();

    pollServer_ = server;
}

O2PollServer *O0BaseAuth::pollServer() const
{
    return pollServer_;
}

void O0BaseAuth::setLoggingFunction( std::function<void (const QString&, LogLevel)> function)
{
    sLoggingFunction = std::move( function );
}

void O0BaseAuth::log(const QString& message, LogLevel level)
{
    if ( sLoggingFunction )
    {
        sLoggingFunction( message, level );
    }
    else
    {
        switch ( level )
        {
            case LogLevel::Debug:
                qDebug() << message;
                break;
            case LogLevel::Warning:
                qWarning() << message;
                break;
            case LogLevel::Critical:
                qCritical() << message;
                break;
        }
    }
}

QByteArray O0BaseAuth::createQueryParameters(const QList<O0RequestParameter> &parameters) {
    QByteArray ret;
    bool first = true;
    for (const O0RequestParameter& h : parameters) {
        if (first) {
            first = false;
        } else {
            ret.append("&");
        }
        ret.append(QUrl::toPercentEncoding(h.name) + "=" + QUrl::toPercentEncoding(h.value));
    }
    return ret;
}
