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
#include "qgsspatialiteconnection.h"
#include "qgsspatialiteprovider.h"
#include "qgsogrprovider.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsdbquerylog.h"

#include <QRegularExpression>
#include <QTextCodec>

#include <chrono>

QgsSpatiaLiteProviderConnection::QgsSpatiaLiteProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "spatialite" );
  setDefaultCapabilities();
  // TODO: QGIS 4: move into QgsSettings::Section::Providers group
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "SpatiaLite" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name );
  QgsDataSourceUri dsUri;
  dsUri.setDatabase( settings.value( QStringLiteral( "sqlitepath" ) ).toString() );
  setUri( dsUri.uri() );
}

QgsSpatiaLiteProviderConnection::QgsSpatiaLiteProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = QStringLiteral( "spatialite" );
  QgsDataSourceUri dsUri { uri };
  QgsDataSourceUri dsUriCleaned;
  dsUriCleaned.setDatabase( dsUri.database() );
  setUri( dsUriCleaned.uri() );
  setDefaultCapabilities();
}

void QgsSpatiaLiteProviderConnection::store( const QString &name ) const
{
  // TODO: QGIS 4: move into QgsSettings::Section::Providers group
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "SpatiaLite" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name );
  settings.setValue( QStringLiteral( "sqlitepath" ), pathFromUri() );
}

void QgsSpatiaLiteProviderConnection::remove( const QString &name ) const
{
  // TODO: QGIS 4: move into QgsSettings::Section::Providers group
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "SpatiaLite" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.remove( name );
}

QString QgsSpatiaLiteProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  Q_UNUSED( schema );  // spatialite does not support schema
  return uri() + QStringLiteral( " table=%1" ).arg( QgsSqliteUtils::quotedIdentifier( name ) );
}

void QgsSpatiaLiteProviderConnection::createVectorTable( const QString &schema,
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
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  QMap<QString, QVariant> opts { *options };
  opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
  opts[ QStringLiteral( "update" ) ] = true;
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult res = QgsSpatiaLiteProvider::createEmptyLayer(
                                   uri() + QStringLiteral( " table=%1 (geom)" ).arg( QgsSqliteUtils::quotedIdentifier( name ) ),
                                   fields,
                                   wkbType,
                                   srs,
                                   overwrite,
                                   &map,
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
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : QStringLiteral( "SELECT * FROM %1" ).arg( tUri.quotedTablename() );
  return options;
}

QgsVectorLayer *QgsSpatiaLiteProviderConnection::createSqlVectorLayer( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options ) const
{
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  QgsDataSourceUri tUri( uri( ) );

  tUri.setSql( options.filter );
  tUri.setTable( '(' + options.sql + ')' );

  if ( ! options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  return new QgsVectorLayer{ tUri.uri(), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey() };
}

void QgsSpatiaLiteProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  QString errCause;

  QgsSqliteHandle *hndl = QgsSqliteHandle::openDb( pathFromUri() );
  if ( !hndl )
  {
    errCause = QObject::tr( "Connection to database failed" );
  }

  if ( errCause.isEmpty() )
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
        QgsDebugMsg( QStringLiteral( "Failed to run VACUUM after deleting table on database %1" )
                     .arg( pathFromUri() ) );
      }

      QgsSqliteHandle::closeDb( hndl );
    }
  }
  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name, errCause ) );
  }
}


void QgsSpatiaLiteProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  // TODO: maybe an index?
  QString sql( QStringLiteral( "ALTER TABLE %1 RENAME TO %2" )
               .arg( QgsSqliteUtils::quotedIdentifier( name ),
                     QgsSqliteUtils::quotedIdentifier( newName ) ) );
  executeSqlDirect( sql );
  sql = QStringLiteral( "UPDATE geometry_columns SET f_table_name = lower(%2) WHERE lower(f_table_name) = lower(%1)" )
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  executeSqlDirect( sql );
  sql = QStringLiteral( "UPDATE layer_styles SET f_table_name = lower(%2) WHERE f_table_name = lower(%1)" )
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  try
  {
    executeSqlDirect( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QgsDebugMsgLevel( QStringLiteral( "Warning: error while updating the styles, perhaps there are no styles stored in this GPKG: %1" ).arg( ex.what() ), 4 );
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
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  executeSqlDirect( QStringLiteral( "VACUUM" ) );
}

void QgsSpatiaLiteProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{

  checkCapability( Capability::CreateSpatialIndex );

  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
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

  executeSqlPrivate( QStringLiteral( "SELECT CreateSpatialIndex(%1, %2)" ).arg( QgsSqliteUtils::quotedString( name ),
                     QgsSqliteUtils::quotedString( ( geometryColumnName ) ) ) );
}

bool QgsSpatiaLiteProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::CreateSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  const QList<QVariantList> res = executeSqlPrivate( QStringLiteral( "SELECT spatial_index_enabled FROM geometry_columns WHERE lower(f_table_name) = lower(%1) AND lower(f_geometry_column) = lower(%2)" )
                                  .arg( QgsSqliteUtils::quotedString( name ),
                                        QgsSqliteUtils::quotedString( geometryColumn ) ) ).rows();
  return !res.isEmpty() && !res.at( 0 ).isEmpty() && res.at( 0 ).at( 0 ).toInt() == 1;
}

