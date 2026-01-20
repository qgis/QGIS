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

#include "qgsapplication.h"
#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"

#include "moc_qgsauthpkcs12method.cpp"

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

const QString QgsAuthPkcs12Method::AUTH_METHOD_KEY = u"PKI-PKCS#12"_s;
const QString QgsAuthPkcs12Method::AUTH_METHOD_DESCRIPTION = u"PKI PKCS#12 authentication"_s;
const QString QgsAuthPkcs12Method::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "PKI PKCS#12 authentication" );

#ifndef QT_NO_SSL
QMap<QString, QgsPkiConfigBundle *> QgsAuthPkcs12Method::sPkiConfigBundleCache = QMap<QString, QgsPkiConfigBundle *>();
#endif


QgsAuthPkcs12Method::QgsAuthPkcs12Method()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( QStringList() << u"ows"_s << u"wfs"_s // convert to lowercase
                                  << u"wcs"_s << u"wms"_s << u"postgres"_s );
}

QgsAuthPkcs12Method::~QgsAuthPkcs12Method()
{
#ifndef QT_NO_SSL
  qDeleteAll( sPkiConfigBundleCache );
  sPkiConfigBundleCache.clear();
#endif
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

bool QgsAuthPkcs12Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
{
#ifndef QT_NO_SSL
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );

  // TODO: is this too restrictive, to intercept only HTTPS connections?
  if ( request.url().scheme().toLower() != "https"_L1 )
  {
    QgsDebugMsgLevel( u"Update request SSL config SKIPPED for authcfg %1: not HTTPS"_s.arg( authcfg ), 2 );
    return true;
  }

  QgsDebugMsgLevel( u"Update request SSL config: HTTPS connection for authcfg: %1"_s.arg( authcfg ), 2 );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugError( u"Update request SSL config FAILED for authcfg: %1: PKI bundle invalid"_s.arg( authcfg ) );
    return false;
  }

  QgsDebugMsgLevel( u"Update request SSL config: PKI bundle valid for authcfg: %1"_s.arg( authcfg ), 2 );

  QSslConfiguration sslConfig = request.sslConfiguration();
  //QSslConfiguration sslConfig( QSslConfiguration::defaultConfiguration() );

  sslConfig.setLocalCertificate( pkibundle->clientCert() );
  sslConfig.setPrivateKey( pkibundle->clientCertKey() );

  // add extra CAs from the bundle, QNAM will prepend the trusted CAs in createRequest()
  if ( pkibundle->config().config( u"addcas"_s, u"false"_s ) == "true"_L1 )
  {
    if ( pkibundle->config().config( u"addrootca"_s, u"false"_s ) == "true"_L1 )
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
#else
  return false;
#endif
}

bool QgsAuthPkcs12Method::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider )
{
#ifndef QT_NO_SSL
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );

  QgsDebugMsgLevel( u"Update URI items for authcfg: %1"_s.arg( authcfg ), 2 );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugError( u"Update URI items FAILED: PKI bundle invalid"_s );
    return false;
  }

  QgsDebugMsgLevel( u"Update URI items: PKI bundle valid"_s, 2 );

  const QString pkiTempFileBase = u"tmppki_%1.pem"_s;

  // save client cert to temp file
  const QString certFilePath = QgsAuthCertUtils::pemTextToTempFile(
    pkiTempFileBase.arg( QUuid::createUuid().toString() ),
    pkibundle->clientCert().toPem()
  );
  if ( certFilePath.isEmpty() )
  {
    return false;
  }

  // save client cert key to temp file
  const QString keyFilePath = QgsAuthCertUtils::pemTextToTempFile(
    pkiTempFileBase.arg( QUuid::createUuid().toString() ),
    pkibundle->clientCertKey().toPem()
  );
  if ( keyFilePath.isEmpty() )
  {
    return false;
  }

  // add extra CAs from the bundle
  QList<QSslCertificate> cas;
  if ( pkibundle->config().config( u"addcas"_s, u"false"_s ) == "true"_L1 )
  {
    if ( pkibundle->config().config( u"addrootca"_s, u"false"_s ) == "true"_L1 )
    {
      cas = QgsAuthCertUtils::casMerge( QgsApplication::authManager()->trustedCaCerts(), pkibundle->caChain() );
    }
    else
    {
      cas = QgsAuthCertUtils::casMerge( QgsApplication::authManager()->trustedCaCerts(), QgsAuthCertUtils::casRemoveSelfSigned( pkibundle->caChain() ) );
    }
  }
  else
  {
    cas = QgsApplication::authManager()->trustedCaCerts();
  }

  // save CAs to temp file
  const QString caFilePath = QgsAuthCertUtils::pemTextToTempFile(
    pkiTempFileBase.arg( QUuid::createUuid().toString() ),
    QgsAuthCertUtils::certsToPemText( cas )
  );

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
#else
  return false;
