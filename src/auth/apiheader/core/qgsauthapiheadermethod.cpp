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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include "moc_qgsauthapiheadermethod.cpp"

#ifdef HAVE_GUI
#include "qgsauthapiheaderedit.h"
#endif

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QUuid>

const QString QgsAuthApiHeaderMethod::AUTH_METHOD_KEY = u"APIHeader"_s;
const QString QgsAuthApiHeaderMethod::AUTH_METHOD_DESCRIPTION = u"API Header"_s;
const QString QgsAuthApiHeaderMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "API Header" );

QMap<QString, QgsAuthMethodConfig> QgsAuthApiHeaderMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthApiHeaderMethod::QgsAuthApiHeaderMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest );
  setDataProviders( QStringList() << u"ows"_s << u"wfs"_s // convert to lowercase
                                  << u"wcs"_s << u"wms"_s );
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

bool QgsAuthApiHeaderMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QgsAuthMethodConfig config = getMethodConfig( authcfg );
  if ( !config.isValid() )
  {
    QgsDebugError( u"Update request config FAILED for authcfg: %1: config invalid"_s.arg( authcfg ) );
    return false;
  }

  QMapIterator<QString, QString> i( config.configMap() );
  while ( i.hasNext() )
  {
    i.next();

    const QString headerKey = i.key();
    const QString headerValue = i.value();

    QgsDebugMsgLevel( u"HTTP Header: %1=%2"_s.arg( headerKey ).arg( headerValue ), 2 );

    if ( !headerKey.isEmpty() )
    {
      request.setRawHeader( u"%1"_s.arg( headerKey ).toLocal8Bit(), u"%1"_s.arg( headerValue ).toLocal8Bit() );
    }
    else
    {
      QgsDebugError( u"The header key was empty, we shouldn't have empty header keys at this point"_s );
    }
  }

  return true;
}

void QgsAuthApiHeaderMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

void QgsAuthApiHeaderMethod::updateMethodConfig( QgsAuthMethodConfig &config )
{
  Q_UNUSED( config );
  // NOTE: add updates as method version() increases due to config storage changes
}

QgsAuthMethodConfig QgsAuthApiHeaderMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsAuthMethodConfig config;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    config = sAuthConfigCache.value( authcfg );
    QgsDebugMsgLevel( u"Retrieved config for authcfg: %1"_s.arg( authcfg ), 2 );
    return config;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, config, fullconfig ) )
  {
    QgsDebugError( u"Retrieve config FAILED for authcfg: %1"_s.arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, config );

  return config;
}

void QgsAuthApiHeaderMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &config )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( u"Putting token config for authcfg: %1"_s.arg( authcfg ), 2 );
  sAuthConfigCache.insert( authcfg, config );
}

void QgsAuthApiHeaderMethod::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsgLevel( u"Removed token config for authcfg: %1"_s.arg( authcfg ), 2 );
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
