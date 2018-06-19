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
#include "qgsowsconnection.h"

#ifdef HAVE_GUI
#include "qgsafssourceselect.h"
#endif

const QString AFS_KEY = QStringLiteral( "arcgisfeatureserver" );
const QString AFS_DESCRIPTION = QStringLiteral( "ArcGIS Feature Server data provider" );


QGISEXTERN QgsAfsProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsAfsProvider( *uri, options );
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

#ifdef HAVE_GUI
QGISEXTERN QgsAfsSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsAfsSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsAfsRootItem( parentItem, QStringLiteral( "ArcGisFeatureServer" ), QStringLiteral( "arcgisfeatureserver:" ) );
  }

  // path schema: afs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "afs:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "arcgisfeatureserver" ) ).contains( connectionName ) )
    {
      QgsOwsConnection connection( QStringLiteral( "arcgisfeatureserver" ), connectionName );
      return new QgsAfsConnectionItem( parentItem, QStringLiteral( "ArcGisFeatureServer" ), path, connection.uri().param( QStringLiteral( "url" ) ) );
    }
  }

  return nullptr;
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
