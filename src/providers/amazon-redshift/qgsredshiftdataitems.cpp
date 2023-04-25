/***************************************************************************
   qgsredshiftdataitems.cpp
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
#include "qgsredshiftdataitems.h"

#include <QMessageBox>
#include <climits>

#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgsfieldsitem.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsprojectitem.h"
#include "qgsprojectstorageregistry.h"
#include "qgsredshiftconn.h"
#include "qgsredshiftconnpool.h"
#include "qgsredshiftprojectstorage.h"
#include "qgsredshiftprovider.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

bool QgsRedshiftUtils::deleteLayer( const QString &uri, QString &errCause )
{
  QgsDebugMsg( "deleting layer " + uri );

  QgsDataSourceUri dsUri( uri );
  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsRedshiftConn::quotedIdentifier( schemaName ) + '.';
  }
  schemaTableName += QgsRedshiftConn::quotedIdentifier( tableName );

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // handle deletion of views
  QString sqlViewCheck = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" )
                         .arg( QgsRedshiftConn::quotedValue( schemaTableName ) );
  QgsRedshiftResult resViewCheck( conn->PQexec( sqlViewCheck ) );
  QString type = resViewCheck.PQgetvalue( 0, 0 );
  if ( type == QLatin1String( "v" ) )
  {
    bool isMaterializedView = conn->isMaterializedView( schemaName, tableName );
    QString sql = QStringLiteral( "DROP %1VIEW %2" ).arg( isMaterializedView ? QStringLiteral( "MATERIALIZED " ) : QString(), schemaTableName );
    QgsRedshiftResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      errCause =
        QObject::tr( "Unable to delete view %1: \n%2" ).arg( schemaTableName, result.PQresultErrorMessage() );
      conn->unref();
      return false;
    }
    conn->unref();
    return true;
  }

  // check the geometry column count
  QString sql = QString( "SELECT count(*) "
                         "FROM pg_class as c, pg_namespace as n, pg_attribute "
                         "as att, pg_type as t "
                         "WHERE n.nspname=%1"
                         "AND t.typname='geometry'"
                         "AND c.relnamespace=n.oid and c.relname=%2"
                         "AND att.attrelid=c.oid AND att.atttypid=t.oid" )
                .arg( QgsRedshiftConn::quotedValue( schemaName ), QgsRedshiftConn::quotedValue( tableName ) );
  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" ).arg( schemaTableName, result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  int count = result.PQgetvalue( 0, 0 ).toInt();

  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    sql = QStringLiteral( "DROP COLUMN %1.%2.%3" )
          .arg( QgsRedshiftConn::quotedIdentifier( schemaName ), QgsRedshiftConn::quotedIdentifier( tableName ),
                QgsRedshiftConn::quotedIdentifier( geometryCol ) );
  }
  else
  {
    // drop the table
    sql = QStringLiteral( "DROP TABLE %1.%2" )
          .arg( QgsRedshiftConn::quotedIdentifier( schemaName ), QgsRedshiftConn::quotedIdentifier( tableName ) );
  }

  result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" ).arg( schemaTableName, result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

bool QgsRedshiftUtils::deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade )
{
  QgsDebugMsg( "deleting schema " + schema );

  if ( schema.isEmpty() )
    return false;

  QString schemaName = QgsRedshiftConn::quotedIdentifier( schema );

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // drop the schema
  QString sql = QStringLiteral( "DROP SCHEMA %1 %2" ).arg( schemaName, cascade ? QStringLiteral( "CASCADE" ) : QString() );

  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    errCause = QObject::tr( "Unable to delete schema %1: \n%2" ).arg( schemaName, result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

// ---------------------------------------------------------------------------
QgsRSConnectionItem::QgsRSConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "Redshift" ) )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsRSConnectionItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsDataSourceUri uri = QgsRedshiftConn::connUri( mName );
  // TODO: we need to cancel somehow acquireConnection() if deleteLater() was
  // called on this item to avoid later credential dialog if connection failed
  QgsRedshiftConn *conn = QgsRedshiftConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo( false ) );
    return items;
  }

  QList<QgsRedshiftSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get schemas" ), mPath + "/error" ) );
    return items;
  }

  const auto constSchemas = schemas;
  for ( const QgsRedshiftSchemaProperty &schema : constSchemas )
  {
    QgsRSSchemaItem *schemaItem = new QgsRSSchemaItem( this, mName, schema.name, mPath + '/' + schema.name );
    if ( !schema.description.isEmpty() )
    {
      schemaItem->setToolTip( schema.description );
    }
    items.append( schemaItem );
  }

  return items;
}

bool QgsRSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsRSConnectionItem *o = qobject_cast<const QgsRSConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

void QgsRSConnectionItem::refreshSchema( const QString &schema )
{
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    if ( child->name() == schema || schema.isEmpty() )
    {
      child->refresh();
    }
  }
}

bool QgsRSConnectionItem::handleDrop( const QMimeData *data, const QString &toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceUri uri = QgsRedshiftConn::connUri( mName );

  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  const auto constLst = lst;
  for ( const QgsMimeDataUtils::Uri &u : constLst )
  {
    // open the source layer
    bool owner;
    QString error;
    QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
    if ( !srcLayer )
    {
      importResults.append( tr( "%1: %2" ).arg( u.name, error ) );
      hasError = true;
      continue;
    }

    if ( srcLayer->isValid() )
    {
      uri.setDataSource( QString(), u.name,
                         srcLayer->geometryType() != Qgis::GeometryType::Null ? QStringLiteral( "geom" )
                         : QString() );
      QgsDebugMsg( "URI " + uri.uri( false ) );

      if ( !toSchema.isNull() )
      {
        uri.setSchema( toSchema );
      }

      std::unique_ptr<QgsVectorLayerExporterTask> exportTask( new QgsVectorLayerExporterTask(
            srcLayer, uri.uri( false ), QStringLiteral( "redshift" ), srcLayer->crs(), QVariantMap(), owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar
        // from data items
        QMessageBox::information( nullptr, tr( "Import to Redshift database" ), tr( "Import was successful." ) );
        refreshSchema( toSchema );
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this,
               [ = ]( Qgis::VectorExportResult error, const QString & errorMessage )
      {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to Redshift database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage,
                              QgsMessageOutput::MessageText );
          output->showMessage();
        }
        refreshSchema( toSchema );
      } );

      QgsApplication::taskManager()->addTask( exportTask.release() );
    }
    else
    {
      importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
      hasError = true;
    }
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to Redshift database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ),
                        QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsRSLayerItem::QgsRSLayerItem( QgsDataItem *parent, const QString &name, const QString &path,
                                Qgis::BrowserLayerType layerType, const QgsRedshiftLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "redshift" ) ), mLayerProperty( layerProperty )
{
  mCapabilities |=
    Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;
  mUri = createUri();

  setState( Qgis::BrowserItemState::NotPopulated );
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QString QgsRSLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

QString QgsRSLayerItem::createUri()
{
  QgsRSConnectionItem *connItem = qobject_cast<QgsRSConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  const QString &connName = connItem->name();

  QgsDataSourceUri uri( QgsRedshiftConn::connUri( connName ).connectionInfo( false ) );

  const QgsSettings &settings = QgsSettings();
  QString basekey = QStringLiteral( "/Redshift/connections/%1" ).arg( connName );

  QStringList defPk(
    settings
    .value(
      QStringLiteral( "%1/keys/%2/%3" ).arg( basekey, mLayerProperty.schemaName, mLayerProperty.tableName ),
      QVariant( !mLayerProperty.pkCols.isEmpty() ? QStringList( mLayerProperty.pkCols.at( 0 ) ) : QStringList() ) )
    .toStringList() );

  const bool useEstimatedMetadata = QgsRedshiftConn::useEstimatedMetadata( connName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );

  QStringList cols;
  for ( const auto &col : defPk )
  {
    cols << QgsRedshiftConn::quotedIdentifier( col );
  }

  uri.setDataSource( mLayerProperty.schemaName,
                     mLayerProperty.tableName, mLayerProperty.geometryColName,
                     mLayerProperty.sql, cols.join( ',' ),
                     mLayerProperty.databaseName );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( uri.wkbType() != Qgis::WkbType::NoGeometry && mLayerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );

  QgsDebugMsg( QStringLiteral( "layer uri: %1" ).arg( uri.uri( false ) ) );
  return uri.uri( false );
}

// ---------------------------------------------------------------------------
QgsRSSchemaItem::QgsRSSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name,
                                  const QString &path )
  : QgsDatabaseSchemaItem( parent, name, path, QStringLiteral( "Redshift" ) ), mConnectionName( connectionName )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
}

QVector<QgsDataItem *> QgsRSSchemaItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsDataSourceUri uri = QgsRedshiftConn::connUri( mConnectionName );
  QgsRedshiftConn *conn = QgsRedshiftConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );

  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo( false ) );
    return items;
  }

  QVector<QgsRedshiftLayerProperty> layerProperties;
  bool ok = conn->supportedLayers( layerProperties, QgsRedshiftConn::publicSchemaOnly( mConnectionName ),
                                   QgsRedshiftConn::allowGeometrylessTables( mConnectionName ), mName );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get layers" ), mPath + "/error" ) );
    QgsRedshiftConnPool::instance()->releaseConnection( conn );
    return items;
  }

  bool dontResolveType = QgsRedshiftConn::dontResolveType( mConnectionName );
  bool estimatedMetadata = QgsRedshiftConn::useEstimatedMetadata( mConnectionName );
  const auto constLayerProperties = layerProperties;
  for ( QgsRedshiftLayerProperty layerProperty : constLayerProperties )
  {
    if ( layerProperty.schemaName != mName )
      continue;

    if ( !layerProperty.geometryColName.isNull() &&
         ( layerProperty.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown ||
           layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      if ( dontResolveType )
      {
        // QgsDebugMsg( QStringLiteral( "skipping column %1.%2 without type
        // constraint" ).arg( layerProperty.schemaName ).arg(
        // layerProperty.tableName ) );
        continue;
      }
      conn->retrieveLayerTypes( layerProperty, estimatedMetadata );
    }

    for ( int i = 0; i < layerProperty.size(); i++ )
    {
      QgsDataItem *layerItem = nullptr;
      layerItem = createLayer( layerProperty.at( i ) );
      if ( layerItem )
        items.append( layerItem );
    }
  }

  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "redshift" );
  if ( QgsRedshiftConn::allowProjectsInDatabase( mConnectionName ) && storage )
  {
    QgsRedshiftProjectUri postUri;
    postUri.connInfo = uri;
    postUri.schemaName = mName;
    QString schemaUri = QgsRedshiftProjectStorage::encodeUri( postUri );
    const QStringList projectNames = storage->listProjects( schemaUri );
    for ( const QString &projectName : projectNames )
    {
      QgsRedshiftProjectUri projectUri( postUri );
      projectUri.projectName = projectName;
      items.append( new QgsProjectItem( this, projectName, QgsRedshiftProjectStorage::encodeUri( projectUri ) ) );
    }
  }

  return items;
}

QgsRSLayerItem *QgsRSSchemaItem::createLayer( QgsRedshiftLayerProperty layerProperty )
{
  // QgsDebugMsg( "schemaName = " + layerProperty.schemaName + " tableName = " +
  // layerProperty.tableName + " geometryColName = " +
  // layerProperty.geometryColName );
  QString tip;
  if ( layerProperty.isView && !layerProperty.isMaterializedView )
  {
    tip = tr( "View" );
  }
  else if ( layerProperty.isView && layerProperty.isMaterializedView )
  {
    tip = tr( "Materialized view" );
  }
  else
  {
    tip = tr( "Table" );
  }

  Qgis::WkbType wkbType = layerProperty.types.at( 0 );

  tip += tr( "\n%1 as %2" ).arg( layerProperty.geometryColName, QgsRedshiftConn::displayStringForWkbType( wkbType ) );

  if ( layerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    tip += tr( " (srid %1)" ).arg( layerProperty.srids.at( 0 ) );
  else
    tip += tr( " (unknown srid)" );

  if ( !layerProperty.tableComment.isEmpty() )
  {
    tip = layerProperty.tableComment + '\n' + tip;
  }

  Qgis::BrowserLayerType layerType;

  Qgis::GeometryType geomType = QgsWkbTypes::geometryType( ( Qgis::WkbType )wkbType );
  switch ( geomType )
  {
    case Qgis::GeometryType::Point:
      layerType = Qgis::BrowserLayerType::Point;
      break;
    case Qgis::GeometryType::Line:
      layerType = Qgis::BrowserLayerType::Line;
      break;
    case Qgis::GeometryType::Polygon:
      layerType = Qgis::BrowserLayerType::Polygon;
      break;
    default:
      if ( !layerProperty.geometryColName.isEmpty() )
        return nullptr;

      layerType = Qgis::BrowserLayerType::TableLayer;
      tip = tr( "as geometryless table" );
  }

  QgsRSLayerItem *layerItem = new QgsRSLayerItem( this, layerProperty.defaultName(),
      mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

QVector<QgsDataItem *> QgsRSLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, uri() + QStringLiteral( "/columns/ " ), createUri(), providerKey(),
                                         mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return children;
}

// ---------------------------------------------------------------------------
QgsRSRootItem::QgsRSRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "Redshift" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconRedshift.svg" );
  populate();
}

QVector<QgsDataItem *> QgsRSRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsRedshiftConn::connectionList();
  for ( const QString &connName : list )
  {
    connections << new QgsRSConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

void QgsRSRootItem::onConnectionsChanged()
{
  refresh();
}

QMainWindow *QgsRSRootItem::sMainWindow = nullptr;

QString QgsRedshiftDataItemProvider::name()
{
  return QStringLiteral( "Redshift" );
}

QString QgsRedshiftDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "redshift" );
}

int QgsRedshiftDataItemProvider::capabilities() const
{
  return QgsDataProvider::Database;
}

QgsDataItem *QgsRedshiftDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsRSRootItem( parentItem, QStringLiteral( "Amazon Redshift" ), QStringLiteral( "rs:" ) );
}

bool QgsRSSchemaItem::layerCollection() const
{
  return true;
}
