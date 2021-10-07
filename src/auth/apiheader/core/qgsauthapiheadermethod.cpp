/***************************************************************************
    qgsauthapiheadermethod.cpp
    --------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Tom Cummins
    author               : Tom Cummins
    email                : tom cumminsc9 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthapiheadermethod.h"

#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#ifdef HAVE_GUI
#include "qgsauthapiheaderedit.h"
#endif

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QUuid>

const QString QgsAuthApiHeaderMethod::AUTH_METHOD_KEY = QStringLiteral( "APIHeader" );
const QString QgsAuthApiHeaderMethod::AUTH_METHOD_DESCRIPTION = QStringLiteral( "API Header" );
const QString QgsAuthApiHeaderMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "API Header" );

QMap<QString, QgsAuthMethodConfig> QgsAuthApiHeaderMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthApiHeaderMethod::QgsAuthApiHeaderMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest );
  setDataProviders( QStringList()
                    << QStringLiteral( "ows" )
                    << QStringLiteral( "wfs" )  // convert to lowercase
                    << QStringLiteral( "wcs" )
                    << QStringLiteral( "wms" ) );
}

QString QgsAuthApiHeaderMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthApiHeaderMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthApiHeaderMethod::displayDescription() const
{
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthApiHeaderMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString headerKey = mconfig.config( QStringLiteral( "headerKey" ) );
  const QString headerValue = mconfig.config( QStringLiteral( "headerValue" ) );

  if ( !headerKey.isEmpty() && !headerValue.isEmpty() )
  {
    request.setRawHeader( QStringLiteral( "%1" ).arg( headerKey ).toLocal8Bit(),  QStringLiteral( "%1" ).arg( headerValue ).toLocal8Bit() );
  }
  return true;
}

void QgsAuthApiHeaderMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

void QgsAuthApiHeaderMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Updating old style auth method config" ) );
  }

  // NOTE: add updates as method version() increases due to config storage changes
}

QgsAuthMethodConfig QgsAuthApiHeaderMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  const QMutexLocker locker( &mMutex );
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

void QgsAuthApiHeaderMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsg( QStringLiteral( "Putting token config for authcfg: %1" ).arg( authcfg ) );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthApiHeaderMethod::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsg( QStringLiteral( "Removed token config for authcfg: %1" ).arg( authcfg ) );
  }
}

#ifdef HAVE_GUI
QWidget *QgsAuthApiHeaderMethod::editWidget( QWidget *parent ) const
{
  return new QgsAuthApiHeaderEdit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthApiHeaderMethodMetadata();
}
#endif
