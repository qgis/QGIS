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
#include "qgscolumntypethread.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsprojectstorageregistry.h"
#include "qgsvectorlayer.h"

#ifdef HAVE_GUI
#include "qgspgnewconnection.h"
#include "qgsnewnamedialog.h"
#include "qgspgsourceselect.h"
#endif

#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <climits>

QGISEXTERN bool deleteLayer( const QString &uri, QString &errCause );
QGISEXTERN bool deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade = false );


// ---------------------------------------------------------------------------
QgsPGConnectionItem::QgsPGConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsPGConnectionItem::createChildren()
{

  QVector<QgsDataItem *>items;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( mName );
  // TODO: wee need to cancel somehow acquireConnection() if deleteLater() was called on this item to avoid later credential dialog if connection failed
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

  Q_FOREACH ( const QgsPostgresSchemaProperty &schema, schemas )
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

#ifdef HAVE_GUI
QList<QAction *> QgsPGConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionRefresh = new QAction( tr( "Refresh" ), parent );
  connect( actionRefresh, &QAction::triggered, this, &QgsPGConnectionItem::refreshConnection );
  lst.append( actionRefresh );

  QAction *separator = new QAction( parent );
  separator->setSeparator( true );
  lst.append( separator );

  QAction *actionEdit = new QAction( tr( "Edit Connection…" ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsPGConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete Connection" ), this );
  connect( actionDelete, &QAction::triggered, this, &QgsPGConnectionItem::deleteConnection );
  lst.append( actionDelete );

  QAction *separator2 = new QAction( parent );
  separator2->setSeparator( true );
  lst.append( separator2 );

  QAction *actionCreateSchema = new QAction( tr( "Create Schema…" ), parent );
  connect( actionCreateSchema, &QAction::triggered, this, &QgsPGConnectionItem::createSchema );
  lst.append( actionCreateSchema );

  return lst;
}

void QgsPGConnectionItem::editConnection()
{
  QgsPgNewConnection nc( nullptr, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( mParent )
      mParent->refreshConnections();
  }
}

void QgsPGConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsPostgresConn::deleteConnection( mName );
  // the parent should be updated
  if ( mParent )
    mParent->refreshConnections();
}

void QgsPGConnectionItem::refreshConnection()
{
  refresh();
  // the parent should be updated
  if ( mParent )
    mParent->refreshConnections();
}

void QgsPGConnectionItem::createSchema()
{
  QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( mName );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Create Schema" ), tr( "Unable to create schema." ) );
    return;
  }

  //create the schema
  QString sql = QStringLiteral( "CREATE SCHEMA %1" ).arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Create Schema" ), tr( "Unable to create schema %1\n%2" ).arg( schemaName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  refresh();
  // the parent should be updated
  if ( mParent )
    mParent->refreshConnections();
}
#endif

void QgsPGConnectionItem::refreshSchema( const QString &schema )
{
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    if ( child->name() == schema || schema.isEmpty() )
    {
      child->refresh();
    }
  }
}

