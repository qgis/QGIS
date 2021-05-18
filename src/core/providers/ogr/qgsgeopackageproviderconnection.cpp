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
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "ogr" );
  setDefaultCapabilities();
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name );
  setUri( settings.value( QStringLiteral( "path" ) ).toString() );
}

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = QStringLiteral( "ogr" );
  // Cleanup the URI in case it contains other information other than the file path
  if ( uri.contains( '|' ) )
  {
    setUri( uri.left( uri.indexOf( '|' ) ).trimmed() );
  }
  setDefaultCapabilities();
}

void QgsGeoPackageProviderConnection::store( const QString &name ) const
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name );
  settings.setValue( QStringLiteral( "path" ), uri() );
}

void QgsGeoPackageProviderConnection::remove( const QString &name ) const
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.remove( name );
}

QString QgsGeoPackageProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  if ( tableInfo.flags().testFlag( QgsAbstractDatabaseProviderConnection::TableFlag::Raster ) )
  {
    return QStringLiteral( "GPKG:%1:%2" ).arg( uri(), name );
  }
  else
  {
    return uri() + QStringLiteral( "|layername=%1" ).arg( name );
  }
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
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  QMap<QString, QVariant> opts { *options };
  opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
  opts[ QStringLiteral( "update" ) ] = true;
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult errCode = QgsOgrProvider::createEmptyLayer(
                                       uri(),
                                       fields,
                                       wkbType,
                                       srs,
                                       overwrite,
                                       &map,
                                       &errCause,
                                       &opts
                                     );
  if ( errCode != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

void QgsGeoPackageProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  QString errCause;
  const QString layerUri { QStringLiteral( "%1|layername=%2" ).arg( uri(), name ) };
  if ( ! QgsOgrProviderUtils::deleteLayer( layerUri, errCause ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name, errCause ) );
  }
}


void QgsGeoPackageProviderConnection::dropRasterTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropRasterTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  executeGdalSqlPrivate( QStringLiteral( "DROP TABLE %1" ).arg( name ) );
}


void QgsGeoPackageProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
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

QgsAbstractDatabaseProviderConnection::QueryResult QgsGeoPackageProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeGdalSqlPrivate( sql, feedback );
}

void QgsGeoPackageProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  Q_UNUSED( name );
  checkCapability( Capability::Vacuum );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  executeGdalSqlPrivate( QStringLiteral( "VACUUM" ) );
}

void QgsGeoPackageProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

  QString geometryColumnName { options.geometryColumnName };
  if ( geometryColumnName.isEmpty() )
  {
    // Can we guess it?
    try
    {
      const auto tp { table( schema, name ) };
      geometryColumnName = tp.geometryColumn();
    }
    catch ( QgsProviderConnectionException & )
    {
      // pass
    }
  }

  if ( geometryColumnName.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Geometry column name not specified while creating spatial index" ) );
  }

  executeGdalSqlPrivate( QStringLiteral( "SELECT CreateSpatialIndex(%1, %2)" ).arg( QgsSqliteUtils::quotedString( name ),
                         QgsSqliteUtils::quotedString( ( geometryColumnName ) ) ) );
}

bool QgsGeoPackageProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::CreateSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  const QList<QList<QVariant> > res = executeGdalSqlPrivate( QStringLiteral( "SELECT HasSpatialIndex(%1, %2)" ).arg( QgsSqliteUtils::quotedString( name ),
                                      QgsSqliteUtils::quotedString( geometryColumn ) ) ).rows();
  return !res.isEmpty() && !res.at( 0 ).isEmpty() && res.at( 0 ).at( 0 ).toBool();
}

void QgsGeoPackageProviderConnection::deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::DeleteSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  executeGdalSqlPrivate( QStringLiteral( "SELECT DisableSpatialIndex(%1, %2)" ).arg( QgsSqliteUtils::quotedString( name ),
                         QgsSqliteUtils::quotedString( geometryColumn ) ) );
}

