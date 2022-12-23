/***************************************************************************
  qgspostgresprovidermetadatautils.cpp - QgsPostgresProviderMetadataUtils

 ---------------------
 begin                : 29.8.2022
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresprovidermetadatautils.h"
#include "qgspostgresproviderconnection.h"
#include "qgscoordinatetransform.h"

#include <QTextStream>

QList<QgsLayerMetadataProviderResult> QgsPostgresProviderMetadataUtils::searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &uri, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback )
{
  Q_UNUSED( searchContext );
  QList<QgsLayerMetadataProviderResult> results;
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( conn && ( ! feedback || ! feedback->isCanceled() ) )
  {

    QString schemaName { QStringLiteral( "public" ) };
    const QString schemaQuery = QStringLiteral( "SELECT table_schema FROM information_schema.tables WHERE table_name = 'qgis_layer_metadata'" );
    QgsPostgresResult res( conn->LoggedPQexec( "QgsPostgresProviderMetadata", schemaQuery ) );
    if ( res.PQntuples( ) > 0 )
    {
      schemaName = res.PQgetvalue( 0, 0 );
    }

    QStringList where;

    if ( ! searchString.isEmpty() )
    {
      where.push_back( QStringLiteral( R"SQL((
      abstract ILIKE %1 OR
      identifier ILIKE %1 OR
      REGEXP_REPLACE(UPPER(array_to_string((xpath('//keyword', qmd))::varchar[], ' ')),'</?KEYWORD>', '', 'g') ILIKE %1
      ))SQL" ).arg( QgsPostgresConn::quotedValue( QString( searchString ).prepend( QChar( '%' ) ).append( QChar( '%' ) ) ) ) );
    }

    if ( ! geographicExtent.isEmpty() )
    {
      where.push_back( QStringLiteral( "ST_Intersects( extent, ST_GeomFromText( %1, 4326 ) )" ).arg( QgsPostgresConn::quotedValue( geographicExtent.asWktPolygon() ) ) );
    }

    const QString listQuery = QStringLiteral( R"SQL(
            SELECT
               f_table_catalog
              ,f_table_schema
              ,f_table_name
              ,f_geometry_column
              ,identifier
              ,title
              ,abstract
              ,geometry_type
              ,ST_AsText( extent )
              ,crs
              ,layer_type
              ,qmd
              ,owner
              ,update_time
           FROM %1.qgis_layer_metadata
             %2
           )SQL" ).arg( QgsPostgresConn::quotedIdentifier( schemaName ), where.isEmpty() ? QString() : ( QStringLiteral( " WHERE %1 " ).arg( where.join( QLatin1String( " AND " ) ) ) ) );

    res = conn->LoggedPQexec( "QgsPostgresProviderMetadata", listQuery );

    if ( res.PQresultStatus() != PGRES_TUPLES_OK )
    {
      throw QgsProviderConnectionException( QObject::tr( "Error while fetching metadata from %1: %2" ).arg( dsUri.connectionInfo( false ), res.PQresultErrorMessage() ) );
    }

    for ( int row = 0; row < res.PQntuples( ); ++row )
    {

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      QgsLayerMetadata metadata;
      QDomDocument doc;
      doc.setContent( res.PQgetvalue( row, 11 ) );
      metadata.readMetadataXml( doc.documentElement() );

      QgsLayerMetadataProviderResult result { metadata };
      QgsDataSourceUri uri { dsUri };
      uri.setDatabase( res.PQgetvalue( row, 0 ) );
      uri.setSchema( res.PQgetvalue( row, 1 ) );
      uri.setTable( res.PQgetvalue( row, 2 ) );
      uri.setGeometryColumn( res.PQgetvalue( row, 3 ) );
      result.setStandardUri( QStringLiteral( "http://mrcc.com/qgis.dtd" ) );
      result.setGeometryType( QgsWkbTypes::geometryType( QgsWkbTypes::parseType( res.PQgetvalue( row, 7 ) ) ) );
      QgsPolygon geographicExtent;
      geographicExtent.fromWkt( res.PQgetvalue( row, 8 ) );
      result.setGeographicExtent( geographicExtent );
      result.setAuthid( res.PQgetvalue( row, 9 ) );
      const QString layerType { res.PQgetvalue( row, 10 ) };
      if ( layerType == QLatin1String( "raster" ) )
      {
        result.setDataProviderName( QStringLiteral( "postgresraster" ) );
        result.setLayerType( QgsMapLayerType::RasterLayer );
      }
      else if ( layerType == QLatin1String( "vector" ) )
      {
        result.setDataProviderName( QStringLiteral( "postgres" ) );
        result.setLayerType( QgsMapLayerType::VectorLayer );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Unsupported layer type '%1': skipping metadata record" ).arg( layerType ) );
        continue;
      }
      result.setUri( uri.uri() );
      results.append( result );
    }
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection to database %1 failed" ).arg( dsUri.connectionInfo( false ) ) );
  }
  return results;
}

bool QgsPostgresProviderMetadataUtils::saveLayerMetadata( const QgsMapLayerType &layerType, const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage )
{
  QgsDataSourceUri dsUri( uri );

  QString layerTypeString;

  if ( layerType == QgsMapLayerType::VectorLayer )
  {
    layerTypeString = QStringLiteral( "vector" );
  }
  else if ( layerType == QgsMapLayerType::RasterLayer )
  {
    layerTypeString = QStringLiteral( "raster" );
  }
  else
  {
    // Unsupported!
    return false;
  }

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errorMessage = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  // Try to load metadata
  QString schemaName { dsUri.schema().isEmpty() ? QStringLiteral( "public" ) : dsUri.schema() };
  const QString schemaQuery = QStringLiteral( "SELECT table_schema FROM information_schema.tables WHERE table_name = 'qgis_layer_metadata'" );
  QgsPostgresResult res( conn->LoggedPQexec( "QgsPostgresProviderMetadataUtils", schemaQuery ) );
  const bool metadataTableFound { res.PQntuples( ) > 0 };
  if ( metadataTableFound )
  {
    schemaName = res.PQgetvalue( 0, 0 ) ;
  }
  else
  {
    QgsPostgresResult res( conn->LoggedPQexec( QStringLiteral( "QgsPostgresProviderMetadataUtils" ),
                           QStringLiteral( R"SQL(
            CREATE TABLE %1.qgis_layer_metadata (
              id SERIAL PRIMARY KEY
              ,f_table_catalog VARCHAR NOT NULL
              ,f_table_schema VARCHAR NOT NULL
              ,f_table_name VARCHAR NOT NULL
              ,f_geometry_column VARCHAR
              ,identifier TEXT NOT NULL
              ,title TEXT NOT NULL
              ,abstract TEXT
              ,geometry_type VARCHAR
              ,extent GEOMETRY(POLYGON, 4326)
              ,crs VARCHAR
              ,layer_type VARCHAR NOT NULL
              ,qmd XML NOT NULL
              ,owner VARCHAR(63) DEFAULT CURRENT_USER
              ,update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
              UNIQUE (f_table_catalog, f_table_schema, f_table_name, f_geometry_column, geometry_type, crs, layer_type)
            )
          )SQL" ).arg( QgsPostgresConn::quotedIdentifier( schemaName ) ) ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      errorMessage = QObject::tr( "Unable to save layer metadata. It's not possible to create the destination table on the database. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
      conn->unref();
      return false;
    }
  }

  const QString wkbTypeString = QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) );

  const QgsCoordinateReferenceSystem metadataCrs { metadata.crs() };
  QgsCoordinateReferenceSystem destCrs {QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) };
  QgsRectangle extents;

  const auto cExtents { metadata.extent().spatialExtents() };
  for ( const auto &ext : std::as_const( cExtents ) )
  {
    QgsRectangle bbox {  ext.bounds.toRectangle()  };
    // Note: a default transform context is used here because we don't need high accuracy
    QgsCoordinateTransform ct { ext.extentCrs, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsCoordinateTransformContext() };
    ct.transform( bbox );
    extents.combineExtentWith( bbox );
  }

  // export metadata to XML
  QDomImplementation domImplementation;
  QDomDocumentType documentType = domImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument document( documentType );

  QDomElement rootNode = document.createElement( QStringLiteral( "qgis" ) );
  rootNode.setAttribute( QStringLiteral( "version" ), Qgis::version() );
  document.appendChild( rootNode );

  if ( !metadata.writeMetadataXml( rootNode, document ) )
  {
    errorMessage = QObject::tr( "Error exporting metadata to XML" );
    return false;
  }

  QString metadataXml;
  QTextStream textStream( &metadataXml );
  document.save( textStream, 2 );

  // Note: in the construction of the INSERT and UPDATE strings the qmd values
  // can contain user entered strings, which may themselves include %## values that would be
  // replaced by the QString.arg function.  To ensure that the final SQL string is not corrupt these
  // two values are both replaced in the final .arg call of the string construction.

  QString upsertSql = QStringLiteral( R"SQL(
            INSERT INTO %1.qgis_layer_metadata(
               f_table_catalog
              ,f_table_schema
              ,f_table_name
              ,f_geometry_column
              ,identifier
              ,title
              ,abstract
              ,geometry_type
              ,extent
              ,crs
              ,layer_type
              ,qmd) VALUES (
               %2,%3,%4,%5,%6,%7,%8,%9,ST_GeomFromText(%10, 4326),%11,%12,XMLPARSE(DOCUMENT %13))
             )SQL" )
                      .arg( QgsPostgresConn::quotedIdentifier( schemaName ) )
                      .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                      .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                      .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                      .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) )
                      .arg( QgsPostgresConn::quotedValue( metadata.identifier() ) )
                      .arg( QgsPostgresConn::quotedValue( metadata.title() ) )
                      .arg( QgsPostgresConn::quotedValue( metadata.abstract() ) )
                      .arg( QgsPostgresConn::quotedValue( wkbTypeString ) )
                      .arg( QgsPostgresConn::quotedValue( extents.asWktPolygon() ) )
                      .arg( QgsPostgresConn::quotedValue( metadataCrs.authid() ) )
                      .arg( QgsPostgresConn::quotedValue( layerTypeString ) )
                      // Must be the final .arg replacement - see above
                      .arg( QgsPostgresConn::quotedValue( metadataXml ) );

  QString checkQuery = QStringLiteral( R"SQL(
            SELECT
              id
           FROM %1.qgis_layer_metadata
             WHERE
                f_table_catalog=%2
                AND f_table_schema=%3
                AND f_table_name=%4
                AND f_geometry_column %5
                AND layer_type = %7
           )SQL" )
                       .arg( QgsPostgresConn::quotedIdentifier( schemaName ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                       .arg( dsUri.geometryColumn().isEmpty() ?
                             QStringLiteral( "IS NULL" ) :
                             QStringLiteral( "=%1" ).arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) ) )
                       .arg( QgsPostgresConn::quotedValue( layerTypeString ) );

  res = conn->LoggedPQexec( "QgsPostgresProviderMetadataUtils", checkQuery );
  if ( res.PQntuples() > 0 )
  {
    const qulonglong id { res.PQgetvalue( 0, 0 ).toULongLong() };
    upsertSql = QStringLiteral( R"SQL(
                UPDATE %1.qgis_layer_metadata
                  SET
                  owner=CURRENT_USER
                  ,title=%3
                  ,abstract=%4
                  ,geometry_type=%5
                  ,extent=ST_GeomFromText(%6, 4326)
                  ,crs=%7
                  ,qmd=XMLPARSE(DOCUMENT %8)
                    WHERE
                        id = %2
                 )SQL" )
                .arg( QgsPostgresConn::quotedIdentifier( schemaName ) )
                .arg( id )
                .arg( QgsPostgresConn::quotedValue( metadata.title() ) )
                .arg( QgsPostgresConn::quotedValue( metadata.abstract() ) )
                .arg( QgsPostgresConn::quotedValue( wkbTypeString ) )
                .arg( QgsPostgresConn::quotedValue( extents.asWktPolygon() ) )
                .arg( QgsPostgresConn::quotedValue( metadataCrs.authid() ) )
                // Must be the final .arg replacement - see above
                .arg( QgsPostgresConn::quotedValue( metadataXml ) );

  }

  res = conn->LoggedPQexec( "QgsPostgresProviderMetadataUtils", upsertSql );

  bool saved = res.PQresultStatus() == PGRES_COMMAND_OK;
  if ( !saved )
    errorMessage = QObject::tr( "Unable to save layer metadata. It's not possible to insert a new record into the qgis_layer_metadata table. Maybe this is due to table permissions (user=%1). Please contact your database administrator." ).arg( dsUri.username() );

  conn->unref();

  return saved;
}
