/***************************************************************************
    qgsdamengproviderconnection.cpp - QgsDamengProviderConnection
                        ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengproviderconnection.h"
#include "qgsdamengconn.h"
#include "qgsdamengconnpool.h"
#include "qgssettings.h"
#include "qgsdamengprovider.h"
#include "qgsexception.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"
#include <QRegularExpression>
#include <QIcon>

#include <chrono>

 // From configuration
const QStringList QgsDamengProviderConnection::CONFIGURATION_PARAMETERS = {
  QStringLiteral( "sysdbaOnly" ),
  QStringLiteral( "dontResolveType" ),
  QStringLiteral( "allowGeometrylessTables" ),
  QStringLiteral( "saveUsername" ),
  QStringLiteral( "savePassword" ),
  QStringLiteral( "estimatedMetadata" ),
  QStringLiteral( "projectsInDatabase" ),
};

const QString QgsDamengProviderConnection::SETTINGS_BASE_KEY = QStringLiteral( "/Dameng/connections/" );

QgsDamengProviderConnection::QgsDamengProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "dameng" );
  // Remove the sql and table empty parts
  const QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*)raw" };
  setUri( QgsDamengConn::connUri( name ).uri().replace( removePartsRe, QString() ) );

  QgsSettings settings;
  settings.beginGroup( SETTINGS_BASE_KEY );
  settings.beginGroup( name );

  QVariantMap config;

  for ( const QString &p : std::as_const( CONFIGURATION_PARAMETERS ) )
  {
    const QVariant val = settings.value( p );
    if ( val.isValid() )
    {
      config.insert( p, val );
    }
  }

  settings.endGroup();
  settings.endGroup();

  setConfiguration( config );
  setDefaultCapabilities();
}

QgsDamengProviderConnection::QgsDamengProviderConnection( const QString &uri, const QVariantMap &configuration ) :
  QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = QStringLiteral( "dameng" );
  setDefaultCapabilities();
}

void QgsDamengProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
  {
    Capability::DropVectorTable,
    Capability::CreateVectorTable,
    Capability::RenameSchema,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::RenameVectorTable,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    //Capability::Transaction,
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
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePart,
    GeometryColumnCapability::Curves
  };
  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::PrimaryKeys,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn,
    Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds,
  };
}

void QgsDamengProviderConnection::store( const QString &name ) const
{
  // TODO: move this to class configuration?
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( SETTINGS_BASE_KEY );
  settings.beginGroup( name );

  // From URI
  const QgsDataSourceUri dsUri { uri() };
  settings.setValue( "host", dsUri.host() );
  settings.setValue( "port", dsUri.port() );
  settings.setValue( "database", dsUri.database() );
  settings.setValue( "username", dsUri.username() );
  settings.setValue( "password", dsUri.password() );
  settings.setValue( "authcfg", dsUri.authConfigId() );

  for ( const auto &p : std::as_const( CONFIGURATION_PARAMETERS ) )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) );
    }
  }
  settings.endGroup();
  settings.endGroup();
}

void QgsDamengProviderConnection::remove( const QString &name ) const
{
  QgsDamengConn::deleteConnection( name );
}


void QgsDamengProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant>* options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri { uri() };
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column if it's not aspatial
  if ( wkbType != Qgis::WkbType::Unknown && wkbType != Qgis::WkbType::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "GEOM" ) ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult res = QgsDamengProvider::createEmptyLayer(
    newUri.uri(),
    fields,
    wkbType,
    srs,
    overwrite,
    &map,
    &errCause,
    options
  );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QgsVectorLayer *QgsDamengProviderConnection::createSqlVectorLayer( const SqlVectorLayerOptions &options ) const
{
  // Precondition
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  QgsDataSourceUri tUri( uri() );

  tUri.setSql( options.filter );
  tUri.disableSelectAtId( options.disableSelectAtId );

  if ( !options.primaryKeyColumns.isEmpty() )
  {
    tUri.setKeyColumn( options.primaryKeyColumns.join( ',' ) );
    tUri.setTable( QStringLiteral( "(%1)" ).arg( options.sql ) );
  }
  else
  {
    int pkId { 0 };
    while ( options.sql.contains( QStringLiteral( "_rowid%1_" ).arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId++;
    }
    tUri.setKeyColumn( QStringLiteral( "rowid" ) );

    int sqlId { 0 };
    while ( options.sql.contains( QStringLiteral( "_subq_%1_" ).arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId++;
    }
    tUri.setTable( QStringLiteral( "( SELECT cast( ROWID as bigint ) AS _rowid%1_, * FROM (%2\n ) AS _subq_%3_\n )" ).arg( QString::number( pkId ), options.sql, QString::number( sqlId ) ) );
  }

  if ( !options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  QgsVectorLayer::LayerOptions vectorLayerOptions { false, true };
  vectorLayerOptions.skipCrsValidation = true;
  return new QgsVectorLayer { tUri.uri(), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey(), vectorLayerOptions };
}

QString QgsDamengProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  return dsUri.uri( false );
}

void QgsDamengProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  executeSqlPrivate( QStringLiteral( "DROP TABLE if exists %1.%2" )
    .arg( QgsDamengConn::quotedIdentifier( schema ), QgsDamengConn::quotedIdentifier( name ) ) );
}

void QgsDamengProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  dropTablePrivate( schema, name );
}


void QgsDamengProviderConnection::renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const
{
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
    .arg( QgsDamengConn::quotedIdentifier( schema ), QgsDamengConn::quotedIdentifier( name ), QgsDamengConn::quotedIdentifier( newName ) ) );
}

void QgsDamengProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  renameTablePrivate( schema, name, newName );
}


void QgsDamengProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsDamengConn::quotedIdentifier( name ) ) );

}

void QgsDamengProviderConnection::dropSchema( const QString &name, bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlPrivate( QStringLiteral( "DROP SCHEMA %1 %2" )
    .arg( QgsDamengConn::quotedIdentifier( name ), force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsDamengProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return execSqlPrivate( sql, true, feedback );
}

QList<QVariantList> QgsDamengProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolDamengConn> dmconn ) const
{
  return execSqlPrivate( sql, resolveTypes, feedback, dmconn ).rows();
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsDamengProviderConnection::execSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolDamengConn> dmconn ) const
{
  if ( !dmconn )
  {
    dmconn = std::make_shared<QgsPoolDamengConn>( QgsDataSourceUri( uri() ).connectionInfo( false ) );
  }

  std::shared_ptr<QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator> iterator = std::make_shared<QgsDamengProviderResultIterator>( resolveTypes );
  QueryResult results( iterator );

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
  {
    return results;
  }

  QgsDamengConn *conn = dmconn->get();

  if ( !conn )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }
  else
  {
    if ( feedback && feedback->isCanceled() )
      return results;

    QMetaObject::Connection qtConnection;
    if ( feedback )
    {
      qtConnection = QObject::connect( feedback, &QgsFeedback::canceled, [ &dmconn ]
        {
          if ( dmconn )
            dmconn->get()->DMCancel();
        } );
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::unique_ptr<QgsDamengResult> res = std::make_unique<QgsDamengResult>( conn->LoggedDMexec( "QgsDamengProviderConnection", sql ) );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );

    if ( feedback )
    {
      QObject::disconnect( qtConnection );
    }

    QString errCause;
    if ( !conn->DMconnStatus() || !res->result() )
    {
      errCause = QObject::tr( "Connection error: %1 returned %2 [%3]" )
                  .arg( sql )
                  .arg( conn->DMconnStatus() )
                  .arg( conn->DMconnErrorMessage() );
    }
    else
    {
      const QString err { res->DMresultErrorMessage() };
      if ( ! err.isEmpty() )
      {
        errCause = QObject::tr( "SQL error: %1 returned %2 [%3]" )
                   .arg( sql )
                   .arg( conn->DMconnStatus() )
                   .arg( err );
      }
    }

    if ( !errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }

    QgsDMResult *result = res->result();
    if ( result->ntuples() > 0 )
    {
      // Get column names
      const int numFields { result->nfields() };
      for ( int rowIdx = 0; rowIdx < numFields; rowIdx++)
      {
        results.appendColumn( res->DMfname( rowIdx ) );
      }

      // Try to convert value types at least for basic simple types that can be directly mapped to Python
      if ( resolveTypes )
      {
        // Collect oids
        QStringList oids;
        oids.reserve( numFields );
        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++)
        {
          if ( feedback && feedback->isCanceled() )
          {
            break;
          }
          const QString oidStr { QString::number( res->DMftype( rowIdx ) ) };
          oids.push_back( oidStr );
        }

        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++ )
        {
          static const QStringList intTypes = {
                                                QStringLiteral( "byte" ),
                                                QStringLiteral( "tinyint" ),
                                                QStringLiteral( "smallint" ),
                                                QStringLiteral( "int" ),
                                                QStringLiteral( "bigint" ),
                                              };
          static const QStringList floatTypes = {
                                                  QStringLiteral( "float" ),
                                                  QStringLiteral( "double" ),
                                                  QStringLiteral( "real" ),
                                                  QStringLiteral( "numeric" ),
                                                  QStringLiteral( "decimal" )
                                                };
          static const QStringList stringTypes = {
                                                   QStringLiteral( "char" ),
                                                   QStringLiteral( "varchar" ),
                                                   QStringLiteral( "text" )
                                                 };
          static const QStringList byteTypes = {
                                                 QStringLiteral( "binary" ),
                                                 QStringLiteral( "varbinary" ),
                                                 QStringLiteral( "image" )
                                               };

          const QString typName { res->DMftypeName( rowIdx, res->DMftype( rowIdx ) ) };
          QMetaType::Type vType { QMetaType::Type::QString };
          if ( floatTypes.contains( typName ) )
          {
            vType = QMetaType::Type::Double;
          }
          else if ( intTypes.contains( typName ) )
          {
            vType = QMetaType::Type::LongLong;
          }
          else if ( stringTypes.contains( typName ) )
          {
            vType = QMetaType::Type::QString;
          }
          else if ( byteTypes.contains( typName ) )
          {
            vType = QMetaType::Type::QByteArray;
          }
          else if ( typName == QLatin1String( "date" ) )
          {
            vType = QMetaType::Type::QDate;
          }
          else if ( typName.startsWith( QLatin1String( "timestamp" ) ) )
          {
            vType = QMetaType::Type::QDateTime;
          }
          else if ( typName == QLatin1String( "time" ) )
          {
            vType = QMetaType::Type::QTime;
          }
          else if ( typName == QLatin1String( "bit" ) )
          {
            vType = QMetaType::Type::Bool;
          }
          else
          {
            // Just a warning, usually ok
            QgsDebugMsgLevel( QStringLiteral( "Unhandled Dameng type %1, assuming string" ).arg( typName ), 2 );
          }
          static_cast<QgsDamengProviderResultIterator *>( iterator.get() )->typeMap[rowIdx] = vType;
        }
      }
    }
    if ( !errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }
    static_cast<QgsDamengProviderResultIterator *>( iterator.get() )->result = std::move( res );
  }
  return results;
}

QVariantList QgsDamengProviderResultIterator::nextRowPrivate()
{
  // Get results
  QVariantList row;

  if ( !result )
  {
    return row;
  }

  QgsDMResult *res = result->result();
  row.reserve( res->nfields() );

  for ( int col = 0; col < res->nfields(); ++col )
  {
    if ( mResolveTypes )
    {
      row.push_back( res->value( col ) );
    }
    else
    {
      row.push_back( res->value( col ).toString() );
    }
  }
  
  ++mRowIndex;

  return row;
}

bool QgsDamengProviderResultIterator::hasNextRowPrivate() const
{
  QgsDMResult *res;
  
  if ( result )
  {
    res = result->result();

    bool hasnext = res->fetchNext();
    if ( !hasnext )
      res->finish();

    return hasnext;
  }
  
  return false;
}

long long QgsDamengProviderResultIterator::rowCountPrivate() const
{
  return result ? result->DMntuples() : static_cast<long long>( Qgis::FeatureCountState::UnknownCount );
}


void QgsDamengProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );

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

  const QString indexName = QStringLiteral( "sidx_%1_%2" ).arg( name, geometryColumnName );
  executeSql( QStringLiteral( "CREATE SPATIAL INDEX %1 ON %2.%3(%4);" )
    .arg( QgsDamengConn::quotedIdentifier( indexName ), QgsDamengConn::quotedIdentifier( schema ), QgsDamengConn::quotedIdentifier( name ), QgsDamengConn::quotedIdentifier( geometryColumnName ) ) );
}

bool QgsDamengProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::SpatialIndexExists );

  const QList<QVariantList> res = executeSql( QStringLiteral( "select count(*) from ALL_IND_COLUMNS A,( select TYPE$,ID from SYSINDEXES ) S,( select ID,NAME from SYS.VSYSOBJECTS ) O "
                                                            " where A.TABLE_OWNER = %1 and A.TABLE_NAME = %2 and A.COLUMN_NAME = %3 and "
                                                            " S.TYPE$ = \'ST\' and A.index_name in O.NAME and S.ID = O.ID;"
                                                           ).arg(
                                                                QgsDamengConn::quotedValue( schema ),
                                                                QgsDamengConn::quotedValue( name ),
                                                                QgsDamengConn::quotedValue( geometryColumn ) ) );
  return !res.isEmpty() && !res.at( 0 ).isEmpty() && res.at( 0 ).at( 0 ).toBool();
}

void QgsDamengProviderConnection::deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::DeleteSpatialIndex );

  const QList<QVariantList> res = executeSql( QStringLiteral( "select A.index_name from ALL_IND_COLUMNS A,( select TYPE$,ID from SYS.VSYSINDEXES ) S,( select ID,NAME from SYS.VSYSOBJECTS ) O "
                                                            " where A.TABLE_OWNER = %1 and A.TABLE_NAME = %2 and A.COLUMN_NAME = %3 and "
                                                            " S.TYPE$ = \'ST\' and A.index_name in O.NAME and S.ID = O.ID;"
                                                           ).arg(
                                                            QgsDamengConn::quotedValue( schema ),
                                                            QgsDamengConn::quotedValue( name ),
                                                            QgsDamengConn::quotedValue( geometryColumn )
                                                           ) );
  if ( res.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "No spatial index exists for %1.%2" ).arg( schema, name ) );

  const QString indexName = res.at( 0 ).at( 0 ).toString();

  executeSql( QStringLiteral( "DROP INDEX %1.%2;" ).arg( QgsDamengConn::quotedIdentifier( schema ),
    QgsDamengConn::quotedIdentifier( indexName ) ) );
}

QStringList QgsDamengProviderConnection::schemas() const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;
  QString errCause;
  const QgsDataSourceUri dsUri { uri() };
  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    QList<QgsDamengSchemaProperty> schemaProperties;
    bool ok = conn->getSchemas( schemaProperties );
    QgsDamengConnPool::instance()->releaseConnection( conn );
    if ( !ok )
    {
      errCause = QObject::tr( "Could not retrieve schemas: %1" ).arg( uri() );
    }
    else
    {
      for ( const auto &s : std::as_const( schemaProperties ) )
      {
        schemas.push_back( s.name );
      }
    }
  }
  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( errCause );
  }
  return schemas;
}

QList<QgsDamengProviderConnection::TableProperty> QgsDamengProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback *feedback ) const
{
  return tablesPrivate( schema, QString(), flags, feedback );
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsDamengProviderConnection::table( const QString &schema, const QString &table, QgsFeedback *feedback ) const
{
  const QList<QgsDamengProviderConnection::TableProperty> properties { tablesPrivate( schema, table, TableFlags(), feedback ) };
  if ( !properties.empty() )
  {
    return properties.first();
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Table '%1' was not found in schema '%2'" )
                                            .arg( table, schema ) );
  }
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsDamengProviderConnection::tablesPrivate( const QString &schema, const QString &table, const TableFlags &flags, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );

  QList<QgsDamengProviderConnection::TableProperty> tables;
  QString errCause;

  // TODO: set flags from the connection if flags argument is 0
  const QgsDataSourceUri dsUri { uri() };
  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( feedback && feedback->isCanceled() )
    return {};

  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    bool ok { false };
    QVector<QgsDamengLayerProperty> properties;
    const bool aspatial { !flags || flags.testFlag( TableFlag::Aspatial ) };

    if ( !table.isEmpty() )
    {
      QgsDamengLayerProperty property;
      ok = conn->supportedLayer( property, schema, table );
      if ( ok )
      {
        properties.push_back( property );
      }
    }
    else
    {
      ok = conn->supportedLayers( properties, schema == QStringLiteral( "SYSDBA" ), aspatial, schema );
    }

    if ( !ok )
    {
      errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( uri() );
    }
    else
    {
      bool dontResolveType = configuration().value( QStringLiteral( "dontResolveType" ), false ).toBool();
      bool useEstimatedMetadata = configuration().value( QStringLiteral( "estimatedMetadata" ), false ).toBool();

      // Cannot be const:
      for ( auto &pr : properties )
      {
        // Classify
        TableFlags prFlags;
        if ( pr.isView )
        {
          prFlags.setFlag( QgsDamengProviderConnection::TableFlag::View );
        }
        if ( pr.isMaterializedView )
        {
          prFlags.setFlag( QgsDamengProviderConnection::TableFlag::MaterializedView );
        }
        if ( pr.nSpCols != 0 )
        {
          prFlags.setFlag( QgsDamengProviderConnection::TableFlag::Vector );
        }
        else
        {
          prFlags.setFlag( QgsDamengProviderConnection::TableFlag::Aspatial );
        }
        // Filter
        if ( !flags || ( prFlags & flags ) )
        {
          // retrieve layer types if needed
          if ( !dontResolveType && (!pr.geometryColName.isNull() &&
            ( pr.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown ||
              pr.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) ) )
          {
            conn->retrieveLayerTypes( pr, useEstimatedMetadata );
          }
          QgsDamengProviderConnection::TableProperty property;
          property.setFlags( prFlags );
          for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ); i++)
          {
            property.addGeometryColumnType( pr.types.at( i ), QgsCoordinateReferenceSystem::fromEpsgId( pr.srids.at( i ) ) );
          }
          property.setTableName( pr.tableName );
          property.setSchema( pr.schemaName );
          property.setGeometryColumn( pr.geometryColName );
          // These are candidates, not actual PKs
          // property.setPrimaryKeyColumns( pr.pkCols );
          property.setGeometryColumnCount( static_cast<int>( pr.nSpCols ) );
          property.setComment( pr.tableComment );

          // Get PKs
          if ( pr.isView || pr.isMaterializedView )
          {
            // Set the candidates
            property.setPrimaryKeyColumns( pr.pkCols );
          }
          else  // Fetch and set the real pks
          {
            try
            {
              QString sql = QStringLiteral( "select distinct( COLUMN_NAME ) from ALL_CONS_COLUMNS a "
                " where a.OWNER = \'%1\' and a.table_name = \'%2\' and exists("
                " select OWNER,TABLE_NAME,CONSTRAINT_TYPE from ALL_CONSTRAINTS u "
                " where u.OWNER = a.OWNER and u.table_name = a.table_name and "
                " ( CONSTRAINT_TYPE = \'P\' or CONSTRAINT_TYPE = \'U\') ); "
              ).arg( pr.schemaName, pr.tableName );
              const auto pks = execSql( sql ).rows();
              QStringList pkNames;
              for ( const auto &pk : std::as_const( pks ) )
              {
                pkNames.push_back( pk.first().toString() );
              }
              property.setPrimaryKeyColumns( pkNames );
            }
            catch ( const QgsProviderConnectionException &ex )
            {
              QgsDebugError( QStringLiteral( "Error retrieving primary keys: %1" ).arg( ex.what() ) );
            }
          }

          tables.push_back( property );
        }
      }
    }
    QgsDamengConnPool::instance()->releaseConnection( conn );
  }
  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( errCause );
  }
  return tables;
}

QgsFields QgsDamengProviderConnection::fields( const QString &schema, const QString &tableName, QgsFeedback *feedback ) const
{
  // Try the base implementation first and fall back to a more complex approach for the
  // few Dameng-specific corner cases that do not work with the base implementation.
  try
  {
    return QgsAbstractDatabaseProviderConnection::fields( schema, tableName, feedback );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    // This table might expose multiple geometry columns ( different geom type or SRID )
    // but we are only interested in fields here, so let's pick the first one.
    TableProperty tableInfo { table( schema, tableName, feedback ) };
    if ( feedback && feedback->isCanceled() )
      return QgsFields();

    try
    {
      QgsDataSourceUri tUri { tableUri( schema, tableName ) };

      if ( tableInfo.geometryColumnTypes().count() > 1 )
      {
        const auto geomColTypes( tableInfo.geometryColumnTypes() );
        TableProperty::GeometryColumnType geomCol { geomColTypes.first() };
        tUri.setGeometryColumn( tableInfo.geometryColumn() );
        tUri.setWkbType( geomCol.wkbType );
        tUri.setSrid( QString::number( geomCol.crs.postgisSrid() ) );
      }

      if ( tableInfo.primaryKeyColumns().count() > 0 )
      {
        const auto constPkCols( tableInfo.primaryKeyColumns() );
        tUri.setKeyColumn( constPkCols.first() );
      }

      tUri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ), QLatin1String( "0" ) );
      QgsVectorLayer::LayerOptions options { false, true };
      options.skipCrsValidation = true;

      QgsVectorLayer vl { tUri.uri(), QStringLiteral( "temp_layer" ), mProviderKey, options };

      if ( vl.isValid() )
      {
        return vl.fields();
      }
    }
    catch ( QgsProviderConnectionException&)
    {
      // fall-through
    }
    throw ex;
  }
}


QIcon QgsDamengProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconDameng.svg" ) );
}

QList<QgsVectorDataProvider::NativeType> QgsDamengProviderConnection::nativeTypes() const
{
  QList<QgsVectorDataProvider::NativeType> types;
  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( QgsDataSourceUri { uri() } .connectionInfo( false ) );
  if ( conn )
  {
    types = conn->nativeTypes();
    QgsDamengConnPool::instance()->releaseConnection( conn );
  }
  if ( types.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  }
  return types;
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsDamengProviderConnection::sqlOptions( const QString &layerSource )
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

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsDamengProviderConnection::sqlDictionary()
{
  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
    { { Qgis::SqlKeywordCategory::Keyword,
        {
          QStringLiteral( "ABORT" ),
          QStringLiteral( "ABSOLUTE" ),
          QStringLiteral( "ABSTRACT" ),
          QStringLiteral( "ACCESSED" ),
          QStringLiteral( "ACCOUNT" ),
          QStringLiteral( "ACROSS" ),
          QStringLiteral( "ACTION" ),
          QStringLiteral( "ADD" ),
          QStringLiteral( "ADMIN" ),
          QStringLiteral( "ADVANCED" ),
          QStringLiteral( "AFTER" ),
          QStringLiteral( "AGGREGATE" ),
          QStringLiteral( "ALL" ),
          QStringLiteral( "ALLOW_DATETIME" ),
          QStringLiteral( "ALLOW_IP" ),
          QStringLiteral( "ALTER" ),
          QStringLiteral( "ANALYZE" ),
          QStringLiteral( "AND" ),
          QStringLiteral( "ANY" ),
          QStringLiteral( "APPLY" ),
          QStringLiteral( "APR" ),
          QStringLiteral( "ARCHIVE" ),
          QStringLiteral( "ARCHIVEDIR" ),
          QStringLiteral( "ARCHIVELOG" ),
          QStringLiteral( "ARCHIVESTYLE" ),
          QStringLiteral( "ARRAY" ),
          QStringLiteral( "ARRAYLEN" ),
          QStringLiteral( "AS" ),
          QStringLiteral( "ASC" ),
          QStringLiteral( "ASCII" ),
          QStringLiteral( "ASENSITIVE" ),
          QStringLiteral( "ASSIGN" ),
          QStringLiteral( "ASYNCHRONOUS" ),
          QStringLiteral( "AT" ),
          QStringLiteral( "ATTACH" ),
          QStringLiteral( "AUDIT" ),
          QStringLiteral( "AUG" ),
          QStringLiteral( "AUTHID" ),
          QStringLiteral( "AUTHORIZATION" ),
          QStringLiteral( "AUTO" ),
          QStringLiteral( "AUTOEXTEND" ),
          QStringLiteral( "AUTONOMOUS_TRANSACTION" ),
          QStringLiteral( "AUTO_INCREMENT" ),
          QStringLiteral( "AVG" ),
          QStringLiteral( "BACKED" ),
          QStringLiteral( "BACKUP" ),
          QStringLiteral( "BACKUPDIR" ),
          QStringLiteral( "BACKUPINFO" ),
          QStringLiteral( "BACKUPNAME" ),
          QStringLiteral( "BACKUPSET" ),
          QStringLiteral( "BADFILE" ),
          QStringLiteral( "BAKFILE" ),
          QStringLiteral( "BASE" ),
          QStringLiteral( "BATCH" ),
          QStringLiteral( "BCT" ),
          QStringLiteral( "BEFORE" ),
          QStringLiteral( "BEGIN" ),
          QStringLiteral( "BETWEEN" ),
          QStringLiteral( "BIGDATEDIFF" ),
          QStringLiteral( "BIGINT" ),
          QStringLiteral( "BINARY" ),
          QStringLiteral( "BIT" ),
          QStringLiteral( "BITMAP" ),
          QStringLiteral( "BLOB" ),
          QStringLiteral( "BLOCK" ),
          QStringLiteral( "BOOL" ),
          QStringLiteral( "BOOLEAN" ),
          QStringLiteral( "BOTH" ),
          QStringLiteral( "BRANCH" ),
          QStringLiteral( "BREADTH" ),
          QStringLiteral( "BREAK" ),
          QStringLiteral( "BSTRING" ),
          QStringLiteral( "BTREE" ),
          QStringLiteral( "BUFFER" ),
          QStringLiteral( "BUILD" ),
          QStringLiteral( "BULK" ),
          QStringLiteral( "BY" ),
          QStringLiteral( "BYDAY" ),
          QStringLiteral( "BYHOUR" ),
          QStringLiteral( "BYMINUTE" ),
          QStringLiteral( "BYMONTH" ),
          QStringLiteral( "BYMONTHDAY" ),
          QStringLiteral( "BYSECOND" ),
          QStringLiteral( "BYTE" ),
          QStringLiteral( "BYWEEKNO" ),
          QStringLiteral( "BYYEARDAY" ),
          QStringLiteral( "CACHE" ),
          QStringLiteral( "CALCULATE" ),
          QStringLiteral( "CALL" ),
          QStringLiteral( "CASCADE" ),
          QStringLiteral( "CASCADED" ),
          QStringLiteral( "CASE" ),
          QStringLiteral( "CASE_SENSITIVE" ),
          QStringLiteral( "CAST" ),
          QStringLiteral( "CATALOG" ),
          QStringLiteral( "CATCH" ),
          QStringLiteral( "CHAIN" ),
          QStringLiteral( "CHANGE" ),
          QStringLiteral( "CHANNEL" ),
          QStringLiteral( "CHAR" ),
          QStringLiteral( "CHARACTER" ),
          QStringLiteral( "CHARACTERISTICS" ),
          QStringLiteral( "CHECK" ),
          QStringLiteral( "CHECKPOINT" ),
          QStringLiteral( "CIPHER" ),
          QStringLiteral( "CLASS" ),
          QStringLiteral( "CLEAR" ),
          QStringLiteral( "CLOB" ),
          QStringLiteral( "CLOSE" ),
          QStringLiteral( "CLUSTER" ),
          QStringLiteral( "CLUSTERBTR" ),
          QStringLiteral( "COLLATE" ),
          QStringLiteral( "COLLATION" ),
          QStringLiteral( "COLLECT" ),
          QStringLiteral( "COLUMN" ),
          QStringLiteral( "COLUMNS" ),
          QStringLiteral( "COMMENT" ),
          QStringLiteral( "COMMIT" ),
          QStringLiteral( "COMMITTED" ),
          QStringLiteral( "COMMITWORK" ),
          QStringLiteral( "COMPILE" ),
          QStringLiteral( "COMPLETE" ),
          QStringLiteral( "COMPRESS" ),
          QStringLiteral( "COMPRESSED" ),
          QStringLiteral( "CONDITIONAL" ),
          QStringLiteral( "CONFIGURE" ),
          QStringLiteral( "CONNECT" ),
          QStringLiteral( "CONNECT_BY_ISCYCLE" ),
          QStringLiteral( "CONNECT_BY_ISLEAF" ),
          QStringLiteral( "CONNECT_BY_ROOT" ),
          QStringLiteral( "CONNECT_IDLE_TIME" ),
          QStringLiteral( "CONNECT_TIME" ),
          QStringLiteral( "CONST" ),
          QStringLiteral( "CONSTANT" ),
          QStringLiteral( "CONSTRAINT" ),
          QStringLiteral( "CONSTRAINTS" ),
          QStringLiteral( "CONSTRUCTOR" ),
          QStringLiteral( "CONTAINS" ),
          QStringLiteral( "CONTENT" ),
          QStringLiteral( "CONTEXT" ),
          QStringLiteral( "CONTINUE" ),
          QStringLiteral( "CONVERT" ),
          QStringLiteral( "COPY" ),
          QStringLiteral( "CORRESPONDING" ),
          QStringLiteral( "CORRUPT" ),
          QStringLiteral( "COUNT" ),
          QStringLiteral( "COUNTER" ),
          QStringLiteral( "CPU_PER_CALL" ),
          QStringLiteral( "CPU_PER_SESSION" ),
          QStringLiteral( "CRC" ),
          QStringLiteral( "CREATE" ),
          QStringLiteral( "CREATION" ),
          QStringLiteral( "CROSS" ),
          QStringLiteral( "CRYPTO" ),
          QStringLiteral( "CTLFILE" ),
          QStringLiteral( "CUBE" ),
          QStringLiteral( "CUMULATIVE" ),
          QStringLiteral( "CURRENT" ),
          QStringLiteral( "CURRENT_SCHEMA" ),
          QStringLiteral( "CURRENT_USER" ),
          QStringLiteral( "CURSOR" ),
          QStringLiteral( "CYCLE" ),
          QStringLiteral( "DAILY" ),
          QStringLiteral( "DANGLING" ),
          QStringLiteral( "DATA" ),
          QStringLiteral( "DATABASE" ),
          QStringLiteral( "DATAFILE" ),
          QStringLiteral( "DATE" ),
          QStringLiteral( "DATEADD" ),
          QStringLiteral( "DATEDIFF" ),
          QStringLiteral( "DATEPART" ),
          QStringLiteral( "DATETIME" ),
          QStringLiteral( "DAY" ),
          QStringLiteral( "DBA_RECYCLEBIN" ),
          QStringLiteral( "DBFILE" ),
          QStringLiteral( "DB_FILE_NAME_CONVERT" ),
          QStringLiteral( "DDL" ),
          QStringLiteral( "DDL_CLONE" ),
          QStringLiteral( "DEBUG" ),
          QStringLiteral( "DEC" ),
          QStringLiteral( "DECIMAL" ),
          QStringLiteral( "DECLARE" ),
          QStringLiteral( "DECODE" ),
          QStringLiteral( "DEFAULT" ),
          QStringLiteral( "DEFAULTS" ),
          QStringLiteral( "DEFERRABLE" ),
          QStringLiteral( "DEFERRED" ),
          QStringLiteral( "DEFINER" ),
          QStringLiteral( "DELETE" ),
          QStringLiteral( "DELETING" ),
          QStringLiteral( "DELIMITED" ),
          QStringLiteral( "DELTA" ),
          QStringLiteral( "DEMAND" ),
          QStringLiteral( "DENSE_RANK" ),
          QStringLiteral( "DEPTH" ),
          QStringLiteral( "DEREF" ),
          QStringLiteral( "DESC" ),
          QStringLiteral( "DESTINATION" ),
          QStringLiteral( "DETACH" ),
          QStringLiteral( "DETERMINISTIC" ),
          QStringLiteral( "DEVICE" ),
          QStringLiteral( "DIAGNOSTICS" ),
          QStringLiteral( "DICTIONARY" ),
          QStringLiteral( "DIRECTORY" ),
          QStringLiteral( "DISABLE" ),
          QStringLiteral( "DISCONNECT" ),
          QStringLiteral( "DISKGROUP" ),
          QStringLiteral( "DISKSPACE" ),
          QStringLiteral( "DISTINCT" ),
          QStringLiteral( "DISTRIBUTED" ),
          QStringLiteral( "DML" ),
          QStringLiteral( "DO" ),
          QStringLiteral( "DOCUMENT" ),
          QStringLiteral( "DOMAIN" ),
          QStringLiteral( "DOUBLE" ),
          QStringLiteral( "DOWN" ),
          QStringLiteral( "DROP" ),
          QStringLiteral( "DUMP" ),
          QStringLiteral( "DUPLICATE" ),
          QStringLiteral( "EACH" ),
          QStringLiteral( "EDITIONABLE" ),
          QStringLiteral( "ELSE" ),
          QStringLiteral( "ELSEIF" ),
          QStringLiteral( "ELSIF" ),
          QStringLiteral( "EMPTY" ),
          QStringLiteral( "ENABLE" ),
          QStringLiteral( "ENCLOSED" ),
          QStringLiteral( "ENCODING" ),
          QStringLiteral( "ENCRYPT" ),
          QStringLiteral( "ENCRYPTION" ),
          QStringLiteral( "END" ),
          QStringLiteral( "EQU" ),
          QStringLiteral( "ERROR" ),
          QStringLiteral( "ERRORS" ),
          QStringLiteral( "ESCAPE" ),
          QStringLiteral( "ESCAPED" ),
          QStringLiteral( "EVALNAME" ),
          QStringLiteral( "EVENTINFO" ),
          QStringLiteral( "EVENTS" ),
          QStringLiteral( "EXCEPT" ),
          QStringLiteral( "EXCEPTION" ),
          QStringLiteral( "EXCEPTIONS" ),
          QStringLiteral( "EXCEPTION_INIT" ),
          QStringLiteral( "EXCHANGE" ),
          QStringLiteral( "EXCLUDE" ),
          QStringLiteral( "EXCLUDING" ),
          QStringLiteral( "EXCLUSIVE" ),
          QStringLiteral( "EXEC" ),
          QStringLiteral( "EXECUTE" ),
          QStringLiteral( "EXISTS" ),
          QStringLiteral( "EXIT" ),
          QStringLiteral( "EXPIRE" ),
          QStringLiteral( "EXPLAIN" ),
          QStringLiteral( "EXTENDS" ),
          QStringLiteral( "EXTERN" ),
          QStringLiteral( "EXTERNAL" ),
          QStringLiteral( "EXTERNALLY" ),
          QStringLiteral( "EXTRACT" ),
          QStringLiteral( "FAILED_LOGIN_ATTEMPS" ),
          QStringLiteral( "FAILED_LOGIN_ATTEMPTS" ),
          QStringLiteral( "FAST" ),
          QStringLiteral( "FEB" ),
          QStringLiteral( "FETCH" ),
          QStringLiteral( "FIELDS" ),
          QStringLiteral( "FILE" ),
          QStringLiteral( "FILEGROUP" ),
          QStringLiteral( "FILESIZE" ),
          QStringLiteral( "FILLFACTOR" ),
          QStringLiteral( "FINAL" ),
          QStringLiteral( "FINALLY" ),
          QStringLiteral( "FIRST" ),
          QStringLiteral( "FLASHBACK" ),
          QStringLiteral( "FLOAT" ),
          QStringLiteral( "FOLLOWING" ),
          QStringLiteral( "FOLLOWS" ),
          QStringLiteral( "FOR" ),
          QStringLiteral( "FORALL" ),
          QStringLiteral( "FORCE" ),
          QStringLiteral( "FOREIGN" ),
          QStringLiteral( "FORMAT" ),
          QStringLiteral( "FREQ" ),
          QStringLiteral( "FREQUENCE" ),
          QStringLiteral( "FRI" ),
          QStringLiteral( "FROM" ),
          QStringLiteral( "FULL" ),
          QStringLiteral( "FULLY" ),
          QStringLiteral( "FUNCTION" ),
          QStringLiteral( "GENERATED" ),
          QStringLiteral( "GET" ),
          QStringLiteral( "GLOBAL" ),
          QStringLiteral( "GLOBALLY" ),
          QStringLiteral( "GOTO" ),
          QStringLiteral( "GRANT" ),
          QStringLiteral( "GREAT" ),
          QStringLiteral( "GROUP" ),
          QStringLiteral( "GROUPING" ),
          QStringLiteral( "HASH" ),
          QStringLiteral( "HASHPARTMAP" ),
          QStringLiteral( "HAVING" ),
          QStringLiteral( "HEXTORAW" ),
          QStringLiteral( "HIDE" ),
          QStringLiteral( "HIGH" ),
          QStringLiteral( "HOLD" ),
          QStringLiteral( "HOUR" ),
          QStringLiteral( "HOURLY" ),
          QStringLiteral( "HUGE" ),
          QStringLiteral( "IDENTIFIED" ),
          QStringLiteral( "IDENTIFIER" ),
          QStringLiteral( "IDENTITY" ),
          QStringLiteral( "IDENTITY_INSERT" ),
          QStringLiteral( "IF" ),
          QStringLiteral( "IFNULL" ),
          QStringLiteral( "IGNORE_ROW_ON_DUPKEY_INDEX" ),
          QStringLiteral( "IMAGE" ),
          QStringLiteral( "IMMEDIATE" ),
          QStringLiteral( "IN" ),
          QStringLiteral( "INCLUDE" ),
          QStringLiteral( "INCLUDING" ),
          QStringLiteral( "INCREASE" ),
          QStringLiteral( "INCREMENT" ),
          QStringLiteral( "INDENT" ),
          QStringLiteral( "INDEX" ),
          QStringLiteral( "INDEXES" ),
          QStringLiteral( "INDICES" ),
          QStringLiteral( "INITIAL" ),
          QStringLiteral( "INITIALIZED" ),
          QStringLiteral( "INITIALLY" ),
          QStringLiteral( "INLINE" ),
          QStringLiteral( "INNER" ),
          QStringLiteral( "INNERID" ),
          QStringLiteral( "INPUT" ),
          QStringLiteral( "INSENSITIVE" ),
          QStringLiteral( "INSERT" ),
          QStringLiteral( "INSERTING" ),
          QStringLiteral( "INSTANCE" ),
          QStringLiteral( "INSTANTIABLE" ),
          QStringLiteral( "INSTEAD" ),
          QStringLiteral( "INT" ),
          QStringLiteral( "INTEGER" ),
          QStringLiteral( "INTENT" ),
          QStringLiteral( "INTERSECT" ),
          QStringLiteral( "INTERVAL" ),
          QStringLiteral( "INTO" ),
          QStringLiteral( "INVISIBLE" ),
          QStringLiteral( "IS" ),
          QStringLiteral( "ISOLATION" ),
          QStringLiteral( "JAN" ),
          QStringLiteral( "JAVA" ),
          QStringLiteral( "JOB" ),
          QStringLiteral( "JOIN" ),
          QStringLiteral( "JSON" ),
          QStringLiteral( "JSON_TABLE" ),
          QStringLiteral( "JUL" ),
          QStringLiteral( "JUN" ),
          QStringLiteral( "KEEP" ),
          QStringLiteral( "KEY" ),
          QStringLiteral( "KEYS" ),
          QStringLiteral( "LABEL" ),
          QStringLiteral( "LARGE" ),
          QStringLiteral( "LAST" ),
          QStringLiteral( "LAX" ),
          QStringLiteral( "LEADING" ),
          QStringLiteral( "LEFT" ),
          QStringLiteral( "LEFTARG" ),
          QStringLiteral( "LESS" ),
          QStringLiteral( "LEVEL" ),
          QStringLiteral( "LEVELS" ),
          QStringLiteral( "LEXER" ),
          QStringLiteral( "LIKE" ),
          QStringLiteral( "LIMIT" ),
          QStringLiteral( "LINES" ),
          QStringLiteral( "LINK" ),
          QStringLiteral( "LIST" ),
          QStringLiteral( "LNNVL" ),
          QStringLiteral( "LOB" ),
          QStringLiteral( "LOCAL" ),
          QStringLiteral( "LOCALLY" ),
          QStringLiteral( "LOCAL_OBJECT" ),
          QStringLiteral( "LOCATION" ),
          QStringLiteral( "LOCK" ),
          QStringLiteral( "LOCKED" ),
          QStringLiteral( "LOG" ),
          QStringLiteral( "LOGFILE" ),
          QStringLiteral( "LOGGING" ),
          QStringLiteral( "LOGIN" ),
          QStringLiteral( "LOGOFF" ),
          QStringLiteral( "LOGON" ),
          QStringLiteral( "LOGOUT" ),
          QStringLiteral( "LONG" ),
          QStringLiteral( "LONGVARBINARY" ),
          QStringLiteral( "LONGVARCHAR" ),
          QStringLiteral( "LOOP" ),
          QStringLiteral( "LSN" ),
          QStringLiteral( "MANUAL" ),
          QStringLiteral( "MAP" ),
          QStringLiteral( "MAPPED" ),
          QStringLiteral( "MAR" ),
          QStringLiteral( "MATCH" ),
          QStringLiteral( "MATCHED" ),
          QStringLiteral( "MATERIALIZED" ),
          QStringLiteral( "MAX" ),
          QStringLiteral( "MAXPIECESIZE" ),
          QStringLiteral( "MAXSIZE" ),
          QStringLiteral( "MAXVALUE" ),
          QStringLiteral( "MAX_RUN_DURATION" ),
          QStringLiteral( "MAY" ),
          QStringLiteral( "MEMBER" ),
          QStringLiteral( "MEMORY" ),
          QStringLiteral( "MEM_SPACE" ),
          QStringLiteral( "MERGE" ),
          QStringLiteral( "MICRO" ),
          QStringLiteral( "MIN" ),
          QStringLiteral( "MINEXTENTS" ),
          QStringLiteral( "MINUS" ),
          QStringLiteral( "MINUTE" ),
          QStringLiteral( "MINUTELY" ),
          QStringLiteral( "MINVALUE" ),
          QStringLiteral( "MIRROR" ),
          QStringLiteral( "MOD" ),
          QStringLiteral( "MODE" ),
          QStringLiteral( "MODIFY" ),
          QStringLiteral( "MON" ),
          QStringLiteral( "MONEY" ),
          QStringLiteral( "MONITORING" ),
          QStringLiteral( "MONTH" ),
          QStringLiteral( "MONTHLY" ),
          QStringLiteral( "MOUNT" ),
          QStringLiteral( "MOVE" ),
          QStringLiteral( "MOVEMENT" ),
          QStringLiteral( "MULTISET" ),
          QStringLiteral( "NATIONAL" ),
          QStringLiteral( "NATURAL" ),
          QStringLiteral( "NCHAR" ),
          QStringLiteral( "NCHARACTER" ),
          QStringLiteral( "NEVER" ),
          QStringLiteral( "NEW" ),
          QStringLiteral( "NEXT" ),
          QStringLiteral( "NO" ),
          QStringLiteral( "NOARCHIVELOG" ),
          QStringLiteral( "NOAUDIT" ),
          QStringLiteral( "NOBRANCH" ),
          QStringLiteral( "NOCACHE" ),
          QStringLiteral( "NOCOPY" ),
          QStringLiteral( "NOCYCLE" ),
          QStringLiteral( "NODE" ),
          QStringLiteral( "NOLOGGING" ),
          QStringLiteral( "NOMAXVALUE" ),
          QStringLiteral( "NOMINVALUE" ),
          QStringLiteral( "NOMONITORING" ),
          QStringLiteral( "NONE" ),
          QStringLiteral( "NONEDITIONABLE" ),
          QStringLiteral( "NOORDER" ),
          QStringLiteral( "NOPARALLEL" ),
          QStringLiteral( "NORMAL" ),
          QStringLiteral( "NOROWDEPENDENCIES" ),
          QStringLiteral( "NOSORT" ),
          QStringLiteral( "NOT" ),
          QStringLiteral( "NOT_ALLOW_DATETIME" ),
          QStringLiteral( "NOT_ALLOW_IP" ),
          QStringLiteral( "NOV" ),
          QStringLiteral( "NOVALIDATE" ),
          QStringLiteral( "NOWAIT" ),
          QStringLiteral( "NULL" ),
          QStringLiteral( "NULLS" ),
          QStringLiteral( "NUMBER" ),
          QStringLiteral( "NUMERIC" ),
          QStringLiteral( "OBJECT" ),
          QStringLiteral( "OCT" ),
          QStringLiteral( "OF" ),
          QStringLiteral( "OFF" ),
          QStringLiteral( "OFFLINE" ),
          QStringLiteral( "OFFSET" ),
          QStringLiteral( "OIDINDEX" ),
          QStringLiteral( "OLD" ),
          QStringLiteral( "ON" ),
          QStringLiteral( "ONCE" ),
          QStringLiteral( "ONLINE" ),
          QStringLiteral( "ONLY" ),
          QStringLiteral( "OPEN" ),
          QStringLiteral( "OPERATOR" ),
          QStringLiteral( "OPTIMIZE" ),
          QStringLiteral( "OPTION" ),
          QStringLiteral( "OPTIONALLY" ),
          QStringLiteral( "OR" ),
          QStringLiteral( "ORDER" ),
          QStringLiteral( "ORDINALITY" ),
          QStringLiteral( "OUT" ),
          QStringLiteral( "OUTER" ),
          QStringLiteral( "OUTFILE" ),
          QStringLiteral( "OVER" ),
          QStringLiteral( "OVERLAPS" ),
          QStringLiteral( "OVERLAY" ),
          QStringLiteral( "OVERRIDE" ),
          QStringLiteral( "OVERRIDING" ),
          QStringLiteral( "PACKAGE" ),
          QStringLiteral( "PAD" ),
          QStringLiteral( "PAGE" ),
          QStringLiteral( "PARALLEL" ),
          QStringLiteral( "PARALLEL_ENABLE" ),
          QStringLiteral( "PARMS" ),
          QStringLiteral( "PARTIAL" ),
          QStringLiteral( "PARTITION" ),
          QStringLiteral( "PARTITIONS" ),
          QStringLiteral( "PASSING" ),
          QStringLiteral( "PASSWORD" ),
          QStringLiteral( "PASSWORD_GRACE_TIME" ),
          QStringLiteral( "PASSWORD_LIFE_TIME" ),
          QStringLiteral( "PASSWORD_LOCK_TIME" ),
          QStringLiteral( "PASSWORD_POLICY" ),
          QStringLiteral( "PASSWORD_REUSE_MAX" ),
          QStringLiteral( "PASSWORD_REUSE_TIME" ),
          QStringLiteral( "PATH" ),
          QStringLiteral( "PENDANT" ),
          QStringLiteral( "PERCENT" ),
          QStringLiteral( "PIPE" ),
          QStringLiteral( "PIPELINED" ),
          QStringLiteral( "PIVOT" ),
          QStringLiteral( "PLACING" ),
          QStringLiteral( "PLS_INTEGER" ),
          QStringLiteral( "PRAGMA" ),
          QStringLiteral( "PREBUILT" ),
          QStringLiteral( "PRECEDES" ),
          QStringLiteral( "PRECEDING" ),
          QStringLiteral( "PRECISION" ),
          QStringLiteral( "PRESERVE" ),
          QStringLiteral( "PRETTY" ),
          QStringLiteral( "PRIMARY" ),
          QStringLiteral( "PRINT" ),
          QStringLiteral( "PRIOR" ),
          QStringLiteral( "PRIVATE" ),
          QStringLiteral( "PRIVILEGE" ),
          QStringLiteral( "PRIVILEGES" ),
          QStringLiteral( "PROCEDURE" ),
          QStringLiteral( "PROFILE" ),
          QStringLiteral( "PROTECTED" ),
          QStringLiteral( "PUBLIC" ),
          QStringLiteral( "PURGE" ),
          QStringLiteral( "QUERY" ),
          QStringLiteral( "QUERY_REWRITE_INTEGRITY" ),
          QStringLiteral( "QUOTA" ),
          QStringLiteral( "RAISE" ),
          QStringLiteral( "RANDOMLY" ),
          QStringLiteral( "RANGE" ),
          QStringLiteral( "RAWTOHEX" ),
          QStringLiteral( "READ" ),
          QStringLiteral( "READONLY" ),
          QStringLiteral( "READ_PER_CALL" ),
          QStringLiteral( "READ_PER_SESSION" ),
          QStringLiteral( "REAL" ),
          QStringLiteral( "REBUILD" ),
          QStringLiteral( "RECORD" ),
          QStringLiteral( "RECORDS" ),
          QStringLiteral( "RECYCLEBIN" ),
          QStringLiteral( "REDUCED" ),
          QStringLiteral( "REF" ),
          QStringLiteral( "REFERENCE" ),
          QStringLiteral( "REFERENCES" ),
          QStringLiteral( "REFERENCING" ),
          QStringLiteral( "REFRESH" ),
          QStringLiteral( "REJECT" ),
          QStringLiteral( "RELATED" ),
          QStringLiteral( "RELATIVE" ),
          QStringLiteral( "RELEASE" ),
          QStringLiteral( "RENAME" ),
          QStringLiteral( "REPEAT" ),
          QStringLiteral( "REPEATABLE" ),
          QStringLiteral( "REPLACE" ),
          QStringLiteral( "REPLAY" ),
          QStringLiteral( "REPLICATE" ),
          QStringLiteral( "RESIZE" ),
          QStringLiteral( "RESTORE" ),
          QStringLiteral( "RESTRICT" ),
          QStringLiteral( "RESTRICT_REFERENCES" ),
          QStringLiteral( "RESULT" ),
          QStringLiteral( "RESULT_CACHE" ),
          QStringLiteral( "RETURN" ),
          QStringLiteral( "RETURNING" ),
          QStringLiteral( "REVERSE" ),
          QStringLiteral( "REVOKE" ),
          QStringLiteral( "RIGHT" ),
          QStringLiteral( "RIGHTARG" ),
          QStringLiteral( "ROLE" ),
          QStringLiteral( "ROLLBACK" ),
          QStringLiteral( "ROLLFILE" ),
          QStringLiteral( "ROLLUP" ),
          QStringLiteral( "ROOT" ),
          QStringLiteral( "ROW" ),
          QStringLiteral( "ROWCOUNT" ),
          QStringLiteral( "ROWDEPENDENCIES" ),
          QStringLiteral( "ROWID" ),
          QStringLiteral( "ROWNUM" ),
          QStringLiteral( "ROWS" ),
          QStringLiteral( "RULE" ),
          QStringLiteral( "SALT" ),
          QStringLiteral( "SAMPLE" ),
          QStringLiteral( "SAT" ),
          QStringLiteral( "SAVE" ),
          QStringLiteral( "SAVEPOINT" ),
          QStringLiteral( "SBT" ),
          QStringLiteral( "SBYTE" ),
          QStringLiteral( "SCHEMA" ),
          QStringLiteral( "SCHEMABINDING" ),
          QStringLiteral( "SCN" ),
          QStringLiteral( "SCOPE" ),
          QStringLiteral( "SCROLL" ),
          QStringLiteral( "SEALED" ),
          QStringLiteral( "SEARCH" ),
          QStringLiteral( "SECOND" ),
          QStringLiteral( "SECONDLY" ),
          QStringLiteral( "SECTION" ),
          QStringLiteral( "SEED" ),
          QStringLiteral( "SEGMENT" ),
          QStringLiteral( "SELECT" ),
          QStringLiteral( "SELF" ),
          QStringLiteral( "SENSITIVE" ),
          QStringLiteral( "SEP" ),
          QStringLiteral( "SEQUENCE" ),
          QStringLiteral( "SERERR" ),
          QStringLiteral( "SERIALIZABLE" ),
          QStringLiteral( "SERVER" ),
          QStringLiteral( "SESSION" ),
          QStringLiteral( "SESSION_PER_USER" ),
          QStringLiteral( "SET" ),
          QStringLiteral( "SETS" ),
          QStringLiteral( "SHADOW" ),
          QStringLiteral( "SHARE" ),
          QStringLiteral( "SHORT" ),
          QStringLiteral( "SHOW" ),
          QStringLiteral( "SHUTDOWN" ),
          QStringLiteral( "SIBLINGS" ),
          QStringLiteral( "SIMPLE" ),
          QStringLiteral( "SINCE" ),
          QStringLiteral( "SIZE" ),
          QStringLiteral( "SIZEOF" ),
          QStringLiteral( "SKIP" ),
          QStringLiteral( "SMALLINT" ),
          QStringLiteral( "SNAPSHOT" ),
          QStringLiteral( "SOME" ),
          QStringLiteral( "SOUND" ),
          QStringLiteral( "SPACE" ),
          QStringLiteral( "SPAN" ),
          QStringLiteral( "SPATIAL" ),
          QStringLiteral( "SPEED" ),
          QStringLiteral( "SPFILE" ),
          QStringLiteral( "SPLIT" ),
          QStringLiteral( "SQL" ),
          QStringLiteral( "SQL_CALC_FOUND_ROWS" ),
          QStringLiteral( "STANDBY" ),
          QStringLiteral( "STARTING" ),
          QStringLiteral( "STARTUP" ),
          QStringLiteral( "STAT" ),
          QStringLiteral( "STATEMENT" ),
          QStringLiteral( "STATIC" ),
          QStringLiteral( "STDDEV" ),
          QStringLiteral( "STOP" ),
          QStringLiteral( "STORAGE" ),
          QStringLiteral( "STORE" ),
          QStringLiteral( "STRICT" ),
          QStringLiteral( "STRING" ),
          QStringLiteral( "STRIPING" ),
          QStringLiteral( "STRUCT" ),
          QStringLiteral( "STYLE" ),
          QStringLiteral( "SUBPARTITION" ),
          QStringLiteral( "SUBPARTITIONS" ),
          QStringLiteral( "SUBSCRIBE" ),
          QStringLiteral( "SUBSTITUTABLE" ),
          QStringLiteral( "SUBSTRING" ),
          QStringLiteral( "SUBTYPE" ),
          QStringLiteral( "SUCCESSFUL" ),
          QStringLiteral( "SUM" ),
          QStringLiteral( "SUN" ),
          QStringLiteral( "SUSPEND" ),
          QStringLiteral( "SWITCH" ),
          QStringLiteral( "SYNC" ),
          QStringLiteral( "SYNCHRONOUS" ),
          QStringLiteral( "SYNONYM" ),
          QStringLiteral( "SYSTEM" ),
          QStringLiteral( "SYS_CONNECT_BY_PATH" ),
          QStringLiteral( "TABLE" ),
          QStringLiteral( "TABLESPACE" ),
          QStringLiteral( "TASK" ),
          QStringLiteral( "TEMPLATE" ),
          QStringLiteral( "TEMPORARY" ),
          QStringLiteral( "TERMINATED" ),
          QStringLiteral( "TEXT" ),
          QStringLiteral( "THAN" ),
          QStringLiteral( "THEN" ),
          QStringLiteral( "THREAD" ),
          QStringLiteral( "THROUGH" ),
          QStringLiteral( "THROW" ),
          QStringLiteral( "THU" ),
          QStringLiteral( "TIES" ),
          QStringLiteral( "TIME" ),
          QStringLiteral( "TIMER" ),
          QStringLiteral( "TIMES" ),
          QStringLiteral( "TIMESTAMP" ),
          QStringLiteral( "TIMESTAMPADD" ),
          QStringLiteral( "TIMESTAMPDIFF" ),
          QStringLiteral( "TIME_ZONE" ),
          QStringLiteral( "TINYINT" ),
          QStringLiteral( "TO" ),
          QStringLiteral( "TOP" ),
          QStringLiteral( "TRACE" ),
          QStringLiteral( "TRACKING" ),
          QStringLiteral( "TRAILING" ),
          QStringLiteral( "TRANSACTION" ),
          QStringLiteral( "TRANSACTIONAL" ),
          QStringLiteral( "TREAT" ),
          QStringLiteral( "TRIGGER" ),
          QStringLiteral( "TRIGGERS" ),
          QStringLiteral( "TRIM" ),
          QStringLiteral( "TRUNCATE" ),
          QStringLiteral( "TRUNCSIZE" ),
          QStringLiteral( "TRXID" ),
          QStringLiteral( "TRY" ),
          QStringLiteral( "TUE" ),
          QStringLiteral( "TYPE" ),
          QStringLiteral( "TYPEDEF" ),
          QStringLiteral( "TYPEOF" ),
          QStringLiteral( "UINT" ),
          QStringLiteral( "ULONG" ),
          QStringLiteral( "UNBOUNDED" ),
          QStringLiteral( "UNCOMMITTED" ),
          QStringLiteral( "UNCONDITIONAL" ),
          QStringLiteral( "UNDER" ),
          QStringLiteral( "UNION" ),
          QStringLiteral( "UNIQUE" ),
          QStringLiteral( "UNLIMITED" ),
          QStringLiteral( "UNLOCK" ),
          QStringLiteral( "UNPIVOT" ),
          QStringLiteral( "UNTIL" ),
          QStringLiteral( "UNUSABLE" ),
          QStringLiteral( "UP" ),
          QStringLiteral( "UPDATE" ),
          QStringLiteral( "UPDATING" ),
          QStringLiteral( "USAGE" ),
          QStringLiteral( "USER" ),
          QStringLiteral( "USE_HASH" ),
          QStringLiteral( "USE_MERGE" ),
          QStringLiteral( "USE_NL" ),
          QStringLiteral( "USE_NL_WITH_INDEX" ),
          QStringLiteral( "USHORT" ),
          QStringLiteral( "USING" ),
          QStringLiteral( "VALIDATE" ),
          QStringLiteral( "VALUE" ),
          QStringLiteral( "VALUES" ),
          QStringLiteral( "VARBINARY" ),
          QStringLiteral( "VARCHAR" ),
          QStringLiteral( "VARCHAR2" ),
          QStringLiteral( "VARIANCE" ),
          QStringLiteral( "VARRAY" ),
          QStringLiteral( "VARYING" ),
          QStringLiteral( "VERIFY" ),
          QStringLiteral( "VERSION" ),
          QStringLiteral( "VERSIONS" ),
          QStringLiteral( "VERSIONS_ENDTIME" ),
          QStringLiteral( "VERSIONS_ENDTRXID" ),
          QStringLiteral( "VERSIONS_OPERATION" ),
          QStringLiteral( "VERSIONS_STARTTIME" ),
          QStringLiteral( "VERSIONS_STARTTRXID" ),
          QStringLiteral( "VERTICAL" ),
          QStringLiteral( "VIEW" ),
          QStringLiteral( "VIRTUAL" ),
          QStringLiteral( "VISIBLE" ),
          QStringLiteral( "VOID" ),
          QStringLiteral( "VSIZE" ),
          QStringLiteral( "WAIT" ),
          QStringLiteral( "WED" ),
          QStringLiteral( "WEEK" ),
          QStringLiteral( "WEEKLY" ),
          QStringLiteral( "WELLFORMED" ),
          QStringLiteral( "WHEN" ),
          QStringLiteral( "WHENEVER" ),
          QStringLiteral( "WHERE" ),
          QStringLiteral( "WHILE" ),
          QStringLiteral( "WITH" ),
          QStringLiteral( "WITHIN" ),
          QStringLiteral( "WITHOUT" ),
          QStringLiteral( "WORK" ),
          QStringLiteral( "WRAPPED" ),
          QStringLiteral( "WRAPPER" ),
          QStringLiteral( "WRITE" ),
          QStringLiteral( "X" ),
          QStringLiteral( "XML" ),
          QStringLiteral( "XMLAGG" ),
          QStringLiteral( "XMLATTRIBUTES" ),
          QStringLiteral( "XMLELEMENT" ),
          QStringLiteral( "XMLNAMESPACES" ),
          QStringLiteral( "XMLPARSE" ),
          QStringLiteral( "XMLQUERY" ),
          QStringLiteral( "XMLSERIALIZE" ),
          QStringLiteral( "XMLTABLE" ),
          QStringLiteral( "YEAR" ),
          QStringLiteral( "YEARLY" ),
          QStringLiteral( "ZONE" ),
        }
      },
      {
        Qgis::SqlKeywordCategory::Aggregate,
        {
          QStringLiteral( "Avg" ),
          QStringLiteral( "COLLECT" ),
          QStringLiteral( "CORR" ),
          QStringLiteral( "Count" ),
          QStringLiteral( "COVAR_POP" ),
          QStringLiteral( "COVAR_SAMP" ),
          QStringLiteral( "CUME_DIST" ),
          QStringLiteral( "DENSE_RANK" ),
          QStringLiteral( "FIRST" ),
          QStringLiteral( "GROUPING" ),
          QStringLiteral( "LAST" ),
          QStringLiteral( "Max" ),
          QStringLiteral( "MEDIAN" ),
          QStringLiteral( "Min" ),
          QStringLiteral( "PERCENTILE_CONT" ),
          QStringLiteral( "PERCENTILE_DISC" ),
          QStringLiteral( "PERCENT_RANK" ),
          QStringLiteral( "RANK" ),
          QStringLiteral( "StdDev" ),
          QStringLiteral( "StdDev_Pop" ),
          QStringLiteral( "StdDev_Samp" ),
          QStringLiteral( "Sum" ),
          QStringLiteral( "Total" ),
          QStringLiteral( "Var_Pop" ),
          QStringLiteral( "Var_Samp" ),
          QStringLiteral( "VARIANCE" ),
          QStringLiteral( "XMLAGG" ),
        }
      },
      {
        Qgis::SqlKeywordCategory::Math,
        {
          QStringLiteral( "Abs" ),
          QStringLiteral( "ACos" ),
          QStringLiteral( "ASin" ),
          QStringLiteral( "ATan" ),
          QStringLiteral( "ATan2" ),
          QStringLiteral( "BITAND" ),
          QStringLiteral( "CEIL" ),
          QStringLiteral( "Cos" ),
          QStringLiteral( "COSH" ),
          QStringLiteral( "Cot" ),
          QStringLiteral( "Degrees" ),
          QStringLiteral( "Exp" ),
          QStringLiteral( "Floor" ),
          QStringLiteral( "Log" ),
          QStringLiteral( "Log10" ),
          QStringLiteral( "MOD" ),
          QStringLiteral( "NANVL" ),
          QStringLiteral( "POWER" ),
          QStringLiteral( "REMAINDER" ),
          QStringLiteral( "Round" ),
          QStringLiteral( "Sign" ),
          QStringLiteral( "Sin" ),
          QStringLiteral( "Sinh" ),
          QStringLiteral( "SORT" ),
          QStringLiteral( "Sqrt" ),
          QStringLiteral( "Tan" ),
          QStringLiteral( "Tanh" ),
          QStringLiteral( "TRUNC" ),
          QStringLiteral( "Var_Pop" ),
          QStringLiteral( "Var_Samp" ),
        }
      },
      {
        Qgis::SqlKeywordCategory::Geospatial,
        {
          QStringLiteral( "DMGEO2.AddEdge" ),
          QStringLiteral( "DMGEO2.AddFace" ),
          QStringLiteral( "DMGEO2.AddGeometryColumn" ),
          QStringLiteral( "DMGEO2.AddNode" ),
          QStringLiteral( "DMGEO2.AddOverviewConstraints" ),
          QStringLiteral( "DMGEO2.AddRasterConstraints" ),
          QStringLiteral( "DMGEO2.AsGML" ),
          QStringLiteral( "DMGEO2.AsTopoJSON" ),
          QStringLiteral( "DMGEO2.Box2D" ),
          QStringLiteral( "DMGEO2.Box3D" ),
          QStringLiteral( "DMGEO2.CopyTopology" ),
          QStringLiteral( "DMGEO2.DropGeometryColumn" ),
          QStringLiteral( "DMGEO2.DropOverviewConstraints" ),
          QStringLiteral( "DMGEO2.DropRasterConstraints" ),
          QStringLiteral( "DMGEO2.Drop_Indexes_Generate_Script" ),
          QStringLiteral( "DMGEO2.Drop_Nation_Tables_Generate_Script" ),
          QStringLiteral( "DMGEO2.Drop_State_Tables_Generate_Script" ),
          QStringLiteral( "DMGEO2.Equals" ),
          QStringLiteral( "DMGEO2.Geocode" ),
          QStringLiteral( "DMGEO2.Geocode_Intersection" ),
          QStringLiteral( "DMGEO2.GeometryType" ),
          QStringLiteral( "DMGEO2.GetEdgeByPoint" ),
          QStringLiteral( "DMGEO2.GetFaceByPoint" ),
          QStringLiteral( "DMGEO2.GetNodeByPoint" ),
          QStringLiteral( "DMGEO2.GetNodeEdges" ),
          QStringLiteral( "DMGEO2.GetRingEdges" ),
          QStringLiteral( "DMGEO2.GetTopoGeomElements" ),
          QStringLiteral( "DMGEO2.GetTopologySRID" ),
          QStringLiteral( "DMGEO2.Get_Geocode_Setting" ),
          QStringLiteral( "DMGEO2.Get_Tract" ),
          QStringLiteral( "DMGEO2.Install_Missing_Indexes" ),
          QStringLiteral( "DMGEO2.Intersects" ),
          QStringLiteral( "DMGEO2.Loader_Generate_Census_Script" ),
          QStringLiteral( "DMGEO2.Loader_Generate_Nation_Script" ),
          QStringLiteral( "DMGEO2.Loader_Generate_Script" ),
          QStringLiteral( "DMGEO2.Missing_Indexes_Generate_Script" ),
          QStringLiteral( "DMGEO2.Normalize_Address" ),
          QStringLiteral( "DMGEO2.Pagc_Normalize_Address" ),
          QStringLiteral( "DMGEO2.Polygonize" ),
          QStringLiteral( "DMGEO2.Populate_Geometry_Columns" ),
          QStringLiteral( "DMGEO2.Populate_Topology_Layer" ),
          QStringLiteral( "DMGEO2.Reverse_Geocode" ),
          QStringLiteral( "DMGEO2.ST_3DArea" ),
          QStringLiteral( "DMGEO2.ST_3DClosestPoint" ),
          QStringLiteral( "DMGEO2.ST_3DDFullyWithin" ),
          QStringLiteral( "DMGEO2.ST_3DDWithin" ),
          QStringLiteral( "DMGEO2.ST_3DDifference" ),
          QStringLiteral( "DMGEO2.ST_3DDistance" ),
          QStringLiteral( "DMGEO2.ST_3DExtent" ),
          QStringLiteral( "DMGEO2.ST_3DIntersection" ),
          QStringLiteral( "DMGEO2.ST_3DIntersects" ),
          QStringLiteral( "DMGEO2.ST_3DLength" ),
          QStringLiteral( "DMGEO2.ST_3DLineInterpolatePoint" ),
          QStringLiteral( "DMGEO2.ST_3DLongestLine" ),
          QStringLiteral( "DMGEO2.ST_3DMakeBox" ),
          QStringLiteral( "DMGEO2.ST_3DMaxDistance" ),
          QStringLiteral( "DMGEO2.ST_3DPerimeter" ),
          QStringLiteral( "DMGEO2.ST_3DShortestLine" ),
          QStringLiteral( "DMGEO2.ST_3DUnion" ),
          QStringLiteral( "DMGEO2.ST_AddBand1" ),
          QStringLiteral( "DMGEO2.ST_AddBand2" ),
          QStringLiteral( "DMGEO2.ST_AddEdgeModFace" ),
          QStringLiteral( "DMGEO2.ST_AddEdgeNewFaces" ),
          QStringLiteral( "DMGEO2.ST_AddMeasure" ),
          QStringLiteral( "DMGEO2.ST_AddPoint" ),
          QStringLiteral( "DMGEO2.ST_Affine" ),
          QStringLiteral( "DMGEO2.ST_Angle" ),
          QStringLiteral( "DMGEO2.ST_ApproximateMedialAxis" ),
          QStringLiteral( "DMGEO2.ST_Area" ),
          QStringLiteral( "DMGEO2.ST_AsBinary" ),
          QStringLiteral( "DMGEO2.ST_AsBinary/ST_AsWKB" ),
          QStringLiteral( "DMGEO2.ST_AsEWKB" ),
          QStringLiteral( "DMGEO2.ST_AsEWKT" ),
          QStringLiteral( "DMGEO2.ST_AsEncodedPolyline" ),
          QStringLiteral( "DMGEO2.ST_AsGDALRaster" ),
          QStringLiteral( "DMGEO2.ST_AsGML" ),
          QStringLiteral( "DMGEO2.ST_AsGeoJSON" ),
          QStringLiteral( "DMGEO2.ST_AsGeobuf" ),
          QStringLiteral( "DMGEO2.ST_AsHEXEWKB" ),
          QStringLiteral( "DMGEO2.ST_AsHexWKB" ),
          QStringLiteral( "DMGEO2.ST_AsJPEG" ),
          QStringLiteral( "DMGEO2.ST_AsKML" ),
          QStringLiteral( "DMGEO2.ST_AsLatLonText" ),
          QStringLiteral( "DMGEO2.ST_AsMVT" ),
          QStringLiteral( "DMGEO2.ST_AsMVTGeom" ),
          QStringLiteral( "DMGEO2.ST_AsPNG" ),
          QStringLiteral( "DMGEO2.ST_AsRaster" ),
          QStringLiteral( "DMGEO2.ST_AsSVG" ),
          QStringLiteral( "DMGEO2.ST_AsTIFF" ),
          QStringLiteral( "DMGEO2.ST_AsTWKB" ),
          QStringLiteral( "DMGEO2.ST_AsText" ),
          QStringLiteral( "DMGEO2.ST_AsX3D" ),
          QStringLiteral( "DMGEO2.ST_Aspect" ),
          QStringLiteral( "DMGEO2.ST_Azimuth" ),
          QStringLiteral( "DMGEO2.ST_Band" ),
          QStringLiteral( "DMGEO2.ST_BandFileSize" ),
          QStringLiteral( "DMGEO2.ST_BandFileTimestamp" ),
          QStringLiteral( "DMGEO2.ST_BandIsNoData" ),
          QStringLiteral( "DMGEO2.ST_BandMetaData" ),
          QStringLiteral( "DMGEO2.ST_BandNoDataValue" ),
          QStringLiteral( "DMGEO2.ST_BandPath" ),
          QStringLiteral( "DMGEO2.ST_BandPixelType" ),
          QStringLiteral( "DMGEO2.ST_Boundary" ),
          QStringLiteral( "DMGEO2.ST_BoundingDiagonal" ),
          QStringLiteral( "DMGEO2.ST_Box2dFromGeoHash" ),
          QStringLiteral( "DMGEO2.ST_Buffer" ),
          QStringLiteral( "DMGEO2.ST_CPAWithin" ),
          QStringLiteral( "DMGEO2.ST_Centroid" ),
          QStringLiteral( "DMGEO2.ST_ChaikinSmoothing" ),
          QStringLiteral( "DMGEO2.ST_Clip" ),
          QStringLiteral( "DMGEO2.ST_ClipByBox2D" ),
          QStringLiteral( "DMGEO2.ST_ClosestPoint" ),
          QStringLiteral( "DMGEO2.ST_ClosestPointOfApproach" ),
          QStringLiteral( "DMGEO2.ST_ClusterDBSCAN" ),
          QStringLiteral( "DMGEO2.ST_ClusterIntersecting" ),
          QStringLiteral( "DMGEO2.ST_ClusterKMeans" ),
          QStringLiteral( "DMGEO2.ST_ClusterWithin" ),
          QStringLiteral( "DMGEO2.ST_Collect" ),
          QStringLiteral( "DMGEO2.ST_CollectionExtract" ),
          QStringLiteral( "DMGEO2.ST_CollectionHomogenize" ),
          QStringLiteral( "DMGEO2.ST_ColorMap" ),
          QStringLiteral( "DMGEO2.ST_ConcaveHull" ),
          QStringLiteral( "DMGEO2.ST_ConstrainedDelaunayTriangles" ),
          QStringLiteral( "DMGEO2.ST_Contains" ),
          QStringLiteral( "DMGEO2.ST_ContainsProperly" ),
          QStringLiteral( "DMGEO2.ST_ConvexHull" ),
          QStringLiteral( "DMGEO2.ST_CoordDim" ),
          QStringLiteral( "DMGEO2.ST_Count" ),
          QStringLiteral( "DMGEO2.ST_CountAgg" ),
          QStringLiteral( "DMGEO2.ST_CoveredBy" ),
          QStringLiteral( "DMGEO2.ST_Covers" ),
          QStringLiteral( "DMGEO2.ST_CreateOverview" ),
          QStringLiteral( "DMGEO2.ST_CreateTopoGeo" ),
          QStringLiteral( "DMGEO2.ST_Crosses" ),
          QStringLiteral( "DMGEO2.ST_CurveToLine" ),
          QStringLiteral( "DMGEO2.ST_DFullyWithin" ),
          QStringLiteral( "DMGEO2.ST_DWithin" ),
          QStringLiteral( "DMGEO2.ST_DelaunayTriangles" ),
          QStringLiteral( "DMGEO2.ST_Difference" ),
          QStringLiteral( "DMGEO2.ST_Dimension" ),
          QStringLiteral( "DMGEO2.ST_Disjoint" ),
          QStringLiteral( "DMGEO2.ST_Distance" ),
          QStringLiteral( "DMGEO2.ST_DistanceCPA" ),
          QStringLiteral( "DMGEO2.ST_DistanceSphere" ),
          QStringLiteral( "DMGEO2.ST_DistanceSpheroid" ),
          QStringLiteral( "DMGEO2.ST_Distinct4ma" ),
          QStringLiteral( "DMGEO2.ST_Dump" ),
          QStringLiteral( "DMGEO2.ST_DumpAsPolygons" ),
          QStringLiteral( "DMGEO2.ST_DumpPoints" ),
          QStringLiteral( "DMGEO2.ST_DumpRings" ),
          QStringLiteral( "DMGEO2.ST_DumpValues" ),
          QStringLiteral( "DMGEO2.ST_EndPoint" ),
          QStringLiteral( "DMGEO2.ST_Envelope" ),
          QStringLiteral( "DMGEO2.ST_Equals" ),
          QStringLiteral( "DMGEO2.ST_EstimatedExtent" ),
          QStringLiteral( "DMGEO2.ST_Expand" ),
          QStringLiteral( "DMGEO2.ST_Extent" ),
          QStringLiteral( "DMGEO2.ST_ExteriorRing" ),
          QStringLiteral( "DMGEO2.ST_Extrude" ),
          QStringLiteral( "DMGEO2.ST_FilterByM" ),
          QStringLiteral( "DMGEO2.ST_FlipCoordinates" ),
          QStringLiteral( "DMGEO2.ST_Force2D" ),
          QStringLiteral( "DMGEO2.ST_Force3D" ),
          QStringLiteral( "DMGEO2.ST_Force3DM" ),
          QStringLiteral( "DMGEO2.ST_Force3DZ" ),
          QStringLiteral( "DMGEO2.ST_Force4D" ),
          QStringLiteral( "DMGEO2.ST_ForceCollection" ),
          QStringLiteral( "DMGEO2.ST_ForceCurve" ),
          QStringLiteral( "DMGEO2.ST_ForceLHR" ),
          QStringLiteral( "DMGEO2.ST_ForcePolygonCCW" ),
          QStringLiteral( "DMGEO2.ST_ForcePolygonCW" ),
          QStringLiteral( "DMGEO2.ST_ForceRHR" ),
          QStringLiteral( "DMGEO2.ST_ForceSFS" ),
          QStringLiteral( "DMGEO2.ST_FrechetDistance" ),
          QStringLiteral( "DMGEO2.ST_FromGDALRaster" ),
          QStringLiteral( "DMGEO2.ST_GDALDrivers" ),
          QStringLiteral( "DMGEO2.ST_GMLToSQL" ),
          QStringLiteral( "DMGEO2.ST_GeneratePoints" ),
          QStringLiteral( "DMGEO2.ST_GeoHash" ),
          QStringLiteral( "DMGEO2.ST_GeoReference" ),
          QStringLiteral( "DMGEO2.ST_GeogFromText" ),
          QStringLiteral( "DMGEO2.ST_GeogFromWKB" ),
          QStringLiteral( "DMGEO2.ST_GeographyFromText" ),
          QStringLiteral( "DMGEO2.ST_GeomFromEWKB" ),
          QStringLiteral( "DMGEO2.ST_GeomFromEWKT" ),
          QStringLiteral( "DMGEO2.ST_GeomFromGML" ),
          QStringLiteral( "DMGEO2.ST_GeomFromGeoHash" ),
          QStringLiteral( "DMGEO2.ST_GeomFromGeoJSON" ),
          QStringLiteral( "DMGEO2.ST_GeomFromKML" ),
          QStringLiteral( "DMGEO2.ST_GeomFromText" ),
          QStringLiteral( "DMGEO2.ST_GeomFromWKB" ),
          QStringLiteral( "DMGEO2.ST_GeometricMedian" ),
          QStringLiteral( "DMGEO2.ST_GeometryN" ),
          QStringLiteral( "DMGEO2.ST_GeometryType" ),
          QStringLiteral( "DMGEO2.ST_GetFaceEdges" ),
          QStringLiteral( "DMGEO2.ST_Grayscale" ),
          QStringLiteral( "DMGEO2.ST_HasArc" ),
          QStringLiteral( "DMGEO2.ST_HasNoBand" ),
          QStringLiteral( "DMGEO2.ST_HausdorffDistance" ),
          QStringLiteral( "DMGEO2.ST_Height" ),
          QStringLiteral( "DMGEO2.ST_Hexagon" ),
          QStringLiteral( "DMGEO2.ST_HexagonGrid" ),
          QStringLiteral( "DMGEO2.ST_HillShade" ),
          QStringLiteral( "DMGEO2.ST_Histogram" ),
          QStringLiteral( "DMGEO2.ST_InteriorRingN" ),
          QStringLiteral( "DMGEO2.ST_InterpolatePoint" ),
          QStringLiteral( "DMGEO2.ST_Intersection" ),
          QStringLiteral( "DMGEO2.ST_Intersects" ),
          QStringLiteral( "DMGEO2.ST_InvDistWeight4ma" ),
          QStringLiteral( "DMGEO2.ST_IsClosed" ),
          QStringLiteral( "DMGEO2.ST_IsCollection" ),
          QStringLiteral( "DMGEO2.ST_IsEmpty" ),
          QStringLiteral( "DMGEO2.ST_IsPlanar" ),
          QStringLiteral( "DMGEO2.ST_IsPolygonCCW" ),
          QStringLiteral( "DMGEO2.ST_IsPolygonCW" ),
          QStringLiteral( "DMGEO2.ST_IsSimple" ),
          QStringLiteral( "DMGEO2.ST_IsSolid" ),
          QStringLiteral( "DMGEO2.ST_IsValidDetail" ),
          QStringLiteral( "DMGEO2.ST_IsValidReason" ),
          QStringLiteral( "DMGEO2.ST_IsValidTrajectory" ),
          QStringLiteral( "DMGEO2.ST_Length" ),
          QStringLiteral( "DMGEO2.ST_LengthSpheroid" ),
          QStringLiteral( "DMGEO2.ST_LineCrossingDirection" ),
          QStringLiteral( "DMGEO2.ST_LineFromEncodedPolyline" ),
          QStringLiteral( "DMGEO2.ST_LineFromMultiPoint" ),
          QStringLiteral( "DMGEO2.ST_LineInterpolatePoint" ),
          QStringLiteral( "DMGEO2.ST_LineInterpolatePoints" ),
          QStringLiteral( "DMGEO2.ST_LineLocatePoint" ),
          QStringLiteral( "DMGEO2.ST_LineSubstring" ),
          QStringLiteral( "DMGEO2.ST_LineToCurve" ),
          QStringLiteral( "DMGEO2.ST_LocateAlong" ),
          QStringLiteral( "DMGEO2.ST_LocateBetween" ),
          QStringLiteral( "DMGEO2.ST_LocateBetweenElevations" ),
          QStringLiteral( "DMGEO2.ST_LongestLine" ),
          QStringLiteral( "DMGEO2.ST_M" ),
          QStringLiteral( "DMGEO2.ST_MakeBox2D" ),
          QStringLiteral( "DMGEO2.ST_MakeEmptyCoverage" ),
          QStringLiteral( "DMGEO2.ST_MakeEmptyRaster" ),
          QStringLiteral( "DMGEO2.ST_MakeEnvelope" ),
          QStringLiteral( "DMGEO2.ST_MakeLine" ),
          QStringLiteral( "DMGEO2.ST_MakePoint" ),
          QStringLiteral( "DMGEO2.ST_MakePolygon" ),
          QStringLiteral( "DMGEO2.ST_MakeSolid" ),
          QStringLiteral( "DMGEO2.ST_MakeValid" ),
          QStringLiteral( "DMGEO2.ST_MapAlgebra ( callback function version )" ),
          QStringLiteral( "DMGEO2.ST_MapAlgebra ( expression version )" ),
          QStringLiteral( "DMGEO2.ST_MapAlgebraExpr" ),
          QStringLiteral( "DMGEO2.ST_MapAlgebraFct" ),
          QStringLiteral( "DMGEO2.ST_MapAlgebraFctNgb" ),
          QStringLiteral( "DMGEO2.ST_Max4ma" ),
          QStringLiteral( "DMGEO2.ST_MaxDistance" ),
          QStringLiteral( "DMGEO2.ST_MaximumInscribedCircle" ),
          QStringLiteral( "DMGEO2.ST_Mean4ma" ),
          QStringLiteral( "DMGEO2.ST_MemSize" ),
          QStringLiteral( "DMGEO2.ST_MemUnion" ),
          QStringLiteral( "DMGEO2.ST_MetaData" ),
          QStringLiteral( "DMGEO2.ST_Min4ma" ),
          QStringLiteral( "DMGEO2.ST_MinConvexHull" ),
          QStringLiteral( "DMGEO2.ST_MinDist4ma" ),
          QStringLiteral( "DMGEO2.ST_MinimumBoundingCircle" ),
          QStringLiteral( "DMGEO2.ST_MinimumClearance" ),
          QStringLiteral( "DMGEO2.ST_MinimumClearanceLine" ),
          QStringLiteral( "DMGEO2.ST_MinkowskiSum" ),
          QStringLiteral( "DMGEO2.ST_ModEdgeHeal" ),
          QStringLiteral( "DMGEO2.ST_ModEdgeSplit" ),
          QStringLiteral( "DMGEO2.ST_NDims" ),
          QStringLiteral( "DMGEO2.ST_NPoints" ),
          QStringLiteral( "DMGEO2.ST_NRings" ),
          QStringLiteral( "DMGEO2.ST_NearestValue" ),
          QStringLiteral( "DMGEO2.ST_Neighborhood" ),
          QStringLiteral( "DMGEO2.ST_NewEdgeHeal" ),
          QStringLiteral( "DMGEO2.ST_Node" ),
          QStringLiteral( "DMGEO2.ST_Normalize" ),
          QStringLiteral( "DMGEO2.ST_NotSameAlignmentReason" ),
          QStringLiteral( "DMGEO2.ST_NumBands" ),
          QStringLiteral( "DMGEO2.ST_NumGeometries" ),
          QStringLiteral( "DMGEO2.ST_NumInteriorRings" ),
          QStringLiteral( "DMGEO2.ST_NumPatches" ),
          QStringLiteral( "DMGEO2.ST_OffsetCurve" ),
          QStringLiteral( "DMGEO2.ST_Orientation" ),
          QStringLiteral( "DMGEO2.ST_Overlaps" ),
          QStringLiteral( "DMGEO2.ST_PatchN" ),
          QStringLiteral( "DMGEO2.ST_Perimeter" ),
          QStringLiteral( "DMGEO2.ST_PixelAsCentroid" ),
          QStringLiteral( "DMGEO2.ST_PixelAsCentroids" ),
          QStringLiteral( "DMGEO2.ST_PixelAsPoint" ),
          QStringLiteral( "DMGEO2.ST_PixelAsPoints" ),
          QStringLiteral( "DMGEO2.ST_PixelAsPolygon" ),
          QStringLiteral( "DMGEO2.ST_PixelAsPolygons" ),
          QStringLiteral( "DMGEO2.ST_PixelHeight" ),
          QStringLiteral( "DMGEO2.ST_PixelOfValue" ),
          QStringLiteral( "DMGEO2.ST_PixelWidth" ),
          QStringLiteral( "DMGEO2.ST_PointFromGeoHash" ),
          QStringLiteral( "DMGEO2.ST_PointFromWKB" ),
          QStringLiteral( "DMGEO2.ST_PointInsideCircle" ),
          QStringLiteral( "DMGEO2.ST_PointN" ),
          QStringLiteral( "DMGEO2.ST_PointOnSurface" ),
          QStringLiteral( "DMGEO2.ST_Points" ),
          QStringLiteral( "DMGEO2.ST_Polygon" ),
          QStringLiteral( "DMGEO2.ST_Polygonize" ),
          QStringLiteral( "DMGEO2.ST_Project" ),
          QStringLiteral( "DMGEO2.ST_Quantile" ),
          QStringLiteral( "DMGEO2.ST_Range4ma" ),
          QStringLiteral( "DMGEO2.ST_RastFromHexWKB" ),
          QStringLiteral( "DMGEO2.ST_RastFromWKB" ),
          QStringLiteral( "DMGEO2.ST_RasterToWorldCoord" ),
          QStringLiteral( "DMGEO2.ST_RasterToWorldCoordX" ),
          QStringLiteral( "DMGEO2.ST_RasterToWorldCoordY" ),
          QStringLiteral( "DMGEO2.ST_Reclass" ),
          QStringLiteral( "DMGEO2.ST_ReducePrecision" ),
          QStringLiteral( "DMGEO2.ST_Relate" ),
          QStringLiteral( "DMGEO2.ST_RelateMatch" ),
          QStringLiteral( "DMGEO2.ST_RemEdgeModFace" ),
          QStringLiteral( "DMGEO2.ST_RemEdgeNewFace" ),
          QStringLiteral( "DMGEO2.ST_RemovePoint" ),
          QStringLiteral( "DMGEO2.ST_RemoveRepeatedPoints" ),
          QStringLiteral( "DMGEO2.ST_Resample" ),
          QStringLiteral( "DMGEO2.ST_Rescale" ),
          QStringLiteral( "DMGEO2.ST_Resize" ),
          QStringLiteral( "DMGEO2.ST_Reskew" ),
          QStringLiteral( "DMGEO2.ST_Retile" ),
          QStringLiteral( "DMGEO2.ST_Reverse" ),
          QStringLiteral( "DMGEO2.ST_Rotate" ),
          QStringLiteral( "DMGEO2.ST_RotateX" ),
          QStringLiteral( "DMGEO2.ST_RotateY" ),
          QStringLiteral( "DMGEO2.ST_RotateZ" ),
          QStringLiteral( "DMGEO2.ST_Rotation" ),
          QStringLiteral( "DMGEO2.ST_Roughness" ),
          QStringLiteral( "DMGEO2.ST_SRID" ),
          QStringLiteral( "DMGEO2.ST_SameAlignment" ),
          QStringLiteral( "DMGEO2.ST_Scale" ),
          QStringLiteral( "DMGEO2.ST_ScaleX" ),
          QStringLiteral( "DMGEO2.ST_ScaleY" ),
          QStringLiteral( "DMGEO2.ST_Segmentize" ),
          QStringLiteral( "DMGEO2.ST_SetBandIndex" ),
          QStringLiteral( "DMGEO2.ST_SetBandIsNoData" ),
          QStringLiteral( "DMGEO2.ST_SetBandNoDataValue" ),
          QStringLiteral( "DMGEO2.ST_SetBandPath" ),
          QStringLiteral( "DMGEO2.ST_SetEffectiveArea" ),
          QStringLiteral( "DMGEO2.ST_SetGeoReference" ),
          QStringLiteral( "DMGEO2.ST_SetPoint" ),
          QStringLiteral( "DMGEO2.ST_SetRotation" ),
          QStringLiteral( "DMGEO2.ST_SetSRID" ),
          QStringLiteral( "DMGEO2.ST_SetScale" ),
          QStringLiteral( "DMGEO2.ST_SetSkew" ),
          QStringLiteral( "DMGEO2.ST_SetUpperLeft" ),
          QStringLiteral( "DMGEO2.ST_SetValue" ),
          QStringLiteral( "DMGEO2.ST_SetValues" ),
          QStringLiteral( "DMGEO2.ST_SharedPaths" ),
          QStringLiteral( "DMGEO2.ST_ShiftLongitude" ),
          QStringLiteral( "DMGEO2.ST_ShortestLine" ),
          QStringLiteral( "DMGEO2.ST_Simplify" ),
          QStringLiteral( "DMGEO2.ST_SimplifyPreserveTopology" ),
          QStringLiteral( "DMGEO2.ST_SimplifyVW" ),
          QStringLiteral( "DMGEO2.ST_SkewX" ),
          QStringLiteral( "DMGEO2.ST_SkewY" ),
          QStringLiteral( "DMGEO2.ST_Slope" ),
          QStringLiteral( "DMGEO2.ST_Snap" ),
          QStringLiteral( "DMGEO2.ST_SnapToGrid" ),
          QStringLiteral( "DMGEO2.ST_Split" ),
          QStringLiteral( "DMGEO2.ST_Square" ),
          QStringLiteral( "DMGEO2.ST_SquareGrid" ),
          QStringLiteral( "DMGEO2.ST_StartPoint" ),
          QStringLiteral( "DMGEO2.ST_StdDev4ma" ),
          QStringLiteral( "DMGEO2.ST_StraightSkeleton" ),
          QStringLiteral( "DMGEO2.ST_Subdivide" ),
          QStringLiteral( "DMGEO2.ST_Sum4ma" ),
          QStringLiteral( "DMGEO2.ST_Summary" ),
          QStringLiteral( "DMGEO2.ST_SummaryStats" ),
          QStringLiteral( "DMGEO2.ST_SummaryStatsAgg" ),
          QStringLiteral( "DMGEO2.ST_SwapOrdinates" ),
          QStringLiteral( "DMGEO2.ST_SymDifference" ),
          QStringLiteral( "DMGEO2.ST_TPI" ),
          QStringLiteral( "DMGEO2.ST_TRI" ),
          QStringLiteral( "DMGEO2.ST_Tesselate" ),  //#spellok
          QStringLiteral( "DMGEO2.ST_Tile" ),
          QStringLiteral( "DMGEO2.ST_TileEnvelope" ),
          QStringLiteral( "DMGEO2.ST_Touches" ),
          QStringLiteral( "DMGEO2.ST_TransScale" ),
          QStringLiteral( "DMGEO2.ST_Transform" ),
          QStringLiteral( "DMGEO2.ST_Translate" ),
          QStringLiteral( "DMGEO2.ST_UnaryUnion" ),
          QStringLiteral( "DMGEO2.ST_Union" ),
          QStringLiteral( "DMGEO2.ST_UpperLeftX" ),
          QStringLiteral( "DMGEO2.ST_UpperLeftY" ),
          QStringLiteral( "DMGEO2.ST_Value" ),
          QStringLiteral( "DMGEO2.ST_ValueCount" ),
          QStringLiteral( "DMGEO2.ST_Volume" ),
          QStringLiteral( "DMGEO2.ST_VoronoiLines" ),
          QStringLiteral( "DMGEO2.ST_VoronoiPolygons" ),
          QStringLiteral( "DMGEO2.ST_Width" ),
          QStringLiteral( "DMGEO2.ST_Within" ),
          QStringLiteral( "DMGEO2.ST_WorldToRasterCoord" ),
          QStringLiteral( "DMGEO2.ST_WorldToRasterCoordX" ),
          QStringLiteral( "DMGEO2.ST_WorldToRasterCoordY" ),
          QStringLiteral( "DMGEO2.ST_WrapX" ),
          QStringLiteral( "DMGEO2.ST_X" ),
          QStringLiteral( "DMGEO2.ST_XMax" ),
          QStringLiteral( "DMGEO2.ST_XMin" ),
          QStringLiteral( "DMGEO2.ST_Y" ),
          QStringLiteral( "DMGEO2.ST_YMax" ),
          QStringLiteral( "DMGEO2.ST_YMin" ),
          QStringLiteral( "DMGEO2.ST_Z" ),
          QStringLiteral( "DMGEO2.ST_ZMax" ),
          QStringLiteral( "DMGEO2.ST_ZMin" ),
          QStringLiteral( "DMGEO2.ST_Zmflag" ),
          QStringLiteral( "DMGEO2.Set_Geocode_Setting" ),
          QStringLiteral( "DMGEO2.TopoElementArray_Agg" ),
          QStringLiteral( "DMGEO2.TopoGeo_AddLineString" ),
          QStringLiteral( "DMGEO2.TopoGeo_AddPoint" ),
          QStringLiteral( "DMGEO2.TopoGeo_AddPolygon" ),
          QStringLiteral( "DMGEO2.TopoGeom_addElement" ),
          QStringLiteral( "DMGEO2.TopoGeom_remElement" ),
          QStringLiteral( "DMGEO2.TopologySummary" ),
          QStringLiteral( "DMGEO2.Topology_Load_Tiger" ),
          QStringLiteral( "DMGEO2.UpdateGeometrySRID" ),
          QStringLiteral( "DMGEO2.UpdateRasterSRID" ),
          QStringLiteral( "DMGEO2.ValidateTopology" ),
          QStringLiteral( "DMGEO2.box2d" ),
          QStringLiteral( "DMGEO2.clearTopoGeom" ),
          QStringLiteral( "DMGEO2.geometry_dump" ),
          QStringLiteral( "DMGEO2.parse_address" ),
          QStringLiteral( "DMGEO2.standardize_address" ),
          QStringLiteral( "DMGEO2.toTopoGeom" ),
        }
      },
      { Qgis::SqlKeywordCategory::Operator,
        { QStringLiteral( "AND" ),
          QStringLiteral( "OR" ),
          QStringLiteral( "||" ),
          QStringLiteral( "<" ),
          QStringLiteral( "<=" ),
          QStringLiteral( ">" ),
          QStringLiteral( ">=" ),
          QStringLiteral( "=" ),

          QStringLiteral( "<>" ),
          QStringLiteral( "!=" ),
          QStringLiteral( "^=" ),
          QStringLiteral( "IS" ),
          QStringLiteral( "IS NOT" ),
          QStringLiteral( "IN" ),
          QStringLiteral( "ANY" ),
          QStringLiteral( "SOME" ),

          QStringLiteral( "NOT IN" ),
          QStringLiteral( "LIKE" ),
          QStringLiteral( "GLOB" ),
          QStringLiteral( "MATCH" ),
          QStringLiteral( "REGEXP" ),

          QStringLiteral( "BETWEEN x AND y" ),
          QStringLiteral( "NOT BETWEEN x AND y" ),
          QStringLiteral( "EXISTS" ),

          QStringLiteral( "IS NULL" ),
          QStringLiteral( "IS NOT NULL" ),
          QStringLiteral( "ALL" ),
          QStringLiteral( "NOT" ),

          QStringLiteral( "CASE {column} WHEN {value} THEN {value}" )
        }
      },
      { Qgis::SqlKeywordCategory::Constant,
        { QStringLiteral( "NULL" ),
          QStringLiteral( "FALSE" ),
          QStringLiteral( "TRUE" )
        }
      }
    }
  );
}

