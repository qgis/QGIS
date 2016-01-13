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
#ifndef QT_NO_OPENSSL
#include <QtCrypto>
#include <QSslConfiguration>
#include <QSslError>
#endif

#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


static const QString AUTH_METHOD_KEY = "PKI-PKCS#12";
static const QString AUTH_METHOD_DESCRIPTION = "PKI PKCS#12 authentication";

QMap<QString, QgsPkiConfigBundle *> QgsAuthPkcs12Method::mPkiConfigBundleCache = QMap<QString, QgsPkiConfigBundle *>();


QgsAuthPkcs12Method::QgsAuthPkcs12Method()
    : QgsAuthMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceURI );
  setDataProviders( QStringList()
                    << "ows"
                    << "wfs"  // convert to lowercase
                    << "wcs"
                    << "wms"
                    << "postgres" );
}

QgsAuthPkcs12Method::~QgsAuthPkcs12Method()
{
  qDeleteAll( mPkiConfigBundleCache );
  mPkiConfigBundleCache.clear();
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

  // TODO: is this too restrictive, to intercept only HTTPS connections?
  if ( request.url().scheme().toLower() != QLatin1String( "https" ) )
  {
    QgsDebugMsg( QString( "Update request SSL config SKIPPED for authcfg %1: not HTTPS" ).arg( authcfg ) );
    return true;
  }

  QgsDebugMsg( QString( "Update request SSL config: HTTPS connection for authcfg: %1" ).arg( authcfg ) );

  QgsPkiConfigBundle * pkibundle = getPkiConfigBundle( authcfg );
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

  request.setSslConfiguration( sslConfig );

  return true;
}

bool QgsAuthPkcs12Method::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  QgsDebugMsg( QString( "Update URI items for authcfg: %1" ).arg( authcfg ) );

  QString pkiTempFilePrefix = "tmppki_";

  QgsAuthMethodConfig amConfig;
  if ( !QgsAuthManager::instance()->loadAuthenticationConfig( authcfg, amConfig, true ) )
  {
    QgsDebugMsg( QString( "Update URI items: FAILED to retrieve config for authcfg: %1" ).arg( authcfg ) );
    return false;
  }

  if ( !amConfig.isValid() )
  {
    QgsDebugMsg( QString( "Update URI items: FAILED retrieved invalid Auth method for authcfg: %1" ).arg( authcfg ) );
    return false;
  }

  // get client cent and key
  QSslCertificate clientCert = QgsAuthManager::instance()->getCertIdentityBundle( amConfig.config( "certid" ) ).first;
  QSslKey clientKey = QgsAuthManager::instance()->getCertIdentityBundle( amConfig.config( "certid" ) ).second;

  // get common name of the client certificate
  QString commonName = QgsAuthCertUtils::resolvedCertName( clientCert, false );

  // get CA
  QByteArray caCert = QgsAuthManager::instance()->getTrustedCaCertsPemText();

  // save client cert to temp file
  QFile certFile( QDir::tempPath() + QDir::separator() + pkiTempFilePrefix + QUuid::createUuid() + ".pem" );
  if ( certFile.open( QIODevice::WriteOnly ) )
  {
    certFile.write( clientCert.toPem() );
  }
  else
  {
    QgsDebugMsg( QString( "Update URI items: FAILED to save client cert temporary file" ) );
    return false;
  }

  certFile.setPermissions( QFile::ReadUser | QFile::WriteUser );

  // save key cert to temp file setting it's permission only read to the current user
  QFile keyFile( QDir::tempPath() + QDir::separator() + pkiTempFilePrefix + QUuid::createUuid() + ".pem" );
  if ( keyFile.open( QIODevice::WriteOnly ) )
  {
    keyFile.write( clientKey.toPem() );
  }
  else
  {
    QgsDebugMsg( QString( "Update URI items: FAILED to save client key temporary file" ) );
    return false;
  }

  keyFile.setPermissions( QFile::ReadUser );

  // save CA to tempo file
  QFile caFile( QDir::tempPath() + QDir::separator() + pkiTempFilePrefix + QUuid::createUuid() + ".pem" );
  if ( caFile.open( QIODevice::WriteOnly ) )
  {
    caFile.write( caCert );
  }
  else
  {
    QgsDebugMsg( QString( "Update URI items: FAILED to save CAs to temporary file" ) );
    return false;
  }

  caFile.setPermissions( QFile::ReadUser | QFile::WriteUser );

  // add uri parameters
  QString certparam = "sslcert='" + certFile.fileName() + "'";
  int sslcertindx = connectionItems.indexOf( QRegExp( "^sslcert='.*" ) );
  if ( sslcertindx != -1 )
  {
    connectionItems.replace( sslcertindx, certparam );
  }
  else
  {
    connectionItems.append( certparam );
  }

  QString keyparam = "sslkey='" + keyFile.fileName() + "'";
  int sslkeyindx = connectionItems.indexOf( QRegExp( "^sslkey='.*" ) );
  if ( sslkeyindx != -1 )
  {
    connectionItems.replace( sslkeyindx, keyparam );
  }
  else
  {
    connectionItems.append( keyparam );
  }

  QString caparam = "sslrootcert='" + caFile.fileName() + "'";
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
  removePkiConfigBundle( authcfg );
}

