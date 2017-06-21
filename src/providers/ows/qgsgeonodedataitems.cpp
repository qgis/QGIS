//
// Created by myarjunar on 25/04/17.
//

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
