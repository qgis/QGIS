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


QgsSpatiaLiteProviderConnection::QgsSpatiaLiteProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
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
  const QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*|\([^\)]+\))raw" };
  // Cleanup the URI in case it contains other information other than the file path
  setUri( QString( uri ).replace( removePartsRe, QString() ) );
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
  const auto tableInfo { table( schema, name ) };
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
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  QMap<QString, QVariant> opts { *options };
  opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
  opts[ QStringLiteral( "update" ) ] = true;
  QMap<int, int> map;
  QString errCause;
  QgsVectorLayerExporter::ExportError errCode = QgsSpatiaLiteProvider::createEmptyLayer(
        uri() + QStringLiteral( " table=%1 (geom)" ).arg( QgsSqliteUtils::quotedIdentifier( name ) ),
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
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

void QgsSpatiaLiteProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
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
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name ).arg( errCause ) );
  }
}


void QgsSpatiaLiteProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  // TODO: maybe an index?
  QString sql( QStringLiteral( "ALTER TABLE %1 RENAME TO %2" )
               .arg( QgsSqliteUtils::quotedIdentifier( name ),
                     QgsSqliteUtils::quotedIdentifier( newName ) ) );
  executeSqlPrivate( sql );
  sql = QStringLiteral( "UPDATE geometry_columns SET f_table_name = lower(%2) WHERE lower(f_table_name) = lower(%1)" )
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  executeSqlPrivate( sql );
  sql = QStringLiteral( "UPDATE layer_styles SET f_table_name = lower(%2) WHERE f_table_name = lower(%1)" )
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  try
  {
    executeSqlPrivate( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QgsDebugMsgLevel( QStringLiteral( "Warning: error while updating the styles, perhaps there are no styles stored in this GPKG: %1" ).arg( ex.what() ), 4 );
  }
}

QList<QList<QVariant>> QgsSpatiaLiteProviderConnection::executeSql( const QString &sql ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql );
}

void QgsSpatiaLiteProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  Q_UNUSED( name )
  checkCapability( Capability::Vacuum );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  executeSqlPrivate( QStringLiteral( "VACUUM" ) );
}


QList<QgsSpatiaLiteProviderConnection::TableProperty> QgsSpatiaLiteProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by Spatialite, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  QList<QgsSpatiaLiteProviderConnection::TableProperty> tableInfo;
  QString errCause;
  QList<QVariantList> results;
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
      for ( const auto &tn : executeSqlPrivate( QStringLiteral( "SELECT name FROM sqlite_master WHERE type = 'view'" ) ) )
      {
        viewNames.push_back( tn.first().toString() );
      }

      // Another wierdness: table names are converted to lowercase when out of spatialite gaia functions, let's get them back to their real case here,
      // may need LAUNDER on open, but let's try to make it consistent with how GPKG works.
      QgsStringMap tableNotLowercaseNames;
      for ( const auto &tn : executeSqlPrivate( QStringLiteral( "SELECT name FROM sqlite_master WHERE LOWER(name) != name" ) ) )
      {
        const QString tName { tn.first().toString() };
        tableNotLowercaseNames.insert( tName.toLower(), tName );
      }

      const auto constTables = connection.tables();
      for ( const QgsSpatiaLiteConnection::TableEntry &entry : constTables )
      {
        QString tableName { tableNotLowercaseNames.value( entry.tableName, entry.tableName ) };
        dsUri.setDataSource( QString(), tableName, entry.column, QString(), QString() );
        QgsSpatiaLiteProviderConnection::TableProperty property;
        property.setTableName( tableName );
        // Create a layer and get information from it
        std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique<QgsVectorLayer>( dsUri.uri(), QString(), QLatin1Literal( "spatialite" ) );
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
    throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( pathFromUri() ).arg( errCause ) );
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
  };
}

QList<QVariantList> QgsSpatiaLiteProviderConnection::executeSqlPrivate( const QString &sql ) const
{
  QString errCause;
  QList<QVariantList> results;
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( pathFromUri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS.get(), sql.toUtf8().constData(), nullptr, nullptr ) );
    if ( ogrLayer )
    {
      gdal::ogr_feature_unique_ptr fet;
      QgsFields fields;
      while ( fet.reset( OGR_L_GetNextFeature( ogrLayer ) ), fet )
      {
        QVariantList row;
        // Try to get the right type for the returned values
        if ( fields.isEmpty() )
        {
          fields = QgsOgrUtils::readOgrFields( fet.get(), QTextCodec::codecForName( "UTF-8" ) );
        }
        if ( ! fields.isEmpty() )
        {
          QgsFeature f { QgsOgrUtils::readOgrFeature( fet.get(), fields, QTextCodec::codecForName( "UTF-8" ) ) };
          const QgsAttributes &constAttrs { f.attributes() };
          for ( int i = 0; i < constAttrs.length(); i++ )
          {
            row.push_back( constAttrs.at( i ) );
          }
        }
        else // Fallback to strings
        {
          for ( int i = 0; i < OGR_F_GetFieldCount( fet.get() ); i++ )
          {
            row.push_back( QVariant( QString::fromUtf8( OGR_F_GetFieldAsString( fet.get(), i ) ) ) );
          }
        }

        results.push_back( row );
      }
      GDALDatasetReleaseResultSet( hDS.get(), ogrLayer );
    }
    errCause = CPLGetLastErrorMsg( );
  }
  else
  {
    errCause = QObject::tr( "There was an error opening Spatialite %1!" ).arg( pathFromUri() );
  }
  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql ).arg( errCause ) );
  }
  return results;
}

QString QgsSpatiaLiteProviderConnection::pathFromUri() const
{
  const QgsDataSourceUri dsUri( uri() );
  return dsUri.database();
}