#endif
}

void QgsAuthPkcs12Method::clearCachedConfig( const QString &authcfg )
{
#ifndef QT_NO_SSL
  const QMutexLocker locker( &mMutex );
  removePkiConfigBundle( authcfg );
#endif
}

void QgsAuthPkcs12Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  if ( mconfig.hasConfig( u"oldconfigstyle"_s ) )
  {
    QgsDebugMsgLevel( u"Updating old style auth method config"_s, 2 );

    const QStringList conflist = mconfig.config( u"oldconfigstyle"_s ).split( u"|||"_s );
    mconfig.setConfig( u"bundlepath"_s, conflist.at( 0 ) );
    mconfig.setConfig( u"bundlepass"_s, conflist.at( 1 ) );
    mconfig.removeConfig( u"oldconfigstyle"_s );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

#ifndef QT_NO_SSL
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
      QgsDebugMsgLevel( u"Retrieved PKI bundle for authcfg %1"_s.arg( authcfg ), 2 );
      return bundle;
    }
  }

  // else build PKI bundle
  QgsAuthMethodConfig mconfig;

  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, true ) )
  {
    QgsDebugError( u"PKI bundle for authcfg %1: FAILED to retrieve config"_s.arg( authcfg ) );
    return bundle;
  }

  const QStringList bundlelist = QgsAuthCertUtils::pkcs12BundleToPem( mconfig.config( u"bundlepath"_s ), mconfig.config( u"bundlepass"_s ), false );

  if ( bundlelist.isEmpty() || bundlelist.size() < 2 )
  {
    QgsDebugError( u"PKI bundle for authcfg %1: insert FAILED, PKCS#12 bundle parsing failed"_s.arg( authcfg ) );
    return bundle;
  }

  // init client cert
  // Note: if this is not valid, no sense continuing
  const QSslCertificate clientcert( bundlelist.at( 0 ).toLatin1() );
  if ( !QgsAuthCertUtils::certIsViable( clientcert ) )
  {
    QgsDebugError( u"PKI bundle for authcfg %1: insert FAILED, client cert is not viable"_s.arg( authcfg ) );
    return bundle;
  }

  // !!! DON'T LEAVE THESE UNCOMMENTED !!!
  // QgsDebugMsgLevel( u"PKI bundle key for authcfg: \n%1"_s.arg( bundlelist.at( 1 ) ), 2 );
  // QgsDebugMsgLevel( u"PKI bundle key pass for authcfg: \n%1"_s
  //              .arg( !mconfig.config( u"bundlepass"_s ).isNull() ? mconfig.config( u"bundlepass"_s ) : QString() ), 2 );

  // init key
  const QSslKey clientkey( bundlelist.at( 1 ).toLatin1(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, !mconfig.config( u"bundlepass"_s ).isNull() ? mconfig.config( u"bundlepass"_s ).toUtf8() : QByteArray() );


  if ( clientkey.isNull() )
  {
    QgsDebugError( u"PKI bundle for authcfg %1: insert FAILED, cert key is null"_s.arg( authcfg ) );
    return bundle;
  }

  bundle = new QgsPkiConfigBundle( mconfig, clientcert, clientkey, QgsAuthCertUtils::pkcs12BundleCas( mconfig.config( u"bundlepath"_s ), mconfig.config( u"bundlepass"_s ) ) );

  locker.unlock();
  // cache bundle
  putPkiConfigBundle( authcfg, bundle );

  return bundle;
}

void QgsAuthPkcs12Method::putPkiConfigBundle( const QString &authcfg, QgsPkiConfigBundle *pkibundle )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( u"Putting PKI bundle for authcfg %1"_s.arg( authcfg ), 2 );
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
    QgsDebugMsgLevel( u"Removed PKI bundle for authcfg: %1"_s.arg( authcfg ), 2 );
  }
}
#endif

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
