/***************************************************************************
    qgspostgresdataitems.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgresdataitems.h"

#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprojectstorage.h"
#include "qgspostgresprovider.h"
#include "qgscolumntypethread.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsprojectstorageregistry.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsvectorlayerexporter.h"
#include "qgsprojectitem.h"
#include "qgsfieldsitem.h"
#include <QMessageBox>
#include <climits>

bool QgsPostgresUtils::deleteLayer( const QString &uri, QString &errCause )
{
  QgsDebugMsgLevel( "deleting layer " + uri, 2 );

  QgsDataSourceUri dsUri( uri );
  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  schemaTableName += QgsPostgresConn::quotedIdentifier( tableName );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // handle deletion of views
  QString sqlViewCheck = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" )
                         .arg( QgsPostgresConn::quotedValue( schemaTableName ) );
  QgsPostgresResult resViewCheck( conn->PQexec( sqlViewCheck ) );
  QString type = resViewCheck.PQgetvalue( 0, 0 );
  if ( type == QLatin1String( "v" ) || type == QLatin1String( "m" ) )
  {
    QString sql = QStringLiteral( "DROP %1VIEW %2" ).arg( type == QLatin1String( "m" ) ? QStringLiteral( "MATERIALIZED " ) : QString(), schemaTableName );
    QgsPostgresResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      errCause = QObject::tr( "Unable to delete view %1: \n%2" )
                 .arg( schemaTableName,
                       result.PQresultErrorMessage() );
      conn->unref();
      return false;
    }
    conn->unref();
    return true;
  }


  // check the geometry column count
  QString sql = QString( "SELECT count(*) "
                         "FROM geometry_columns, pg_class, pg_namespace "
                         "WHERE f_table_name=relname AND f_table_schema=nspname "
                         "AND pg_class.relnamespace=pg_namespace.oid "
                         "AND f_table_schema=%1 AND f_table_name=%2" )
                .arg( QgsPostgresConn::quotedValue( schemaName ),
                      QgsPostgresConn::quotedValue( tableName ) );
  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName,
                     result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  int count = result.PQgetvalue( 0, 0 ).toInt();

  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    sql = QStringLiteral( "SELECT DropGeometryColumn(%1,%2,%3)" )
          .arg( QgsPostgresConn::quotedValue( schemaName ),
                QgsPostgresConn::quotedValue( tableName ),
                QgsPostgresConn::quotedValue( geometryCol ) );
  }
  else
  {
    // drop the table
    sql = QStringLiteral( "SELECT DropGeometryTable(%1,%2)" )
          .arg( QgsPostgresConn::quotedValue( schemaName ),
                QgsPostgresConn::quotedValue( tableName ) );
  }

  result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName,
                     result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

bool QgsPostgresUtils::deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade )
{
  QgsDebugMsgLevel( "deleting schema " + schema, 2 );

  if ( schema.isEmpty() )
    return false;

  QString schemaName = QgsPostgresConn::quotedIdentifier( schema );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // drop the schema
  QString sql = QStringLiteral( "DROP SCHEMA %1 %2" )
                .arg( schemaName, cascade ? QStringLiteral( "CASCADE" ) : QString() );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    errCause = QObject::tr( "Unable to delete schema %1: \n%2" )
               .arg( schemaName,
                     result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}


// ---------------------------------------------------------------------------
QgsPGConnectionItem::QgsPGConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "PostGIS" ) )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsPGConnectionItem::createChildren()
{

  QVector<QgsDataItem *>items;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( mName );
  // TODO: we need to cancel somehow acquireConnection() if deleteLater() was called on this item to avoid later credential dialog if connection failed
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo( false ) );
    return items;
  }

  QList<QgsPostgresSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsPostgresConnPool::instance()->releaseConnection( conn );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get schemas" ), mPath + "/error" ) );
    return items;
  }

  const auto constSchemas = schemas;
  for ( const QgsPostgresSchemaProperty &schema : constSchemas )
  {
    QgsPGSchemaItem *schemaItem = new QgsPGSchemaItem( this, mName, schema.name, mPath + '/' + schema.name );
    if ( !schema.description.isEmpty() )
    {
      schemaItem->setToolTip( schema.description );
    }
    items.append( schemaItem );
  }

  return items;
}

bool QgsPGConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsPGConnectionItem *o = qobject_cast<const QgsPGConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}


void QgsPGConnectionItem::refreshSchema( const QString &schema )
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

bool QgsPGConnectionItem::handleDrop( const QMimeData *data, const QString &toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceUri uri = QgsPostgresConn::connUri( mName );

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
      uri.setDataSource( QString(), u.name,  srcLayer->geometryType() != QgsWkbTypes::NullGeometry ? QStringLiteral( "geom" ) : QString() );
      QgsDebugMsgLevel( "URI " + uri.uri( false ), 2 );

      if ( !toSchema.isNull() )
      {
        uri.setSchema( toSchema );
      }

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( new QgsVectorLayerExporterTask( srcLayer, uri.uri( false ), QStringLiteral( "postgres" ), srcLayer->crs(), QVariantMap(), owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to PostGIS database" ), tr( "Import was successful." ) );
        refreshSchema( toSchema );
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( Qgis::VectorExportResult error, const QString & errorMessage )
      {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to PostGIS database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
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
    output->setTitle( tr( "Import to PostGIS database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsPGLayerItem::QgsPGLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsPostgresLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, layerProperty.isRaster ? QStringLiteral( "postgresraster" ) : QStringLiteral( "postgres" ) )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;
  mUri = createUri();
  // No rasters for now
  setState( layerProperty.isRaster ? Qgis::BrowserItemState::Populated : Qgis::BrowserItemState::NotPopulated );
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QString QgsPGLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

QString QgsPGLayerItem::createUri()
{
  QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  const QString &connName = connItem->name();

  QgsDataSourceUri uri( QgsPostgresConn::connUri( connName ).connectionInfo( false ) );

  const QgsSettings &settings = QgsSettings();
  QString basekey = QStringLiteral( "/PostgreSQL/connections/%1" ).arg( connName );

  QStringList defPk( settings.value(
                       QStringLiteral( "%1/keys/%2/%3" ).arg( basekey, mLayerProperty.schemaName, mLayerProperty.tableName ),
                       QVariant( !mLayerProperty.pkCols.isEmpty() ? QStringList( mLayerProperty.pkCols.at( 0 ) ) : QStringList() )
                     ).toStringList() );

  const bool useEstimatedMetadata = QgsPostgresConn::useEstimatedMetadata( connName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );

  QStringList cols;
  for ( const auto &col : defPk )
  {
    cols << QgsPostgresConn::quotedIdentifier( col );
  }

  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, cols.join( ',' ) );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( uri.wkbType() != QgsWkbTypes::NoGeometry && mLayerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );

  QgsDebugMsgLevel( QStringLiteral( "layer uri: %1" ).arg( uri.uri( false ) ), 2 );
  return uri.uri( false );
}

// ---------------------------------------------------------------------------
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path )
  : QgsDatabaseSchemaItem( parent, name, path, QStringLiteral( "PostGIS" ) )
  , mConnectionName( connectionName )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
}

QVector<QgsDataItem *> QgsPGSchemaItem::createChildren()
{
  QVector<QgsDataItem *>items;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );

  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo( false ) );
    return items;
  }

  QVector<QgsPostgresLayerProperty> layerProperties;
  const bool ok = conn->supportedLayers( layerProperties,
                                         QgsPostgresConn::geometryColumnsOnly( mConnectionName ),
                                         QgsPostgresConn::publicSchemaOnly( mConnectionName ),
                                         QgsPostgresConn::allowGeometrylessTables( mConnectionName ), mName );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get layers" ), mPath + "/error" ) );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    return items;
  }

  const bool dontResolveType = QgsPostgresConn::dontResolveType( mConnectionName );
  const bool estimatedMetadata = QgsPostgresConn::useEstimatedMetadata( mConnectionName );
  const auto constLayerProperties = layerProperties;
  for ( QgsPostgresLayerProperty layerProperty : constLayerProperties )
  {
    if ( layerProperty.schemaName != mName )
      continue;

    if ( !layerProperty.geometryColName.isNull() &&
         !layerProperty.isRaster &&
         ( layerProperty.types.value( 0, QgsWkbTypes::Unknown ) == QgsWkbTypes::Unknown  ||
           layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      if ( dontResolveType )
      {
        QgsDebugMsgLevel( QStringLiteral( "skipping column %1.%2 without type constraint" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ), 2 );
        continue;
      }
      // If the table is empty there is no way we can retrieve layer types, let's make a copy and restore it
      QgsPostgresLayerProperty propertyCopy { layerProperty };
      conn->retrieveLayerTypes( layerProperty, estimatedMetadata );
      if ( layerProperty.size() == 0 )
      {
        layerProperty = propertyCopy;
      }
    }

    for ( int i = 0; i < layerProperty.size(); i++ )
    {
      QgsDataItem *layerItem = nullptr;
      layerItem = createLayer( layerProperty.at( i ) );
      if ( layerItem )
        items.append( layerItem );
    }
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "postgresql" );
  if ( QgsPostgresConn::allowProjectsInDatabase( mConnectionName ) && storage )
  {
    QgsPostgresProjectUri postUri;
    postUri.connInfo = uri;
    postUri.schemaName = mName;
    QString schemaUri = QgsPostgresProjectStorage::encodeUri( postUri );
    const QStringList projectNames = storage->listProjects( schemaUri );
    for ( const QString &projectName : projectNames )
    {
      QgsPostgresProjectUri projectUri( postUri );
      projectUri.projectName = projectName;
      items.append( new QgsProjectItem( this, projectName, QgsPostgresProjectStorage::encodeUri( projectUri ) ) );
    }
  }

  return items;
}


QgsPGLayerItem *QgsPGSchemaItem::createLayer( QgsPostgresLayerProperty layerProperty )
{
  //QgsDebugMsg( "schemaName = " + layerProperty.schemaName + " tableName = " + layerProperty.tableName + " geometryColName = " + layerProperty.geometryColName );
  QString tip;
  if ( layerProperty.isView && ! layerProperty.isMaterializedView )
  {
    tip = tr( "View" );
  }
  else if ( layerProperty.isView && layerProperty.isMaterializedView )
  {
    tip = tr( "Materialized view" );
  }
  else if ( layerProperty.isRaster )
  {
    tip = tr( "Raster" );
  }
  else if ( layerProperty.isForeignTable )
  {
    tip = tr( "Foreign table" );
  }
  else
  {
    tip = tr( "Table" );
  }

  QgsWkbTypes::Type wkbType = layerProperty.types.at( 0 );
  if ( ! layerProperty.isRaster )
  {
    tip += tr( "\n%1 as %2" ).arg( layerProperty.geometryColName, QgsPostgresConn::displayStringForWkbType( wkbType ) );
  }

  if ( layerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    tip += tr( " (srid %1)" ).arg( layerProperty.srids.at( 0 ) );
  else
    tip += tr( " (unknown srid)" );

  if ( !layerProperty.tableComment.isEmpty() )
  {
    tip = layerProperty.tableComment + '\n' + tip;
  }


  Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::Raster;
  if ( ! layerProperty.isRaster )
  {
    QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( ( QgsWkbTypes::Type )wkbType );
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        layerType = Qgis::BrowserLayerType::Point;
        break;
      case QgsWkbTypes::LineGeometry:
        layerType = Qgis::BrowserLayerType::Line;
        break;
      case QgsWkbTypes::PolygonGeometry:
        layerType = Qgis::BrowserLayerType::Polygon;
        break;
      default:
        if ( !layerProperty.geometryColName.isEmpty() )
        {
          QgsDebugMsgLevel( QStringLiteral( "Adding layer item %1.%2 without type constraint as geometryless table" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ), 2 );
        }
        layerType = Qgis::BrowserLayerType::TableLayer;
        tip = tr( "as geometryless table" );
    }
  }

  QgsPGLayerItem *layerItem = new QgsPGLayerItem( this, layerProperty.defaultName(), mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

QVector<QgsDataItem *> QgsPGLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, uri() + QStringLiteral( "/columns/ " ), createUri(), providerKey(), mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return children;
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "PostGIS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconPostgis.svg" );
  populate();
}

QVector<QgsDataItem *> QgsPGRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsPostgresConn::connectionList();
  for ( const QString &connName : list )
  {
    connections << new QgsPGConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

void QgsPGRootItem::onConnectionsChanged()
{
  refresh();
}

QString QgsPostgresDataItemProvider::name()
{
  return QStringLiteral( "PostGIS" );
}

QString QgsPostgresDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "postgres" );
}

int QgsPostgresDataItemProvider::capabilities() const
{
  return QgsDataProvider::Database;
}

QgsDataItem *QgsPostgresDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsPGRootItem( parentItem, QObject::tr( "PostgreSQL" ), QStringLiteral( "pg:" ) );
}


bool QgsPGSchemaItem::layerCollection() const
{
  return true;
}
