/***************************************************************************
  QgsGeoPackageProviderConnection.cpp - QgsGeoPackageProviderConnection

 ---------------------
 begin                : 6.8.2019
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
#include "qgsgeopackageproviderconnection.h"
#include "qgsogrdbconnection.h"
#include "qgssettings.h"
#include "qgsogrprovider.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"

#include <sqlite3.h>

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &name ):
  QgsAbstractDatabaseProviderConnection( name )
{
  setDefaultCapabilities();
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name );
  setUri( settings.value( QStringLiteral( "path" ) ).toString() );
}

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &name, const QString &uri ):
  QgsAbstractDatabaseProviderConnection( name )
{
  setDefaultCapabilities();
  setUri( uri );
}

void QgsGeoPackageProviderConnection::store( QVariantMap guiConfig ) const
{
  Q_UNUSED( guiConfig );
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name() );
  settings.setValue( QStringLiteral( "path" ), uri() );
}

void QgsGeoPackageProviderConnection::remove() const
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.remove( name() );
}


void QgsGeoPackageProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  auto opts { *options };
  opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
  opts[ QStringLiteral( "update" ) ] = true;
  QMap<int, int> map;
  QString errCause;
  QgsVectorLayerExporter::ExportError errCode = QgsOgrProvider::createEmptyLayer(
        uri(),
        fields,
        wkbType,
        srs,
        overwrite,
        &map,
        &errCause,
        &opts
      );
  if ( errCode != QgsVectorLayerExporter::ExportError::NoError )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occourred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

void QgsGeoPackageProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  QString errCause;
  QString layerUri { uri() };
  layerUri.append( QStringLiteral( "|layername=%1" ).arg( name ) );
  if ( ! QgsOgrProviderUtils::deleteLayer( layerUri, errCause ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name ).arg( errCause ) );
  }
}


void QgsGeoPackageProviderConnection::dropRasterTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropRasterTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  executeGdalSqlPrivate( QStringLiteral( "DROP TABLE %1" ).arg( name ) );
}


void QgsGeoPackageProviderConnection::renameTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  // TODO: won't work for RASTER layers
  // TODO: maybe an index?
  QString sql( QStringLiteral( "ALTER TABLE %1 RENAME TO %2" )
               .arg( QgsSqliteUtils::quotedIdentifier( name ),
                     QgsSqliteUtils::quotedIdentifier( newName ) ) );
  executeGdalSqlPrivate( sql );
  sql = QStringLiteral( "UPDATE layer_styles SET f_table_name = %2 WHERE f_table_name = %1" )
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  try
  {
    executeGdalSqlPrivate( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QgsDebugMsgLevel( QStringLiteral( "Warning: error while updating the styles, perhaps there are no styles stored in this GPKG: %1" ).arg( ex.what() ), 4 );
  }
}

QList<QVariantList> QgsGeoPackageProviderConnection::executeSql( const QString &sql ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeGdalSqlPrivate( sql );
}

void QgsGeoPackageProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  Q_UNUSED( name );
  checkCapability( Capability::Vacuum );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  executeGdalSqlPrivate( QStringLiteral( "VACUUM" ) );
}

static int collect_table_info( void *tableProperties, int, char **argv, char ** )
{
  QgsGeoPackageProviderConnection::TableProperty property;
  property.setTableName( QString::fromUtf8( argv[ 0 ] ) );
  property.setPkColumns( { QLatin1String( "fid" ) } );
  property.setGeometryColumnCount( 0 );
  static const QStringList aspatialTypes = { QStringLiteral( "attributes" ), QStringLiteral( "aspatial" ) };
  const auto data_type = QString::fromUtf8( argv[ 1 ] );
  // Table type
  if ( data_type == QStringLiteral( "tiles" ) )
  {
    property.setFlag( QgsGeoPackageProviderConnection::Raster );
  }
  else if ( data_type == QStringLiteral( "features" ) )
  {
    property.setFlag( QgsGeoPackageProviderConnection::Vector );
    property.setGeometryColumn( QStringLiteral( "geom" ) );
    property.setGeometryColumnCount( 1 );
  }
  if ( aspatialTypes.contains( data_type ) )
  {
    property.setFlag( QgsGeoPackageProviderConnection::Aspatial );
    property.addGeometryType( QgsWkbTypes::Type::NoGeometry, 0 );
  }
  else
  {
    bool ok;
    property.addGeometryType( QgsWkbTypes::parseType( QString::fromUtf8( argv[ 4 ] ) ), QString::fromUtf8( argv[ 3 ] ).toInt( &ok ) );
    if ( !ok )
    {
      throw QgsProviderConnectionException( QObject::tr( "Error fetching srs_id table information: %1" ).arg( QString::fromUtf8( argv[ 3 ] ) ) );
    }
  }
  property.setComment( QString::fromUtf8( argv[ 2 ] ) );
  static_cast<QList<QgsGeoPackageProviderConnection::TableProperty>*>( tableProperties )->push_back( property );
  return 0;
}

QList<QgsGeoPackageProviderConnection::TableProperty> QgsGeoPackageProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  QList<QgsGeoPackageProviderConnection::TableProperty> tableInfo;
  QString errCause;
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( QLatin1String( "ogr" ), uri() ) );
  QString baseUri = pieces[QStringLiteral( "path" )].toString();
  if ( !baseUri.isEmpty() )
  {
    char *errmsg = nullptr;
    sqlite3_database_unique_ptr database;
    int status = database.open_v2( baseUri, SQLITE_OPEN_READONLY, nullptr );
    if ( status == SQLITE_OK )
    {
      char *sql = sqlite3_mprintf( "SELECT c.table_name, data_type, description, c.srs_id, g.geometry_type_name "
                                   "FROM gpkg_contents c LEFT JOIN gpkg_geometry_columns g ON (c.table_name = g.table_name);" );
      try
      {
        status = sqlite3_exec(
                   database.get(),              /* An open database */
                   sql,                         /* SQL to be evaluated */
                   collect_table_info,          /* Callback function */
                   &tableInfo,                  /* 1st argument to callback */
                   &errmsg                      /* Error msg written here */
                 );
      }
      catch ( QgsProviderConnectionException &ex )
      {
        errCause = ex.what();
      }
      sqlite3_free( sql );
      if ( status != SQLITE_OK )
      {
        errCause = QStringLiteral( "There was an error reading tables from GPKG layer %1: %2" ).arg( uri(), QString::fromUtf8( errmsg ) );
      }
      sqlite3_free( errmsg );
    }
    else
    {
      errCause = QStringLiteral( "There was an error opening GPKG %1" ).arg( uri() );
    }
  }
  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( name() ).arg( errCause ) );
  }
  // Filters
  if ( flags )
  {
    QList<QgsGeoPackageProviderConnection::TableProperty> filteredTableInfo;
    for ( const auto &ti : qgis::as_const( tableInfo ) )
    {
      if ( ti.flags() & flags )
      {
        filteredTableInfo.push_back( ti );
      }
    }
    tableInfo = filteredTableInfo;
  }
  return tableInfo ;
}