QList<QgsGeoPackageProviderConnection::TableProperty> QgsGeoPackageProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{

  // List of GPKG quoted system and dummy tables names to be excluded from the tables listing
  static const QStringList excludedTableNames { { QStringLiteral( "\"ogr_empty_table\"" ) } };

  checkCapability( Capability::Tables );

  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

  QList<QgsGeoPackageProviderConnection::TableProperty> tableInfo;
  QString errCause;
  QList<QVariantList> results;

  try
  {
    const QString sql = QStringLiteral( "SELECT c.table_name, data_type, description, c.srs_id, g.geometry_type_name, g.column_name "
                                        "FROM gpkg_contents c LEFT JOIN gpkg_geometry_columns g ON (c.table_name = g.table_name) "
                                        "WHERE c.table_name NOT IN (%1)" ).arg( excludedTableNames.join( ',' ) );
    results = executeSql( sql );

    for ( const auto &row : std::as_const( results ) )
    {

      if ( row.size() != 6 )
      {
        throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: wrong number of columns returned by query" ).arg( uri() ) );
      }

      QgsGeoPackageProviderConnection::TableProperty property;
      property.setTableName( row.at( 0 ).toString() );
      property.setPrimaryKeyColumns( { QStringLiteral( "fid" ) } );
      property.setGeometryColumnCount( 0 );
      static const QStringList aspatialTypes = { QStringLiteral( "attributes" ), QStringLiteral( "aspatial" ) };
      const QString dataType = row.at( 1 ).toString();

      // Table type
      if ( dataType == QLatin1String( "tiles" ) || dataType == QLatin1String( "2d-gridded-coverage" ) )
      {
        property.setFlag( QgsGeoPackageProviderConnection::Raster );
      }
      else if ( dataType == QLatin1String( "features" ) )
      {
        property.setFlag( QgsGeoPackageProviderConnection::Vector );
        property.setGeometryColumn( row.at( 5 ).toString() );
        property.setGeometryColumnCount( 1 );
      }

      if ( aspatialTypes.contains( dataType ) )
      {
        property.setFlag( QgsGeoPackageProviderConnection::Aspatial );
        property.addGeometryColumnType( QgsWkbTypes::Type::NoGeometry, QgsCoordinateReferenceSystem() );
      }
      else
      {
        bool ok;
        int srid = row.at( 3 ).toInt( &ok );

        if ( !ok )
        {
          throw QgsProviderConnectionException( QObject::tr( "Error fetching srs_id table information: %1" ).arg( row.at( 3 ).toString() ) );
        }

        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromEpsgId( srid );
        property.addGeometryColumnType( QgsWkbTypes::parseType( row.at( 4 ).toString() ),  crs );
      }

      property.setComment( row.at( 4 ).toString() );
      tableInfo.push_back( property );
    }

  }
  catch ( QgsProviderConnectionException &ex )
  {
    errCause = ex.what();
  }

  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( uri(), errCause ) );
  }
  // Filters
  if ( flags )
  {
    tableInfo.erase( std::remove_if( tableInfo.begin(), tableInfo.end(), [ & ]( const QgsAbstractDatabaseProviderConnection::TableProperty & ti )
    {
      return !( ti.flags() & flags );
    } ), tableInfo.end() );
  }
  return tableInfo ;
}

QIcon QgsGeoPackageProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mGeoPackage.svg" ) );
}

