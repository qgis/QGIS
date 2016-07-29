/***************************************************************************
    qgswmsconnection.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../providers/wms/qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgsdatasourceuri.h"
#include "qgsnewhttpconnection.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgswmsconnection.h"
#include "qgsnetworkaccessmanager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QUrl>

#include <QNetworkRequest>
#include <QNetworkReply>

QgsWMSConnection::QgsWMSConnection( const QString& theConnName )
    : mConnName( theConnName )
{
  QgsDebugMsg( "theConnName = " + theConnName );

  QSettings settings;

  QString key = "/Qgis/connections-wms/" + mConnName;
  QString credentialsKey = "/Qgis/WMS/" + mConnName;

  QStringList connStringParts;

  mUri.setParam( "url", settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    mUri.setParam( "username", username );
    mUri.setParam( "password", password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( "authcfg", authcfg );
  }

  QString referer = settings.value( key + "/referer" ).toString();
  if ( !referer.isEmpty() )
  {
    mUri.setParam( "referer", referer );
  }

  bool ignoreGetMap = settings.value( key + "/ignoreGetMapURI", false ).toBool();
  bool ignoreGetFeatureInfo = settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool();
  bool ignoreAxisOrientation = settings.value( key + "/ignoreAxisOrientation", false ).toBool();
  bool invertAxisOrientation = settings.value( key + "/invertAxisOrientation", false ).toBool();
  bool smoothPixmapTransform = settings.value( key + "/smoothPixmapTransform", false ).toBool();
  QString dpiMode = settings.value( key + "/dpiMode", "all" ).toString();

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

  if ( smoothPixmapTransform )
  {
    mUri.setParam( "SmoothPixmapTransform", "1" );
  }

  if ( !dpiMode.isEmpty() )
  {
    mUri.setParam( "dpiMode", dpiMode );
  }

  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsWMSConnection::~QgsWMSConnection()
{

}

QgsDataSourceURI QgsWMSConnection::uri()
{
  return mUri;
}

QStringList QgsWMSConnection::connectionList()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wms" );
  return settings.childGroups();
}

QString QgsWMSConnection::selectedConnection()
{
  QSettings settings;
  return settings.value( "/Qgis/connections-wms/selected" ).toString();
}

void QgsWMSConnection::setSelectedConnection( const QString& name )
{
  QSettings settings;
  settings.setValue( "/Qgis/connections-wms/selected", name );
}

void QgsWMSConnection::deleteConnection( const QString& name )
{
  QSettings settings;
  settings.remove( "/Qgis/connections-wms/" + name );
  settings.remove( "/Qgis/WMS/" + name );
}