void QgsGeoPackageProviderConnection::setDefaultCapabilities()
{
  mCapabilities =
  {
    Capability::Tables,
    Capability::CreateVectorTable,
    Capability::DropVectorTable,
    Capability::RenameTable,
    Capability::Vacuum,
    Capability::Spatial,
    Capability::TableExists,
    Capability::ExecuteSql,
  };
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,4,0)
  mCapabilities |= Capability::DropRasterTable;
#endif
}

QList<QVariantList> QgsGeoPackageProviderConnection::executeGdalSqlPrivate( const QString &sql ) const
{
  QString errCause;
  QList<QVariantList> results;
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS.get(), sql.toUtf8().constData(), nullptr, nullptr ) );
    if ( ogrLayer )
    {
      gdal::ogr_feature_unique_ptr fet;
      while ( fet.reset( OGR_L_GetNextFeature( ogrLayer ) ), fet )
      {
        QVariantList row;
        for ( int i = 0; i < OGR_F_GetFieldCount( fet.get() ); i++ )
        {
          row.push_back( QVariant( QString::fromUtf8( OGR_F_GetFieldAsString( fet.get(), i ) ) ) );
        }
        results.push_back( row );
      }
      GDALDatasetReleaseResultSet( hDS.get(), ogrLayer );
    }
    errCause = CPLGetLastErrorMsg( );
  }
  else
  {
    errCause = QObject::tr( "There was an error opening GPKG %1!" ).arg( uri() );
  }
  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql ).arg( errCause ) );
  }
  return results;
}

