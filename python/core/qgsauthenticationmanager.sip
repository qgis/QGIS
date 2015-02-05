class QSqlDatabase;

class QgsAuthManager : QObject
{
%TypeHeaderCode
#include <qgsauthenticationmanager.h>
%End

  public:

    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2
    };

    static QgsAuthManager *instance();

    QSqlDatabase authDbConnection() const;

    const QString authDbConfigTable() const;

    bool init();

    bool isDisabled() const;

    const QString disabledMessage() const;

    const QString authenticationDbPath() const;

    bool setMasterPassword( bool verify = false );

    bool setMasterPassword( const QString& pass, bool verify = false );

    bool verifyMasterPassword();

    bool masterPasswordIsSet() const;

    bool masterPasswordHashInDb() const;

    void clearMasterPassword();

    bool masterPasswordSame( const QString& pass ) const;

    bool resetMasterPassword( const QString& newpassword, const QString& oldpass, bool keepbackup, QString *backuppath /In,Out/ = 0 );

    const QString authManTag() const;


    void registerProviders();

    void updateConfigProviderTypes();

    QgsAuthProvider* configProvider( const QString& authid );

    QgsAuthType::ProviderType configProviderType( const QString& authid );

    const QString uniqueConfigId() const;

    bool configIdUnique( const QString &id ) const;

    QStringList configIds() const;

    QHash<QString, QgsAuthConfigBase> availableConfigs();


    bool storeAuthenticationConfig( QgsAuthConfigBase &config /In,Out/ );

    bool updateAuthenticationConfig( const QgsAuthConfigBase& config );

    bool loadAuthenticationConfig( const QString& authid, QgsAuthConfigBase &config, bool full = false );

    bool removeAuthenticationConfig( const QString& authid );

    bool removeAllAuthenticationConfigs();

    bool eraseAuthenticationDatabase();

    bool updateNetworkRequest( QNetworkRequest &request /In,Out/, const QString& authid );

    bool updateNetworkReply( QNetworkReply *reply, const QString& authid );

  signals:
    void messageOut( const QString& message, const QString& tag = smAuthManTag, QgsAuthManager::MessageLevel level = INFO ) const;

    void masterPasswordVerified( bool verified ) const;

  public slots:
    void clearAllCachedConfigs();

    void clearCachedConfig( const QString& authid );

  protected:
    explicit QgsAuthManager();
    ~QgsAuthManager();
};
