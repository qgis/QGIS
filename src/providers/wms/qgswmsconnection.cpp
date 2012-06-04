/***************************************************************************
    qgswmsconnection.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : May 2011
    original             : (C) 2011 by Martin Dobias

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
  }

  bool ignoreGetMap = settings.value( key + "/ignoreGetMapURI", false ).toBool();
  bool ignoreGetFeatureInfo = settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool();
  bool ignoreAxisOrientation = settings.value( key + "/ignoreAxisOrientation", false ).toBool();
  bool invertAxisOrientation = settings.value( key + "/invertAxisOrientation", false ).toBool();

  QString connArgs, delim;


  if ( ignoreGetMap )
  {
    connArgs += delim + "GetMap";
    delim = ";";
  }

  if ( ignoreGetFeatureInfo )
  {
    connArgs += delim + "GetFeatureInfo";
    delim = ";";
  }

  if ( ignoreAxisOrientation )
  {
    connArgs += delim + "AxisOrientation";
    delim = ";";
  }

  if ( invertAxisOrientation )
  {
    connArgs += delim + "InvertAxisOrientation";
    delim = ";";
  }

  if( !connArgs.isEmpty() )
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

QgsWmsProvider *QgsWMSConnection::provider()
{
  // TODO: Create and bind to data provider

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  QgsWmsProvider *wmsProvider =
    ( QgsWmsProvider* ) pReg->provider( "wms", mConnectionInfo );

  return wmsProvider;
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