QList<QgsSpatiaLiteProviderConnection::TableProperty> QgsSpatiaLiteProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
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
        msg = QStringLiteral( "%1 (%2)" ).arg( msg, msgDetails );
      }
      throw QgsProviderConnectionException( QObject::tr( "Error fetching table information for connection: %1" ).arg( pathFromUri() ) );
    }
    else
    {

      const QString connectionInfo = QStringLiteral( "dbname='%1'" ).arg( QString( connection.path() ).replace( '\'', QLatin1String( "\\'" ) ) );
      QgsDataSourceUri dsUri( connectionInfo );

      // Need to store it here because provider (and underlying gaia library) returns views as spatial table if they have geometries
      QStringList viewNames;
      const QList<QList<QVariant>> viewRows { executeSqlPrivate( QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'view'" ) ).rows() };
      for ( const QList<QVariant> &tn : std::as_const( viewRows ) )
      {
        viewNames.push_back( tn.first().toString() );
      }

      // Another weirdness: table names are converted to lowercase when out of spatialite gaia functions, let's get them back to their real case here,
      // may need LAUNDER on open, but let's try to make it consistent with how GPKG works.
      QgsStringMap tableNotLowercaseNames;
      const QList<QList<QVariant>> lowerTables { executeSqlPrivate( QStringLiteral( "SELECT name FROM sqlite_master WHERE LOWER(name) != name" ) ).rows() };
      for ( const QList<QVariant> &tn : std::as_const( lowerTables ) )
      {
        const QString tName { tn.first().toString() };
        tableNotLowercaseNames.insert( tName.toLower(), tName );
      }

      const QList<QgsSpatiaLiteConnection::TableEntry> constTables = connection.tables();
      for ( const QgsSpatiaLiteConnection::TableEntry &entry : constTables )
      {
        QString tableName { tableNotLowercaseNames.value( entry.tableName, entry.tableName ) };
        dsUri.setDataSource( QString(), tableName, entry.column, QString(), QString() );
        QgsSpatiaLiteProviderConnection::TableProperty property;
        property.setTableName( tableName );
        // Create a layer and get information from it
        // Use OGR because it's way faster
        std::unique_ptr< QgsVectorLayer > vl = std::make_unique<QgsVectorLayer>( dsUri.database() + "|layername=" + dsUri.table(), QString(), QLatin1String( "ogr" ), QgsVectorLayer::LayerOptions( false, true ) );
        if ( vl->isValid() )
        {
          if ( vl->isSpatial() )
          {
            property.setGeometryColumnCount( 1 );
            property.setGeometryColumn( entry.column );
            property.setFlag( QgsSpatiaLiteProviderConnection::TableFlag::Vector );
            property.setGeometryColumnTypes( {{ vl->wkbType(), vl->crs() }} );
          }
          else
          {
            property.setGeometryColumnCount( 0 );
            property.setGeometryColumnTypes( {{ QgsWkbTypes::NoGeometry, QgsCoordinateReferenceSystem() }} );
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

          if ( ! pkNames.isEmpty() )
          {
            property.setPrimaryKeyColumns( pkNames );
          }

          tableInfo.push_back( property );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer is not valid: %1" ).arg( dsUri.uri() ), 2 );
        }
      }
    }
  }
  catch ( QgsProviderConnectionException &ex )
  {
    errCause = ex.what();
  }

  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( pathFromUri(), errCause ) );
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

QIcon QgsSpatiaLiteProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconSpatialite.svg" ) );
}

void QgsSpatiaLiteProviderConnection::setDefaultCapabilities()
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
    Capability::DeleteField,
    Capability::AddField,
    Capability::SqlLayers,
  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePart,
  };
  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn
  };

}

