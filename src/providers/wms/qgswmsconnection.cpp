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

QgsWMSConnection::QgsWMSConnection( QString theConnName ) :
    mConnName( theConnName )
{
  QgsDebugMsg( "theConnName = " + theConnName );

  QSettings settings;

  QString key = "/Qgis/connections-wms/" + mConnName;
  QString credentialsKey = "/Qgis/WMS/" + mConnName;

  QStringList connStringParts;

  mConnectionInfo = settings.value( key + "/url" ).toString();
  mUri.setParam( "url",  settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    if ( password.isEmpty() )
    {
      //password = QInputDialog::getText( this, tr( "WMS Password for %1" ).arg( theConnName ), "Password", QLineEdit::Password );
      password = QInputDialog::getText( 0, tr( "WMS Password for %1" ).arg( mConnName ), "Password", QLineEdit::Password );
    }
    mConnectionInfo = "username=" + username + ",password=" + password + ",url=" + mConnectionInfo;
    mUri.setParam( "username", username );
    mUri.setParam( "password", password );
  }

#ifndef QT_NO_OPENSSL
  bool validcert = settings.value( credentialsKey + "/ssl/validcert", false ).toBool();
  if ( validcert )
  {
    QString storetype = settings.value( credentialsKey + "/ssl/store" ).toString(); // int(enum) -> string
    QString certid = settings.value( credentialsKey + "/ssl/certid" ).toString();
    QString keyid = settings.value( credentialsKey + "/ssl/keyid" ).toString();
    bool haskeypass = settings.value( credentialsKey + "/ssl/haskeypass", false ).toBool();
    QString issuerid = settings.value( credentialsKey + "/ssl/issuerid" ).toString();
    bool issuerself = settings.value( credentialsKey + "/ssl/issuerself", false ).toBool();

    QString sslinfo;

    // store/cert/key should always be non-empty for cert to have validated
    sslinfo.append( ",storetype=" + storetype + ",certid=" + certid + ",keyid=" + keyid );
    mUri.setParam( "storetype", storetype );
    mUri.setParam( "certid", certid );
    mUri.setParam( "keyid", keyid );
    if ( haskeypass )
    {
      sslinfo.append( ",haskeypass=1" );
      mUri.setParam( "haskeypass", "1" );
    }
    if ( !issuerid.isEmpty() )
    {
      sslinfo.append( ",issuerid=" + issuerid );
      mUri.setParam( "issuerid", issuerid );
    }
    if ( issuerself )
    {
      sslinfo.append( ",issuerself=1" );
      mUri.setParam( "issuerself", "1" );
    }
    mConnectionInfo.append( sslinfo );
  }
#endif

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

  QString connArgs, delim;

  if ( ignoreGetMap )
  {
    connArgs += delim + "GetMap";
    delim = ";";
    mUri.setParam( "IgnoreGetMapUrl", "1" );
  }

  if ( ignoreGetFeatureInfo )
  {
    connArgs += delim + "GetFeatureInfo";
    delim = ";";
    mUri.setParam( "IgnoreGetFeatureInfoUrl", "1" );
  }

  if ( ignoreAxisOrientation )
  {
    connArgs += delim + "AxisOrientation";
    delim = ";";
    mUri.setParam( "IgnoreAxisOrientation", "1" );
  }

  if ( invertAxisOrientation )
  {
    connArgs += delim + "InvertAxisOrientation";
    delim = ";";
    mUri.setParam( "InvertAxisOrientation", "1" );
  }

  if ( smoothPixmapTransform )
  {
    connArgs += delim + "SmoothPixmapTransform";
    delim = ";";
    mUri.setParam( "SmoothPixmapTransform", "1" );
  }

  if ( !dpiMode.isEmpty() )
  {
    connArgs += delim + "dpiMode=" + dpiMode;
    delim = ";";
    mUri.setParam( "dpiMode", dpiMode );
  }

  if ( !connArgs.isEmpty() )
  {
    connArgs.prepend( "ignoreUrl=" );

    if ( mConnectionInfo.startsWith( "username=" ) )
    {
      mConnectionInfo.prepend( connArgs + "," );
    }
    else
    {
      mConnectionInfo.prepend( connArgs + ",url=" );
    }
  }

  QgsDebugMsg( QString( "Connection info: '%1'." ).arg( mConnectionInfo ) );
}

QgsWMSConnection::~QgsWMSConnection()
{

}

QString QgsWMSConnection::connectionInfo()
{
  return mConnectionInfo;
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

void QgsWMSConnection::setSelectedConnection( QString name )
{
  QSettings settings;
  settings.setValue( "/Qgis/connections-wms/selected", name );
}

void QgsWMSConnection::deleteConnection( QString name )
{
  QSettings settings;
  settings.remove( "/Qgis/connections-wms/" + name );
  settings.remove( "/Qgis/WMS/" + name );
}
