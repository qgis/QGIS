/***************************************************************************
      qgsafsproviderextern.cpp
      ------------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsafsdataitems.h"
#include "qgsafsprovider.h"
#include "qgsafssourceselect.h"
#include "qgsowsconnection.h"

const QString AFS_KEY = "arcgisfeatureserver";
const QString AFS_DESCRIPTION = "ArcGIS Feature Server data provider";


QGISEXTERN QgsAfsProvider * classFactory( const QString *uri )
{
  return new QgsAfsProvider( *uri );
}

QGISEXTERN QString providerKey()
{
  return AFS_KEY;
}

QGISEXTERN QString description()
{
  return AFS_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QgsAfsSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsAfsSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  if ( thePath.isEmpty() )
  {
    return new QgsAfsRootItem( parentItem, "ArcGisFeatureServer", "arcgisfeatureserver:" );
  }

  // path schema: afs:/connection name (used by OWS)
  if ( thePath.startsWith( "afs:/" ) )
  {
    QString connectionName = thePath.split( '/' ).last();
    if ( QgsOWSConnection::connectionList( "ArcGisFeatureServer" ).contains( connectionName ) )
    {
      QgsOWSConnection connection( "ArcGisFeatureServer", connectionName );
      return new QgsAfsConnectionItem( parentItem, "ArcGisFeatureServer", thePath, connection.uri().param( "url" ) );
    }
  }

  return 0;
}

/*
QGISEXTERN bool saveStyle( const QString& uri, const QString& qmlStyle, const QString& sldStyle,
                           const QString& styleName, const QString& styleDescription,
                          const QString& uiFileContent, bool useAsDefault, QString& errCause )
{

}

QGISEXTERN QString loadStyle( const QString& uri, QString& errCause )
{

}

QGISEXTERN int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                           QStringList &descriptions, QString& errCause )
{

}

QGISEXTERN QString getStyleById( const QString& uri, QString styleId, QString& errCause )
{

}
*/
