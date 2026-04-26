#ifndef QGSAIMODELROUTER_H
#define QGSAIMODELROUTER_H

#include "qgis_app.h"
#include "qgsaimodels.h"

#include <QMap>
#include <QObject>
#include <QQueue>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrl>

class QNetworkRequest;

class APP_EXPORT QgsAiModelRouter : public QObject
{
    Q_OBJECT

  public:
    enum class Provider
    {
      OpenAi,
      Claude,
      Plan
    };
    Q_ENUM( Provider )

    struct ProviderSettings
    {
      QString endpoint;
      QString model;
      QString authConfigId;
      bool enabled = false;
    };

    explicit QgsAiModelRouter( QObject *parent = nullptr );

    ProviderSettings providerSettings( Provider provider ) const;
    void setProviderSettings( Provider provider, const ProviderSettings &settings );

    QString startChatRequest( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream = true );
    void cancelRequest( const QString &requestId );
    bool hasActiveRequest( const QString &requestId ) const;

    bool storeApiKey( Provider provider, const QString &apiKey, QString *errorMessage = nullptr );
    bool setPlanSessionToken( const QString &token, QString *errorMessage = nullptr );
    void setPlanAuthConfigId( const QString &authConfigId );

    bool applyAuthentication( Provider provider, QNetworkRequest &request, QString *errorMessage = nullptr ) const;
    Provider resolveProvider() const;

    QString providerDisplayName( Provider provider ) const;
    QByteArray buildRequestPayload( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream ) const;
    QString sanitizeErrorText( const QString &errorText ) const;
    bool hasStoredApiKey( Provider provider ) const;

  signals:
    void requestProgress( const QString &requestId, const QString &chunk );
    void requestFinished( const QString &requestId, bool success, const QString &providerName, const QString &responseText, const QString &errorMessage, int httpStatus, int retryCount, bool retriable, qint64 latencyMs );

  private slots:
    void onReplyReadyRead();
    void onReplyFinished();

  private:
    struct RequestContext
    {
      QString requestId;
      Provider provider = Provider::OpenAi;
      QList<QgsAiChatMessage> messages;
      bool stream = true;
      int attempt = 0;
      int maxRetries = 1;
      qint64 startedAtMs = 0;
      QString streamingBuffer;
      QString aggregatedText;
      QNetworkReply *reply = nullptr;
    };

    bool dispatchRequest( RequestContext &context );
    bool shouldRetry( int httpStatus, QNetworkReply::NetworkError networkError, int attempt, int maxRetries ) const;
    QString extractTextFromResponse( Provider provider, const QJsonObject &object ) const;
    QString extractTextFromStreamEvent( Provider provider, const QJsonObject &object ) const;
    QString roleForProvider( Provider provider, QgsAiChatRole role ) const;

    static QString generateRequestId();
    QString authHeaderName( Provider provider ) const;
    QString authHeaderValue( Provider provider, const QString &secret ) const;
    QString authConfigSettingKey( Provider provider ) const;
    QString providerSettingPrefix( Provider provider ) const;
    QString endpointSettingKey( Provider provider ) const;
    QString modelSettingKey( Provider provider ) const;
    QString enabledSettingKey( Provider provider ) const;
    QString apiKeySettingKey( Provider provider ) const;
    QString planAuthConfigIdSettingKey() const;
    QString planSessionTokenSettingKey() const;
    QString storedApiKey( Provider provider ) const;
    bool hasConfiguredCredential( Provider provider ) const;
    void loadPersistedProviderSettings();
    void persistProviderSettings( Provider provider, const ProviderSettings &settings ) const;
    RequestContext *contextFromReply( QNetworkReply *reply );

    QMap<Provider, ProviderSettings> mProviderSettings;
    QMap<QString, RequestContext> mRequests;
};

#endif // QGSAIMODELROUTER_H
