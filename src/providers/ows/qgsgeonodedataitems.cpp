/***************************************************************************
    qgsgeonodedataitems.cpp
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderregistry.h"
#include "qgsowsdataitems.h"
#include "qgslogger.h"
#include "qgsnewhttpconnection.h"
#include "qgsgeonodenewconnection.h"
#include "qgsgeonodedataitems.h"
#include "qgsgeocmsdataitems.h"


QgsDataItem *QgsGeoNodeDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "thePath = " + path );
  if ( path.isEmpty() )
  {
    return new QgsGeoCMSRootItem( parentItem, QStringLiteral( "GeoNode" ), QStringLiteral( "geonode:" ) );
  }

  // path schema: geonode:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsGeoNodeConnection::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection *connection = new QgsGeoNodeConnection( connectionName );
      return new QgsGeoCMSConnectionItem( parentItem, QStringLiteral( "GeoNode" ), path, connection );
    }
  }

  return nullptr;
}
