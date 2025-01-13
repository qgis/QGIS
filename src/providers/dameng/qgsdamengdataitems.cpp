/***************************************************************************
    qgsdamengdataitems.cpp
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

#include "qgsdamengdataitems.h"
#include "qgsdamengconn.h"
#include "qgsdamengconnpool.h"
#include "qgsdamengprojectstorage.h"
#include "qgsdamengprovider.h"
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
#include <qgsabstractdatabaseproviderconnection.h>

bool QgsDamengUtils::deleteLayer( const QString &uri, QString &errCause )
{
  QgsDebugMsgLevel( "deleting layer " + uri, 2 );

  QgsDataSourceUri dsUri( uri );
  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsDamengConn::quotedIdentifier( schemaName ) + '.';
  }
  schemaTableName += QgsDamengConn::quotedIdentifier( tableName );

  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  QgsDMResult *resViewCheck( conn->DMexec( QStringLiteral( "select ID from SYS.SYSOBJECTS where name = \'%1\' and TYPE$ = \'SCH\';" ).arg( schemaName ) ) );
  resViewCheck->fetchNext();
  QString schemaId = resViewCheck->value( 0 ).toString();

  // handle deletion of views
  QString sqlViewCheck = QStringLiteral( "select INFO5 from SYSOBJECTS "
                                         " where SUBTYPE$ = \'VIEW\' and "
                                         " NAME = %1 and %2 in SCHID; " )
                         .arg( QgsDamengConn::quotedValue( tableName ) ).arg( schemaId );
  resViewCheck = conn->DMexec( sqlViewCheck );
  if ( resViewCheck->fetchNext() )
  {
    QString sql = resViewCheck->value( 0 ).toString() == "0x"
                  ? QStringLiteral( "DROP VIEW %2" ).arg( schemaTableName )
                  : QStringLiteral( "DROP MATERIALIZED VIEW %2" ).arg( schemaTableName );

    QgsDamengResult result( conn->DMexec( sql ) );
    if ( result.DMresultStatus() != DmResCommandOk )
    {
      errCause = QObject::tr( "Unable to delete view %1: \n%2" )
        .arg( schemaTableName, result.DMresultErrorMessage() );
      conn->unref();
      return false;
    }

    conn->unref();
    return true;
  }

  // check deletion of materialized views
  if ( tableName.startsWith( "MTAB$_" ) )
  {
    tableName = tableName.mid( 6 );
    QString sql =  QStringLiteral( "DROP MATERIALIZED VIEW %1.%2" )
                        .arg( QgsDamengConn::quotedIdentifier( schemaName ) )
                        .arg( QgsDamengConn::quotedIdentifier( tableName ) );

    QgsDamengResult result( conn->DMexec( sql ) );
    if ( result.DMresultStatus() != DmResCommandOk )
    {
      errCause = QObject::tr( "Unable to delete table %1: \n%2" )
        .arg( schemaTableName, result.DMresultErrorMessage() );
      conn->unref();
      return false;
    }

    conn->unref();
    return true;
  }

  // check the geometry column count
  QString sql = QString( "select count(*) from SYSGEO2.geometry_columns "
                         " where F_TABLE_SCHEMA = %1 and F_TABLE_NAME = %2;" )
                .arg( QgsDamengConn::quotedValue( schemaName ),
                      QgsDamengConn::quotedValue( tableName ) );
  QgsDMResult *result( conn->DMexec( sql ) );
  if ( !result || !result->execstatus() )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName, result->getMsg() );
    conn->unref();
    return false;
  }

  result->fetchNext();
  int count = result->value( 0 ).toInt();

  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    sql = QStringLiteral( "alter table %1.%2 drop column if exists %3;" )
          .arg( QgsDamengConn::quotedIdentifier( schemaName ),
                QgsDamengConn::quotedIdentifier( tableName ),
                QgsDamengConn::quotedIdentifier( geometryCol ) );
  }
  else
  {
    // drop the table
    sql = QStringLiteral( "Drop table if exists %1.%2" )
          .arg( QgsDamengConn::quotedIdentifier( schemaName ),
                QgsDamengConn::quotedIdentifier( tableName ) );
  }

  result = conn->DMexec( sql );
  if ( !result || !result->execstatus() )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName, result->getMsg() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

bool QgsDamengUtils::deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade )
{
  QgsDebugMsgLevel( "deleting schema " + schema, 2 );

  if ( schema.isEmpty() )
    return false;

  QString schemaName = QgsDamengConn::quotedIdentifier( schema );

  QgsDamengConn *conn = QgsDamengConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // drop the schema
  QString sql = QStringLiteral( "DROP SCHEMA %1 %2" )
                .arg( schemaName, cascade ? QStringLiteral( "CASCADE" ) : QString() );

  QgsDamengResult result( conn->DMexec( sql ) );
  if ( result.DMresultStatus() != DmResCommandOk )
  {
    errCause = QObject::tr( "Unable to delete schema %1: \n%2" )
               .arg( schemaName, result.DMresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}


// ---------------------------------------------------------------------------
QgsDamengConnectionItem::QgsDamengConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "Dameng" ) )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsDamengConnectionItem::createChildren()
{
  QVector<QgsDataItem *>items;

  QgsDataSourceUri uri = QgsDamengConn::connUri( mName );
  // TODO: we need to cancel somehow acquireConnection() if deleteLater() was called on this item to avoid later credential dialog if connection failed
  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugError( "Connection failed - " + uri.connectionInfo( false ) );
    return items;
  }

  QList<QgsDamengSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsDamengConnPool::instance()->releaseConnection( conn );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get schemas" ), mPath + "/error" ) );
    return items;
  }

  const auto constSchemas = schemas;
  for ( const QgsDamengSchemaProperty &schema : constSchemas )
  {
    QgsDamengSchemaItem *schemaItem = new QgsDamengSchemaItem( this, mName, schema.name, mPath + '/' + schema.name );
    if ( !schema.description.isEmpty() )
    {
      schemaItem->setToolTip( schema.description );
    }
    items.append( schemaItem );
  }

  return items;
}

bool QgsDamengConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsDamengConnectionItem *o = qobject_cast< const QgsDamengConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

void QgsDamengConnectionItem::refreshSchema( const QString &schema )
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

bool QgsDamengConnectionItem::handleDrop( const QMimeData *data, const QString &toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceUri uri = QgsDamengConn::connUri( mName );

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
      // Try to get source col from uri
      QString geomColumn { QStringLiteral( "geom" ) };

      if ( !srcLayer->dataProvider()->uri().geometryColumn().isEmpty() )
      {
        geomColumn = srcLayer->dataProvider()->uri().geometryColumn();
      }

      uri.setDataSource( QString(), u.name,  srcLayer->geometryType() != Qgis::GeometryType::Null ? QStringLiteral( "GEOM" ) : QString() );

      QgsDebugMsgLevel( "URI " + uri.uri( false ), 2 );

      if ( !toSchema.isNull() )
      {
        uri.setSchema( toSchema );
      }

      std::unique_ptr<QgsVectorLayerExporterTask> exportTask( new QgsVectorLayerExporterTask( srcLayer, uri.uri( false ), QStringLiteral( "dameng" ), srcLayer->crs(), QVariantMap(), owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [=]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to Dameng database" ), tr( "Import was successful." ) );
        refreshSchema( toSchema );
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [=]( Qgis::VectorExportResult error, const QString & errorMessage ){
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to Dameng database" ) );
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
    output->setTitle( tr( "Import to Dameng database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsDamengLayerItem::QgsDamengLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsDamengLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "dameng" ) )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;
  mUri = createUri();
  
  setState( Qgis::BrowserItemState::NotPopulated );
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QString QgsDamengLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

QString QgsDamengLayerItem::createUri()
{
  QgsDamengConnectionItem *connItem = qobject_cast<QgsDamengConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugError( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  const QString &connName = connItem->name();

  QgsDataSourceUri uri( QgsDamengConn::connUri( connName ).connectionInfo( false ) );

  const QgsSettings &settings = QgsSettings();
  QString basekey = QStringLiteral( "/Dameng/connections/%1" ).arg( connName );

  QStringList defPk( settings.value(
                       QStringLiteral( "%1/keys/%2/%3" ).arg( basekey, mLayerProperty.schemaName, mLayerProperty.tableName ),
                       QVariant( !mLayerProperty.pkCols.isEmpty() ? QStringList( mLayerProperty.pkCols.at( 0 ) ) : QStringList() )
                     ).toStringList() );

  const bool useEstimatedMetadata = QgsDamengConn::useEstimatedMetadata( connName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );

  QStringList cols;
  for ( const auto &col : defPk )
  {
    cols << QgsDamengConn::quotedIdentifier( col );
  }

  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, cols.join( ',' ) );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( uri.wkbType() != Qgis::WkbType::NoGeometry && mLayerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );

  QgsDebugMsgLevel( QStringLiteral( "layer uri: %1" ).arg( uri.uri( false ) ), 2 );
  return uri.uri( false );
}

// ---------------------------------------------------------------------------
QgsDamengSchemaItem::QgsDamengSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path )
  : QgsDatabaseSchemaItem( parent, name, path, QStringLiteral( "Dameng" ) )
  , mConnectionName( connectionName )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
}

QVector<QgsDataItem *> QgsDamengSchemaItem::createChildren()
{
  QVector<QgsDataItem *>items;

  QgsDataSourceUri uri = QgsDamengConn::connUri( mConnectionName );
  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );

  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugError( "Connection failed - " + uri.connectionInfo( false ) );
    return items;
  }

  QVector<QgsDamengLayerProperty> layerProperties;
  const bool ok = conn->supportedLayers( layerProperties, QgsDamengConn::sysdbaSchemaOnly( mConnectionName ), QgsDamengConn::allowGeometrylessTables( mConnectionName ), mName );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get layers" ), mPath + "/error" ) );
    QgsDamengConnPool::instance()->releaseConnection( conn );
    return items;
  }

  const bool dontResolveType = QgsDamengConn::dontResolveType( mConnectionName );
  const bool estimatedMetadata = QgsDamengConn::useEstimatedMetadata( mConnectionName );
  const auto constLayerProperties = layerProperties;
  for ( QgsDamengLayerProperty layerProperty : constLayerProperties )
  {
    if ( layerProperty.schemaName != mName )
      continue;

    if ( !layerProperty.geometryColName.isNull() &&
         ( layerProperty.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown  ||
           layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      if ( dontResolveType )
      {
        QgsDebugMsgLevel( QStringLiteral( "skipping column %1.%2 without type constraint" ).arg( layerProperty.schemaName, layerProperty.tableName ), 2 );
        continue;
      }
      // If the table is empty there is no way we can retrieve layer types, let's make a copy and restore it
      QgsDamengLayerProperty propertyCopy { layerProperty };
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

  QgsDamengConnPool::instance()->releaseConnection( conn );

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "dameng" );
  if ( QgsDamengConn::allowProjectsInDatabase( mConnectionName ) && storage )
  {
    QgsDamengProjectUri postUri;
    postUri.connInfo = uri;
    postUri.schemaName = mName;
    QString schemaUri = QgsDamengProjectStorage::encodeUri( postUri );
    const QStringList projectNames = storage->listProjects( schemaUri );
    for ( const QString &projectName : projectNames )
    {
      QgsDamengProjectUri projectUri( postUri );
      projectUri.projectName = projectName;
      items.append( new QgsProjectItem( this, projectName, QgsDamengProjectStorage::encodeUri( projectUri ) ) );
    }
  }

  return items;
}


QgsDamengLayerItem *QgsDamengSchemaItem::createLayer( QgsDamengLayerProperty layerProperty )
{
  QString tip;
  if ( layerProperty.isView && !layerProperty.isMaterializedView )
  {
    tip = tr( "View" );
  }
  else if ( layerProperty.isMaterializedView )
  {
    tip = tr( "Materialized view" );
  }
  else
  {
    tip = tr( "Table" );
  }

  Qgis::WkbType wkbType = layerProperty.types.at( 0 );
  tip += tr( "\n%1 as %2" ).arg( layerProperty.geometryColName, QgsDamengConn::displayStringForWkbType( wkbType ) );

  if ( layerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    tip += tr( " (srid %1)" ).arg( layerProperty.srids.at( 0 ) );
  else
    tip += tr( " (unknown srid)" );

  if ( !layerProperty.tableComment.isEmpty() )
  {
    tip = layerProperty.tableComment + '\n' + tip;
  }


  Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::Raster;

  Qgis::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
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
      {
        QgsDebugMsgLevel( QStringLiteral( "Adding layer item %1.%2 without type constraint as geometryless table" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ), 2 );
      }
      layerType = Qgis::BrowserLayerType::TableLayer;
      tip = tr( "as geometryless table" );
  }

  QgsDamengLayerItem *layerItem = new QgsDamengLayerItem( this, layerProperty.defaultName(), mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

QVector<QgsDataItem *> QgsDamengLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, uri() + QStringLiteral( "/columns/ " ), createUri(), providerKey(), mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return children;
}

// ---------------------------------------------------------------------------
QgsDamengRootItem::QgsDamengRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "Dameng" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconDameng.svg" );
  populate();
}

QVector<QgsDataItem *> QgsDamengRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsDamengConn::connectionList();
  for ( const QString &connName : list )
  {
    connections << new QgsDamengConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

void QgsDamengRootItem::onConnectionsChanged()
{
  refresh();
}

QString QgsDamengDataItemProvider::name()
{
  return QStringLiteral( "Dameng" );
}

QString QgsDamengDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "dameng" );
}

Qgis::DataItemProviderCapabilities QgsDamengDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::Databases;
}

QgsDataItem *QgsDamengDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsDamengRootItem( parentItem, QStringLiteral( "Dameng" ), QStringLiteral( "dm:" ) );
}


bool QgsDamengSchemaItem::layerCollection() const
{
  return true;
}
