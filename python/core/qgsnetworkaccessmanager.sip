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

%Feature PROXY_FACTORY

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
    void insertProxyFactory(QNetworkProxyFactory *factory /TransferTo/);

    //! remove a factory from the proxy factories list
    void removeProxyFactory(QNetworkProxyFactory *factory /TransferBack/);

    //! retrieve fall back proxy (for urls that no factory returned proxies for)
    const QList<QNetworkProxyFactory *> proxyFactories() const; 
%End

    //! retrieve exclude list (urls shouldn't use the fallback proxy)
    const QStringList &excludeList() const;

    //! retrieve fall back proxy (for urls that no factory returned proxies for)
    const QNetworkProxy &fallbackProxy() const;

    //! set fallback proxy and URL that shouldn't use it.
    void setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes );

  signals:
    void requestAboutToBeCreated( Operation, const QNetworkRequest &, QIODevice * );
    void requestCreated( QNetworkReply * );

  protected:
    virtual QNetworkReply *createRequest( Operation op, const QNetworkRequest & req, QIODevice *outgoingData = 0 );
    
  private:
    QgsNetworkAccessManager( QObject * );
};
