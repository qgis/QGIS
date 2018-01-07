/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkaccessmanager.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsNetworkAccessManager : QNetworkAccessManager
{
%Docstring
 network access manager for QGIS

.. versionadded:: 1.5

This class implements the QGIS network access manager.  It's a singleton
that can be used across QGIS.

Plugins can insert proxy factories and thereby redirect requests to
individual proxies.

If no proxy factories are there or none returns a proxy for an URL a
fallback proxy can be set.  There's also a exclude list that defines URLs
that the fallback proxy should not be used for, then no proxy will be used.
%End

%TypeHeaderCode
#include "qgsnetworkaccessmanager.h"
%End
  public:
    static QgsNetworkAccessManager *instance();

    QgsNetworkAccessManager( QObject *parent = 0 );

    void insertProxyFactory( QNetworkProxyFactory *factory /Transfer/ );
%Docstring
insert a factory into the proxy factories list
%End

    void removeProxyFactory( QNetworkProxyFactory *factory /TransferBack/ );
%Docstring
remove a factory from the proxy factories list
%End

    const QList<QNetworkProxyFactory *> proxyFactories() const;
%Docstring
retrieve proxy factory list
%End

    const QNetworkProxy &fallbackProxy() const;
%Docstring
retrieve fall back proxy (for urls that no factory returned proxies for)
%End

    QStringList excludeList() const;
%Docstring
retrieve exclude list (urls shouldn't use the fallback proxy)
%End

    void setFallbackProxyAndExcludes( const QNetworkProxy &proxy, const QStringList &excludes );
%Docstring
set fallback proxy and URL that shouldn't use it.
%End

    static QString cacheLoadControlName( QNetworkRequest::CacheLoadControl control );
%Docstring
Get name for QNetworkRequest.CacheLoadControl
%End

    static QNetworkRequest::CacheLoadControl cacheLoadControlFromName( const QString &name );
%Docstring
Get QNetworkRequest.CacheLoadControl from name
%End

    void setupDefaultProxyAndCache();
%Docstring
Setup the NAM according to the user's settings
%End

    bool useSystemProxy() const;
%Docstring
return whether the system proxy should be used
%End

  signals:
    void requestAboutToBeCreated( QNetworkAccessManager::Operation, const QNetworkRequest &, QIODevice * );
    void requestCreated( QNetworkReply * );
    void requestTimedOut( QNetworkReply * );

  protected:
    virtual QNetworkReply *createRequest( QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0 );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsnetworkaccessmanager.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