bool QgsPGConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  return handleDrop( data, QString() );
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
  Q_FOREACH ( const QgsMimeDataUtils::Uri &u, lst )
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
      QgsDebugMsg( "URI " + uri.uri( false ) );

      if ( !toSchema.isNull() )
      {
        uri.setSchema( toSchema );
      }

      QVariantMap options;
      options.insert( QStringLiteral( "forceSinglePartGeometryType" ), true );
      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( new QgsVectorLayerExporterTask( srcLayer, uri.uri( false ), QStringLiteral( "postgres" ), srcLayer->crs(), options, owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to PostGIS database" ), tr( "Import was successful." ) );
        refreshSchema( toSchema );
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
      {
        if ( error != QgsVectorLayerExporter::ErrUserCanceled )
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
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsPGLayerItem::QgsPGLayerItem( QgsDataItem *parent, const QString &name, const QString &path, QgsLayerItem::LayerType layerType, const QgsPostgresLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "postgres" ) )
  , mLayerProperty( layerProperty )
{
  mUri = createUri();
  setState( Populated );
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QString QgsPGLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

#ifdef HAVE_GUI
QList<QAction *> QgsPGLayerItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QString typeName = mLayerProperty.isView ? tr( "View" ) : tr( "Table" );

  QAction *actionRenameLayer = new QAction( tr( "Rename %1…" ).arg( typeName ), parent );
  connect( actionRenameLayer, &QAction::triggered, this, &QgsPGLayerItem::renameLayer );
  lst.append( actionRenameLayer );

  QAction *actionDeleteLayer = new QAction( tr( "Delete %1" ).arg( typeName ), parent );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsPGLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );

  if ( !mLayerProperty.isView )
  {
    QAction *actionTruncateLayer = new QAction( tr( "Truncate %1" ).arg( typeName ), parent );
    connect( actionTruncateLayer, &QAction::triggered, this, &QgsPGLayerItem::truncateTable );
    lst.append( actionTruncateLayer );
  }

  if ( mLayerProperty.isMaterializedView )
  {
    QAction *actionRefreshMaterializedView = new QAction( tr( "Refresh Materialized View" ), parent );
    connect( actionRefreshMaterializedView, &QAction::triggered, this, &QgsPGLayerItem::refreshMaterializedView );
    lst.append( actionRefreshMaterializedView );
  }

  return lst;
}

void QgsPGLayerItem::deleteLayer()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Table" ),
                              QObject::tr( "Are you sure you want to delete %1.%2?" ).arg( mLayerProperty.schemaName, mLayerProperty.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QString errCause;
  bool res = ::deleteLayer( mUri, errCause );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Delete Table" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Delete Table" ), tr( "Table deleted successfully." ) );
    if ( mParent )
      mParent->refresh();
  }
}

void QgsPGLayerItem::renameLayer()
{
  QString typeName = mLayerProperty.isView ? tr( "View" ) : tr( "Table" );
  QString lowerTypeName = mLayerProperty.isView ? tr( "view" ) : tr( "table" );

  QgsNewNameDialog dlg( tr( "%1 %2.%3" ).arg( lowerTypeName, mLayerProperty.schemaName, mLayerProperty.tableName ), mLayerProperty.tableName );
  dlg.setWindowTitle( tr( "Rename %1" ).arg( typeName ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == mLayerProperty.tableName )
    return;

  QString schemaName = mLayerProperty.schemaName;
  QString tableName = mLayerProperty.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  QString oldName = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );
  QString newName = QgsPostgresConn::quotedIdentifier( dlg.name() );

  QgsDataSourceUri dsUri( mUri );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1." ).arg( lowerTypeName ) );
    return;
  }

  //rename the layer
  QString sql;
  if ( mLayerProperty.isView )
  {
    sql = QStringLiteral( "ALTER %1 VIEW %2 RENAME TO %3" ).arg( mLayerProperty.relKind == QLatin1String( "m" ) ? QStringLiteral( "MATERIALIZED" ) : QString(),
          oldName, newName );
  }
  else
  {
    sql = QStringLiteral( "ALTER TABLE %1 RENAME TO %2" ).arg( oldName, newName );
  }

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1 %2\n%3" ).arg( lowerTypeName, mName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  if ( mParent )
    mParent->refresh();
}

void QgsPGLayerItem::truncateTable()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate %1.%2?\n\nThis will delete all data within the table." ).arg( mLayerProperty.schemaName, mLayerProperty.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri dsUri( mUri );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ), tr( "Unable to truncate table." ) );
    return;
  }

  QString schemaName = mLayerProperty.schemaName;
  QString tableName = mLayerProperty.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  QString sql = QStringLiteral( "TRUNCATE TABLE %1" ).arg( tableRef );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ), tr( "Unable to truncate %1\n%2" ).arg( mName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Truncate Table" ), tr( "Table truncated successfully." ) );
}