void QgsAuthPkcs12Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( "oldconfigstyle" ) )
  {
    QgsDebugMsg( "Updating old style auth method config" );

    QStringList conflist = mconfig.config( "oldconfigstyle" ).split( "|||" );
    mconfig.setConfig( "bundlepath", conflist.at( 0 ) );
    mconfig.setConfig( "bundlepass", conflist.at( 1 ) );
    mconfig.removeConfig( "oldconfigstyle" );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

QgsPkiConfigBundle *QgsAuthPkcs12Method::getPkiConfigBundle( const QString &authcfg )
{
  QgsPkiConfigBundle * bundle = nullptr;

  // check if it is cached
  if ( mPkiConfigBundleCache.contains( authcfg ) )
  {
    bundle = mPkiConfigBundleCache.value( authcfg );
    if ( bundle )
    {
      QgsDebugMsg( QString( "Retrieved PKI bundle for authcfg %1" ).arg( authcfg ) );
      return bundle;
    }
  }

  // else build PKI bundle
  QgsAuthMethodConfig mconfig;

  if ( !QgsAuthManager::instance()->loadAuthenticationConfig( authcfg, mconfig, true ) )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: FAILED to retrieve config" ).arg( authcfg ) );
    return bundle;
  }

  QStringList bundlelist = QgsAuthCertUtils::pkcs12BundleToPem( mconfig.config( "bundlepath" ),
                           mconfig.config( "bundlepass" ), false );

  // init client cert
  // Note: if this is not valid, no sense continuing
  QSslCertificate clientcert( bundlelist.at( 0 ).toAscii() );
  if ( !clientcert.isValid() )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, client cert is not valid" ).arg( authcfg ) );
    return bundle;
  }

  // init key
  QSslKey clientkey( bundlelist.at( 1 ).toAscii(),
                     QSsl::Rsa,
                     QSsl::Pem,
                     QSsl::PrivateKey,
                     !mconfig.config( "bundlepass" ).isNull() ? mconfig.config( "bundlepass" ).toUtf8() : QByteArray() );


  if ( clientkey.isNull() )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, cert key is null" ).arg( authcfg ) );
    return bundle;
  }

  bundle = new QgsPkiConfigBundle( mconfig, clientcert, clientkey );

  // cache bundle
  putPkiConfigBundle( authcfg, bundle );

  return bundle;
}

void QgsAuthPkcs12Method::putPkiConfigBundle( const QString &authcfg, QgsPkiConfigBundle *pkibundle )
{
  QgsDebugMsg( QString( "Putting PKI bundle for authcfg %1" ).arg( authcfg ) );
  mPkiConfigBundleCache.insert( authcfg, pkibundle );
}

void QgsAuthPkcs12Method::removePkiConfigBundle( const QString &authcfg )
{
  if ( mPkiConfigBundleCache.contains( authcfg ) )
  {
    QgsPkiConfigBundle * pkibundle = mPkiConfigBundleCache.take( authcfg );
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

/** Required key function (used to map the plugin to a data store type)
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
