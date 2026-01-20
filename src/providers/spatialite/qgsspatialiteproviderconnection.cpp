/***************************************************************************
  QgsSpatialiteProviderConnection.cpp - QgsSpatialiteProviderConnection

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
#include "qgsspatialiteproviderconnection.h"

#include <chrono>

#include "qgsapplication.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgsogrprovider.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"
#include "qgsspatialiteconnection.h"
#include "qgsspatialiteprovider.h"
#include "qgsvectorlayer.h"

#include <QRegularExpression>
#include <QTextCodec>

QgsSpatiaLiteProviderConnection::QgsSpatiaLiteProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = u"spatialite"_s;
  setDefaultCapabilities();
  // TODO: QGIS 5: move into QgsSettings::Section::Providers group
  QgsSettings settings;
  settings.beginGroup( u"SpatiaLite"_s );
  settings.beginGroup( u"connections"_s );
  settings.beginGroup( name );
  QgsDataSourceUri dsUri;
  dsUri.setDatabase( settings.value( u"sqlitepath"_s ).toString() );
  setUri( dsUri.uri() );
}

QgsSpatiaLiteProviderConnection::QgsSpatiaLiteProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = u"spatialite"_s;
  QgsDataSourceUri dsUri { uri };
  QgsDataSourceUri dsUriCleaned;
  dsUriCleaned.setDatabase( dsUri.database() );
  setUri( dsUriCleaned.uri() );
  setDefaultCapabilities();
}

void QgsSpatiaLiteProviderConnection::store( const QString &name ) const
{
  // TODO: QGIS 5: move into QgsSettings::Section::Providers group
  QgsSettings settings;
  settings.beginGroup( u"SpatiaLite"_s );
  settings.beginGroup( u"connections"_s );
  settings.beginGroup( name );
  settings.setValue( u"sqlitepath"_s, pathFromUri() );
}

void QgsSpatiaLiteProviderConnection::remove( const QString &name ) const
{
  // TODO: QGIS 5: move into QgsSettings::Section::Providers group
  QgsSettings settings;
  settings.beginGroup( u"SpatiaLite"_s );
  settings.beginGroup( u"connections"_s );
  settings.remove( name );
}

QString QgsSpatiaLiteProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  Q_UNUSED( schema ); // spatialite does not support schema
  return uri() + u" table=%1"_s.arg( QgsSqliteUtils::quotedIdentifier( name ) );
}

void QgsSpatiaLiteProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  QMap<QString, QVariant> opts { *options };
  opts[u"layerName"_s] = QVariant( name );
  opts[u"update"_s] = true;
  QMap<int, int> map;
  QString errCause;
  QString createdLayerUri;
  Qgis::VectorExportResult res = QgsSpatiaLiteProvider::createEmptyLayer(
    uri() + u" table=%1 (geom)"_s.arg( QgsSqliteUtils::quotedIdentifier( name ) ),
    fields,
    wkbType,
    srs,
    overwrite,
    &map,
    createdLayerUri,
    &errCause,
    &opts
  );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}


QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsSpatiaLiteProviderConnection::sqlOptions( const QString &layerSource )
{
  SqlVectorLayerOptions options;
  const QgsDataSourceUri tUri( layerSource );
  options.primaryKeyColumns = tUri.keyColumn().split( ',' );
  options.disableSelectAtId = tUri.selectAtIdDisabled();
  options.geometryColumn = tUri.geometryColumn();
  options.filter = tUri.sql();
  const QString trimmedTable { tUri.table().trimmed() };
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : u"SELECT * FROM %1"_s.arg( tUri.quotedTablename() );
  return options;
}

QgsVectorLayer *QgsSpatiaLiteProviderConnection::createSqlVectorLayer( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options ) const
{
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  QgsDataSourceUri tUri( uri() );

  tUri.setSql( options.filter );
  tUri.setTable( '(' + sanitizeSqlForQueryLayer( options.sql ) + ')' );

  if ( !options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  return new QgsVectorLayer { tUri.uri( false ), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey() };
}

QString QgsSpatiaLiteProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &options, QVariantMap &providerOptions ) const
{
  if ( !options.schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QgsDataSourceUri destUri( uri() );
  destUri.setTable( options.layerName );
  destUri.setGeometryColumn( options.wkbType != Qgis::WkbType::NoGeometry ? ( options.geometryColumn.isEmpty() ? u"geom"_s : options.geometryColumn ) : QString() );

  if ( !options.primaryKeyColumns.isEmpty() )
  {
    if ( options.primaryKeyColumns.length() > 1 )
    {
      QgsMessageLog::logMessage( u"Multiple primary keys are not supported by Spatialite, ignoring"_s, QString(), Qgis::MessageLevel::Info );
    }
    destUri.setKeyColumn( options.primaryKeyColumns.at( 0 ) );
  }

  providerOptions.clear();

  return destUri.uri( false );
}

void QgsSpatiaLiteProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  QString errCause;

  QgsSqliteHandle *hndl = QgsSqliteHandle::openDb( pathFromUri() );
  if ( !hndl )
  {
    errCause = QObject::tr( "Connection to database failed" );
  }
  else
  {
    sqlite3 *sqlite_handle = hndl->handle();
    int ret;
    if ( !gaiaDropTable( sqlite_handle, name.toUtf8().constData() ) )
    {
      // unexpected error
      errCause = QObject::tr( "Unable to delete table %1\n" ).arg( name );
      QgsSqliteHandle::closeDb( hndl );
    }
    else
    {
      // TODO: remove spatial indexes?
      // run VACUUM to free unused space and compact the database
      ret = sqlite3_exec( sqlite_handle, "VACUUM", nullptr, nullptr, nullptr );
      if ( ret != SQLITE_OK )
      {
        QgsDebugError( u"Failed to run VACUUM after deleting table on database %1"_s
                         .arg( pathFromUri() ) );
      }

      QgsSqliteHandle::closeDb( hndl );
    }
  }
  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name, errCause ) );
  }
}


void QgsSpatiaLiteProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  // TODO: maybe an index?
  QString sql( u"ALTER TABLE %1 RENAME TO %2"_s
                 .arg( QgsSqliteUtils::quotedIdentifier( name ), QgsSqliteUtils::quotedIdentifier( newName ) ) );
  executeSqlDirect( sql );
  sql = u"UPDATE geometry_columns SET f_table_name = lower(%2) WHERE lower(f_table_name) = lower(%1)"_s
          .arg( QgsSqliteUtils::quotedString( name ), QgsSqliteUtils::quotedString( newName ) );
  executeSqlDirect( sql );
  sql = u"UPDATE layer_styles SET f_table_name = lower(%2) WHERE f_table_name = lower(%1)"_s
          .arg( QgsSqliteUtils::quotedString( name ), QgsSqliteUtils::quotedString( newName ) );
  try
  {
    executeSqlDirect( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QgsDebugMsgLevel( u"Warning: error while updating the styles, perhaps there are no styles stored in this GPKG: %1"_s.arg( ex.what() ), 4 );
  }
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsSpatiaLiteProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql, feedback );
}

void QgsSpatiaLiteProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  Q_UNUSED( name )
  checkCapability( Capability::Vacuum );
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  executeSqlDirect( u"VACUUM"_s );
}

void QgsSpatiaLiteProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );

  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
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

  executeSqlPrivate( u"SELECT CreateSpatialIndex(%1, %2)"_s.arg( QgsSqliteUtils::quotedString( name ), QgsSqliteUtils::quotedString( ( geometryColumnName ) ) ) );
}

bool QgsSpatiaLiteProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::CreateSpatialIndex );
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  const QList<QVariantList> res = executeSqlPrivate( u"SELECT spatial_index_enabled FROM geometry_columns WHERE lower(f_table_name) = lower(%1) AND lower(f_geometry_column) = lower(%2)"_s
                                                       .arg( QgsSqliteUtils::quotedString( name ), QgsSqliteUtils::quotedString( geometryColumn ) ) )
                                    .rows();
  return !res.isEmpty() && !res.at( 0 ).isEmpty() && res.at( 0 ).at( 0 ).toInt() == 1;
}

QList<QgsSpatiaLiteProviderConnection::TableProperty> QgsSpatiaLiteProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by Spatialite, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  QList<QgsSpatiaLiteProviderConnection::TableProperty> tableInfo;
  QString errCause;
  try
  {
    QgsSpatiaLiteConnection connection( pathFromUri() );
    QgsSpatiaLiteConnection::Error err = connection.fetchTables( true );
    if ( err != QgsSpatiaLiteConnection::NoError )
    {
      QString msg;
      switch ( err )
      {
        case QgsSpatiaLiteConnection::NotExists:
          msg = QObject::tr( "Database does not exist" );
          break;
        case QgsSpatiaLiteConnection::FailedToOpen:
          msg = QObject::tr( "Failed to open database" );
          break;
        case QgsSpatiaLiteConnection::FailedToCheckMetadata:
          msg = QObject::tr( "Failed to check metadata" );
          break;
        case QgsSpatiaLiteConnection::FailedToGetTables:
          msg = QObject::tr( "Failed to get list of tables" );
          break;
        default:
          msg = QObject::tr( "Unknown error" );
          break;
      }
      QString msgDetails = connection.errorMessage();
      if ( !msgDetails.isEmpty() )
      {
        msg = u"%1 (%2)"_s.arg( msg, msgDetails );
      }
      throw QgsProviderConnectionException( QObject::tr( "Error fetching table information for connection: %1" ).arg( pathFromUri() ) );
    }
    else
    {
      const QString connectionInfo = u"dbname='%1'"_s.arg( QString( connection.path() ).replace( '\'', "\\'"_L1 ) );
      QgsDataSourceUri dsUri( connectionInfo );

      // Need to store it here because provider (and underlying gaia library) returns views as spatial table if they have geometries
      QStringList viewNames;
      const QList<QList<QVariant>> viewRows { executeSqlPrivate( u"SELECT name FROM sqlite_master WHERE type = 'view'"_s ).rows() };
      for ( const QList<QVariant> &tn : std::as_const( viewRows ) )
      {
        viewNames.push_back( tn.first().toString() );
      }

      // Another weirdness: table names are converted to lowercase when out of spatialite gaia functions, let's get them back to their real case here,
      // may need LAUNDER on open, but let's try to make it consistent with how GPKG works.
      QgsStringMap tableNotLowercaseNames;
      const QList<QList<QVariant>> lowerTables { executeSqlPrivate( u"SELECT name FROM sqlite_master WHERE LOWER(name) != name"_s ).rows() };
      for ( const QList<QVariant> &tn : std::as_const( lowerTables ) )
      {
        const QString tName { tn.first().toString() };
        tableNotLowercaseNames.insert( tName.toLower(), tName );
      }

      const QList<QgsSpatiaLiteConnection::TableEntry> constTables = connection.tables();
      for ( const QgsSpatiaLiteConnection::TableEntry &entry : constTables )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        QString tableName { tableNotLowercaseNames.value( entry.tableName, entry.tableName ) };
        dsUri.setDataSource( QString(), tableName, entry.column, QString(), QString() );
        QgsSpatiaLiteProviderConnection::TableProperty property;
        property.setTableName( tableName );
        // Create a layer and get information from it
        // Use OGR because it's way faster
        auto vl = std::make_unique<QgsVectorLayer>( dsUri.database() + "|layername=" + dsUri.table(), QString(), "ogr"_L1, QgsVectorLayer::LayerOptions( false, true ) );
        if ( vl->isValid() )
        {
          if ( vl->isSpatial() )
          {
            property.setGeometryColumnCount( 1 );
            property.setGeometryColumn( entry.column );
            property.setFlag( QgsSpatiaLiteProviderConnection::TableFlag::Vector );
            property.setGeometryColumnTypes( { { vl->wkbType(), vl->crs() } } );
          }
          else
          {
            property.setGeometryColumnCount( 0 );
            property.setGeometryColumnTypes( { { Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() } } );
            property.setFlag( QgsSpatiaLiteProviderConnection::TableFlag::Aspatial );
          }

          if ( viewNames.contains( tableName ) )
          {
            property.setFlag( QgsSpatiaLiteProviderConnection::TableFlag::View );
          }

          const QgsAttributeList constPkIdxs { vl->dataProvider()->pkAttributeIndexes() };
          QStringList pkNames;
          for ( const int &pkIdx : std::as_const( constPkIdxs ) )
          {
            // Better safe than sorry
            if ( pkIdx < vl->fields().count() )
              pkNames.append( vl->fields().at( pkIdx ).name() );
          }

          if ( !pkNames.isEmpty() )
          {
            property.setPrimaryKeyColumns( pkNames );
          }

          tableInfo.push_back( property );
        }
        else
        {
          QgsDebugMsgLevel( u"Layer is not valid: %1"_s.arg( dsUri.uri() ), 2 );
        }
      }
    }
  }
  catch ( QgsProviderConnectionException &ex )
  {
    errCause = ex.what();
  }

  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( pathFromUri(), errCause ) );
  }
  // Filters
  if ( flags )
  {
    tableInfo.erase( std::remove_if( tableInfo.begin(), tableInfo.end(), [&]( const QgsAbstractDatabaseProviderConnection::TableProperty &ti ) {
                       return !( ti.flags() & flags );
                     } ),
                     tableInfo.end() );
  }
  return tableInfo;
}

QIcon QgsSpatiaLiteProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconSpatialite.svg"_s );
}

void QgsSpatiaLiteProviderConnection::setDefaultCapabilities()
{
  mCapabilities = {
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
    Capability::DeleteField,
    Capability::AddField,
    Capability::SqlLayers,
  };
  mGeometryColumnCapabilities = {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePoint,
    GeometryColumnCapability::SingleLineString,
    GeometryColumnCapability::SinglePolygon,
  };
  mSqlLayerDefinitionCapabilities = {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn
  };
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsSpatiaLiteProviderConnection::executeSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{
  QgsDatabaseQueryLogWrapper logWrapper( sql, uri(), providerKey(), u"QgsSpatiaLiteProviderConnection"_s, QGS_QUERY_LOG_ORIGIN );

  if ( feedback && feedback->isCanceled() )
  {
    logWrapper.setCanceled();
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  QString errCause;
  gdal::dataset_unique_ptr hDS( GDALOpenEx( pathFromUri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    if ( feedback && feedback->isCanceled() )
    {
      logWrapper.setCanceled();
      return QgsAbstractDatabaseProviderConnection::QueryResult();
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS.get(), sql.toUtf8().constData(), nullptr, nullptr ) );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // Read fields
    if ( ogrLayer )
    {
      auto iterator = std::make_shared<QgsSpatialiteProviderResultIterator>( std::move( hDS ), ogrLayer );
      QgsAbstractDatabaseProviderConnection::QueryResult results( iterator );
      results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );

      gdal::ogr_feature_unique_ptr fet;
      if ( fet.reset( OGR_L_GetNextFeature( ogrLayer ) ), fet )
      {
        const QgsFields fields { QgsOgrUtils::readOgrFields( fet.get(), QTextCodec::codecForName( "UTF-8" ) ) };

        // geom column name
        QString geomColumnName;

        OGRFeatureDefnH featureDef = OGR_F_GetDefnRef( fet.get() );

        if ( featureDef )
        {
          if ( OGR_F_GetGeomFieldCount( fet.get() ) > 0 )
          {
            OGRGeomFieldDefnH geomFldDef { OGR_F_GetGeomFieldDefnRef( fet.get(), 0 ) };
            if ( geomFldDef )
            {
              geomColumnName = OGR_GFld_GetNameRef( geomFldDef );
            }
          }
        }

        // Add other fields
        for ( const auto &f : std::as_const( fields ) )
        {
          results.appendColumn( f.name() );
        }

        // Append geom
        if ( !geomColumnName.isEmpty() )
        {
          results.appendColumn( geomColumnName );
          iterator->setGeometryColumnName( geomColumnName );
        }

        iterator->setFields( fields );
      }

      // Check for errors
      if ( CE_Failure == CPLGetLastErrorType() || CE_Fatal == CPLGetLastErrorType() )
      {
        errCause = CPLGetLastErrorMsg();
      }

      if ( !errCause.isEmpty() )
      {
        logWrapper.setError( errCause );
        throw QgsProviderConnectionException( QObject::tr( "Error executing SQL statement %1: %2" ).arg( sql, errCause ) );
      }

      OGR_L_ResetReading( ogrLayer );
      iterator->nextRow();
      return results;
    }

    // Check for errors
    if ( CE_Failure == CPLGetLastErrorType() || CE_Fatal == CPLGetLastErrorType() )
    {
      errCause = CPLGetLastErrorMsg();
    }
  }
  else
  {
    errCause = QObject::tr( "There was an error opening GPKG %1!" ).arg( uri() );
  }

  if ( !errCause.isEmpty() )
  {
    logWrapper.setError( errCause );
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql, errCause ) );
  }

  return QgsAbstractDatabaseProviderConnection::QueryResult();
}


void QgsSpatialiteProviderResultIterator::setFields( const QgsFields &fields )
{
  mFields = fields;
}


QgsSpatialiteProviderResultIterator::QgsSpatialiteProviderResultIterator( gdal::dataset_unique_ptr hDS, OGRLayerH ogrLayer )
  : mHDS( std::move( hDS ) )
  , mOgrLayer( ogrLayer )
{
  if ( mOgrLayer )
  {
    // Do not scan the layer!
    mRowCount = OGR_L_GetFeatureCount( mOgrLayer, false );
  }
}

QgsSpatialiteProviderResultIterator::~QgsSpatialiteProviderResultIterator()
{
  if ( mHDS )
  {
    GDALDatasetReleaseResultSet( mHDS.get(), mOgrLayer );
  }
}

QVariantList QgsSpatialiteProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow = mNextRow;
  mNextRow = nextRowInternal();
  return currentRow;
}

QVariantList QgsSpatialiteProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mHDS && mOgrLayer )
  {
    gdal::ogr_feature_unique_ptr fet;
    if ( fet.reset( OGR_L_GetNextFeature( mOgrLayer ) ), fet )
    {
      if ( !mFields.isEmpty() )
      {
        QgsFeature f { QgsOgrUtils::readOgrFeature( fet.get(), mFields, QTextCodec::codecForName( "UTF-8" ) ) };
        const QgsAttributes constAttrs = f.attributes();
        for ( const QVariant &attribute : constAttrs )
        {
          row.push_back( attribute );
        }

        // Geom goes last
        if ( !mGeometryColumnName.isEmpty() )
        {
          row.push_back( f.geometry().asWkt() );
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

long long QgsSpatialiteProviderResultIterator::rowCountPrivate() const
{
  return mRowCount;
}


void QgsSpatialiteProviderResultIterator::setGeometryColumnName( const QString &geometryColumnName )
{
  mGeometryColumnName = geometryColumnName;
}


bool QgsSpatialiteProviderResultIterator::hasNextRowPrivate() const
{
  return !mNextRow.isEmpty();
}

bool QgsSpatiaLiteProviderConnection::executeSqlDirect( const QString &sql ) const
{
  sqlite3_database_unique_ptr database;
  int result = database.open( pathFromUri() );
  if ( result != SQLITE_OK )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql, database.errorMessage() ) );
  }

  QString errorMessage;
  result = database.exec( sql, errorMessage );
  if ( result != SQLITE_OK )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql, errorMessage ) );
  }
  return true;
}

QString QgsSpatiaLiteProviderConnection::pathFromUri() const
{
  const QgsDataSourceUri dsUri( uri() );
  return dsUri.database();
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsSpatiaLiteProviderConnection::sqlDictionary()
{
  /*
   * List from: http://www.gaia-gis.it/gaia-sins/spatialite-sql-4.2.0.html
   *
   import requests, re
   result = requests.get('http://www.gaia-gis.it/gaia-sins/spatialite-sql-4.2.0.html')
   functions = {}
   for r in result.content.decode('utf8').split('\n'):
       if 'name="' in r:
           section = (re.findall(r'.*name="[^"]+">([^<]+)<.*', r)[0])
           functions[section] = []
           m = re.match(r'\t\t\t\t<td>(.*?) :.*', r)
           if m:
               functions[section].append(m.group(1))
   for title, f in functions.items():
       print(f"// {title}")
       for _f in f:
           print( f"QStringLiteral( \"{_f}\" ),")
   */

  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
    { { Qgis::SqlKeywordCategory::Math, { // SQL math functions
                                          u"Abs( x [Double precision] )"_s, u"Acos( x [Double precision] )"_s, u"Asin( x [Double precision] )"_s, u"Atan( x [Double precision] )"_s, u"Ceil( x [Double precision] )"_s, u"Cos( x [Double precision] )"_s, u"Cot( x [Double precision] )"_s, u"Degrees( x [Double precision] )"_s, u"Exp( x [Double precision] )"_s, u"Floor( x [Double precision] )"_s, u"Ln( x [Double precision] )"_s, u"Log( b [Double precision] , x [Double precision] )"_s, u"Log2( x [Double precision] )"_s, u"Log10( x [Double precision] )"_s, u"PI( void )"_s, u"Pow( x [Double precision] , y [Double precision] )"_s, u"Radians( x [Double precision] )"_s, u"Sign( x [Double precision] )"_s, u"Sin( x [Double precision] )"_s, u"Sqrt( x [Double precision] )"_s, u"Stddev_pop( x [Double precision] )"_s, u"Stddev_samp( x [Double precision] )"_s, u"Tan( x [Double precision] )"_s, u"Var_pop( x [Double precision] )"_s, u"Var_samp( x [Double precision] )"_s
                                        } },
      { Qgis::SqlKeywordCategory::Function, {

                                              // Specific
                                              u"last_insert_rowid"_s,

                                              // SQL Version Info [and build options testing] functions
                                              u"spatialite_version( void )"_s,
                                              u"spatialite_target_cpu( void )"_s,
                                              u"proj4_version( void )"_s,
                                              u"geos_version( void )"_s,
                                              u"lwgeom_version( void )"_s,
                                              u"libxml2_version( void )"_s,
                                              u"HasIconv( void )"_s,
                                              u"HasMathSQL( void )"_s,
                                              u"HasGeoCallbacks( void )"_s,
                                              u"HasProj( void )"_s,
                                              u"HasGeos( void )"_s,
                                              u"HasGeosAdvanced( void )"_s,
                                              u"HasGeosTrunk( void )"_s,
                                              u"HasLwGeom( void )"_s,
                                              u"HasLibXML2( void )"_s,
                                              u"HasEpsg( void )"_s,
                                              u"HasFreeXL( void )"_s,
                                              u"HasGeoPackage( void )"_s,

                                              // Generic SQL functions
                                              u"CastToInteger( value [Generic] )"_s,
                                              u"CastToDouble( value [Generic] )"_s,
                                              u"CastToText( value [Generic] )"_s,
                                              u"CastToBlob( value [Generic] )"_s,
                                              u"ForceAsNull( val1 [Generic] , val2 [Generic])"_s,
                                              u"CreateUUID( void )"_s,
                                              u"MD5Checksum( BLOB | TEXT )"_s,
                                              u"MD5TotalChecksum( BLOB | TEXT )"_s,

                                              // SQL utility functions for BLOB objects
                                              u"IsZipBlob( content [BLOB] )"_s,
                                              u"IsPdfBlob( content [BLOB] )"_s,
                                              u"IsGifBlob( image [BLOB] )"_s,
                                              u"IsPngBlob( image [BLOB] )"_s,
                                              u"IsTiffBlob( image [BLOB] )"_s,
                                              u"IsJpegBlob( image [BLOB] )"_s,
                                              u"IsExifBlob( image [BLOB] )"_s,
                                              u"IsExifGpsBlob( image [BLOB] )"_s,
                                              u"IsWebpBlob( image [BLOB] )"_s,
                                              u"GetMimeType( payload [BLOB] )"_s,
                                              u"BlobFromFile( filepath [String] )"_s,
                                              u"BlobToFile( payload [BLOB] , filepath [String] )"_s,
                                              u"CountUnsafeTriggers( )"_s,

                                              // SQL functions supporting XmlBLOB
                                              u"XB_Create(  xmlPayload [BLOB] )"_s,
                                              u"XB_GetPayload( xmlObject [XmlBLOB] [ , indent [Integer] ] )"_s,
                                              u"XB_GetDocument( xmlObject [XmlBLOB] [ , indent [Integer] ] )"_s,
                                              u"XB_SchemaValidate(  xmlObject [XmlBLOB] , schemaURI [Text] [ , compressed [Boolean] ] )"_s,
                                              u"XB_Compress( xmlObject [XmlBLOB] )"_s,
                                              u"XB_Uncompress( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsValid( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsCompressed( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsSchemaValidated( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsIsoMetadata( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsSldSeVectorStyle( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsSldSeRasterStyle( xmlObject [XmlBLOB] )"_s,
                                              u"XB_IsSvg( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetDocumentSize( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetEncoding( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetSchemaURI( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetInternalSchemaURI( xmlPayload [BLOB] )"_s,
                                              u"XB_GetFileId( xmlObject [XmlBLOB] )"_s,
                                              u"XB_SetFileId( xmlObject [XmlBLOB] , fileId [String] )"_s,
                                              u"XB_AddFileId( xmlObject [XmlBLOB] , fileId [String] , IdNameSpacePrefix [String] , IdNameSpaceURI [String] , CsNameSpacePrefix [String] , CsNameSpaceURI [String] )"_s,
                                              u"XB_GetParentId( xmlObject [XmlBLOB] )"_s,
                                              u"XB_SetParentId( xmlObject [XmlBLOB] , parentId [String] )"_s,
                                              u"XB_AddParentId( xmlObject [XmlBLOB] , parentId [String] , IdNameSpacePrefix [String] , IdNameSpaceURI [String] , CsNameSpacePrefix [String] , CsNameSpaceURI [String] )"_s,
                                              u"XB_GetTitle( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetAbstract( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetGeometry( xmlObject [XmlBLOB] )"_s,
                                              u"XB_GetLastParseError( [void] )"_s,
                                              u"XB_GetLastValidateError( [void] )"_s,
                                              u"XB_IsValidXPathExpression( expr [Text] )"_s,
                                              u"XB_GetLastXPathError( [void] )"_s,
                                              u"XB_CacheFlush( [void] )"_s,
                                              u"XB_LoadXML( filepath-or-URL [String] )"_s,
                                              u"XB_StoreXML( XmlObject [XmlBLOB] , filepath [String] )"_s,

                                            } },
      { Qgis::SqlKeywordCategory::Geospatial, {
                                                // SQL functions reporting GEOS / LWGEOM errors and warnings
                                                u"GEOS_GetLastWarningMsg( [void] )"_s,
                                                u"GEOS_GetLastErrorMsg( [void] )"_s,
                                                u"GEOS_GetLastAuxErrorMsg( [void] )"_s,
                                                u"GEOS_GetCriticalPointFromMsg( [void] )"_s,
                                                u"LWGEOM_GetLastWarningMsg( [void] )"_s,
                                                u"LWGEOM_GetLastErrorMsg( [void] )"_s,

                                                // SQL length/distance unit-conversion functions
                                                u"CvtToKm( x [Double precision] )"_s,
                                                u"CvtToDm( x [Double precision] )"_s,
                                                u"CvtToCm( x [Double precision] )"_s,
                                                u"CvtToMm( x [Double precision] )"_s,
                                                u"CvtToKmi( x [Double precision] )"_s,
                                                u"CvtToIn( x [Double precision] )"_s,
                                                u"CvtToFt( x [Double precision] )"_s,
                                                u"CvtToYd( x [Double precision] )"_s,
                                                u"CvtToMi( x [Double precision] )"_s,
                                                u"CvtToFath( x [Double precision] )"_s,
                                                u"CvtToCh( x [Double precision] )"_s,
                                                u"CvtToLink( x [Double precision] )"_s,
                                                u"CvtToUsIn( x [Double precision] )"_s,
                                                u"CvtToUsFt( x [Double precision] )"_s,
                                                u"CvtToUsYd( x [Double precision] )"_s,
                                                u"CvtToUsMi( x [Double precision] )"_s,
                                                u"CvtToUsCh( x [Double precision] )"_s,
                                                u"CvtToIndFt( x [Double precision] )"_s,
                                                u"CvtToIndYd( x [Double precision] )"_s,
                                                u"CvtToIndCh( x [Double precision] )"_s,

                                                // SQL conversion functions from DD/DMS notations (longitude/latitude)
                                                u"LongLatToDMS( longitude [Double precision] , latitude [Double precision] )"_s,
                                                u"LongitudeFromDMS( dms_expression [Sting] )"_s,

                                                // SQL utility functions [
                                                u"GeomFromExifGpsBlob( image [BLOB] )"_s,
                                                u"ST_Point( x [Double precision] , y [Double precision]  )"_s,
                                                u"MakeLine( pt1 [PointGeometry] , pt2 [PointGeometry] )"_s,
                                                u"MakeLine( geom [PointGeometry] )"_s,
                                                u"MakeLine( geom [MultiPointGeometry] , direction [Boolean] )"_s,
                                                u"SquareGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )"_s,
                                                u"TriangularGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )"_s,
                                                u"HexagonalGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )"_s,
                                                u"Extent( geom [Geometry] )"_s,
                                                u"ToGARS( geom [Geometry] )"_s,
                                                u"GARSMbr( code [String] )"_s,
                                                u"MbrMinX( geom [Geometry])"_s,
                                                u"MbrMinY( geom [Geometry])"_s,
                                                u"MbrMaxX( geom [Geometry])"_s,
                                                u"MbrMaxY( geom [Geometry])"_s,
                                                u"ST_MinZ( geom [Geometry])"_s,
                                                u"ST_MaxZ( geom [Geometry])"_s,
                                                u"ST_MinM( geom [Geometry])"_s,
                                                u"ST_MaxM( geom [Geometry])"_s,

                                                // SQL functions for constructing a geometric object given its Well-known Text Representation
                                                u"GeomFromText( wkt [String] [ , SRID [Integer]] )"_s,
                                                u"ST_WKTToSQL( wkt [String] )"_s,
                                                u"PointFromText( wktPoint [String] [ , SRID [Integer]] )"_s,
                                                u"LineFromText( wktLineString [String] [ , SRID [Integer]] )"_s,
                                                u"PolyFromText( wktPolygon [String] [ , SRID [Integer]] )"_s,
                                                u"MPointFromText( wktMultiPoint [String] [ , SRID [Integer]] )"_s,
                                                u"MLineFromText( wktMultiLineString [String] [ , SRID [Integer]] )"_s,
                                                u"MPolyFromText( wktMultiPolygon [String] [ , SRID [Integer]] )"_s,
                                                u"GeomCollFromText( wktGeometryCollection [String] [ , SRID [Integer]] )"_s,
                                                u"BdPolyFromText( wktMultilinestring [String] [ , SRID [Integer]] )"_s,
                                                u"BdMPolyFromText( wktMultilinestring [String] [ , SRID [Integer]] )"_s,

                                                // SQL functions for constructing a geometric object given its Well-known Binary Representation
                                                u"GeomFromWKB( wkbGeometry [Binary] [ , SRID [Integer]] )"_s,
                                                u"ST_WKBToSQL( wkbGeometry [Binary] )"_s,
                                                u"PointFromWKB( wkbPoint [Binary] [ , SRID [Integer]] )"_s,
                                                u"LineFromWKB( wkbLineString [Binary] [ , SRID [Integer]] )"_s,
                                                u"PolyFromWKB( wkbPolygon [Binary] [ , SRID [Integer]] )"_s,
                                                u"MPointFromWKB( wkbMultiPoint [Binary] [ , SRID [Integer]] )"_s,
                                                u"MLineFromWKB( wkbMultiLineString [Binary] [ , SRID [Integer]] )"_s,
                                                u"MPolyFromWKB( wkbMultiPolygon [Binary] [ , SRID [Integer]] )"_s,
                                                u"GeomCollFromWKB( wkbGeometryCollection [Binary] [ , SRID [Integer]] )"_s,
                                                u"BdPolyFromWKB( wkbMultilinestring [Binary] [ , SRID [Integer]] )"_s,
                                                u"BdMPolyFromWKB( wkbMultilinestring [Binary] [ , SRID [Integer]] )"_s,

                                                // SQL functions for obtaining the Well-known Text / Well-known Binary Representation of a geometric object
                                                u"AsText( geom [Geometry] )"_s,
                                                u"AsWKT( geom [Geometry] [ , precision [Integer] ] )"_s,
                                                u"AsBinary( geom [Geometry] )"_s,

                                                // SQL functions supporting exotic geometric formats
                                                u"AsSVG( geom [Geometry] [ , relative [Integer] [ , precision [Integer] ] ] )"_s,
                                                u"AsKml( geom [Geometry] [ , precision [Integer] ] )"_s,
                                                u"GeomFromKml( KmlGeometry [String] )"_s,
                                                u"AsGml( geom [Geometry] [ , precision [Integer] ] )"_s,
                                                u"GeomFromGML( gmlGeometry [String] )"_s,
                                                u"AsGeoJSON( geom [Geometry] [ , precision [Integer] [ , options [Integer] ] ] )"_s,
                                                u"GeomFromGeoJSON( geoJSONGeometry [String] )"_s,
                                                u"AsEWKB( geom [Geometry] )"_s,
                                                u"GeomFromEWKB( ewkbGeometry [String] )"_s,
                                                u"AsEWKT( geom [Geometry] )"_s,
                                                u"GeomFromEWKT( ewktGeometry [String] )"_s,
                                                u"AsFGF( geom [Geometry] )"_s,
                                                u"GeomFromFGF( fgfGeometry [Binary] [ , SRID [Integer]] )"_s,

                                                // SQL functions on type Geometry
                                                u"Dimension( geom [Geometry] )"_s,
                                                u"CoordDimension( geom [Geometry] )"_s,
                                                u"ST_NDims( geom [Geometry] )"_s,
                                                u"ST_Is3D( geom [Geometry] )"_s,
                                                u"ST_IsMeasured( geom [Geometry] )"_s,
                                                u"GeometryType( geom [Geometry] )"_s,
                                                u"SRID( geom [Geometry] )"_s,
                                                u"SetSRID( geom [Geometry] , SRID [Integer] )"_s,
                                                u"IsEmpty( geom [Geometry] )"_s,
                                                u"IsSimple( geom [Geometry] )"_s,
                                                u"IsValid( geom [Geometry] )"_s,
                                                u"IsValidReason( geom [Geometry] )"_s,
                                                u"IsValidDetail( geom [Geometry] )"_s,
                                                u"Boundary( geom [Geometry] )"_s,
                                                u"Envelope( geom [Geometry] )"_s,
                                                u"ST_Expand( geom [Geometry] , amount [Double precision] )"_s,
                                                u"ST_NPoints( geom [Geometry] )"_s,
                                                u"ST_NRings( geom [Geometry] )"_s,
                                                u"ST_Reverse( geom [Geometry] )"_s,
                                                u"ST_ForceLHR( geom [Geometry] )"_s,

                                                // SQL functions attempting to repair malformed Geometries
                                                u"SanitizeGeometry( geom [Geometry] )"_s,

                                                // SQL Geometry-compression functions
                                                u"CompressGeometry( geom [Geometry] )"_s,
                                                u"UncompressGeometry( geom [Geometry] )"_s,

                                                // SQL Geometry-type casting functions
                                                u"CastToPoint( geom [Geometry] )"_s,
                                                u"CastToLinestring( geom [Geometry] )"_s,
                                                u"CastToPolygon( geom [Geometry] )"_s,
                                                u"CastToMultiPoint( geom [Geometry] )"_s,
                                                u"CastToMultiLinestring( geom [Geometry] )"_s,
                                                u"CastToMultiPolygon( geom [Geometry] )"_s,
                                                u"CastToGeometryCollection( geom [Geometry] )"_s,
                                                u"CastToMulti( geom [Geometry] )"_s,
                                                u"CastToSingle( geom [Geometry] )"_s,

                                                // SQL Space-dimensions casting functions
                                                u"CastToXY( geom [Geometry] )"_s,
                                                u"CastToXYZ( geom [Geometry] )"_s,
                                                u"CastToXYM( geom [Geometry] )"_s,
                                                u"CastToXYZM( geom [Geometry] )"_s,

                                                // SQL functions on type Point
                                                u"X( pt [Point] )"_s,
                                                u"Y( pt [Point] )"_s,
                                                u"Z( pt [Point] )"_s,
                                                u"M( pt [Point] )"_s,

                                                // SQL functions on type Curve [Linestring or Ring]
                                                u"StartPoint( c [Curve] )"_s,
                                                u"EndPoint( c [Curve] )"_s,
                                                u"GLength( c [Curve] )"_s,
                                                u"Perimeter( s [Surface] )"_s,
                                                u"GeodesicLength( c [Curve] )"_s,
                                                u"GreatCircleLength( c [Curve] )"_s,
                                                u"IsClosed( c [Curve] )"_s,
                                                u"IsRing( c [Curve] )"_s,
                                                u"PointOnSurface( s [Surface/Curve] )"_s,
                                                u"Simplify( c [Curve] , tolerance [Double precision] )"_s,
                                                u"SimplifyPreserveTopology( c [Curve] , tolerance [Double precision] )"_s,

                                                // SQL functions on type LineString
                                                u"NumPoints( line [LineString] )"_s,
                                                u"PointN( line [LineString] , n [Integer] )"_s,
                                                u"AddPoint( line [LineString] , point [Point] [ , position [Integer] ] )"_s,
                                                u"SetPoint( line [LineString] , position [Integer] , point [Point] )"_s,
                                                u"RemovePoint( line [LineString] , position [Integer] )"_s,

                                                // SQL functions on type Surface [Polygon or Ring]
                                                u"Centroid( s [Surface] )"_s,
                                                u"Area( s [Surface] )"_s,

                                                // SQL functions on type Polygon
                                                u"ExteriorRing( polyg [Polygon] )"_s,
                                                u"NumInteriorRing( polyg [Polygon] )"_s,
                                                u"InteriorRingN( polyg [Polygon] , n [Integer] )"_s,

                                                // SQL functions on type GeomCollection
                                                u"NumGeometries( geom [GeomCollection] )"_s,
                                                u"GeometryN( geom [GeomCollection] , n [Integer] )"_s,

                                                // SQL functions that test approximate spatial relationships via MBRs
                                                u"MbrEqual( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"MbrDisjoint( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"MbrTouches( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"MbrWithin( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"MbrOverlaps( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"MbrIntersects( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"ST_EnvIntersects( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"MbrContains( geom1 [Geometry] , geom2 [Geometry] )"_s,

                                                // SQL functions that test spatial relationships
                                                u"Equals( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Disjoint( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Touches( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Within( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Overlaps( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Crosses( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Intersects( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Contains( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Covers( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"CoveredBy( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"Relate( geom1 [Geometry] , geom2 [Geometry] , patternMatrix [String] )"_s,

                                                // SQL functions for distance relationships
                                                u"Distance( geom1 [Geometry] , geom2 [Geometry] )"_s,

                                                // SQL functions that implement spatial operators
                                                u"MakeValid( geom [Geometry] )"_s,
                                                u"MakeValidDiscarded( geom [Geometry] )"_s,
                                                u"Segmentize( geom [Geometry], dist [Double precision]  )"_s,
                                                u"Split( geom [Geometry], blade [Geometry]  )"_s,
                                                u"SplitLeft( geom [Geometry], blade [Geometry]  )"_s,
                                                u"SplitRight( geom [Geometry], blade [Geometry]  )"_s,
                                                u"Azimuth( pt1 [Geometry], pt2 [Geometry]  )"_s,
                                                u"Project( start_point [Geometry], distance [Double precision], azimuth [Double precision]  )"_s,
                                                u"SnapToGrid( geom [Geometry] , size [Double precision]  )"_s,
                                                u"GeoHash( geom [Geometry] )"_s,
                                                u"AsX3D( geom [Geometry] )"_s,
                                                u"MaxDistance( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"ST_3DDistance( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"ST_3DMaxDistance( geom1 [Geometry] , geom2 [Geometry] )"_s,
                                                u"ST_Node( geom [Geometry] )"_s,
                                                u"SelfIntersections( geom [Geometry] )"_s,

                                                // SQL functions for coordinate transformations
                                                u"Transform( geom [Geometry] , newSRID [Integer] )"_s,
                                                u"SridFromAuthCRS( auth_name [String] , auth_SRID [Integer] )"_s,
                                                u"ShiftCoords( geom [Geometry] , shiftX [Double precision] , shiftY [Double precision] )"_s,
                                                u"ST_Translate( geom [Geometry] , shiftX [Double precision] , shiftY [Double precision] , shiftZ [Double precision] )"_s,
                                                u"ST_Shift_Longitude( geom [Geometry] )"_s,
                                                u"NormalizeLonLat( geom [Geometry] )"_s,
                                                u"ScaleCoords( geom [Geometry] , scaleX [Double precision] [ , scaleY [Double precision] ] )"_s,
                                                u"RotateCoords( geom [Geometry] , angleInDegrees [Double precision] )"_s,
                                                u"ReflectCoords( geom [Geometry] , xAxis [Integer] , yAxis [Integer] )"_s,
                                                u"SwapCoords( geom [Geometry] )"_s,

                                                // SQL functions for Spatial-MetaData and Spatial-Index handling
                                                u"InitSpatialMetaData( void )"_s,
                                                u"InsertEpsgSrid( srid [Integer] )"_s,
                                                u"DiscardGeometryColumn( table [String] , column [String] )"_s,
                                                u"RegisterVirtualGeometry( table [String] )"_s,
                                                u"DropVirtualGeometry( table [String] )"_s,
                                                u"CreateSpatialIndex( table [String] , column [String] )"_s,
                                                u"CreateMbrCache( table [String] , column [String] )"_s,
                                                u"DisableSpatialIndex( table [String] , column [String] )"_s,
                                                u"CheckShadowedRowid( table [String] )"_s,
                                                u"CheckWithoutRowid( table [String] )"_s,
                                                u"CheckSpatialIndex( void )"_s,
                                                u"RecoverSpatialIndex( [ no_check"_s,
                                                u"InvalidateLayerStatistics( [ void )"_s,
                                                u"UpdateLayerStatistics( [ void )"_s,
                                                u"GetLayerExtent( table [String] [ , column [String] [ , mode [Boolean]] ] )"_s,
                                                u"CreateTopologyTables( SRID [Integer] , dims"_s,
                                                u"CreateRasterCoveragesTable( [void] )"_s,

                                                // SQL functions supporting the MetaCatalog and related Statistics
                                                u"CreateMetaCatalogTables( transaction [Integer] )"_s,
                                                u"UpdateMetaCatalogStatistics( transaction [Integer] , table_name [String] , column_name [String] )"_s,

                                                // SQL functions supporting SLD/SE Styled Layers
                                                u"CreateStylingTables()"_s,
                                                u"RegisterExternalGraphic( xlink_href [String] , resource [BLOB] )"_s,
                                                u"RegisterVectorStyledLayer( f_table_name [String] , f_geometry_column [String] , style [BLOB] )"_s,
                                                u"RegisterRasterStyledLayer( coverage_name [String] , style [BLOB] )"_s,
                                                u"RegisterStyledGroup( group_name [String] , f_table_name [String] , f_geometry_column [String] [ , paint_order [Integer] ] )"_s,
                                                u"SetStyledGroupInfos( group_name [String] , title [String] , abstract [String] )"_s,
                                                u"RegisterGroupStyle( group_name [String] , style [BLOB] )"_s,

                                                // SQL functions supporting ISO Metadata
                                                u"CreateIsoMetadataTables()"_s,
                                                u"RegisterIsoMetadata( scope [String] , metadata [BLOB] )"_s,
                                                u"GetIsoMetadataId( fileIdentifier [String] )"_s,

                                                // SQL functions implementing FDO/OGR compatibility
                                                u"CheckSpatialMetaData( void )"_s,
                                                u"AutoFDOStart( void )"_s,
                                                u"AutoFDOStop( void )"_s,
                                                u"InitFDOSpatialMetaData( void )"_s,
                                                u"DiscardFDOGeometryColumn( table [String] , column [String] )"_s,

                                                // SQL functions implementing OGC GeoPackage compatibility
                                                u"CheckGeoPackageMetaData( void )"_s,
                                                u"AutoGPKGStart( void )"_s,
                                                u"AutoGPKGStop( void )"_s,
                                                u"gpkgCreateBaseTables( void )"_s,
                                                u"gpkgInsertEpsgSRID( srid [Integer] )"_s,
                                                u"gpkgAddTileTriggers( tile_table_name [String] )"_s,
                                                u"gpkgGetNormalZoom( tile_table_name [String] , inverted_zoom_level [Integer] )"_s,
                                                u"gpkgGetNormalRow( tile_table_name [String] , normal_zoom_level [Integer] , inverted_row_number [Integer] )"_s,
                                                u"gpkgGetImageType( image [Blob] )"_s,
                                                u"gpkgAddGeometryTriggers( table_name [String] , geometry_column_name [String] )"_s,
                                                u"gpkgAddSpatialIndex( table_name [String] , geometry_column_name [String] )"_s,
                                                u"gpkgMakePoint (x [Double precision] , y [Double precision] )"_s,
                                                u"gpkgMakePointZ (x [Double precision] , y [Double precision] , z [Double precision] )"_s,
                                                u"gpkgMakePointM (x [Double precision] , y [Double precision] , m [Double precision] )"_s,
                                                u"gpkgMakePointZM (x [Double precision] , y [Double precision] , z [Double precision] , m [Double precision] )"_s,
                                                u"IsValidGPB( geom [Blob] )"_s,
                                                u"AsGPB( geom [BLOB encoded geometry] )"_s,
                                                u"GeomFromGPB( geom [GPKG Blob Geometry] )"_s,
                                                u"CastAutomagic( geom [Blob] )"_s,
                                                u"GPKG_IsAssignable( expected_type_name [String] , actual_type_name [String] )"_s,

                                              } } }
  );
}

Qgis::DatabaseProviderTableImportCapabilities QgsSpatiaLiteProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapability::SetGeometryColumnName | Qgis::DatabaseProviderTableImportCapability::SetPrimaryKeyName;
}

QString QgsSpatiaLiteProviderConnection::defaultPrimaryKeyColumnName() const
{
  return u"pk"_s;
}

void QgsSpatiaLiteProviderConnection::deleteField( const QString &fieldName, const QString &, const QString &tableName, bool ) const
{
  QgsVectorLayer::LayerOptions options { false, false };
  options.skipCrsValidation = true;
  std::unique_ptr<QgsVectorLayer> vl { std::make_unique<QgsVectorLayer>( u"%1|layername=%2"_s.arg( pathFromUri(), tableName ), u"temp_layer"_s, u"ogr"_s, options ) };
  if ( !vl->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a valid layer for table '%1'" ).arg( tableName ) );
  }
  if ( vl->fields().lookupField( fieldName ) == -1 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not delete field '%1' of table '%2': field does not exist" ).arg( fieldName, tableName ) );
  }
  if ( !vl->dataProvider()->deleteAttributes( { vl->fields().lookupField( fieldName ) } ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Unknown error deleting field '%1' of table '%2'" ).arg( fieldName, tableName ) );
  }
}

QList<QgsVectorDataProvider::NativeType> QgsSpatiaLiteProviderConnection::nativeTypes() const
{
  return QgsSpatiaLiteConnection::nativeTypes();
}
