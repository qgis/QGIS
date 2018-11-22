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

QGISEXTERN QVariantMap decodeUri( const QString &uri )
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  return components;
}
