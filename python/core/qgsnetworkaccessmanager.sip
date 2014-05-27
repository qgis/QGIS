%Feature PROXY_FACTORY

/*
 * \class QgsNetworkAccessManager
 * \brief network access manager for QGIS
 * \ingroup core
 * \since 1.5
 *
 * This class implements the QGIS network access manager.  It's a singleton
 * that can be used across QGIS.
 *
 * Plugins can insert proxy factories and thereby redirect requests to
 * individual proxies.
 *
 * If no proxy factories are there or none returns a proxy for an URL a
 * fallback proxy can be set.  There's also a exclude list that defines URLs
 * that the fallback proxy should not be used for, then no proxy will be used.
 *
 */
class QgsNetworkAccessManager : QNetworkAccessManager
{
%TypeHeaderCode
#include <qgsnetworkaccessmanager.h>
%End

  public:
    //! returns a pointer to the single instance
    // and creates that instance on the first call.
    static QgsNetworkAccessManager *instance();

    //! destructor
    ~QgsNetworkAccessManager();

%If (PROXY_FACTORY)
    //! insert a factory into the proxy factories list
    void insertProxyFactory( QNetworkProxyFactory *factory /Transfer/ );

    //! remove a factory from the proxy factories list
    void removeProxyFactory( QNetworkProxyFactory *factory /TransferBack/ );

    //! retrieve proxy factory list
    const QList<QNetworkProxyFactory *> proxyFactories() const;
%End

    //! retrieve fall back proxy (for urls that no factory returned proxies for)
    const QNetworkProxy &fallbackProxy() const;

    //! retrieve exclude list (urls shouldn't use the fallback proxy)
    const QStringList &excludeList() const;

    //! set fallback proxy and URL that shouldn't use it.
    void setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes );

    //! Get name for QNetworkRequest::CacheLoadControl
    static QString cacheLoadControlName( QNetworkRequest::CacheLoadControl theControl );

    //! Get QNetworkRequest::CacheLoadControl from name
    static QNetworkRequest::CacheLoadControl cacheLoadControlFromName( const QString &theName );

    //! Setup the NAM according to the user's settings
    void setupDefaultProxyAndCache();

    bool useSystemProxy();

  signals:
    void requestAboutToBeCreated( QNetworkAccessManager::Operation, const QNetworkRequest &, QIODevice * );
    void requestCreated( QNetworkReply * );
    void requestTimedOut( QNetworkReply * );

  protected:
    virtual QNetworkReply *createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0 );

  private:
    QgsNetworkAccessManager( QObject *parent = 0 );
};
