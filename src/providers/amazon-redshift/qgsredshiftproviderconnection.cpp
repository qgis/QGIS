/***************************************************************************
   qgsredshiftproviderconnection.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftproviderconnection.h"

#include "qgsapplication.h"
#include "qgsexception.h"
#include "qgsredshiftconn.h"
#include "qgsredshiftconnpool.h"
#include "qgsredshiftprovider.h"
#include "qgssettings.h"

extern "C"
{
#include <libpq-fe.h>
}

QgsRedshiftProviderConnection::QgsRedshiftProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "redshift" );
  // Remove the sql and table empty parts
  const QRegularExpression removePartsRe{R"raw(\s*sql=\s*|\s*table=""\s*)raw"};
  setUri( QgsRedshiftConn::connUri( name ).uri().replace( removePartsRe, QString() ) );
  setDefaultCapabilities();
}

QgsRedshiftProviderConnection::QgsRedshiftProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = QStringLiteral( "redshift" );
  setDefaultCapabilities();
}

void QgsRedshiftProviderConnection::setDefaultCapabilities()
{
  mCapabilities = {Capability::DropVectorTable, Capability::CreateVectorTable, Capability::RenameSchema,
                   Capability::DropSchema,      Capability::CreateSchema,      Capability::RenameVectorTable,
                   Capability::Vacuum,          Capability::ExecuteSql,        Capability::SqlLayers,
                   Capability::Tables,          Capability::Schemas,           Capability::Spatial,
                   Capability::TableExists,     Capability::DeleteField,       Capability::DeleteFieldCascade,
                   Capability::AddField
                  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePart,
  };
}

void QgsRedshiftProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  executeSqlPrivate( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsRedshiftConn::quotedIdentifier( schema.toLower() ) )
                     .arg( QgsRedshiftConn::quotedIdentifier( name.toLower() ) ) );
}

void QgsRedshiftProviderConnection::createVectorTable( const QString &schema, const QString &name,
    const QgsFields &fields, Qgis::WkbType wkbType,
    const QgsCoordinateReferenceSystem &srs, bool overwrite,
    const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri{uri()};
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column if it's not aspatial
  if ( wkbType != Qgis::WkbType::Unknown && wkbType != Qgis::WkbType::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "geom" ) ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult res =
    QgsRedshiftProvider::createEmptyLayer( newUri.uri(), fields, wkbType, srs, overwrite, &map, &errCause, options );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException(
      QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsRedshiftProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo{table( schema.toLower(), name.toLower() )}; // check that table actually exists
  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name.toLower() );
  dsUri.setSchema( schema.toLower() );
  return dsUri.uri( false );
}

void QgsRedshiftProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  dropTablePrivate( schema, name );
}

void QgsRedshiftProviderConnection::renameTablePrivate( const QString &schema, const QString &name,
    const QString &newName ) const
{
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsRedshiftConn::quotedIdentifier( schema ) )
                     .arg( QgsRedshiftConn::quotedIdentifier( name ) )
                     .arg( QgsRedshiftConn::quotedIdentifier( newName ) ) );
}

void QgsRedshiftProviderConnection::renameVectorTable( const QString &schema, const QString &name,
    const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  renameTablePrivate( schema, name, newName );
}

void QgsRedshiftProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsRedshiftConn::quotedIdentifier( name ) ) );
}

void QgsRedshiftProviderConnection::dropSchema( const QString &name, bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlPrivate( QStringLiteral( "DROP SCHEMA %1 %2" )
                     .arg( QgsRedshiftConn::quotedIdentifier( name ) )
                     .arg( force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

void QgsRedshiftProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlPrivate( QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                     .arg( QgsRedshiftConn::quotedIdentifier( name ) )
                     .arg( QgsRedshiftConn::quotedIdentifier( newName ) ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsRedshiftProviderConnection::execSql( const QString &sql,
    QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return execSqlPrivate( sql, true, feedback );
}

QList<QVariantList> QgsRedshiftProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes,
    QgsFeedback *feedback,
    std::shared_ptr<QgsPoolRedshiftConn> conn ) const
{
  QStringList columnNames;
  return execSqlPrivate( sql, resolveTypes, feedback, conn ).rows();
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsRedshiftProviderConnection::execSqlPrivate(
  const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolRedshiftConn> rsConn ) const
{
  if ( !rsConn )
  {
    rsConn = std::make_shared<QgsPoolRedshiftConn>( QgsDataSourceUri( uri() ).connectionInfo( false ) );
  }

  std::shared_ptr<QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator> iterator =
    std::make_shared<QgsRedshiftProviderResultIterator>( resolveTypes );
  QueryResult results( iterator );

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
  {
    return results;
  }

  QgsRedshiftConn *conn = rsConn->get();

  if ( !conn )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }
  else
  {
    if ( feedback && feedback->isCanceled() )
    {
      return results;
    }

    // This is gross but I tried with both conn and a context QObject without
    // success: the lambda is never called.
    QMetaObject::Connection qtConnection;
    if ( feedback )
    {
      qtConnection = QObject::connect( feedback, &QgsFeedback::canceled, [&rsConn]
      {
        if ( rsConn )
          rsConn->get()->PQCancel();
      } );
    }

    std::unique_ptr<QgsRedshiftResult> res = std::make_unique<QgsRedshiftResult>( conn->PQexec( sql ) );

    if ( feedback )
    {
      QObject::disconnect( qtConnection );
    }

    QString errCause;
    if ( conn->PQstatus() != CONNECTION_OK || !res->result() )
    {
      errCause = QObject::tr( "Connection error: %1 returned %2 [%3]" )
                 .arg( sql )
                 .arg( conn->PQstatus() )
                 .arg( conn->PQerrorMessage() );
    }
    else
    {
      const QString err{conn->PQerrorMessage()};
      if ( !err.isEmpty() )
      {
        errCause = QObject::tr( "SQL error: %1 returned %2 [%3]" ).arg( sql ).arg( conn->PQstatus() ).arg( err );
      }
    }

    const long long numRows{res->PQntuples()};

    if ( numRows > 0 )
    {
      // Get column names
      for ( int rowIdx = 0; rowIdx < res->PQnfields(); rowIdx++ )
      {
        results.appendColumn( res->PQfname( rowIdx ) );
      }

      // Try to convert value types at least for basic simple types that can be
      // directly mapped to Python
      const int numFields{res->PQnfields()};
      if ( resolveTypes )
      {
        // Collect oids
        QStringList oids;
        oids.reserve( numFields );
        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++ )
        {
          if ( feedback && feedback->isCanceled() )
          {
            break;
          }
          QString oidStr{QString::number( res->PQftype( rowIdx ) )};
          // TODO(Marcel): temporary fix until Redshift can return properly
          // geometries
          if ( oidStr == QStringLiteral( "3999" ) )
            oidStr = QString( "3000" );
          oids.push_back( oidStr );
        }

        const QList<QVariantList> typesResolved( executeSqlPrivate(
              QStringLiteral( "SELECT oid, typname FROM pg_type WHERE oid IN (%1)" ).arg( oids.join( ',' ) ), false,
              nullptr, rsConn ) );
        QgsStringMap oidTypeMap;
        for ( const auto &typeRes : std::as_const( typesResolved ) )
        {
          const QString oid{typeRes.constLast().toString()};
          if ( !oidTypeMap.contains( oid ) )
          {
            oidTypeMap.insert( typeRes.constFirst().toString(), typeRes.constLast().toString() );
          }
        }

        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++ )
        {
          static const QStringList intTypes =
          {
            QStringLiteral( "oid" ),
            QStringLiteral( "int2" ),
            QStringLiteral( "int4" ),
            QStringLiteral( "int8" ),
          };
          static const QStringList floatTypes = {QStringLiteral( "float4" ), QStringLiteral( "float8" ),
                                                 QStringLiteral( "numeric" )
                                                };

          const QString typName{oidTypeMap[oids.at( rowIdx )]};
          QVariant::Type vType{QVariant::Type::String};
          if ( floatTypes.contains( typName ) )
          {
            vType = QVariant::Double;
          }
          else if ( intTypes.contains( typName ) )
          {
            vType = QVariant::LongLong;
          }
          else if ( typName == QLatin1String( "date" ) )
          {
            vType = QVariant::Date;
          }
          else if ( typName.startsWith( QLatin1String( "timestamp" ) ) )
          {
            vType = QVariant::DateTime;
          }
          else if ( typName == QLatin1String( "time" ) )
          {
            vType = QVariant::Time;
          }
          else if ( typName == QLatin1String( "bool" ) )
          {
            vType = QVariant::Bool;
          }
          else if ( typName == QLatin1String( "char" ) )
          {
            vType = QVariant::Char;
          }
          else
          {
            // Just a warning, usually ok
            QgsDebugMsgLevel( QStringLiteral( "Unhandled Redshift type %1, assuming string" ).arg( typName ), 2 );
          }
          static_cast<QgsRedshiftProviderResultIterator *>( iterator.get() )->typeMap[rowIdx] = vType;
        }
      }
    }
    if ( !errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }
    static_cast<QgsRedshiftProviderResultIterator *>( iterator.get() )->result = std::move( res );
  }
  return results;
}

QVariantList QgsRedshiftProviderResultIterator::nextRowPrivate()
{
  // Get results
  QVariantList row;

  if ( mRowIndex >= result->PQntuples() )
  {
    return row;
  }

  for ( int colIdx = 0; colIdx < result->PQnfields(); colIdx++ )
  {
    if ( mResolveTypes )
    {
      const QVariant::Type vType{typeMap.value( colIdx, QVariant::Type::String )};
      QVariant val{result->PQgetvalue( mRowIndex, colIdx )};
      // Special case for bools: 'f' and 't'
      if ( vType == QVariant::Bool )
      {
        const QString boolStrVal{val.toString()};
        if ( !boolStrVal.isEmpty() )
        {
          val = boolStrVal == 't';
        }
      }
      else if ( val.canConvert( static_cast<int>( vType ) ) )
      {
        val.convert( static_cast<int>( vType ) );
      }
      row.push_back( val );
    }
    else
    {
      row.push_back( result->PQgetvalue( mRowIndex, colIdx ) );
    }
  }
  ++mRowIndex;
  return row;
}

bool QgsRedshiftProviderResultIterator::hasNextRowPrivate() const
{
  return mRowIndex < result->PQntuples();
}

long long QgsRedshiftProviderResultIterator::rowCountPrivate() const
{
  return result ? result->PQntuples() :
         static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
}

void QgsRedshiftProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::Vacuum );
  executeSql( QStringLiteral( "VACUUM FULL %1.%2" )
              .arg( QgsRedshiftConn::quotedIdentifier( schema ) )
              .arg( QgsRedshiftConn::quotedIdentifier( name ) ) );
}

QList<QgsRedshiftProviderConnection::TableProperty> QgsRedshiftProviderConnection::tables( const QString &schema,
    const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  QList<QgsRedshiftProviderConnection::TableProperty> tables;
  QString errCause;
  // TODO: set flags from the connection if flags argument is 0
  const QgsDataSourceUri dsUri{uri()};
  QgsRedshiftConn *conn = QgsRedshiftConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    bool ok = conn->getTableInfo( false, true, schema );
    if ( !ok )
    {
      errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( uri() );
    }
    else
    {
      QVector<QgsRedshiftLayerProperty> properties;
      const bool aspatial{!flags || flags.testFlag( TableFlag::Aspatial )};
      conn->supportedLayers( properties, schema == QStringLiteral( "public" ), aspatial, schema );
      bool dontResolveType = configuration().value( QStringLiteral( "dontResolveType" ), false ).toBool();
      bool useEstimatedMetadata = configuration().value( QStringLiteral( "estimatedMetadata" ), false ).toBool();

      // Cannot be const:
      for ( auto &pr : properties )
      {
        // Classify
        TableFlags prFlags;
        if ( pr.isView )
        {
          prFlags.setFlag( QgsRedshiftProviderConnection::TableFlag::View );
        }
        if ( pr.isMaterializedView )
        {
          prFlags.setFlag( QgsRedshiftProviderConnection::TableFlag::MaterializedView );
        }

        if ( pr.nSpCols != 0 )
        {
          prFlags.setFlag( QgsRedshiftProviderConnection::TableFlag::Vector );
        }
        else
        {
          prFlags.setFlag( QgsRedshiftProviderConnection::TableFlag::Aspatial );
        }
        // Filter
        if ( !flags || ( prFlags & flags ) )
        {
          // retrieve layer types if needed
          if ( !dontResolveType &&
               ( !pr.geometryColName.isNull() &&
                 ( pr.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown ||
                   pr.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) ) )
          {
            conn->retrieveLayerTypes( pr, useEstimatedMetadata );
          }
          QgsRedshiftProviderConnection::TableProperty property;
          property.setFlags( prFlags );
          for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ); i++ )
          {
            property.addGeometryColumnType( pr.types.at( i ),
                                            QgsCoordinateReferenceSystem::fromEpsgId( pr.srids.at( i ) ) );
          }
          property.setTableName( pr.tableName );
          property.setSchema( pr.schemaName );
          property.setGeometryColumn( pr.geometryColName );
          property.setPrimaryKeyColumns( pr.pkCols );
          property.setGeometryColumnCount( static_cast<int>( pr.nSpCols ) );
          property.setComment( pr.tableComment );
          tables.push_back( property );
        }
      }
    }
    QgsRedshiftConnPool::instance()->releaseConnection( conn );
  }
  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( errCause );
  }
  return tables;
}

QStringList QgsRedshiftProviderConnection::schemas() const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;
  QString errCause;
  const QgsDataSourceUri dsUri{uri()};
  QgsRedshiftConn *conn = QgsRedshiftConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    QList<QgsRedshiftSchemaProperty> schemaProperties;
    bool ok = conn->getSchemas( schemaProperties );
    QgsRedshiftConnPool::instance()->releaseConnection( conn );
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

void QgsRedshiftProviderConnection::store( const QString &name ) const
{
  QString baseKey = QStringLiteral( "/Redshift/connections/" );
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( name );

  // From URI
  const QgsDataSourceUri dsUri{uri()};
  settings.setValue( "host", dsUri.host() );
  settings.setValue( "port", dsUri.port() );
  settings.setValue( "database", dsUri.database() );
  settings.setValue( "username", dsUri.username() );
  settings.setValue( "password", dsUri.password() );
  settings.setValue( "authcfg", dsUri.authConfigId() );
  settings.setEnumValue( "sslmode", dsUri.sslMode() );

  // From configuration
  static const QStringList configurationParameters{QStringLiteral( "publicOnly" ),
      QStringLiteral( "dontResolveType" ),
      QStringLiteral( "allowGeometrylessTables" ),
      QStringLiteral( "saveUsername" ),
      QStringLiteral( "savePassword" ),
      QStringLiteral( "estimatedMetadata" ),
      QStringLiteral( "projectsInDatabase" )};
  for ( const auto &p : configurationParameters )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) );
    }
  }
  settings.endGroup();
  settings.endGroup();
}

void QgsRedshiftProviderConnection::remove( const QString &name ) const
{
  QgsRedshiftConn::deleteConnection( name );
}

QIcon QgsRedshiftProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconRedshift.svg" ) );
}

QList<QgsVectorDataProvider::NativeType> QgsRedshiftProviderConnection::nativeTypes() const
{
  QList<QgsVectorDataProvider::NativeType> types;
  QgsRedshiftConn *conn =
    QgsRedshiftConnPool::instance()->acquireConnection( QgsDataSourceUri{uri()}.connectionInfo( false ) );
  if ( conn )
  {
    types = conn->nativeTypes();
    QgsRedshiftConnPool::instance()->releaseConnection( conn );
  }
  if ( types.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  }
  return types;
}
