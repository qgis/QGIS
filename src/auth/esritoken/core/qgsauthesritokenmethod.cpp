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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"

#include "moc_qgsauthesritokenmethod.cpp"

#ifdef HAVE_GUI
#include "qgsauthesritokenedit.h"
#endif

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QUuid>

const QString QgsAuthEsriTokenMethod::AUTH_METHOD_KEY = u"EsriToken"_s;
const QString QgsAuthEsriTokenMethod::AUTH_METHOD_DESCRIPTION = u"ESRI token"_s;
const QString QgsAuthEsriTokenMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "ESRI token" );

QMap<QString, QgsAuthMethodConfig> QgsAuthEsriTokenMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthEsriTokenMethod::QgsAuthEsriTokenMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest );
  setDataProviders( QStringList() << u"arcgismapserver"_s << u"arcgisfeatureserver"_s );
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
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthEsriTokenMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QgsAuthMethodConfig config = getMethodConfig( authcfg );
  if ( !config.isValid() )
  {
    QgsDebugError( u"Update request config FAILED for authcfg: %1: config invalid"_s.arg( authcfg ) );
    return false;
  }

  const QString token = config.config( u"token"_s );

  if ( !token.isEmpty() )
  {
    request.setRawHeader( "X-Esri-Authorization", u"Bearer %1 "_s.arg( token ).toLocal8Bit() );
  }
  return true;
}

void QgsAuthEsriTokenMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

void QgsAuthEsriTokenMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( u"oldconfigstyle"_s ) )
  {
    QgsDebugMsgLevel( u"Updating old style auth method config"_s, 2 );
  }

  // NOTE: add updates as method version() increases due to config storage changes
}

QgsAuthMethodConfig QgsAuthEsriTokenMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsAuthMethodConfig mconfig;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    mconfig = sAuthConfigCache.value( authcfg );
    QgsDebugMsgLevel( u"Retrieved config for authcfg: %1"_s.arg( authcfg ), 2 );
    return mconfig;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugError( u"Retrieve config FAILED for authcfg: %1"_s.arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, mconfig );

  return mconfig;
}

void QgsAuthEsriTokenMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( u"Putting token config for authcfg: %1"_s.arg( authcfg ), 2 );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthEsriTokenMethod::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsgLevel( u"Removed token config for authcfg: %1"_s.arg( authcfg ), 2 );
  }
}

#ifdef HAVE_GUI
QWidget *QgsAuthEsriTokenMethod::editWidget( QWidget *parent ) const
{
  return new QgsAuthEsriTokenEdit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthEsriTokenMethodMetadata();
}
#endif
