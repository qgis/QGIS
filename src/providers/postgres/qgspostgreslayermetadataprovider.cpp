/***************************************************************************
  qgspostgreslayermetadataprovider.cpp - QgsPostgresLayerMetadataProvider

 ---------------------
 begin                : 17.8.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgreslayermetadataprovider.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsfeedback.h"


QString QgsPostgresLayerMetadataProvider::id() const
{
  return QStringLiteral( "postgres" );
}

QgsLayerMetadataSearchResults QgsPostgresLayerMetadataProvider::search( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{
  QgsLayerMetadataSearchResults results;
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) ) };

  if ( md && ( ! feedback || ! feedback->isCanceled() ) )
  {
    const QMap<QString, QgsAbstractProviderConnection *> constConnections { md->connections( ) };
    for ( const QgsAbstractProviderConnection *conn : std::as_const( constConnections ) )
    {

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( conn->configuration().value( QStringLiteral( "metadataInDatabase" ), false ).toBool() )
      {
        if ( const QgsAbstractDatabaseProviderConnection *dbConn = static_cast<const QgsAbstractDatabaseProviderConnection *>( conn ) )
        {
          try
          {
            const QList<QgsLayerMetadataProviderResult> res { dbConn->searchLayerMetadata( searchContext, searchString, geographicExtent, feedback ) };
            for ( const QgsLayerMetadataProviderResult &result : std::as_const( res ) )
            {
              results.addMetadata( result );
            }
          }
          catch ( const QgsProviderConnectionException &ex )
          {
            results.addError( QObject::tr( "An error occurred while searching for metadata in connection %1: %2" ).arg( conn->uri(), ex.what() ) );
          }
        }
      }
    }
  }

  return results;
}


