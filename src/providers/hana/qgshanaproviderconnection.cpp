/***************************************************************************
   qgshanaproviderconnection.cpp  -  QgsHanaProviderConnection
   --------------------------------------
   Date      : 07-04-2020
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaproviderconnection.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanaprovider.h"
#include "qgshanaresultset.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgsapplication.h"
#include "qgsexception.h"
#include "qgssettings.h"

#include "odbc/PreparedStatement.h"

QgsHanaProviderConnection::QgsHanaProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "hana" );
  QgsHanaSettings settings( name, true );
  setUri( settings.toDataSourceUri().uri() );
  setCapabilities();
}

QgsHanaProviderConnection::QgsHanaProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QgsHanaUtils::connectionInfo( QgsDataSourceUri( uri ) ), configuration )
{
  mProviderKey = QStringLiteral( "hana" );
  setCapabilities();
}

void QgsHanaProviderConnection::setCapabilities()
{
  /*
   * Capability::DropSchema         | CREATE SCHEMA from SYSTEMPRIVILEGE
   * Capability::CreateSchema       | CREATE SCHEMA from SYSTEMPRIVILEGE
   * Capability::CreateVectorTable  | Note
   * Capability::DropVectorTable    | Note
   * Capability::RenameVectorTable  | Note
   * Capability::ExecuteSql         | Note
   * Capability::SqlLayers          | Note
   * Capability::Tables             | CATALOG READ or DATA ADMIN from SYSTEMPRIVILEGE
   * Capability::Schemas            | CATALOG READ or DATA ADMIN from SYSTEMPRIVILEGE
   * Capability::TableExists        | CATALOG READ or DATA ADMIN from SYSTEMPRIVILEGE
   * Capability::Spatial,           | Always TRUE
   *
   * Note: Everyone has this privilege, but the execution might fail if the user does
   *       not have the necessary privileges for one of the objects in the query.
   */

  mCapabilities =
  {
    Capability::CreateVectorTable,
    Capability::DropVectorTable,
    Capability::RenameVectorTable,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    Capability::Spatial
  };

  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  const QString sql = QStringLiteral( "SELECT OBJECT_TYPE, PRIVILEGE, SCHEMA_NAME, OBJECT_NAME FROM PUBLIC.EFFECTIVE_PRIVILEGES "
                                      "WHERE USER_NAME = CURRENT_USER AND IS_VALID = 'TRUE'" );
  QgsHanaResultSetRef rsPrivileges = conn->executeQuery( sql );
  while ( rsPrivileges->next() )
  {
    QString objType = rsPrivileges->getString( 1 );
    QString privType = rsPrivileges->getString( 2 );
    if ( objType == QLatin1String( "SYSTEMPRIVILEGE" ) )
    {
      if ( privType == QLatin1String( "CREATE SCHEMA" ) )
        mCapabilities |= Capability::CreateSchema | Capability::DropSchema | Capability::RenameSchema;
      else if ( privType == QLatin1String( "CATALOG READ" ) || privType == QLatin1String( "DATA ADMIN" ) )
        mCapabilities |= Capability::Schemas | Capability::Tables | Capability::TableExists;
    }
    else if ( objType == QLatin1String( "TABLE" ) || objType == QLatin1String( "VIEW" ) )
    {
      if ( privType == QLatin1String( "SELECT" ) )
      {
        QString schemaName = rsPrivileges->getString( 3 );
        QString objName = rsPrivileges->getString( 4 );

        if ( schemaName == QLatin1String( "SYS" ) && objName == QLatin1String( "SCHEMAS" ) )
          mCapabilities |= Capability::Schemas;
        else if ( objName == QLatin1String( "TABLE_COLUMNS" ) )
          mCapabilities |= Capability::Tables | Capability::TableExists;
      }
    }
  }

  rsPrivileges->close();
}

void QgsHanaProviderConnection::dropTable( const QString &schema, const QString &name ) const
{
  executeSqlStatement( QStringLiteral( "DROP TABLE %1.%2" )
                       .arg( QgsHanaUtils::quotedIdentifier( schema ) )
                       .arg( QgsHanaUtils::quotedIdentifier( name ) ) );
}

void QgsHanaProviderConnection::createVectorTable( const QString &schema,
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
  // Set geometry column if it's not aspatial
  if ( wkbType != QgsWkbTypes::Type::Unknown &&  wkbType != QgsWkbTypes::Type::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "geom" ) ).toString() );
  }
  QMap<int, int> map;
  QString errCause;
  QgsVectorLayerExporter::ExportError errCode = QgsHanaProvider::createEmptyLayer(
        newUri.uri(),
        fields,
        wkbType,
        srs,
        overwrite,
        &map,
        &errCause,
        options
      );
  if ( errCode != QgsVectorLayerExporter::ExportError::NoError )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsHanaProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };

  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  dsUri.setGeometryColumn( tableInfo .geometryColumn() );
  return dsUri.uri( false );
}

void QgsHanaProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  dropTable( schema, name );
}

void QgsHanaProviderConnection::renameTable( const QString &schema, const QString &name, const QString &newName ) const
{
  executeSqlStatement( QStringLiteral( "RENAME TABLE %1.%2 TO %1.%3" )
                       .arg( QgsHanaUtils::quotedIdentifier( schema ) )
                       .arg( QgsHanaUtils::quotedIdentifier( name ) )
                       .arg( QgsHanaUtils::quotedIdentifier( newName ) ) );
}

void QgsHanaProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  renameTable( schema, name, newName );
}

void QgsHanaProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlStatement( QStringLiteral( "CREATE SCHEMA %1" )
                       .arg( QgsHanaUtils::quotedIdentifier( name ) ) );
}

void QgsHanaProviderConnection::dropSchema( const QString &name,  bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlStatement( QStringLiteral( "DROP SCHEMA %1 %2" )
                       .arg( QgsHanaUtils::quotedIdentifier( name ) )
                       .arg( force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

void QgsHanaProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlStatement( QStringLiteral( "RENAME SCHEMA %1 TO %2" )
                       .arg( QgsHanaUtils::quotedIdentifier( name ), QgsHanaUtils::quotedIdentifier( newName ) ) );
}

QList<QVariantList> QgsHanaProviderConnection::executeSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
    return QList<QVariantList>();

  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  if ( feedback && feedback->isCanceled() )
    return QList<QVariantList>();

  bool isQuery = false;

  try
  {
    odbc::PreparedStatementRef stmt = conn->prepareStatement( sql );
    isQuery = stmt->getMetaDataUnicode()->getColumnCount() > 0;
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( ex.what() );
  }

  if ( isQuery )
    return executeSqlQuery( *conn, sql, feedback );
  else
  {
    executeSqlStatement( *conn, sql );
    return QList<QVariantList>();
  }
}

QList<QVariantList> QgsHanaProviderConnection::executeSqlQuery( QgsHanaConnection &conn, const QString &sql, QgsFeedback *feedback ) const
{
  QList<QVariantList> results;

  try
  {
    QgsHanaResultSetRef resultSet = conn.executeQuery( sql );
    const unsigned short nColumns = resultSet->getMetadata().getColumnCount();
    while ( resultSet->next() )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      QVariantList row;
      for ( unsigned short i = 1; i <= nColumns; ++i )
        row.push_back( resultSet->getValue( i ) );
      results.push_back( row );
    }
    resultSet->close();
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( ex.what() );
  }

  return results;
}

void QgsHanaProviderConnection::executeSqlStatement( QgsHanaConnection &conn, const QString &sql ) const
{
  try
  {
    conn.execute( sql );
    conn.commit();
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( ex.what() );
  }
}

void QgsHanaProviderConnection::executeSqlStatement( const QString &sql ) const
{
  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  executeSqlStatement( *conn, sql );
}

QList<QgsHanaProviderConnection::TableProperty> QgsHanaProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );

  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  QList<QgsHanaProviderConnection::TableProperty> tables;

  try
  {
    const QVector<QgsHanaLayerProperty> layers = conn->getLayersFull( schema, flags.testFlag( TableFlag::Aspatial ), false );
    for ( const QgsHanaLayerProperty &layerInfo :  layers )
    {
      // Classify
      TableFlags prFlags;
      if ( layerInfo.isView )
        prFlags.setFlag( QgsHanaProviderConnection::TableFlag::View );
      else if ( !layerInfo.geometryColName.isEmpty() )
        prFlags.setFlag( QgsHanaProviderConnection::TableFlag::Vector );
      else
        prFlags.setFlag( QgsHanaProviderConnection::TableFlag::Aspatial );

      // Filter
      if ( ! flags || ( prFlags & flags ) )
      {
        QgsHanaProviderConnection::TableProperty property;
        property.setFlags( prFlags );

        QgsCoordinateReferenceSystem crs = conn->getCrs( layerInfo.srid );
        property.addGeometryColumnType( layerInfo.type, crs );

        property.setTableName( layerInfo.tableName );
        property.setSchema( layerInfo.schemaName );
        property.setGeometryColumn( layerInfo.geometryColName );
        property.setPrimaryKeyColumns( layerInfo.pkCols );
        property.setGeometryColumnCount( layerInfo.geometryColName.isEmpty() ? 0 : 1 );
        property.setComment( layerInfo.tableComment );
        tables.push_back( property );
      }
    }
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve tables: %1, %2" ).arg( uri(), ex.what() ) );
  }

  return tables;
}

QStringList QgsHanaProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );

  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  try
  {
    QStringList schemas;
    const QVector<QgsHanaSchemaProperty> schemaProperties = conn->getSchemas( QString() );
    for ( const QgsHanaSchemaProperty &s : schemaProperties )
      schemas.push_back( s.name );
    return schemas;
  }
  catch ( const QgsHanaException &ex )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve schemas: %1, %2" ).arg( uri(), ex.what() ) );
  }
}

void QgsHanaProviderConnection::store( const QString &name ) const
{
  // delete the original entry first
  remove( name );

  QgsHanaSettings settings( name );
  settings.setFromDataSourceUri( uri() );
  settings.setSaveUserName( true );
  settings.setSavePassword( true );
  settings.save();
}

void QgsHanaProviderConnection::remove( const QString &name ) const
{
  QgsHanaSettings::removeConnection( name );
}

QIcon QgsHanaProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconHana.svg" ) );
}

QList<QgsVectorDataProvider::NativeType> QgsHanaProviderConnection::nativeTypes() const
{
  const QgsDataSourceUri dsUri { uri() };
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );

  QList<QgsVectorDataProvider::NativeType> types = conn->getNativeTypes();
  if ( types.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  return types;
}
