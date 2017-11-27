/***************************************************************************
    qgsauthpkcs12method.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthpkcs12method.h"
#include "qgsauthpkcs12edit.h"

#include <QDir>
#include <QFile>
#include <QUuid>
#ifndef QT_NO_SSL
#include <QtCrypto>
#include <QSslConfiguration>
#include <QSslError>
#endif
#include <QMutexLocker>

#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"


static const QString AUTH_METHOD_KEY = QStringLiteral( "PKI-PKCS#12" );
static const QString AUTH_METHOD_DESCRIPTION = QStringLiteral( "PKI PKCS#12 authentication" );

QMap<QString, QgsPkiConfigBundle *> QgsAuthPkcs12Method::sPkiConfigBundleCache = QMap<QString, QgsPkiConfigBundle *>();


QgsAuthPkcs12Method::QgsAuthPkcs12Method()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( QStringList()
                    << QStringLiteral( "ows" )
                    << QStringLiteral( "wfs" )  // convert to lowercase
                    << QStringLiteral( "wcs" )
                    << QStringLiteral( "wms" )
                    << QStringLiteral( "postgres" ) );
}

QgsAuthPkcs12Method::~QgsAuthPkcs12Method()
{
  qDeleteAll( sPkiConfigBundleCache );
  sPkiConfigBundleCache.clear();
}

QString QgsAuthPkcs12Method::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthPkcs12Method::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthPkcs12Method::displayDescription() const
{
  return tr( "PKI PKCS#12 authentication" );
}

bool QgsAuthPkcs12Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  QMutexLocker locker( &mMutex );

  // TODO: is this too restrictive, to intercept only HTTPS connections?
  if ( request.url().scheme().toLower() != QLatin1String( "https" ) )
  {
    QgsDebugMsg( QString( "Update request SSL config SKIPPED for authcfg %1: not HTTPS" ).arg( authcfg ) );
    return true;
  }

  QgsDebugMsg( QString( "Update request SSL config: HTTPS connection for authcfg: %1" ).arg( authcfg ) );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugMsg( QString( "Update request SSL config FAILED for authcfg: %1: PKI bundle invalid" ).arg( authcfg ) );
    return false;
  }

  QgsDebugMsg( QString( "Update request SSL config: PKI bundle valid for authcfg: %1" ).arg( authcfg ) );

  QSslConfiguration sslConfig = request.sslConfiguration();
  //QSslConfiguration sslConfig( QSslConfiguration::defaultConfiguration() );

  sslConfig.setLocalCertificate( pkibundle->clientCert() );
  sslConfig.setPrivateKey( pkibundle->clientCertKey() );

  // add extra CAs from the bundle, QNAM will prepend the trusted CAs in createRequest()
  if ( pkibundle->config().config( QStringLiteral( "addcas" ), QStringLiteral( "false" ) ) ==  QStringLiteral( "true" ) )
  {
    if ( pkibundle->config().config( QStringLiteral( "addrootca" ), QStringLiteral( "false" ) ) ==  QStringLiteral( "true" ) )
    {
      sslConfig.setCaCertificates( pkibundle->caChain() );
    }
    else
    {
      sslConfig.setCaCertificates( QgsAuthCertUtils::casRemoveSelfSigned( pkibundle->caChain() ) );
    }
  }

  request.setSslConfiguration( sslConfig );

  return true;
}

bool QgsAuthPkcs12Method::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  QMutexLocker locker( &mMutex );

  QgsDebugMsg( QString( "Update URI items for authcfg: %1" ).arg( authcfg ) );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugMsg( "Update URI items FAILED: PKI bundle invalid" );
    return false;
  }
  QgsDebugMsg( "Update URI items: PKI bundle valid" );

  QString pkiTempFileBase = QStringLiteral( "tmppki_%1.pem" );

  // save client cert to temp file
  QString certFilePath = QgsAuthCertUtils::pemTextToTempFile(
                           pkiTempFileBase.arg( QUuid::createUuid().toString() ),
                           pkibundle->clientCert().toPem() );
  if ( certFilePath.isEmpty() )
  {
    return false;
  }

  // save client cert key to temp file
  QString keyFilePath = QgsAuthCertUtils::pemTextToTempFile(
                          pkiTempFileBase.arg( QUuid::createUuid().toString() ),
                          pkibundle->clientCertKey().toPem() );
  if ( keyFilePath.isEmpty() )
  {
    return false;
  }

  // add extra CAs from the bundle
  QList<QSslCertificate> cas;
  if ( pkibundle->config().config( QStringLiteral( "addcas" ), QStringLiteral( "false" ) ) ==  QStringLiteral( "true" ) )
  {
    if ( pkibundle->config().config( QStringLiteral( "addrootca" ), QStringLiteral( "false" ) ) ==  QStringLiteral( "true" ) )
    {
      cas = QgsAuthCertUtils::casMerge( QgsApplication::authManager()->trustedCaCerts(), pkibundle->caChain() );
    }
    else
    {
      cas = QgsAuthCertUtils::casMerge( QgsApplication::authManager()->trustedCaCerts(),
                                        QgsAuthCertUtils::casRemoveSelfSigned( pkibundle->caChain() ) );
    }
  }
  else
  {
    cas = QgsApplication::authManager()->trustedCaCerts();
  }

  // save CAs to temp file
  QString caFilePath = QgsAuthCertUtils::pemTextToTempFile(
                         pkiTempFileBase.arg( QUuid::createUuid().toString() ),
                         QgsAuthCertUtils::certsToPemText( cas ) );

  if ( caFilePath.isEmpty() )
  {
    return false;
  }

  // get common name of the client certificate
  QString commonName = QgsAuthCertUtils::resolvedCertName( pkibundle->clientCert(), false );

  // add uri parameters
  QString userparam = "user='" + commonName + "'";
  int userindx = connectionItems.indexOf( QRegExp( "^user='.*" ) );
  if ( userindx != -1 )
  {
    connectionItems.replace( userindx, userparam );
  }
  else
  {
    connectionItems.append( userparam );
  }

  QString certparam = "sslcert='" + certFilePath + "'";
  int sslcertindx = connectionItems.indexOf( QRegExp( "^sslcert='.*" ) );
  if ( sslcertindx != -1 )
  {
    connectionItems.replace( sslcertindx, certparam );
  }
  else
  {
    connectionItems.append( certparam );
  }

  QString keyparam = "sslkey='" + keyFilePath + "'";
  int sslkeyindx = connectionItems.indexOf( QRegExp( "^sslkey='.*" ) );
  if ( sslkeyindx != -1 )
  {
    connectionItems.replace( sslkeyindx, keyparam );
  }
  else
  {
    connectionItems.append( keyparam );
  }

  QString caparam = "sslrootcert='" + caFilePath + "'";
  int sslcaindx = connectionItems.indexOf( QRegExp( "^sslrootcert='.*" ) );
  if ( sslcaindx != -1 )
  {
    connectionItems.replace( sslcaindx, caparam );
  }
  else
  {
    connectionItems.append( caparam );
  }

  return true;
}

void QgsAuthPkcs12Method::clearCachedConfig( const QString &authcfg )
{
  QMutexLocker locker( &mMutex );
  removePkiConfigBundle( authcfg );
}

void QgsAuthPkcs12Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  QMutexLocker locker( &mMutex );
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( "Updating old style auth method config" );

    QStringList conflist = mconfig.config( QStringLiteral( "oldconfigstyle" ) ).split( QStringLiteral( "|||" ) );
    mconfig.setConfig( QStringLiteral( "bundlepath" ), conflist.at( 0 ) );
    mconfig.setConfig( QStringLiteral( "bundlepass" ), conflist.at( 1 ) );
    mconfig.removeConfig( QStringLiteral( "oldconfigstyle" ) );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

QgsPkiConfigBundle *QgsAuthPkcs12Method::getPkiConfigBundle( const QString &authcfg )
{
  QMutexLocker locker( &mMutex );
  QgsPkiConfigBundle *bundle = nullptr;

  // check if it is cached
  if ( sPkiConfigBundleCache.contains( authcfg ) )
  {
    bundle = sPkiConfigBundleCache.value( authcfg );
    if ( bundle )
    {
      QgsDebugMsg( QString( "Retrieved PKI bundle for authcfg %1" ).arg( authcfg ) );
      return bundle;
    }
  }

  // else build PKI bundle
  QgsAuthMethodConfig mconfig;

  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, true ) )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: FAILED to retrieve config" ).arg( authcfg ) );
    return bundle;
  }

  QStringList bundlelist = QgsAuthCertUtils::pkcs12BundleToPem( mconfig.config( QStringLiteral( "bundlepath" ) ),
                           mconfig.config( QStringLiteral( "bundlepass" ) ), false );

  if ( bundlelist.isEmpty() || bundlelist.size() < 2 )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, PKCS#12 bundle parsing failed" ).arg( authcfg ) );
    return bundle;
  }

  // init client cert
  // Note: if this is not valid, no sense continuing
  QSslCertificate clientcert( bundlelist.at( 0 ).toLatin1() );
  if ( !QgsAuthCertUtils::certIsViable( clientcert ) )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, client cert is not viable" ).arg( authcfg ) );
    return bundle;
  }

  // !!! DON'T LEAVE THESE UNCOMMENTED !!!
  // QgsDebugMsg( QString( "PKI bundle key for authcfg: \n%1" ).arg( bundlelist.at( 1 ) ) );
  // QgsDebugMsg( QString( "PKI bundle key pass for authcfg: \n%1" )
  //              .arg( !mconfig.config( QStringLiteral( "bundlepass" ) ).isNull() ? mconfig.config( QStringLiteral( "bundlepass" ) ) : QStringLiteral() ) );

  // init key
  QSslKey clientkey( bundlelist.at( 1 ).toLatin1(),
                     QSsl::Rsa,
                     QSsl::Pem,
                     QSsl::PrivateKey,
                     !mconfig.config( QStringLiteral( "bundlepass" ) ).isNull() ? mconfig.config( QStringLiteral( "bundlepass" ) ).toUtf8() : QByteArray() );


  if ( clientkey.isNull() )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, cert key is null" ).arg( authcfg ) );
    return bundle;
  }

  bundle = new QgsPkiConfigBundle( mconfig, clientcert, clientkey,
                                   QgsAuthCertUtils::pkcs12BundleCas(
                                     mconfig.config( QStringLiteral( "bundlepath" ) ),
                                     mconfig.config( QStringLiteral( "bundlepass" ) ) ) );

  locker.unlock();
  // cache bundle
  putPkiConfigBundle( authcfg, bundle );

  return bundle;
}

void QgsAuthPkcs12Method::putPkiConfigBundle( const QString &authcfg, QgsPkiConfigBundle *pkibundle )
{
  QMutexLocker locker( &mMutex );
  QgsDebugMsg( QString( "Putting PKI bundle for authcfg %1" ).arg( authcfg ) );
  sPkiConfigBundleCache.insert( authcfg, pkibundle );
}

void QgsAuthPkcs12Method::removePkiConfigBundle( const QString &authcfg )
{
  QMutexLocker locker( &mMutex );
  if ( sPkiConfigBundleCache.contains( authcfg ) )
  {
    QgsPkiConfigBundle *pkibundle = sPkiConfigBundleCache.take( authcfg );
    delete pkibundle;
    pkibundle = nullptr;
    QgsDebugMsg( QString( "Removed PKI bundle for authcfg: %1" ).arg( authcfg ) );
  }
}


//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthPkcs12Method *classFactory()
{
  return new QgsAuthPkcs12Method();
}

/**
 * Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString authMethodKey()
{
  return AUTH_METHOD_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return AUTH_METHOD_DESCRIPTION;
}

/**
 * Required isAuthMethod function. Used to determine if this shared library
 * is an authentication method plugin
 */
QGISEXTERN bool isAuthMethod()
{
  return true;
}

/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthPkcs12Edit *editWidget( QWidget *parent )
{
  return new QgsAuthPkcs12Edit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
