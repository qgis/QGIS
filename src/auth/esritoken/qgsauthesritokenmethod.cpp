/***************************************************************************
    qgsauthesritokenmethod.cpp
    --------------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    author               : Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthesritokenmethod.h"
#include "qgsauthesritokenedit.h"

#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QUuid>

static const QString AUTH_METHOD_KEY = QStringLiteral( "EsriToken" );
static const QString AUTH_METHOD_DESCRIPTION = QStringLiteral( "ESRI token" );

QMap<QString, QgsAuthMethodConfig> QgsAuthEsriTokenMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthEsriTokenMethod::QgsAuthEsriTokenMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest );
  setDataProviders( QStringList()
                    << QStringLiteral( "arcgismapserver" )
                    << QStringLiteral( "arcgisfeatureserver" ) );

}

QString QgsAuthEsriTokenMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthEsriTokenMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthEsriTokenMethod::displayDescription() const
{
  return tr( "ESRI token based authentication" );
}

bool QgsAuthEsriTokenMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString token = mconfig.config( QStringLiteral( "token" ) );

  if ( !token.isEmpty() )
  {
    request.setRawHeader( "X-Esri-Authorization",  QStringLiteral( "Bearer %1 " ).arg( token ).toLocal8Bit() );
  }
  return true;
}

void QgsAuthEsriTokenMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

void QgsAuthEsriTokenMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Updating old style auth method config" ) );
  }

  // NOTE: add updates as method version() increases due to config storage changes
}

QgsAuthMethodConfig QgsAuthEsriTokenMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  QMutexLocker locker( &mMutex );
  QgsAuthMethodConfig mconfig;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    mconfig = sAuthConfigCache.value( authcfg );
    QgsDebugMsg( QStringLiteral( "Retrieved config for authcfg: %1" ).arg( authcfg ) );
    return mconfig;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugMsg( QStringLiteral( "Retrieve config FAILED for authcfg: %1" ).arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, mconfig );

  return mconfig;
}

void QgsAuthEsriTokenMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  QMutexLocker locker( &mMutex );
  QgsDebugMsg( QStringLiteral( "Putting token config for authcfg: %1" ).arg( authcfg ) );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthEsriTokenMethod::removeMethodConfig( const QString &authcfg )
{
  QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsg( QStringLiteral( "Removed token config for authcfg: %1" ).arg( authcfg ) );
  }
}

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthEsriTokenMethod *classFactory()
{
  return new QgsAuthEsriTokenMethod();
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
QGISEXTERN QgsAuthEsriTokenEdit *editWidget( QWidget *parent )
{
  return new QgsAuthEsriTokenEdit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
