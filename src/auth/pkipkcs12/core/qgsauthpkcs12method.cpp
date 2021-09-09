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

#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#ifdef HAVE_GUI
#include "qgsauthpkcs12edit.h"
#endif

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QUuid>
#ifndef QT_NO_SSL
#include <QtCrypto>
#include <QSslConfiguration>
#include <QSslError>
#endif
#include <QMutexLocker>

const QString QgsAuthPkcs12Method::AUTH_METHOD_KEY = QStringLiteral( "PKI-PKCS#12" );
const QString QgsAuthPkcs12Method::AUTH_METHOD_DESCRIPTION = QStringLiteral( "PKI PKCS#12 authentication" );
const QString QgsAuthPkcs12Method::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "PKI PKCS#12 authentication" );

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
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthPkcs12Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );

  // TODO: is this too restrictive, to intercept only HTTPS connections?
  if ( request.url().scheme().toLower() != QLatin1String( "https" ) )
  {
    QgsDebugMsg( QStringLiteral( "Update request SSL config SKIPPED for authcfg %1: not HTTPS" ).arg( authcfg ) );
    return true;
  }

  QgsDebugMsg( QStringLiteral( "Update request SSL config: HTTPS connection for authcfg: %1" ).arg( authcfg ) );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update request SSL config FAILED for authcfg: %1: PKI bundle invalid" ).arg( authcfg ) );
    return false;
  }

  QgsDebugMsg( QStringLiteral( "Update request SSL config: PKI bundle valid for authcfg: %1" ).arg( authcfg ) );

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
  const QMutexLocker locker( &mMutex );

  QgsDebugMsg( QStringLiteral( "Update URI items for authcfg: %1" ).arg( authcfg ) );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update URI items FAILED: PKI bundle invalid" ) );
    return false;
  }
  QgsDebugMsg( QStringLiteral( "Update URI items: PKI bundle valid" ) );

  const QString pkiTempFileBase = QStringLiteral( "tmppki_%1.pem" );

  // save client cert to temp file
  const QString certFilePath = QgsAuthCertUtils::pemTextToTempFile(
                                 pkiTempFileBase.arg( QUuid::createUuid().toString() ),
                                 pkibundle->clientCert().toPem() );
  if ( certFilePath.isEmpty() )
  {
    return false;
  }

  // save client cert key to temp file
  const QString keyFilePath = QgsAuthCertUtils::pemTextToTempFile(
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
  const QString caFilePath = QgsAuthCertUtils::pemTextToTempFile(
                               pkiTempFileBase.arg( QUuid::createUuid().toString() ),
                               QgsAuthCertUtils::certsToPemText( cas ) );

  if ( caFilePath.isEmpty() )
  {
    return false;
  }

  // get common name of the client certificate
  const QString commonName = QgsAuthCertUtils::resolvedCertName( pkibundle->clientCert(), false );

  // add uri parameters
  const QString userparam = "user='" + commonName + "'";
  const thread_local QRegularExpression userRegExp( "^user='.*" );
  const int userindx = connectionItems.indexOf( userRegExp );
  if ( userindx != -1 )
  {
    connectionItems.replace( userindx, userparam );
  }
  else
  {
    connectionItems.append( userparam );
  }

  const QString certparam = "sslcert='" + certFilePath + "'";
  const thread_local QRegularExpression sslcertRegExp( "^sslcert='.*" );
  const int sslcertindx = connectionItems.indexOf( sslcertRegExp );
  if ( sslcertindx != -1 )
  {
    connectionItems.replace( sslcertindx, certparam );
  }
  else
  {
    connectionItems.append( certparam );
  }

  const QString keyparam = "sslkey='" + keyFilePath + "'";
  const thread_local QRegularExpression sslkeyRegExp( "^sslkey='.*" );
  const int sslkeyindx = connectionItems.indexOf( sslkeyRegExp );
  if ( sslkeyindx != -1 )
  {
    connectionItems.replace( sslkeyindx, keyparam );
  }
  else
  {
    connectionItems.append( keyparam );
  }

  const QString caparam = "sslrootcert='" + caFilePath + "'";
  const thread_local QRegularExpression sslcaRegExp( "^sslrootcert='.*" );
  const int sslcaindx = connectionItems.indexOf( sslcaRegExp );
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
  const QMutexLocker locker( &mMutex );
  removePkiConfigBundle( authcfg );
}

