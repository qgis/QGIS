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

//#include "../providers/wms/qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID
//#include "qgisapp.h" //for getThemeIcon
//#include "qgscontexthelp.h"
//#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
//#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
//#include "qgsmanageconnectionsdialog.h"
//#include "qgsmessageviewer.h"
//#include "qgsnewhttpconnection.h"
//#include "qgsnumericsortlistviewitem.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsowsconnection.h"
//#include "qgsnetworkaccessmanager.h"

//#include <QButtonGroup>
//#include <QFileDialog>
//#include <QRadioButton>
//#include <QDomDocument>
//#include <QHeaderView>
//#include <QImageReader>
#include <QInputDialog>
//#include <QMap>
#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QUrl>

#include <QNetworkRequest>
#include <QNetworkReply>

QgsOWSConnection::QgsOWSConnection( const QString & theService, const QString & theConnName ) :
    mConnName( theConnName ),
    mService( theService )
{
  QgsDebugMsg( "theConnName = " + theConnName );

  QSettings settings;

  // WMS (providers/wfs/qgswmsconnection.cpp):
  //QString key = "/Qgis/connections-wms/" + mConnName;
  //QString credentialsKey = "/Qgis/WMS/" + mConnName;

  // WFS (providers/wfs/qgswfsconnection.cpp):
  //QString key = "/Qgis/connections-wfs/" + mConnName + "/url";

  // WCS - there was no WCS before

  QString key = "/Qgis/connections-" + mService.toLower() + "/" + mConnName;
  QString credentialsKey = "/Qgis/" + mService + "/" + mConnName;

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

  bool ignoreGetMap = settings.value( key + "/ignoreGetMapURI", false ).toBool();
  bool ignoreGetFeatureInfo = settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool();
  if ( ignoreGetMap || ignoreGetFeatureInfo )
  {
    QString connArgs = "ignoreUrl=";
    if ( ignoreGetMap )
    {
      connArgs += "GetMap";
      if ( ignoreGetFeatureInfo )
        connArgs += ";";
      mUri.setParam( "ignoreUrl", "GetMap" );
    }
    if ( ignoreGetFeatureInfo )
    {
      connArgs += "GetFeatureInfo";
      mUri.setParam( "ignoreUrl", "GetFeatureInfo" );
    }
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

QgsOWSConnection::~QgsOWSConnection()
{

}

QString QgsOWSConnection::connectionInfo( )
{
  return mConnectionInfo;
}

QgsDataSourceURI QgsOWSConnection::uri()
{
  return mUri;
}
/*
QgsDataProvider * QgsOWSConnection::provider( )
{
  // TODO: remove completely from this class?

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  
  //QMap<QString,QString> keys;

  QgsDataProvider *provider =
    ( QgsDataProvider* ) pReg->provider( "wms", mUri.encodedUri() );

  return provider;
}
*/


QStringList QgsOWSConnection::connectionList( const QString & theService )
{
  QSettings settings;
  //settings.beginGroup( "/Qgis/connections-wms" );
  settings.beginGroup( "/Qgis/connections-" + theService.toLower() );
  return settings.childGroups();
}

QString QgsOWSConnection::selectedConnection( const QString & theService )
{
  QSettings settings;
  //return settings.value( "/Qgis/connections-wms/selected" ).toString();
  return settings.value( "/Qgis/connections-" + theService.toLower() + "/selected" ).toString();
}

void QgsOWSConnection::setSelectedConnection( const QString & theService, const QString & name )
{
  QSettings settings;
  //settings.setValue( "/Qgis/connections-wms/selected", name );
  settings.setValue( "/Qgis/connections-" + theService.toLower() + "/selected", name );
}

void QgsOWSConnection::deleteConnection( const QString & theService, const QString & name )
{
  QSettings settings;
  //settings.remove( "/Qgis/connections-wms/" + name );
  //settings.remove( "/Qgis/WMS/" + name );
  settings.remove( "/Qgis/connections-" + theService.toLower() + "/" + name );
  settings.remove( "/Qgis/" + theService + "/" + name );
}
