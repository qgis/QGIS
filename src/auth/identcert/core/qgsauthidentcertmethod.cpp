/***************************************************************************
    qgsauthidentcertmethod.cpp
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

#include "qgsauthidentcertmethod.h"

#include "qgsapplication.h"
#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"

#include "moc_qgsauthidentcertmethod.cpp"

#ifdef HAVE_GUI
#include "qgsauthidentcertedit.h"
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


const QString QgsAuthIdentCertMethod::AUTH_METHOD_KEY = u"Identity-Cert"_s;
const QString QgsAuthIdentCertMethod::AUTH_METHOD_DESCRIPTION = u"Identity certificate authentication"_s;
const QString QgsAuthIdentCertMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "Identity certificate authentication" );

#ifndef QT_NO_SSL
QMap<QString, QgsPkiConfigBundle *> QgsAuthIdentCertMethod::sPkiConfigBundleCache = QMap<QString, QgsPkiConfigBundle *>();
#endif


QgsAuthIdentCertMethod::QgsAuthIdentCertMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( QStringList() << u"ows"_s << u"wfs"_s // convert to lowercase
                                  << u"wcs"_s << u"wms"_s << u"postgres"_s );
}

QgsAuthIdentCertMethod::~QgsAuthIdentCertMethod()
{
#ifndef QT_NO_SSL
  const QMutexLocker locker( &mMutex );
  qDeleteAll( sPkiConfigBundleCache );
  sPkiConfigBundleCache.clear();
#endif
}

QString QgsAuthIdentCertMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthIdentCertMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthIdentCertMethod::displayDescription() const
{
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthIdentCertMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
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

  request.setSslConfiguration( sslConfig );

  return true;
#else
  return false;
#endif
}

bool QgsAuthIdentCertMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider )
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

  // save CAs to temp file
  const QString caFilePath = QgsAuthCertUtils::pemTextToTempFile(
    pkiTempFileBase.arg( QUuid::createUuid().toString() ),
    QgsApplication::authManager()->trustedCaCertsPemText()
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

void QgsAuthIdentCertMethod::clearCachedConfig( const QString &authcfg )
{
  removePkiConfigBundle( authcfg );
}

void QgsAuthIdentCertMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  if ( mconfig.hasConfig( u"oldconfigstyle"_s ) )
  {
    QgsDebugMsgLevel( u"Updating old style auth method config"_s, 2 );

    const QStringList conflist = mconfig.config( u"oldconfigstyle"_s ).split( u"|||"_s );
    mconfig.setConfig( u"certid"_s, conflist.at( 0 ) );
    mconfig.removeConfig( u"oldconfigstyle"_s );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

#ifndef QT_NO_SSL
QgsPkiConfigBundle *QgsAuthIdentCertMethod::getPkiConfigBundle( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
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

  // get identity from database
  const QPair<QSslCertificate, QSslKey> cibundle( QgsApplication::authManager()->certIdentityBundle( mconfig.config( u"certid"_s ) ) );

  // init client cert
  // Note: if this is not valid, no sense continuing
  const QSslCertificate clientcert( cibundle.first );
  if ( !QgsAuthCertUtils::certIsViable( clientcert ) )
  {
    QgsDebugError( u"PKI bundle for authcfg %1: insert FAILED, client cert is not viable"_s.arg( authcfg ) );
    return bundle;
  }

  // init key
  const QSslKey clientkey( cibundle.second );
  if ( clientkey.isNull() )
  {
    QgsDebugError( u"PKI bundle for authcfg %1: insert FAILED, PEM cert key could not be created"_s.arg( authcfg ) );
    return bundle;
  }

  bundle = new QgsPkiConfigBundle( mconfig, clientcert, clientkey );

  // cache bundle
  putPkiConfigBundle( authcfg, bundle );

  return bundle;
}

void QgsAuthIdentCertMethod::putPkiConfigBundle( const QString &authcfg, QgsPkiConfigBundle *pkibundle )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( u"Putting PKI bundle for authcfg %1"_s.arg( authcfg ), 2 );
  sPkiConfigBundleCache.insert( authcfg, pkibundle );
}

void QgsAuthIdentCertMethod::removePkiConfigBundle( const QString &authcfg )
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
QWidget *QgsAuthIdentCertMethod::editWidget( QWidget *parent ) const
{
  return new QgsAuthIdentCertEdit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthIdentCertMethodMetadata();
}
#endif