void QgsAuthPkcs12Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Updating old style auth method config" ) );

    const QStringList conflist = mconfig.config( QStringLiteral( "oldconfigstyle" ) ).split( QStringLiteral( "|||" ) );
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
      QgsDebugMsg( QStringLiteral( "Retrieved PKI bundle for authcfg %1" ).arg( authcfg ) );
      return bundle;
    }
  }

  // else build PKI bundle
  QgsAuthMethodConfig mconfig;

  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, true ) )
  {
    QgsDebugMsg( QStringLiteral( "PKI bundle for authcfg %1: FAILED to retrieve config" ).arg( authcfg ) );
    return bundle;
  }

  const QStringList bundlelist = QgsAuthCertUtils::pkcs12BundleToPem( mconfig.config( QStringLiteral( "bundlepath" ) ),
                                 mconfig.config( QStringLiteral( "bundlepass" ) ), false );

  if ( bundlelist.isEmpty() || bundlelist.size() < 2 )
  {
    QgsDebugMsg( QStringLiteral( "PKI bundle for authcfg %1: insert FAILED, PKCS#12 bundle parsing failed" ).arg( authcfg ) );
    return bundle;
  }

  // init client cert
  // Note: if this is not valid, no sense continuing
  const QSslCertificate clientcert( bundlelist.at( 0 ).toLatin1() );
  if ( !QgsAuthCertUtils::certIsViable( clientcert ) )
  {
    QgsDebugMsg( QStringLiteral( "PKI bundle for authcfg %1: insert FAILED, client cert is not viable" ).arg( authcfg ) );
    return bundle;
  }

  // !!! DON'T LEAVE THESE UNCOMMENTED !!!
  // QgsDebugMsg( QStringLiteral( "PKI bundle key for authcfg: \n%1" ).arg( bundlelist.at( 1 ) ) );
  // QgsDebugMsg( QStringLiteral( "PKI bundle key pass for authcfg: \n%1" )
  //              .arg( !mconfig.config( QStringLiteral( "bundlepass" ) ).isNull() ? mconfig.config( QStringLiteral( "bundlepass" ) ) : QString() ) );

  // init key
  const QSslKey clientkey( bundlelist.at( 1 ).toLatin1(),
                           QSsl::Rsa,
                           QSsl::Pem,
                           QSsl::PrivateKey,
                           !mconfig.config( QStringLiteral( "bundlepass" ) ).isNull() ? mconfig.config( QStringLiteral( "bundlepass" ) ).toUtf8() : QByteArray() );


  if ( clientkey.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "PKI bundle for authcfg %1: insert FAILED, cert key is null" ).arg( authcfg ) );
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
  const QMutexLocker locker( &mMutex );
  QgsDebugMsg( QStringLiteral( "Putting PKI bundle for authcfg %1" ).arg( authcfg ) );
  sPkiConfigBundleCache.insert( authcfg, pkibundle );
}

void QgsAuthPkcs12Method::removePkiConfigBundle( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sPkiConfigBundleCache.contains( authcfg ) )
  {
    QgsPkiConfigBundle *pkibundle = sPkiConfigBundleCache.take( authcfg );
    delete pkibundle;
    pkibundle = nullptr;
    QgsDebugMsg( QStringLiteral( "Removed PKI bundle for authcfg: %1" ).arg( authcfg ) );
  }
}

#ifdef HAVE_GUI
QWidget *QgsAuthPkcs12Method::editWidget( QWidget *parent ) const
{
  return new QgsAuthPkcs12Edit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthPkcs12MethodMetadata();
}
#endif
