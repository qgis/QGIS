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
/* $Id$ */

#include "../providers/wms/qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID
//#include "qgisapp.h" //for getThemeIcon
//#include "qgscontexthelp.h"
//#include "qgscoordinatereferencesystem.h"
//#include "qgsgenericprojectionselector.h"
//#include "qgslogger.h"
//#include "qgsmanageconnectionsdialog.h"
//#include "qgsmessageviewer.h"
#include "qgsnewhttpconnection.h"
//#include "qgsnumericsortlistviewitem.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgswmsconnection.h"
#include "qgsnetworkaccessmanager.h"

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
  if ( ignoreGetMap || ignoreGetFeatureInfo )
  {
    QString connArgs = "ignoreUrl=";
    if ( ignoreGetMap )
    {
      connArgs += "GetMap";
      if ( ignoreGetFeatureInfo )
        connArgs += ";";
    }
    if ( ignoreGetFeatureInfo )
      connArgs += "GetFeatureInfo";

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

QString QgsWMSConnection::connectionInfo( )
{
  return mConnectionInfo;
}

QgsWmsProvider * QgsWMSConnection::provider( )
{
  // TODO: Create and bind to data provider

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  QgsWmsProvider *wmsProvider =
    ( QgsWmsProvider* ) pReg->provider( "wms", mConnectionInfo );

  return wmsProvider;
}

