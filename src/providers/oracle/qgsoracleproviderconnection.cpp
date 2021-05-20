/***************************************************************************
  qgsOracleproviderconnection.cpp - QgsOracleProviderConnection

 ---------------------
 begin                : 28.12.2020
 copyright            : (C) 2020 by Julien Cabieces
 email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsoracleproviderconnection.h"
#include "qgsoracleconn.h"
#include "qgsoracleconnpool.h"
#include "qgssettings.h"
#include "qgsoracleprovider.h"
#include "qgsexception.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"

#include <QSqlRecord>
#include <QSqlField>

// read from QSettings and used in the provider connection
const QStringList CONFIGURATION_PARAMETERS
{
  QStringLiteral( "geometryColumnsOnly" ),
  QStringLiteral( "allowGeometrylessTables" ),
  QStringLiteral( "disableInvalidGeometryHandling" ),
  QStringLiteral( "onlyExistingTypes" ),
  QStringLiteral( "saveUsername" ),
  QStringLiteral( "savePassword" ),
};

// read from uri and used in the provider connection
const QStringList EXTRA_CONNECTION_PARAMETERS
{
  QStringLiteral( "dboptions" ),
  QStringLiteral( "dbworkspace" )
};

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "oracle" );
  setUri( QgsOracleConn::connUri( name ).uri() );
  setDefaultCapabilities();

  // load existing configuration
  QgsSettings settings;
  QVariantMap configuration;
  for ( const auto &p : CONFIGURATION_PARAMETERS )
  {
    const QVariant v = settings.value( QStringLiteral( "/Oracle/connections/%1/%2" ).arg( name, p ) );
    if ( v.isValid() )
    {
      configuration.insert( p, v );
    }
  }
  setConfiguration( configuration );
}

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = QStringLiteral( "oracle" );
  setDefaultCapabilities();

  // Additional connection information
  const QgsDataSourceUri inputUri( uri );
  QgsDataSourceUri currentUri { QgsDataSourceUri( uri ).connectionInfo( false ) };

  if ( inputUri.hasParam( QStringLiteral( "estimatedMetadata" ) ) )
  {
    currentUri.setUseEstimatedMetadata( inputUri.param( QStringLiteral( "estimatedMetadata" ) ) == QStringLiteral( "true" )
                                        || inputUri.param( QStringLiteral( "estimatedMetadata" ) ) == '1' );
  }

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( inputUri.hasParam( param ) )
    {
      currentUri.setParam( param, inputUri.param( param ) );
    }
  }

  setUri( currentUri.uri() );
}

void QgsOracleProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
  {
    Capability::DropVectorTable,
    Capability::DropRasterTable,
    Capability::CreateVectorTable,
    Capability::RenameVectorTable,
    Capability::RenameRasterTable,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    Capability::Tables,
    Capability::Schemas,
    Capability::Spatial,
    Capability::TableExists,
    Capability::CreateSpatialIndex,
    Capability::SpatialIndexExists,
    Capability::DeleteSpatialIndex,
    Capability::DeleteField,
    Capability::DeleteFieldCascade,
    Capability::AddField
  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::SinglePart,
    GeometryColumnCapability::Curves
  };
}

void QgsOracleProviderConnection::store( const QString &name ) const
{
  QString baseKey = QStringLiteral( "/Oracle/connections/" );
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( name );

  // From URI
  const QgsDataSourceUri dsUri { uri() };
  settings.setValue( "authcfg", dsUri.authConfigId() );
  settings.setValue( "database", dsUri.database() );
  settings.setValue( "username", dsUri.username() );
  settings.setValue( "password", dsUri.password() );
  settings.setValue( "host", dsUri.host() );
  settings.setValue( "port", dsUri.port() );
  settings.setValue( "estimatedMetadata", dsUri.useEstimatedMetadata() );

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( dsUri.hasParam( param ) )
    {
      settings.setValue( param, dsUri.param( param ) );
    }
  }

  // From configuration
  for ( const auto &p : CONFIGURATION_PARAMETERS )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) );
    }
  }
  settings.endGroup();
  settings.endGroup();
}

void QgsOracleProviderConnection::remove( const QString &name ) const
{
  QgsOracleConn::deleteConnection( name );
}

QList<QgsVectorDataProvider::NativeType> QgsOracleProviderConnection::nativeTypes() const
{
  QList<QgsVectorDataProvider::NativeType> types;
  QgsPoolOracleConn conn( QgsDataSourceUri{ uri() }.connectionInfo( false ) );
  if ( conn.get() )
  {
    types = conn.get()->nativeTypes();
  }
  if ( types.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  }
  return types;
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOracleProviderConnection::executeSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{
  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  QgsPoolOracleConn pconn( QgsDataSourceUri{ uri() }.connectionInfo( false ) );
  if ( !pconn.get() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }

  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  QSqlQuery qry( *pconn.get() );
  if ( !qry.exec( sql ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "SQL error: %1 returned %2" )
                                          .arg( qry.lastQuery(),
                                              qry.lastError().text() ) );
  }

  if ( feedback && feedback->isCanceled() )
    return QgsAbstractDatabaseProviderConnection::QueryResult();

  if ( qry.isActive() )
  {
    const QSqlRecord rec { qry.record() };
    const int numCols { rec.count() };
    auto iterator = std::make_shared<QgsOracleProviderResultIterator>( numCols, qry );
    QgsAbstractDatabaseProviderConnection::QueryResult results( iterator );
    for ( int idx = 0; idx < numCols; ++idx )
    {
      results.appendColumn( rec.field( idx ).name() );
    }
    iterator->nextRow();
    return results;
  }

  return QgsAbstractDatabaseProviderConnection::QueryResult();
}

QVariantList QgsOracleProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow( mNextRow );
  mNextRow = nextRowInternal();
  return currentRow;
}

bool QgsOracleProviderResultIterator::hasNextRowPrivate() const
{
  return ! mNextRow.isEmpty();
}

QVariantList QgsOracleProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mQuery.next() )
  {
    for ( int col = 0; col < mColumnCount; ++col )
    {
      row.push_back( mQuery.value( col ) );
    }
  }
  else
  {
    mQuery.finish();
  }
  return row;
}

void QgsOracleProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString,
    QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri { uri() };
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column and if it's not aspatial
  if ( wkbType != QgsWkbTypes::Type::Unknown &&  wkbType != QgsWkbTypes::Type::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "GEOM" ) ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  const Qgis::VectorExportResult res = QgsOracleProvider::createEmptyLayer(
                                         newUri.uri(),
                                         fields,
                                         wkbType,
                                         srs,
                                         overwrite,
                                         map,
                                         errCause,
                                         options
                                       );
  if ( res != Qgis::VectorExportResult::Success )
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
}

QString QgsOracleProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  dsUri.setGeometryColumn( tableInfo.geometryColumn() );
  return dsUri.uri( false );
}


QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsOracleProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables;

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  const bool geometryColumnsOnly { configuration().value( "geometryColumnsOnly", false ).toBool() };
  const bool userTablesOnly { configuration().value( "userTablesOnly", false ).toBool() &&schema.isEmpty() };
  const bool onlyExistingTypes { configuration().value( "onlyExistingTypes", false ).toBool() };
  const bool aspatial { ! flags || flags.testFlag( TableFlag::Aspatial ) };

  QVector<QgsOracleLayerProperty> properties;
  const bool ok = conn->supportedLayers( properties, schema, geometryColumnsOnly, userTablesOnly, aspatial );
  if ( ! ok )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve tables: %1" ).arg( uri() ) );
  }

  for ( auto &pr : properties )
  {
    // Classify
    TableFlags prFlags;
    if ( pr.isView )
    {
      prFlags.setFlag( QgsAbstractDatabaseProviderConnection::TableFlag::View );
    }
    if ( !pr.geometryColName.isEmpty() )
    {
      prFlags.setFlag( QgsAbstractDatabaseProviderConnection::TableFlag::Vector );
    }
    else
    {
      prFlags.setFlag( QgsAbstractDatabaseProviderConnection::TableFlag::Aspatial );
    }

    // Filter
    if ( flags && !( prFlags & flags ) )
      continue;

    // retrieve layer types if needed
    conn->retrieveLayerTypes( pr, dsUri.useEstimatedMetadata(), onlyExistingTypes );

    QgsAbstractDatabaseProviderConnection::TableProperty property;
    property.setFlags( prFlags );
    for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ) ; i++ )
    {
      property.addGeometryColumnType( pr.types.at( i ), QgsCoordinateReferenceSystem::fromEpsgId( pr.srids.at( i ) ) );
    }
    property.setTableName( pr.tableName );
    property.setSchema( pr.ownerName );
    property.setGeometryColumn( pr.geometryColName );
    property.setGeometryColumnCount( ( prFlags & QgsAbstractDatabaseProviderConnection::TableFlag::Aspatial ) ? 0 : 1 );
    property.setPrimaryKeyColumns( pr.isView ? pr.pkCols : conn->getPrimaryKeys( pr.ownerName, pr.tableName ) );

    tables.push_back( property );
  }

  return tables;
}

void QgsOracleProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  executeSqlPrivate( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsOracleConn::quotedIdentifier( schema ) )
                     .arg( QgsOracleConn::quotedIdentifier( name ) ) );

  executeSqlPrivate( QStringLiteral( "DELETE FROM user_sdo_geom_metadata WHERE TABLE_NAME = '%1'" )
                     .arg( name ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOracleProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql, feedback );
}

void QgsOracleProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsOracleConn::quotedIdentifier( schema ),
                           QgsOracleConn::quotedIdentifier( name ),
                           QgsOracleConn::quotedIdentifier( newName ) ) );

  executeSqlPrivate( QStringLiteral( "UPDATE user_sdo_geom_metadata SET TABLE_NAME = '%1' where TABLE_NAME = '%2'" )
                     .arg( newName, name ) );
}

void QgsOracleProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsOracleProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  const QString indexName = conn->createSpatialIndex( schema, name, options.geometryColumnName );
  if ( indexName.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "Failed to create spatial index for %1.%2(%3)" ).arg( schema, name, options.geometryColumnName ) );
}

void QgsOracleProviderConnection::deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  bool isValid;
  const QString indexName = conn->getSpatialIndexName( schema, name, geometryColumn, isValid );

  if ( indexName.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "No spatial index exists for %1.%2(%3)" ).arg( schema, name, geometryColumn ) );

  executeSqlPrivate( QStringLiteral( "DROP INDEX %1" ).arg( indexName ) );
}

bool QgsOracleProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::SpatialIndexExists );

  const QgsDataSourceUri dsUri( uri() );
  QgsPoolOracleConn pconn( dsUri.connectionInfo( false ) );
  QgsOracleConn *conn = pconn.get();
  if ( !conn )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  bool isValid;
  conn->getSpatialIndexName( schema, name, geometryColumn, isValid );
  return isValid;
}

QIcon QgsOracleProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconOracle.svg" ) );
}

QStringList QgsOracleProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;

  QList<QVariantList> users = executeSqlPrivate( QStringLiteral( "SELECT USERNAME FROM ALL_USERS" ) ).rows();
  for ( QVariantList userInfos : users )
    schemas << userInfos.at( 0 ).toString();

  return schemas;
}
