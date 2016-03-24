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
#include "qgsauthidentcertedit.h"

#ifndef QT_NO_OPENSSL
#include <QtCrypto>
#include <QSslConfiguration>
#include <QSslError>
#endif

#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


static const QString AUTH_METHOD_KEY = "Identity-Cert";
static const QString AUTH_METHOD_DESCRIPTION = "Identity certificate authentication";

QMap<QString, QgsPkiConfigBundle *> QgsAuthIdentCertMethod::mPkiConfigBundleCache = QMap<QString, QgsPkiConfigBundle *>();


QgsAuthIdentCertMethod::QgsAuthIdentCertMethod()
    : QgsAuthMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest );
  setDataProviders( QStringList()
                    << "ows"
                    << "wfs"  // convert to lowercase
                    << "wcs"
                    << "wms" );
}

QgsAuthIdentCertMethod::~QgsAuthIdentCertMethod()
{
  qDeleteAll( mPkiConfigBundleCache );
  mPkiConfigBundleCache.clear();
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
  return tr( "PKI stored identity certificate" );
}

bool QgsAuthIdentCertMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
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

void QgsAuthIdentCertMethod::clearCachedConfig( const QString &authcfg )
{
  removePkiConfigBundle( authcfg );
}

void QgsAuthIdentCertMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( "oldconfigstyle" ) )
  {
    QgsDebugMsg( "Updating old style auth method config" );

    QStringList conflist = mconfig.config( "oldconfigstyle" ).split( "|||" );
    mconfig.setConfig( "certid", conflist.at( 0 ) );
    mconfig.removeConfig( "oldconfigstyle" );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

QgsPkiConfigBundle *QgsAuthIdentCertMethod::getPkiConfigBundle( const QString &authcfg )
{
  QgsPkiConfigBundle * bundle = 0;

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

  // get identity from database
  QPair<QSslCertificate, QSslKey> cibundle( QgsAuthManager::instance()->getCertIdentityBundle( mconfig.config( "certid" ) ) );

  // init client cert
  // Note: if this is not valid, no sense continuing
  QSslCertificate clientcert( cibundle.first );
  if ( !clientcert.isValid() )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, client cert is not valid" ).arg( authcfg ) );
    return bundle;
  }

  // init key
  QSslKey clientkey( cibundle.second );
  if ( clientkey.isNull() )
  {
    QgsDebugMsg( QString( "PKI bundle for authcfg %1: insert FAILED, PEM cert key could not be created" ).arg( authcfg ) );
    return bundle;
  }

  bundle = new QgsPkiConfigBundle( mconfig, clientcert, clientkey );

  // cache bundle
  putPkiConfigBundle( authcfg, bundle );

  return bundle;
}

void QgsAuthIdentCertMethod::putPkiConfigBundle( const QString &authcfg, QgsPkiConfigBundle *pkibundle )
{
  QgsDebugMsg( QString( "Putting PKI bundle for authcfg %1" ).arg( authcfg ) );
  mPkiConfigBundleCache.insert( authcfg, pkibundle );
}

void QgsAuthIdentCertMethod::removePkiConfigBundle( const QString &authcfg )
{
  if ( mPkiConfigBundleCache.contains( authcfg ) )
  {
    QgsPkiConfigBundle * pkibundle = mPkiConfigBundleCache.take( authcfg );
    delete pkibundle;
    pkibundle = 0;
    QgsDebugMsg( QString( "Removed PKI bundle for authcfg: %1" ).arg( authcfg ) );
  }
}


//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthIdentCertMethod *classFactory()
{
  return new QgsAuthIdentCertMethod();
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
QGISEXTERN QgsAuthIdentCertEdit *editWidget( QWidget *parent )
{
  return new QgsAuthIdentCertEdit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
