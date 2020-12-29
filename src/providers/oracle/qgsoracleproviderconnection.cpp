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

#include <QSqlRecord>

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "oracle" );
  // Remove the sql and table empty parts
  const QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*)raw" };
  setUri( QgsOracleConn::connUri( name ).uri().replace( removePartsRe, QString() ) );
  setDefaultCapabilities();
}

QgsOracleProviderConnection::QgsOracleProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = QStringLiteral( "oracle" );
  setDefaultCapabilities();
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
    //Capability::RenameSchema,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::RenameVectorTable,
    Capability::RenameRasterTable,
    Capability::Vacuum,
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
  settings.setValue( "dbworkspace", dsUri.param( "dbworkspace" ) );
  settings.setValue( "estimatedMetadata", dsUri.useEstimatedMetadata() );
  settings.setValue( "host", dsUri.host() );
  settings.setValue( "includeGeoAttributes", dsUri.param( "includegeoattributes" ) );
  settings.setValue( "port", dsUri.port() );
  settings.setValue( "schema", dsUri.schema() );
  settings.setValue( "dboptions", dsUri.param( "dboptions" ) );

  // From configuration
  static const QStringList configurationParameters
  {
    QStringLiteral( "allowGeometrylessTables" ),
    QStringLiteral( "geometryColumnsOnly" ),
    QStringLiteral( "onlyExistingTypes" ),
    QStringLiteral( "savePassword" ),
    QStringLiteral( "saveUsername" ),
    QStringLiteral( "userTablesOnly" ),
  };
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

void QgsOracleProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( QStringLiteral( "CREATE USER %1" )
                     .arg( QgsOracleConn::quotedIdentifier( name ) ) );
}

void QgsOracleProviderConnection::dropSchema( const QString &name,  bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlPrivate( QStringLiteral( "DROP USER %1 %2" )
                     .arg( QgsOracleConn::quotedIdentifier( name ) )
                     .arg( force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

QStringList QgsOracleProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;

  QList<QVariantList> users = executeSqlPrivate( QStringLiteral( "SELECT USERNAME FROM ALL_USERS" ) );
  for ( QVariantList userInfos : users )
    schemas << userInfos.at( 0 ).toString();

  return schemas;
}

QList<QVariantList> QgsOracleProviderConnection::executeSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{
  QList<QVariantList> results;

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
  {
    return results;
  }

  QgsPoolOracleConn pconn( QgsDataSourceUri{ uri() }.connectionInfo( false ) );
  if ( !pconn.get() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }

  if ( feedback && feedback->isCanceled() )
  {
    return results;
  }

  QSqlQuery qry( *pconn.get() );
  if ( !qry.exec( sql ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "SQL error: %1 returned %2" )
                                          .arg( qry.lastQuery(),
                                              qry.lastError().text() ) );
  }

  const int nbFields = qry.record().count();
  while ( qry.next() )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return results;
    }

    QVariantList cols;
    for ( int i = 0; i < nbFields; i++ )
      cols << qry.value( i );

    results << cols;
  }

  return results;
}