void QgsPGLayerItem::refreshMaterializedView()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ),
                              QObject::tr( "Are you sure you want to refresh the materialized view %1.%2?\n\nThis will update all data within the table." ).arg( mLayerProperty.schemaName, mLayerProperty.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri dsUri( mUri );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Refresh View" ), tr( "Unable to refresh the view." ) );
    return;
  }

  QString schemaName = mLayerProperty.schemaName;
  QString tableName = mLayerProperty.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  QString sql = QStringLiteral( "REFRESH MATERIALIZED VIEW CONCURRENTLY %1" ).arg( tableRef );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Refresh View" ), tr( "Unable to refresh view %1\n%2" ).arg( mName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Refresh View" ), tr( "Materialized view refreshed successfully." ) );
}
#endif

QString QgsPGLayerItem::createUri()
{
  QString pkColName = !mLayerProperty.pkCols.isEmpty() ? mLayerProperty.pkCols.at( 0 ) : QString();
  QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  QgsDataSourceUri uri( QgsPostgresConn::connUri( connItem->name() ).connectionInfo( false ) );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( uri.wkbType() != QgsWkbTypes::NoGeometry )
    uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );
  QgsDebugMsg( QStringLiteral( "layer uri: %1" ).arg( uri.uri( false ) ) );
  return uri.uri( false );
}

// ---------------------------------------------------------------------------
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
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
  bool ok = conn->supportedLayers( layerProperties,
                                   QgsPostgresConn::geometryColumnsOnly( mConnectionName ),
                                   QgsPostgresConn::publicSchemaOnly( mConnectionName ),
                                   QgsPostgresConn::allowGeometrylessTables( mConnectionName ), mName );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get layers" ), mPath + "/error" ) );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    return items;
  }

  bool dontResolveType = QgsPostgresConn::dontResolveType( mConnectionName );
  Q_FOREACH ( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    if ( layerProperty.schemaName != mName )
      continue;

    if ( !layerProperty.geometryColName.isNull() &&
         ( layerProperty.types.value( 0, QgsWkbTypes::Unknown ) == QgsWkbTypes::Unknown ||
           layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      if ( dontResolveType )
      {
        //QgsDebugMsg( QStringLiteral( "skipping column %1.%2 without type constraint" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ) );
        continue;
      }

      conn->retrieveLayerTypes( layerProperty, true /* useEstimatedMetadata */ );
    }

    for ( int i = 0; i < layerProperty.size(); i++ )
    {
      QgsPGLayerItem *layerItem = createLayer( layerProperty.at( i ) );
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

#ifdef HAVE_GUI
QList<QAction *> QgsPGSchemaItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionRefresh = new QAction( tr( "Refresh" ), parent );
  connect( actionRefresh, &QAction::triggered, this, static_cast<void ( QgsDataItem::* )()>( &QgsDataItem::refresh ) );
  lst.append( actionRefresh );

  QAction *separator = new QAction( parent );
  separator->setSeparator( true );
  lst.append( separator );

  QAction *actionRename = new QAction( tr( "Rename Schema…" ), parent );
  connect( actionRename, &QAction::triggered, this, &QgsPGSchemaItem::renameSchema );
  lst.append( actionRename );

  QAction *actionDelete = new QAction( tr( "Delete Schema" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsPGSchemaItem::deleteSchema );
  lst.append( actionDelete );

  return lst;
}

void QgsPGSchemaItem::deleteSchema()
{
  // check if schema contains tables/views
  QgsDataSourceUri uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
    return;
  }

  QString sql = QStringLiteral( "SELECT table_name FROM information_schema.tables WHERE table_schema='%1'" ).arg( mName );
  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
    conn->unref();
    return;
  }

  QStringList childObjects;
  int maxListed = 10;
  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    childObjects << result.PQgetvalue( idx, 0 );
    QgsPostgresSchemaProperty schema;
    if ( idx == maxListed - 1 )
      break;
  }

  int count = result.PQntuples();
  if ( count > 0 )
  {
    QString objects = childObjects.join( QStringLiteral( "\n" ) );
    if ( count > maxListed )
    {
      objects += QStringLiteral( "\n[%1 additional objects not listed]" ).arg( count - maxListed );
    }
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( mName, objects ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      conn->unref();
      return;
    }
  }
  else
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Are you sure you want to delete the schema '%1'?" ).arg( mName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  QString errCause;
  bool res = ::deleteSchema( mName, uri, errCause, count > 0 );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Delete Schema" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Delete Schema" ), tr( "Schema deleted successfully." ) );
    if ( mParent )
      mParent->refresh();
  }
}