void QgsGeoPackageProviderConnection::setDefaultCapabilities()
{
  mCapabilities =
  {
    Capability::Tables,
    Capability::CreateVectorTable,
    Capability::DropVectorTable,
    Capability::RenameVectorTable,
    Capability::Vacuum,
    Capability::Spatial,
    Capability::TableExists,
    Capability::ExecuteSql,
    Capability::CreateSpatialIndex,
    Capability::SpatialIndexExists,
    Capability::DeleteSpatialIndex,
    Capability::DeleteField,
    Capability::AddField,
    Capability::DropRasterTable
  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePart,
    GeometryColumnCapability::Curves
  };
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsGeoPackageProviderConnection::executeGdalSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{

  if ( feedback && feedback->isCanceled() )
  {
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  QString errCause;
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {

    if ( feedback && feedback->isCanceled() )
    {
      return QgsAbstractDatabaseProviderConnection::QueryResult();
    }

    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS.get(), sql.toUtf8().constData(), nullptr, nullptr ) );

    // Read fields
    if ( ogrLayer )
    {

      auto iterator = std::make_shared<QgsGeoPackageProviderResultIterator>( std::move( hDS ), ogrLayer );
      QgsAbstractDatabaseProviderConnection::QueryResult results( iterator );

      gdal::ogr_feature_unique_ptr fet;
      if ( fet.reset( OGR_L_GetNextFeature( ogrLayer ) ), fet )
      {

        QgsFields fields { QgsOgrUtils::readOgrFields( fet.get(), QTextCodec::codecForName( "UTF-8" ) ) };
        iterator->setFields( fields );

        for ( const auto &f : std::as_const( fields ) )
        {
          results.appendColumn( f.name() );
        }
      }

      // Check for errors
      errCause = CPLGetLastErrorMsg( );

      if ( ! errCause.isEmpty() )
      {
        throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql, errCause ) );
      }

      OGR_L_ResetReading( ogrLayer );
      iterator->nextRow();
      return results;
    }
    errCause = CPLGetLastErrorMsg( );
  }
  else
  {
    errCause = QObject::tr( "There was an error opening GPKG %1!" ).arg( uri() );
  }

  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql, errCause ) );
  }

  return QgsAbstractDatabaseProviderConnection::QueryResult();
}

QVariantList QgsGeoPackageProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow = mNextRow;
  mNextRow = nextRowInternal();
  return currentRow;
}

QVariantList QgsGeoPackageProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mHDS && mOgrLayer )
  {
    gdal::ogr_feature_unique_ptr fet;
    if ( fet.reset( OGR_L_GetNextFeature( mOgrLayer ) ), fet )
    {
      if ( ! mFields.isEmpty() )
      {
        QgsFeature f { QgsOgrUtils::readOgrFeature( fet.get(), mFields, QTextCodec::codecForName( "UTF-8" ) ) };
        const QgsAttributes constAttrs  = f.attributes();
        for ( const QVariant &attribute : constAttrs )
        {
          row.push_back( attribute );
        }
      }
      else // Fallback to strings
      {
        for ( int i = 0; i < OGR_F_GetFieldCount( fet.get() ); i++ )
        {
          row.push_back( QVariant( QString::fromUtf8( OGR_F_GetFieldAsString( fet.get(), i ) ) ) );
        }
      }
    }
    else
    {
      // Release the resources
      GDALDatasetReleaseResultSet( mHDS.get(), mOgrLayer );
      mHDS.release();
    }
  }
  return row;
}

bool QgsGeoPackageProviderResultIterator::hasNextRowPrivate() const
{
  return ! mNextRow.isEmpty();
}

void QgsGeoPackageProviderResultIterator::setFields( const QgsFields &fields )
{
  mFields = fields;
}

QList<QgsVectorDataProvider::NativeType> QgsGeoPackageProviderConnection::nativeTypes() const
{
  QgsVectorLayer::LayerOptions options { false, true };
  options.skipCrsValidation = true;
  const QgsVectorLayer vl { uri(), QStringLiteral( "temp_layer" ), QStringLiteral( "ogr" ), options };
  if ( ! vl.isValid() || ! vl.dataProvider() )
  {
    const QString errorCause = vl.dataProvider() && vl.dataProvider()->hasErrors() ?
                               vl.dataProvider()->errors().join( '\n' ) :
                               QObject::tr( "unknown error" );
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for %1: %2" ).arg( uri(), errorCause ) );
  }
  return vl.dataProvider()->nativeTypes();
}

QgsGeoPackageProviderResultIterator::~QgsGeoPackageProviderResultIterator()
{
  if ( mHDS )
  {
    GDALDatasetReleaseResultSet( mHDS.get(), mOgrLayer );
  }
}
