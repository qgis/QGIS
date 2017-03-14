/***************************************************************************
    qgsowsconnection.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH
    generalized          : (C) 2012 Radim Blazek, based on qgswmsconnection.cpp

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsowsconnection.h"
#include "qgssettings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPicture>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

QgsOwsConnection::QgsOwsConnection( const QString &service, const QString &connName )
  : mConnName( connName )
  , mService( service )
{
  QgsDebugMsg( "theConnName = " + connName );

  QgsSettings settings;

  QString key = "qgis//connections-" + mService.toLower() + '/' + mConnName;
  QString credentialsKey = "qgis//" + mService + '/' + mConnName;

  QStringList connStringParts;

  mConnectionInfo = settings.value( key + "/url" ).toString();
  mUri.setParam( QStringLiteral( "url" ), settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    mUri.setParam( QStringLiteral( "username" ), username );
    mUri.setParam( QStringLiteral( "password" ), password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "authcfg" ), authcfg );
  }
  mConnectionInfo.append( ",authcfg=" + authcfg );

  bool ignoreGetMap = settings.value( key + "/ignoreGetMapURI", false ).toBool();
  bool ignoreGetFeatureInfo = settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool();
  bool ignoreAxisOrientation = settings.value( key + "/ignoreAxisOrientation", false ).toBool();
  bool invertAxisOrientation = settings.value( key + "/invertAxisOrientation", false ).toBool();
  if ( ignoreGetMap )
  {
    mUri.setParam( QStringLiteral( "IgnoreGetMapUrl" ), QStringLiteral( "1" ) );
  }
  if ( ignoreGetFeatureInfo )
  {
    mUri.setParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ), QStringLiteral( "1" ) );
  }
  if ( ignoreAxisOrientation )
  {
    mUri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), QStringLiteral( "1" ) );
  }
  if ( invertAxisOrientation )
  {
    mUri.setParam( QStringLiteral( "InvertAxisOrientation" ), QStringLiteral( "1" ) );
  }

  QgsDebugMsg( QString( "encoded uri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsDataSourceUri QgsOwsConnection::uri() const
{
  return mUri;
}

QStringList QgsOwsConnection::connectionList( const QString &service )
{
  QgsSettings settings;
  settings.beginGroup( "qgis//connections-" + service.toLower() );
  return settings.childGroups();
}

QString QgsOwsConnection::selectedConnection( const QString &service )
{
  QgsSettings settings;
  return settings.value( "qgis//connections-" + service.toLower() + "/selected" ).toString();
}

void QgsOwsConnection::setSelectedConnection( const QString &service, const QString &name )
{
  QgsSettings settings;
  settings.setValue( "qgis//connections-" + service.toLower() + "/selected", name );
}

void QgsOwsConnection::deleteConnection( const QString &service, const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis//connections-" + service.toLower() + '/' + name );
  settings.remove( "qgis//" + service + '/' + name );
}