void QgsPGSchemaItem::renameSchema()
{
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( mName ), mName );
  dlg.setWindowTitle( tr( "Rename Schema" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == mName )
    return;

  QString schemaName = QgsPostgresConn::quotedIdentifier( mName );
  QgsDataSourceUri uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    QMessageBox::warning( nullptr, tr( "Rename Schema" ), tr( "Unable to rename schema." ) );
    return;
  }

  //rename the schema
  QString sql = QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                .arg( schemaName, QgsPostgresConn::quotedIdentifier( dlg.name() ) );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( nullptr, tr( "Rename Schema" ), tr( "Unable to rename schema %1\n%2" ).arg( schemaName,
                          result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( nullptr, tr( "Rename Schema" ), tr( "Schema renamed successfully." ) );
  if ( mParent )
    mParent->refresh();
}
#endif

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
  else
  {
    tip = tr( "Table" );
  }
  QgsWkbTypes::Type wkbType = layerProperty.types.at( 0 );
  tip += tr( "\n%1 as %2 in %3" ).arg( layerProperty.geometryColName, QgsPostgresConn::displayStringForWkbType( wkbType ) ).arg( layerProperty.srids.at( 0 ) );
  if ( !layerProperty.tableComment.isEmpty() )
  {
    tip = layerProperty.tableComment + '\n' + tip;
  }

  QgsLayerItem::LayerType layerType;
  QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( ( QgsWkbTypes::Type )wkbType );
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      layerType = QgsLayerItem::Point;
      break;
    case QgsWkbTypes::LineGeometry:
      layerType = QgsLayerItem::Line;
      break;
    case QgsWkbTypes::PolygonGeometry:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( !layerProperty.geometryColName.isEmpty() )
        return nullptr;

      layerType = QgsLayerItem::TableLayer;
      tip = tr( "as geometryless table" );
  }

  QgsPGLayerItem *layerItem = new QgsPGLayerItem( this, layerProperty.defaultName(), mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

bool QgsPGSchemaItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  QgsPGConnectionItem *conn = qobject_cast<QgsPGConnectionItem *>( parent() );
  if ( !conn )
    return false;

  bool result = conn->handleDrop( data, mName );
  if ( result )
    refresh();

  return result;
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconPostgis.svg" );
  populate();
}

QVector<QgsDataItem *> QgsPGRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  Q_FOREACH ( const QString &connName, QgsPostgresConn::connectionList() )
  {
    connections << new QgsPGConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsPGRootItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsPGRootItem::newConnection );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsPGRootItem::paramWidget()
{
  QgsPgSourceSelect *select = new QgsPgSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsPgSourceSelect::connectionsChanged, this, &QgsPGRootItem::onConnectionsChanged );
  return select;
}

void QgsPGRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsPGRootItem::newConnection()
{
  QgsPgNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    refresh();
  }
}
#endif

QMainWindow *QgsPGRootItem::sMainWindow = nullptr;

QGISEXTERN void registerGui( QMainWindow *mainWindow )
{
  QgsPGRootItem::sMainWindow = mainWindow;
}
