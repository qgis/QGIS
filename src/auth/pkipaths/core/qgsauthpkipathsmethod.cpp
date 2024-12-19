/***************************************************************************
    qgsauthpkipathsmethod.cpp
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

#include "qgsauthpkipathsmethod.h"

#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#ifdef HAVE_GUI
#include "qgsauthpkipathsedit.h"
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

const QString QgsAuthPkiPathsMethod::AUTH_METHOD_KEY = QStringLiteral( "PKI-Paths" );
const QString QgsAuthPkiPathsMethod::AUTH_METHOD_DESCRIPTION = QStringLiteral( "PKI paths authentication" );
const QString QgsAuthPkiPathsMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "PKI paths authentication" );

#ifndef QT_NO_SSL
QMap<QString, QgsPkiConfigBundle *> QgsAuthPkiPathsMethod::sPkiConfigBundleCache = QMap<QString, QgsPkiConfigBundle *>();
#endif


QgsAuthPkiPathsMethod::QgsAuthPkiPathsMethod()
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

QgsAuthPkiPathsMethod::~QgsAuthPkiPathsMethod()
{
#ifndef QT_NO_SSL
  const QMutexLocker locker( &mMutex );
  qDeleteAll( sPkiConfigBundleCache );
  sPkiConfigBundleCache.clear();
#endif
}

QString QgsAuthPkiPathsMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthPkiPathsMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthPkiPathsMethod::displayDescription() const
{
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}


bool QgsAuthPkiPathsMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
#ifndef QT_NO_SSL
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );

  // TODO: is this too restrictive, to intercept only HTTPS connections?
  if ( request.url().scheme().toLower() != QLatin1String( "https" ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Update request SSL config SKIPPED for authcfg %1: not HTTPS" ).arg( authcfg ), 2 );
    return true;
  }

  QgsDebugMsgLevel( QStringLiteral( "Update request SSL config: HTTPS connection for authcfg: %1" ).arg( authcfg ), 2 );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugError( QStringLiteral( "Update request SSL config FAILED for authcfg: %1: PKI bundle invalid" ).arg( authcfg ) );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Update request SSL config: PKI bundle valid for authcfg: %1" ).arg( authcfg ), 2 );

  QSslConfiguration sslConfig = request.sslConfiguration();
  //QSslConfiguration sslConfig( QSslConfiguration::defaultConfiguration() );

  sslConfig.setPrivateKey( pkibundle->clientCertKey() );
  sslConfig.setLocalCertificate( pkibundle->clientCert() );

  // add extra CAs from the bundle
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
#else
  return false;
#endif
}


bool QgsAuthPkiPathsMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
#ifndef QT_NO_SSL
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );

  QgsDebugMsgLevel( QStringLiteral( "Update URI items for authcfg: %1" ).arg( authcfg ), 2 );

  QgsPkiConfigBundle *pkibundle = getPkiConfigBundle( authcfg );
  if ( !pkibundle || !pkibundle->isValid() )
  {
    QgsDebugError( QStringLiteral( "Update URI items FAILED: PKI bundle invalid" ) );
    return false;
  }
  QgsDebugMsgLevel( QStringLiteral( "Update URI items: PKI bundle valid" ), 2 );

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

  // add uri parameters
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

void QgsAuthPkiPathsMethod::clearCachedConfig( const QString &authcfg )
{
#ifndef QT_NO_SSL
  const QMutexLocker locker( &mMutex );
  removePkiConfigBundle( authcfg );
#endif
}

void QgsAuthPkiPathsMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Updating old style auth method config" ), 2 );

    const QStringList conflist = mconfig.config( QStringLiteral( "oldconfigstyle" ) ).split( QStringLiteral( "|||" ) );
    mconfig.setConfig( QStringLiteral( "certpath" ), conflist.at( 0 ) );
    mconfig.setConfig( QStringLiteral( "keypath" ), conflist.at( 1 ) );
    mconfig.setConfig( QStringLiteral( "keypass" ), conflist.at( 2 ) );
    mconfig.removeConfig( QStringLiteral( "oldconfigstyle" ) );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

#ifndef QT_NO_SSL
QgsPkiConfigBundle *QgsAuthPkiPathsMethod::getPkiConfigBundle( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  QgsPkiConfigBundle *bundle = nullptr;

  // check if it is cached
  if ( sPkiConfigBundleCache.contains( authcfg ) )
  {
    bundle = sPkiConfigBundleCache.value( authcfg );
    if ( bundle )
    {
      QgsDebugMsgLevel( QStringLiteral( "Retrieved PKI bundle for authcfg %1" ).arg( authcfg ), 2 );
      return bundle;
    }
  }

  // else build PKI bundle
  QgsAuthMethodConfig mconfig;

  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, true ) )
  {
    QgsDebugError( QStringLiteral( "PKI bundle for authcfg %1: FAILED to retrieve config" ).arg( authcfg ) );
    return bundle;
  }

  // init client cert
  // Note: if this is not valid, no sense continuing
  const QSslCertificate clientcert( QgsAuthCertUtils::certFromFile( mconfig.config( QStringLiteral( "certpath" ) ) ) );
  if ( !QgsAuthCertUtils::certIsViable( clientcert ) )
  {
    QgsDebugError( QStringLiteral( "PKI bundle for authcfg %1: insert FAILED, client cert is not viable" ).arg( authcfg ) );
    return bundle;
  }

  // init key
  const QSslKey clientkey = QgsAuthCertUtils::keyFromFile( mconfig.config( QStringLiteral( "keypath" ) ), mconfig.config( QStringLiteral( "keypass" ) ) );

  if ( clientkey.isNull() )
  {
    QgsDebugError( QStringLiteral( "PKI bundle for authcfg %1: insert FAILED, cert key is null" ).arg( authcfg ) );
    return bundle;
  }

  bundle = new QgsPkiConfigBundle( mconfig, clientcert, clientkey, QgsAuthCertUtils::casFromFile( mconfig.config( QStringLiteral( "certpath" ) ) ) );

  // cache bundle
  putPkiConfigBundle( authcfg, bundle );

  return bundle;
}

void QgsAuthPkiPathsMethod::putPkiConfigBundle( const QString &authcfg, QgsPkiConfigBundle *pkibundle )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( QStringLiteral( "Putting PKI bundle for authcfg %1" ).arg( authcfg ), 2 );
  sPkiConfigBundleCache.insert( authcfg, pkibundle );
}

void QgsAuthPkiPathsMethod::removePkiConfigBundle( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sPkiConfigBundleCache.contains( authcfg ) )
  {
    QgsPkiConfigBundle *pkibundle = sPkiConfigBundleCache.take( authcfg );
    delete pkibundle;
    pkibundle = nullptr;
    QgsDebugMsgLevel( QStringLiteral( "Removed PKI bundle for authcfg: %1" ).arg( authcfg ), 2 );
  }
}
#endif

#ifdef HAVE_GUI
QWidget *QgsAuthPkiPathsMethod::editWidget( QWidget *parent ) const
{
  return new QgsAuthPkiPathsEdit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthPkiPathsMethodMetadata();
}
#endif
