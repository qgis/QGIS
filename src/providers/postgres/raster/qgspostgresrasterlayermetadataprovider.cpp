/***************************************************************************
  qgspostgresrasterlayermetadataprovider.cpp - QgsPostgresRasterLayerMetadataProvider

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
#include "qgspostgresrasterlayermetadataprovider.h"
#include "qgsproviderregistry.h"
#include "qgsfeedback.h"
#include "qgsabstractproviderconnection.h"
#include "qgsprovidermetadata.h"


QString QgsPostgresRasterLayerMetadataProvider::type() const
{
  return QStringLiteral( "postgresraster" );
}

QgsLayerMetadataSearchResult QgsPostgresRasterLayerMetadataProvider::search( const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{
  QgsLayerMetadataSearchResult results;


  // PG raster does not have its own connections, get them from vector PG
  QgsProviderMetadata *pgMd { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) ) };
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgresraster" ) ) };

  if ( md && pgMd && ( ! feedback || ! feedback->isCanceled() ) )
  {
    const QMap<QString, QgsAbstractProviderConnection *> cConnections { pgMd->connections( ) };
    for ( const QgsAbstractProviderConnection *conn : std::as_const( cConnections ) )
    {

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( conn->configuration().value( QStringLiteral( "metadataInDatabase" ), false ).toBool() )
      {
        try
        {
          const QList<QgsLayerMetadataProviderResult> res { md->searchLayerMetadata( conn->uri(), searchString, geographicExtent, feedback ) };
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

  return results;
}


