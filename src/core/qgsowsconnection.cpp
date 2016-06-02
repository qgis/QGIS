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

#include <QInputDialog>
#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QUrl>

#include <QNetworkRequest>
#include <QNetworkReply>

QgsOWSConnection::QgsOWSConnection( const QString & theService, const QString & theConnName )
    : mConnName( theConnName )
    , mService( theService )
{
  QgsDebugMsg( "theConnName = " + theConnName );

  QSettings settings;

  QString key = "/Qgis/connections-" + mService.toLower() + '/' + mConnName;
  QString credentialsKey = "/Qgis/" + mService + '/' + mConnName;

  QStringList connStringParts;

  mConnectionInfo = settings.value( key + "/url" ).toString();
  mUri.setParam( "url", settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    mUri.setParam( "username", username );
    mUri.setParam( "password", password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( "authcfg", authcfg );
  }
  mConnectionInfo.append( ",authcfg=" + authcfg );

  bool ignoreGetMap = settings.value( key + "/ignoreGetMapURI", false ).toBool();
  bool ignoreGetFeatureInfo = settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool();
  bool ignoreAxisOrientation = settings.value( key + "/ignoreAxisOrientation", false ).toBool();
  bool invertAxisOrientation = settings.value( key + "/invertAxisOrientation", false ).toBool();
  if ( ignoreGetMap )
  {
    mUri.setParam( "IgnoreGetMapUrl", "1" );
  }
  if ( ignoreGetFeatureInfo )
  {
    mUri.setParam( "IgnoreGetFeatureInfoUrl", "1" );
  }
  if ( ignoreAxisOrientation )
  {
    mUri.setParam( "IgnoreAxisOrientation", "1" );
  }
  if ( invertAxisOrientation )
  {
    mUri.setParam( "InvertAxisOrientation", "1" );
  }

  QgsDebugMsg( QString( "encoded uri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsOWSConnection::~QgsOWSConnection()
{

}

QString QgsOWSConnection::connectionInfo()
{
  return mConnectionInfo;
}

QgsDataSourceURI QgsOWSConnection::uri() const
{
  return mUri;
}

QStringList QgsOWSConnection::connectionList( const QString & theService )
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-" + theService.toLower() );
  return settings.childGroups();
}

QString QgsOWSConnection::selectedConnection( const QString & theService )
{
  QSettings settings;
  return settings.value( "/Qgis/connections-" + theService.toLower() + "/selected" ).toString();
}

void QgsOWSConnection::setSelectedConnection( const QString & theService, const QString & name )
{
  QSettings settings;
  settings.setValue( "/Qgis/connections-" + theService.toLower() + "/selected", name );
}

void QgsOWSConnection::deleteConnection( const QString & theService, const QString & name )
{
  QSettings settings;
  settings.remove( "/Qgis/connections-" + theService.toLower() + '/' + name );
  settings.remove( "/Qgis/" + theService + '/' + name );
}
