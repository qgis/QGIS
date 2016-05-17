class QgsAuthMethod : QObject
{
%TypeHeaderCode
#include <qgsauthmethod.h>
%End

  public:

    /** Flags that represent the update points (where authentication configurations are expanded)
     * supported by an authentication method. These equate to the 'update*()' virtual functions
     * below, and allow for update point code to skip calling an unused update by a method, because
     * the base virtual funtion will always return true, giving a false impression an update occurred.
     * @note When adding an 'update' member function, also add the corresponding Expansion flag.
     * @note These flags will be added to as new update points are added
     */
    enum Expansion
    {
      // TODO: Figure out all different authentication expansions current layer providers use
      NetworkRequest,
      NetworkReply,
      DataSourceURI,
      GenericDataSourceURI,
      All
    };
    typedef QFlags<QgsAuthMethod::Expansion> Expansions;

    virtual ~QgsAuthMethod();

    /** A non-translated short name representing the auth method */
    virtual QString key() const = 0;

    /** A non-translated short description representing the auth method for use in debug output and About dialog */
    virtual QString description() const = 0;

    /** Translatable display version of the 'description()' */
    virtual QString displayDescription() const = 0;

    /** Increment this if method is significantly updated, allow updater code to be written for previously stored authcfg */
    int version() const;

    /** Flags that represent the update points (where authentication configurations are expanded)
     * supported by an authentication method.
     * @note These should directly correlate to existing 'update*()' member functions
     */
    QgsAuthMethod::Expansions supportedExpansions() const;

    /** The data providers that the method supports, allowing for filtering out authcfgs that are not
     * applicable to a given provider, or where the updating code is not currently implemented.
     */
    QStringList supportedDataProviders() const;

    /** Update a network request with authentication components
     * @param request The network request to update
     * @param authcfg Authentication configuration ID
     * @param dataprovider Textual key for a data provider, e.g. 'postgres', that allows
     * for custom updater code specific to the provider
     * @return Whether the update succeeded
     */
    virtual bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                                       const QString &dataprovider = QString() );

    /** Update a network reply with authentication components
     * @param reply The network reply object to update
     * @param authcfg Authentication configuration ID
     * @param dataprovider Textual key for a data provider, e.g. 'postgres', that allows
     * for custom updater code specific to the provider
     * @return Whether the update succeeded
     */
    virtual bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                                     const QString &dataprovider = QString() );

    /** Update data source connection items with authentication components
     * @param connectionItems QStringlist of 'key=value' pairs, as utilized in QgsDataSourceURI::connectionInfo()
     * @param authcfg Authentication configuration ID
     * @param dataprovider Textual key for a data provider, e.g. 'postgres', that allows
     * for custom updater code specific to the provider
     * @return Whether the update succeeded
     */
    virtual bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                           const QString &dataprovider = QString() );

    /** Clear any cached configuration. Called when the QgsAuthManager deletes an authentication configuration (authcfg).
     * @note It is highly recommended that a cache of authentication components (per requested authcfg)
     * be implemented, to avoid excessive queries on the auth database. Such a cache could be as
     * simple as a QHash or QMap of authcfg -> QgsAuthMethodConfig. See 'Basic' auth method plugin for example.
     */
    virtual void clearCachedConfig( const QString &authcfg ) = 0;

    /** Update an authentication configuration in place
     * @note Useful for updating previously stored authcfgs, when an authentication method has been significantly updated
     */
    virtual void updateMethodConfig( QgsAuthMethodConfig &mconfig ) = 0;

  protected:
    /**
     * Construct a default authentication method
     * @note Non-public since this is an abstract base class
     */
    explicit QgsAuthMethod();

    /** Tag signifying that this is an authentcation method (e.g. for use as title in message log panel output) */
    static QString authMethodTag();

    /** Set the version of the auth method (useful for future upgrading) */
    void setVersion( int version );

    /** Set the support expansions (points in providers where the authentication is injected) of the auth method */
    void setExpansions( const QgsAuthMethod::Expansions& expansions );
    /** Set list of data providers this auth method supports */
    void setDataProviders( const QStringList& dataproviders );
};

typedef QHash<QString, QgsAuthMethod*> QgsAuthMethodsMap;
