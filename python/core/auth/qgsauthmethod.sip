class QgsAuthMethod : QObject
{
%TypeHeaderCode
#include <qgsauthmethod.h>
%End

  public:

    enum Expansion
    {
      NetworkRequest,
      NetworkReply,
      DataSourceURI,
      GenericDataSourceURI,
      All
    };
    typedef QFlags<QgsAuthMethod::Expansion> Expansions;

    virtual ~QgsAuthMethod();

    virtual QString key() const = 0;

    virtual QString description() const = 0;

    virtual QString displayDescription() const = 0;

    int version() const;

    QgsAuthMethod::Expansions supportedExpansions() const;

    QStringList supportedDataProviders() const;

    virtual bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                                       const QString &dataprovider = QString() );

    virtual bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                                     const QString &dataprovider = QString() );

    virtual bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                           const QString &dataprovider = QString() );

    virtual void clearCachedConfig( const QString &authcfg ) = 0;

    virtual void updateMethodConfig( QgsAuthMethodConfig &mconfig ) = 0;

  protected:
    explicit QgsAuthMethod();

    static QString authMethodTag();

    void setVersion( int version );

    void setExpansions( const QgsAuthMethod::Expansions& expansions );
    void setDataProviders( const QStringList& dataproviders );
};

typedef QHash<QString, QgsAuthMethod*> QgsAuthMethodsMap;
