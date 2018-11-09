/***************************************************************************
    qgsamsproviderextern.cpp
    ------------------------
  begin                : Nov 26, 2015
  copyright            : (C) 2015 Sandro Mani
  email                : manisandro@gmail.com
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
#include "qgsamsdataitems.h"
#include "qgsamsprovider.h"
#include "qgsowsconnection.h"

#ifdef HAVE_GUI
#include "qgsamssourceselect.h"
#endif

const QString AMS_KEY = QStringLiteral( "arcgismapserver" );
const QString AMS_DESCRIPTION = QStringLiteral( "ArcGIS Map Server data provider" );


QGISEXTERN QgsAmsProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsAmsProvider( *uri, options );
}

QGISEXTERN QString providerKey()
{
  return AMS_KEY;
}

QGISEXTERN QString description()
{
  return AMS_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}

#ifdef HAVE_GUI
QGISEXTERN QgsAmsSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsAmsSourceSelect( parent, fl, widgetMode );
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
    return new QgsAmsRootItem( parentItem, QStringLiteral( "ArcGisMapServer" ), QStringLiteral( "arcgismapserver:" ) );
  }

  // path schema: ams:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "ams:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "arcgismapserver" ) ).contains( connectionName ) )
    {
      QgsOwsConnection connection( QStringLiteral( "arcgismapserver" ), connectionName );
      return new QgsAmsConnectionItem( parentItem, QStringLiteral( "ArcGisMapServer" ), path, connection.uri().param( QStringLiteral( "url" ) ) );
    }
  }

  return nullptr;
}

QGISEXTERN QVariantMap decodeUri( const QString &uri )
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  return components;
}