QgsAbstractDatabaseProviderConnection::QueryResult QgsSpatiaLiteProviderConnection::executeSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{

  QgsDatabaseQueryLogWrapper logWrapper( sql, uri(), providerKey(), QStringLiteral( "QgsSpatiaLiteProviderConnection" ), QGS_QUERY_LOG_ORIGIN );

  if ( feedback && feedback->isCanceled() )
  {
    logWrapper.setCanceled();
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  QString errCause;
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( pathFromUri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
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
        if ( ! geomColumnName.isEmpty() )
        {
          results.appendColumn( geomColumnName );
          iterator->setGeometryColumnName( geomColumnName );
        }

        iterator->setFields( fields );
      }

      // Check for errors
      if ( CE_Failure == CPLGetLastErrorType() || CE_Fatal == CPLGetLastErrorType() )
      {
        errCause = CPLGetLastErrorMsg( );
      }

      if ( ! errCause.isEmpty() )
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
      errCause = CPLGetLastErrorMsg( );
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


QgsSpatialiteProviderResultIterator::QgsSpatialiteProviderResultIterator( gdal::ogr_datasource_unique_ptr hDS, OGRLayerH ogrLayer )
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
      if ( ! mFields.isEmpty() )
      {
        QgsFeature f { QgsOgrUtils::readOgrFeature( fet.get(), mFields, QTextCodec::codecForName( "UTF-8" ) ) };
        const QgsAttributes constAttrs  = f.attributes();
        for ( const QVariant &attribute : constAttrs )
        {
          row.push_back( attribute );
        }

        // Geom goes last
        if ( ! mGeometryColumnName.isEmpty( ) )
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
  return ! mNextRow.isEmpty();
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
  {
    {
      Qgis::SqlKeywordCategory::Math, {
        // SQL math functions
        QStringLiteral( "Abs( x [Double precision] )" ),
        QStringLiteral( "Acos( x [Double precision] )" ),
        QStringLiteral( "Asin( x [Double precision] )" ),
        QStringLiteral( "Atan( x [Double precision] )" ),
        QStringLiteral( "Ceil( x [Double precision] )" ),
        QStringLiteral( "Cos( x [Double precision] )" ),
        QStringLiteral( "Cot( x [Double precision] )" ),
        QStringLiteral( "Degrees( x [Double precision] )" ),
        QStringLiteral( "Exp( x [Double precision] )" ),
        QStringLiteral( "Floor( x [Double precision] )" ),
        QStringLiteral( "Ln( x [Double precision] )" ),
        QStringLiteral( "Log( b [Double precision] , x [Double precision] )" ),
        QStringLiteral( "Log2( x [Double precision] )" ),
        QStringLiteral( "Log10( x [Double precision] )" ),
        QStringLiteral( "PI( void )" ),
        QStringLiteral( "Pow( x [Double precision] , y [Double precision] )" ),
        QStringLiteral( "Radians( x [Double precision] )" ),
        QStringLiteral( "Sign( x [Double precision] )" ),
        QStringLiteral( "Sin( x [Double precision] )" ),
        QStringLiteral( "Sqrt( x [Double precision] )" ),
        QStringLiteral( "Stddev_pop( x [Double precision] )" ),
        QStringLiteral( "Stddev_samp( x [Double precision] )" ),
        QStringLiteral( "Tan( x [Double precision] )" ),
        QStringLiteral( "Var_pop( x [Double precision] )" ),
        QStringLiteral( "Var_samp( x [Double precision] )" )
      }
    },
    {
      Qgis::SqlKeywordCategory::Function, {

        // Specific
        QStringLiteral( "last_insert_rowid" ),

        // SQL Version Info [and build options testing] functions
        QStringLiteral( "spatialite_version( void )" ),
        QStringLiteral( "spatialite_target_cpu( void )" ),
        QStringLiteral( "proj4_version( void )" ),
        QStringLiteral( "geos_version( void )" ),
        QStringLiteral( "lwgeom_version( void )" ),
        QStringLiteral( "libxml2_version( void )" ),
        QStringLiteral( "HasIconv( void )" ),
        QStringLiteral( "HasMathSQL( void )" ),
        QStringLiteral( "HasGeoCallbacks( void )" ),
        QStringLiteral( "HasProj( void )" ),
        QStringLiteral( "HasGeos( void )" ),
        QStringLiteral( "HasGeosAdvanced( void )" ),
        QStringLiteral( "HasGeosTrunk( void )" ),
        QStringLiteral( "HasLwGeom( void )" ),
        QStringLiteral( "HasLibXML2( void )" ),
        QStringLiteral( "HasEpsg( void )" ),
        QStringLiteral( "HasFreeXL( void )" ),
        QStringLiteral( "HasGeoPackage( void )" ),

        // Generic SQL functions
        QStringLiteral( "CastToInteger( value [Generic] )" ),
        QStringLiteral( "CastToDouble( value [Generic] )" ),
        QStringLiteral( "CastToText( value [Generic] )" ),
        QStringLiteral( "CastToBlob( value [Generic] )" ),
        QStringLiteral( "ForceAsNull( val1 [Generic] , val2 [Generic])" ),
        QStringLiteral( "CreateUUID( void )" ),
        QStringLiteral( "MD5Checksum( BLOB | TEXT )" ),
        QStringLiteral( "MD5TotalChecksum( BLOB | TEXT )" ),

        // SQL utility functions for BLOB objects
        QStringLiteral( "IsZipBlob( content [BLOB] )" ),
        QStringLiteral( "IsPdfBlob( content [BLOB] )" ),
        QStringLiteral( "IsGifBlob( image [BLOB] )" ),
        QStringLiteral( "IsPngBlob( image [BLOB] )" ),
        QStringLiteral( "IsTiffBlob( image [BLOB] )" ),
        QStringLiteral( "IsJpegBlob( image [BLOB] )" ),
        QStringLiteral( "IsExifBlob( image [BLOB] )" ),
        QStringLiteral( "IsExifGpsBlob( image [BLOB] )" ),
        QStringLiteral( "IsWebpBlob( image [BLOB] )" ),
        QStringLiteral( "GetMimeType( payload [BLOB] )" ),
        QStringLiteral( "BlobFromFile( filepath [String] )" ),
        QStringLiteral( "BlobToFile( payload [BLOB] , filepath [String] )" ),
        QStringLiteral( "CountUnsafeTriggers( )" ),

        // SQL functions supporting XmlBLOB
        QStringLiteral( "XB_Create(  xmlPayload [BLOB] )" ),
        QStringLiteral( "XB_GetPayload( xmlObject [XmlBLOB] [ , indent [Integer] ] )" ),
        QStringLiteral( "XB_GetDocument( xmlObject [XmlBLOB] [ , indent [Integer] ] )" ),
        QStringLiteral( "XB_SchemaValidate(  xmlObject [XmlBLOB] , schemaURI [Text] [ , compressed [Boolean] ] )" ),
        QStringLiteral( "XB_Compress( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_Uncompress( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsValid( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsCompressed( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsSchemaValidated( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsIsoMetadata( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsSldSeVectorStyle( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsSldSeRasterStyle( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_IsSvg( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetDocumentSize( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetEncoding( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetSchemaURI( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetInternalSchemaURI( xmlPayload [BLOB] )" ),
        QStringLiteral( "XB_GetFileId( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_SetFileId( xmlObject [XmlBLOB] , fileId [String] )" ),
        QStringLiteral( "XB_AddFileId( xmlObject [XmlBLOB] , fileId [String] , IdNameSpacePrefix [String] , IdNameSpaceURI [String] , CsNameSpacePrefix [String] , CsNameSpaceURI [String] )" ),
        QStringLiteral( "XB_GetParentId( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_SetParentId( xmlObject [XmlBLOB] , parentId [String] )" ),
        QStringLiteral( "XB_AddParentId( xmlObject [XmlBLOB] , parentId [String] , IdNameSpacePrefix [String] , IdNameSpaceURI [String] , CsNameSpacePrefix [String] , CsNameSpaceURI [String] )" ),
        QStringLiteral( "XB_GetTitle( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetAbstract( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetGeometry( xmlObject [XmlBLOB] )" ),
        QStringLiteral( "XB_GetLastParseError( [void] )" ),
        QStringLiteral( "XB_GetLastValidateError( [void] )" ),
        QStringLiteral( "XB_IsValidXPathExpression( expr [Text] )" ),
        QStringLiteral( "XB_GetLastXPathError( [void] )" ),
        QStringLiteral( "XB_CacheFlush( [void] )" ),
        QStringLiteral( "XB_LoadXML( filepath-or-URL [String] )" ),
        QStringLiteral( "XB_StoreXML( XmlObject [XmlBLOB] , filepath [String] )" ),

      }
    },
    {
      Qgis::SqlKeywordCategory::Geospatial, {
        // SQL functions reporting GEOS / LWGEOM errors and warnings
        QStringLiteral( "GEOS_GetLastWarningMsg( [void] )" ),
        QStringLiteral( "GEOS_GetLastErrorMsg( [void] )" ),
        QStringLiteral( "GEOS_GetLastAuxErrorMsg( [void] )" ),
        QStringLiteral( "GEOS_GetCriticalPointFromMsg( [void] )" ),
        QStringLiteral( "LWGEOM_GetLastWarningMsg( [void] )" ),
        QStringLiteral( "LWGEOM_GetLastErrorMsg( [void] )" ),

        // SQL length/distance unit-conversion functions
        QStringLiteral( "CvtToKm( x [Double precision] )" ),
        QStringLiteral( "CvtToDm( x [Double precision] )" ),
        QStringLiteral( "CvtToCm( x [Double precision] )" ),
        QStringLiteral( "CvtToMm( x [Double precision] )" ),
        QStringLiteral( "CvtToKmi( x [Double precision] )" ),
        QStringLiteral( "CvtToIn( x [Double precision] )" ),
        QStringLiteral( "CvtToFt( x [Double precision] )" ),
        QStringLiteral( "CvtToYd( x [Double precision] )" ),
        QStringLiteral( "CvtToMi( x [Double precision] )" ),
        QStringLiteral( "CvtToFath( x [Double precision] )" ),
        QStringLiteral( "CvtToCh( x [Double precision] )" ),
        QStringLiteral( "CvtToLink( x [Double precision] )" ),
        QStringLiteral( "CvtToUsIn( x [Double precision] )" ),
        QStringLiteral( "CvtToUsFt( x [Double precision] )" ),
        QStringLiteral( "CvtToUsYd( x [Double precision] )" ),
        QStringLiteral( "CvtToUsMi( x [Double precision] )" ),
        QStringLiteral( "CvtToUsCh( x [Double precision] )" ),
        QStringLiteral( "CvtToIndFt( x [Double precision] )" ),
        QStringLiteral( "CvtToIndYd( x [Double precision] )" ),
        QStringLiteral( "CvtToIndCh( x [Double precision] )" ),

        // SQL conversion functions from DD/DMS notations (longitude/latitude)
        QStringLiteral( "LongLatToDMS( longitude [Double precision] , latitude [Double precision] )" ),
        QStringLiteral( "LongitudeFromDMS( dms_expression [Sting] )" ),

        // SQL utility functions [
        QStringLiteral( "GeomFromExifGpsBlob( image [BLOB] )" ),
        QStringLiteral( "ST_Point( x [Double precision] , y [Double precision]  )" ),
        QStringLiteral( "MakeLine( pt1 [PointGeometry] , pt2 [PointGeometry] )" ),
        QStringLiteral( "MakeLine( geom [PointGeometry] )" ),
        QStringLiteral( "MakeLine( geom [MultiPointGeometry] , direction [Boolean] )" ),
        QStringLiteral( "SquareGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )" ),
        QStringLiteral( "TriangularGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )" ),
        QStringLiteral( "HexagonalGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )" ),
        QStringLiteral( "Extent( geom [Geometry] )" ),
        QStringLiteral( "ToGARS( geom [Geometry] )" ),
        QStringLiteral( "GARSMbr( code [String] )" ),
        QStringLiteral( "MbrMinX( geom [Geometry])" ),
        QStringLiteral( "MbrMinY( geom [Geometry])" ),
        QStringLiteral( "MbrMaxX( geom [Geometry])" ),
        QStringLiteral( "MbrMaxY( geom [Geometry])" ),
        QStringLiteral( "ST_MinZ( geom [Geometry])" ),
        QStringLiteral( "ST_MaxZ( geom [Geometry])" ),
        QStringLiteral( "ST_MinM( geom [Geometry])" ),
        QStringLiteral( "ST_MaxM( geom [Geometry])" ),

        // SQL functions for constructing a geometric object given its Well-known Text Representation
        QStringLiteral( "GeomFromText( wkt [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "ST_WKTToSQL( wkt [String] )" ),
        QStringLiteral( "PointFromText( wktPoint [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "LineFromText( wktLineString [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "PolyFromText( wktPolygon [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "MPointFromText( wktMultiPoint [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "MLineFromText( wktMultiLineString [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "MPolyFromText( wktMultiPolygon [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "GeomCollFromText( wktGeometryCollection [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "BdPolyFromText( wktMultilinestring [String] [ , SRID [Integer]] )" ),
        QStringLiteral( "BdMPolyFromText( wktMultilinestring [String] [ , SRID [Integer]] )" ),

        // SQL functions for constructing a geometric object given its Well-known Binary Representation
        QStringLiteral( "GeomFromWKB( wkbGeometry [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "ST_WKBToSQL( wkbGeometry [Binary] )" ),
        QStringLiteral( "PointFromWKB( wkbPoint [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "LineFromWKB( wkbLineString [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "PolyFromWKB( wkbPolygon [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "MPointFromWKB( wkbMultiPoint [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "MLineFromWKB( wkbMultiLineString [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "MPolyFromWKB( wkbMultiPolygon [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "GeomCollFromWKB( wkbGeometryCollection [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "BdPolyFromWKB( wkbMultilinestring [Binary] [ , SRID [Integer]] )" ),
        QStringLiteral( "BdMPolyFromWKB( wkbMultilinestring [Binary] [ , SRID [Integer]] )" ),

        // SQL functions for obtaining the Well-known Text / Well-known Binary Representation of a geometric object
        QStringLiteral( "AsText( geom [Geometry] )" ),
        QStringLiteral( "AsWKT( geom [Geometry] [ , precision [Integer] ] )" ),
        QStringLiteral( "AsBinary( geom [Geometry] )" ),

        // SQL functions supporting exotic geometric formats
        QStringLiteral( "AsSVG( geom [Geometry] [ , relative [Integer] [ , precision [Integer] ] ] )" ),
        QStringLiteral( "AsKml( geom [Geometry] [ , precision [Integer] ] )" ),
        QStringLiteral( "GeomFromKml( KmlGeometry [String] )" ),
        QStringLiteral( "AsGml( geom [Geometry] [ , precision [Integer] ] )" ),
        QStringLiteral( "GeomFromGML( gmlGeometry [String] )" ),
        QStringLiteral( "AsGeoJSON( geom [Geometry] [ , precision [Integer] [ , options [Integer] ] ] )" ),
        QStringLiteral( "GeomFromGeoJSON( geoJSONGeometry [String] )" ),
        QStringLiteral( "AsEWKB( geom [Geometry] )" ),
        QStringLiteral( "GeomFromEWKB( ewkbGeometry [String] )" ),
        QStringLiteral( "AsEWKT( geom [Geometry] )" ),
        QStringLiteral( "GeomFromEWKT( ewktGeometry [String] )" ),
        QStringLiteral( "AsFGF( geom [Geometry] )" ),
        QStringLiteral( "GeomFromFGF( fgfGeometry [Binary] [ , SRID [Integer]] )" ),

        // SQL functions on type Geometry
        QStringLiteral( "Dimension( geom [Geometry] )" ),
        QStringLiteral( "CoordDimension( geom [Geometry] )" ),
        QStringLiteral( "ST_NDims( geom [Geometry] )" ),
        QStringLiteral( "ST_Is3D( geom [Geometry] )" ),
        QStringLiteral( "ST_IsMeasured( geom [Geometry] )" ),
        QStringLiteral( "GeometryType( geom [Geometry] )" ),
        QStringLiteral( "SRID( geom [Geometry] )" ),
        QStringLiteral( "SetSRID( geom [Geometry] , SRID [Integer] )" ),
        QStringLiteral( "IsEmpty( geom [Geometry] )" ),
        QStringLiteral( "IsSimple( geom [Geometry] )" ),
        QStringLiteral( "IsValid( geom [Geometry] )" ),
        QStringLiteral( "IsValidReason( geom [Geometry] )" ),
        QStringLiteral( "IsValidDetail( geom [Geometry] )" ),
        QStringLiteral( "Boundary( geom [Geometry] )" ),
        QStringLiteral( "Envelope( geom [Geometry] )" ),
        QStringLiteral( "ST_Expand( geom [Geometry] , amount [Double precision] )" ),
        QStringLiteral( "ST_NPoints( geom [Geometry] )" ),
        QStringLiteral( "ST_NRings( geom [Geometry] )" ),
        QStringLiteral( "ST_Reverse( geom [Geometry] )" ),
        QStringLiteral( "ST_ForceLHR( geom [Geometry] )" ),

        // SQL functions attempting to repair malformed Geometries
        QStringLiteral( "SanitizeGeometry( geom [Geometry] )" ),

        // SQL Geometry-compression functions
        QStringLiteral( "CompressGeometry( geom [Geometry] )" ),
        QStringLiteral( "UncompressGeometry( geom [Geometry] )" ),

        // SQL Geometry-type casting functions
        QStringLiteral( "CastToPoint( geom [Geometry] )" ),
        QStringLiteral( "CastToLinestring( geom [Geometry] )" ),
        QStringLiteral( "CastToPolygon( geom [Geometry] )" ),
        QStringLiteral( "CastToMultiPoint( geom [Geometry] )" ),
        QStringLiteral( "CastToMultiLinestring( geom [Geometry] )" ),
        QStringLiteral( "CastToMultiPolygon( geom [Geometry] )" ),
        QStringLiteral( "CastToGeometryCollection( geom [Geometry] )" ),
        QStringLiteral( "CastToMulti( geom [Geometry] )" ),
        QStringLiteral( "CastToSingle( geom [Geometry] )" ),

        // SQL Space-dimensions casting functions
        QStringLiteral( "CastToXY( geom [Geometry] )" ),
        QStringLiteral( "CastToXYZ( geom [Geometry] )" ),
        QStringLiteral( "CastToXYM( geom [Geometry] )" ),
        QStringLiteral( "CastToXYZM( geom [Geometry] )" ),

        // SQL functions on type Point
        QStringLiteral( "X( pt [Point] )" ),
        QStringLiteral( "Y( pt [Point] )" ),
        QStringLiteral( "Z( pt [Point] )" ),
        QStringLiteral( "M( pt [Point] )" ),

        // SQL functions on type Curve [Linestring or Ring]
        QStringLiteral( "StartPoint( c [Curve] )" ),
        QStringLiteral( "EndPoint( c [Curve] )" ),
        QStringLiteral( "GLength( c [Curve] )" ),
        QStringLiteral( "Perimeter( s [Surface] )" ),
        QStringLiteral( "GeodesicLength( c [Curve] )" ),
        QStringLiteral( "GreatCircleLength( c [Curve] )" ),
        QStringLiteral( "IsClosed( c [Curve] )" ),
        QStringLiteral( "IsRing( c [Curve] )" ),
        QStringLiteral( "PointOnSurface( s [Surface/Curve] )" ),
        QStringLiteral( "Simplify( c [Curve] , tolerance [Double precision] )" ),
        QStringLiteral( "SimplifyPreserveTopology( c [Curve] , tolerance [Double precision] )" ),

        // SQL functions on type LineString
        QStringLiteral( "NumPoints( line [LineString] )" ),
        QStringLiteral( "PointN( line [LineString] , n [Integer] )" ),
        QStringLiteral( "AddPoint( line [LineString] , point [Point] [ , position [Integer] ] )" ),
        QStringLiteral( "SetPoint( line [LineString] , position [Integer] , point [Point] )" ),
        QStringLiteral( "RemovePoint( line [LineString] , position [Integer] )" ),

        // SQL functions on type Surface [Polygon or Ring]
        QStringLiteral( "Centroid( s [Surface] )" ),
        QStringLiteral( "Area( s [Surface] )" ),

        // SQL functions on type Polygon
        QStringLiteral( "ExteriorRing( polyg [Polygon] )" ),
        QStringLiteral( "NumInteriorRing( polyg [Polygon] )" ),
        QStringLiteral( "InteriorRingN( polyg [Polygon] , n [Integer] )" ),

        // SQL functions on type GeomCollection
        QStringLiteral( "NumGeometries( geom [GeomCollection] )" ),
        QStringLiteral( "GeometryN( geom [GeomCollection] , n [Integer] )" ),

        // SQL functions that test approximate spatial relationships via MBRs
        QStringLiteral( "MbrEqual( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "MbrDisjoint( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "MbrTouches( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "MbrWithin( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "MbrOverlaps( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "MbrIntersects( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "ST_EnvIntersects( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "MbrContains( geom1 [Geometry] , geom2 [Geometry] )" ),

        // SQL functions that test spatial relationships
        QStringLiteral( "Equals( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Disjoint( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Touches( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Within( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Overlaps( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Crosses( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Intersects( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Contains( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Covers( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "CoveredBy( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "Relate( geom1 [Geometry] , geom2 [Geometry] , patternMatrix [String] )" ),

        // SQL functions for distance relationships
        QStringLiteral( "Distance( geom1 [Geometry] , geom2 [Geometry] )" ),

        // SQL functions that implement spatial operators
        QStringLiteral( "MakeValid( geom [Geometry] )" ),
        QStringLiteral( "MakeValidDiscarded( geom [Geometry] )" ),
        QStringLiteral( "Segmentize( geom [Geometry], dist [Double precision]  )" ),
        QStringLiteral( "Split( geom [Geometry], blade [Geometry]  )" ),
        QStringLiteral( "SplitLeft( geom [Geometry], blade [Geometry]  )" ),
        QStringLiteral( "SplitRight( geom [Geometry], blade [Geometry]  )" ),
        QStringLiteral( "Azimuth( pt1 [Geometry], pt2 [Geometry]  )" ),
        QStringLiteral( "Project( start_point [Geometry], distance [Double precision], azimuth [Double precision]  )" ),
        QStringLiteral( "SnapToGrid( geom [Geometry] , size [Double precision]  )" ),
        QStringLiteral( "GeoHash( geom [Geometry] )" ),
        QStringLiteral( "AsX3D( geom [Geometry] )" ),
        QStringLiteral( "MaxDistance( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "ST_3DDistance( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "ST_3DMaxDistance( geom1 [Geometry] , geom2 [Geometry] )" ),
        QStringLiteral( "ST_Node( geom [Geometry] )" ),
        QStringLiteral( "SelfIntersections( geom [Geometry] )" ),

        // SQL functions for coordinate transformations
        QStringLiteral( "Transform( geom [Geometry] , newSRID [Integer] )" ),
        QStringLiteral( "SridFromAuthCRS( auth_name [String] , auth_SRID [Integer] )" ),
        QStringLiteral( "ShiftCoords( geom [Geometry] , shiftX [Double precision] , shiftY [Double precision] )" ),
        QStringLiteral( "ST_Translate( geom [Geometry] , shiftX [Double precision] , shiftY [Double precision] , shiftZ [Double precision] )" ),
        QStringLiteral( "ST_Shift_Longitude( geom [Geometry] )" ),
        QStringLiteral( "NormalizeLonLat( geom [Geometry] )" ),
        QStringLiteral( "ScaleCoords( geom [Geometry] , scaleX [Double precision] [ , scaleY [Double precision] ] )" ),
        QStringLiteral( "RotateCoords( geom [Geometry] , angleInDegrees [Double precision] )" ),
        QStringLiteral( "ReflectCoords( geom [Geometry] , xAxis [Integer] , yAxis [Integer] )" ),
        QStringLiteral( "SwapCoords( geom [Geometry] )" ),

        // SQL functions for Spatial-MetaData and Spatial-Index handling
        QStringLiteral( "InitSpatialMetaData( void )" ),
        QStringLiteral( "InsertEpsgSrid( srid [Integer] )" ),
        QStringLiteral( "DiscardGeometryColumn( table [String] , column [String] )" ),
        QStringLiteral( "RegisterVirtualGeometry( table [String] )" ),
        QStringLiteral( "DropVirtualGeometry( table [String] )" ),
        QStringLiteral( "CreateSpatialIndex( table [String] , column [String] )" ),
        QStringLiteral( "CreateMbrCache( table [String] , column [String] )" ),
        QStringLiteral( "DisableSpatialIndex( table [String] , column [String] )" ),
        QStringLiteral( "CheckShadowedRowid( table [String] )" ),
        QStringLiteral( "CheckWithoutRowid( table [String] )" ),
        QStringLiteral( "CheckSpatialIndex( void )" ),
        QStringLiteral( "RecoverSpatialIndex( [ no_check" ),
        QStringLiteral( "InvalidateLayerStatistics( [ void )" ),
        QStringLiteral( "UpdateLayerStatistics( [ void )" ),
        QStringLiteral( "GetLayerExtent( table [String] [ , column [String] [ , mode [Boolean]] ] )" ),
        QStringLiteral( "CreateTopologyTables( SRID [Integer] , dims" ),
        QStringLiteral( "CreateRasterCoveragesTable( [void] )" ),

        // SQL functions supporting the MetaCatalog and related Statistics
        QStringLiteral( "CreateMetaCatalogTables( transaction [Integer] )" ),
        QStringLiteral( "UpdateMetaCatalogStatistics( transaction [Integer] , table_name [String] , column_name [String] )" ),

        // SQL functions supporting SLD/SE Styled Layers
        QStringLiteral( "CreateStylingTables()" ),
        QStringLiteral( "RegisterExternalGraphic( xlink_href [String] , resource [BLOB] )" ),
        QStringLiteral( "RegisterVectorStyledLayer( f_table_name [String] , f_geometry_column [String] , style [BLOB] )" ),
        QStringLiteral( "RegisterRasterStyledLayer( coverage_name [String] , style [BLOB] )" ),
        QStringLiteral( "RegisterStyledGroup( group_name [String] , f_table_name [String] , f_geometry_column [String] [ , paint_order [Integer] ] )" ),
        QStringLiteral( "SetStyledGroupInfos( group_name [String] , title [String] , abstract [String] )" ),
        QStringLiteral( "RegisterGroupStyle( group_name [String] , style [BLOB] )" ),

        // SQL functions supporting ISO Metadata
        QStringLiteral( "CreateIsoMetadataTables()" ),
        QStringLiteral( "RegisterIsoMetadata( scope [String] , metadata [BLOB] )" ),
        QStringLiteral( "GetIsoMetadataId( fileIdentifier [String] )" ),

        // SQL functions implementing FDO/OGR compatibility
        QStringLiteral( "CheckSpatialMetaData( void )" ),
        QStringLiteral( "AutoFDOStart( void )" ),
        QStringLiteral( "AutoFDOStop( void )" ),
        QStringLiteral( "InitFDOSpatialMetaData( void )" ),
        QStringLiteral( "DiscardFDOGeometryColumn( table [String] , column [String] )" ),

        // SQL functions implementing OGC GeoPackage compatibility
        QStringLiteral( "CheckGeoPackageMetaData( void )" ),
        QStringLiteral( "AutoGPKGStart( void )" ),
        QStringLiteral( "AutoGPKGStop( void )" ),
        QStringLiteral( "gpkgCreateBaseTables( void )" ),
        QStringLiteral( "gpkgInsertEpsgSRID( srid [Integer] )" ),
        QStringLiteral( "gpkgAddTileTriggers( tile_table_name [String] )" ),
        QStringLiteral( "gpkgGetNormalZoom( tile_table_name [String] , inverted_zoom_level [Integer] )" ),
        QStringLiteral( "gpkgGetNormalRow( tile_table_name [String] , normal_zoom_level [Integer] , inverted_row_number [Integer] )" ),
        QStringLiteral( "gpkgGetImageType( image [Blob] )" ),
        QStringLiteral( "gpkgAddGeometryTriggers( table_name [String] , geometry_column_name [String] )" ),
        QStringLiteral( "gpkgAddSpatialIndex( table_name [String] , geometry_column_name [String] )" ),
        QStringLiteral( "gpkgMakePoint (x [Double precision] , y [Double precision] )" ),
        QStringLiteral( "gpkgMakePointZ (x [Double precision] , y [Double precision] , z [Double precision] )" ),
        QStringLiteral( "gpkgMakePointM (x [Double precision] , y [Double precision] , m [Double precision] )" ),
        QStringLiteral( "gpkgMakePointZM (x [Double precision] , y [Double precision] , z [Double precision] , m [Double precision] )" ),
        QStringLiteral( "IsValidGPB( geom [Blob] )" ),
        QStringLiteral( "AsGPB( geom [BLOB encoded geometry] )" ),
        QStringLiteral( "GeomFromGPB( geom [GPKG Blob Geometry] )" ),
        QStringLiteral( "CastAutomagic( geom [Blob] )" ),
        QStringLiteral( "GPKG_IsAssignable( expected_type_name [String] , actual_type_name [String] )" ),

      }
    }
  } );
}


void QgsSpatiaLiteProviderConnection::deleteField( const QString &fieldName, const QString &, const QString &tableName, bool ) const
{
  QgsVectorLayer::LayerOptions options { false, false };
  options.skipCrsValidation = true;
  std::unique_ptr<QgsVectorLayer> vl { std::make_unique<QgsVectorLayer>( QStringLiteral( "%1|layername=%2" ).arg( pathFromUri(), tableName ), QStringLiteral( "temp_layer" ), QStringLiteral( "ogr" ), options ) };
  if ( ! vl->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a valid layer for table '%1'" ).arg( tableName ) );
  }
  if ( vl->fields().lookupField( fieldName ) == -1 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not delete field '%1' of table '%2': field does not exist" ).arg( fieldName, tableName ) );
  }
  if ( ! vl->dataProvider()->deleteAttributes( {vl->fields().lookupField( fieldName )} ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Unknown error deleting field '%1' of table '%2'" ).arg( fieldName, tableName ) );
  }
}

QList<QgsVectorDataProvider::NativeType> QgsSpatiaLiteProviderConnection::nativeTypes() const
{
  return QgsSpatiaLiteConnection::nativeTypes();
}
