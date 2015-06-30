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
#include "qgspgnewconnection.h"
#include "qgscolumntypethread.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsnewnamedialog.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <climits>

QGISEXTERN bool deleteLayer( const QString& uri, QString& errCause );
QGISEXTERN bool deleteSchema( const QString& schema, const QgsDataSourceURI& uri, QString& errCause, bool cascade = false );


// ---------------------------------------------------------------------------
QgsPGConnectionItem::QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconConnect.png";
}

QgsPGConnectionItem::~QgsPGConnectionItem()
{
}

QVector<QgsDataItem*> QgsPGConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  QVector<QgsDataItem*>items;

  QgsDataSourceURI uri = QgsPostgresConn::connUri( mName );
  // TODO: wee need to cancel somehow acquireConnection() if deleteLater() was called on this item to avoid later credential dialog if connection failed
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo() );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo() );
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

  foreach ( QgsPostgresSchemaProperty schema, schemas )
  {
    QgsPGSchemaItem * schemaItem = new QgsPGSchemaItem( this, mName, schema.name, mPath + "/" + schema.name );
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

QList<QAction*> QgsPGConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionRefresh = new QAction( tr( "Refresh" ), this );
  connect( actionRefresh, SIGNAL( triggered() ), this, SLOT( refreshConnection() ) );
  lst.append( actionRefresh );

  QAction* separator = new QAction( this );
  separator->setSeparator( true );
  lst.append( separator );

  QAction* actionEdit = new QAction( tr( "Edit Connection..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete Connection" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  QAction* separator2 = new QAction( this );
  separator2->setSeparator( true );
  lst.append( separator2 );

  QAction* actionCreateSchema = new QAction( tr( "Create Schema..." ), this );
  connect( actionCreateSchema, SIGNAL( triggered() ), this, SLOT( createSchema() ) );
  lst.append( actionCreateSchema );

  return lst;
}

void QgsPGConnectionItem::editConnection()
{
  QgsPgNewConnection nc( NULL, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( mParent )
      mParent->refresh();
  }
}

void QgsPGConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( 0, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsPostgresConn::deleteConnection( mName );
  // the parent should be updated
  if ( mParent )
    mParent->refresh();
}

void QgsPGConnectionItem::refreshConnection()
{
  // the parent should be updated
  refresh();
}

void QgsPGConnectionItem::createSchema()
{
  QString schemaName = QInputDialog::getText( 0, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  QgsDataSourceURI uri = QgsPostgresConn::connUri( mName );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo(), false );
  if ( !conn )
  {
    QMessageBox::warning( 0, tr( "Create Schema" ), tr( "Unable to create schema." ) );
    return;
  }

  //create the schema
  QString sql = QString( "CREATE SCHEMA %1" ).arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( 0, tr( "Create Schema" ), tr( "Unable to create schema %1\n%2" ).arg( schemaName )
                          .arg( result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  refresh();
}

bool QgsPGConnectionItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceURI uri = QgsPostgresConn::connUri( mName );

  qApp->setOverrideCursor( Qt::WaitCursor );

  QProgressDialog *progress = new QProgressDialog( tr( "Copying features..." ), tr( "Abort" ), 0, 0, 0 );
  progress->setWindowTitle( tr( "Import layer" ) );
  progress->setWindowModality( Qt::WindowModal );
  progress->show();

  QStringList importResults;
  bool hasError = false;
  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  foreach ( const QgsMimeDataUtils::Uri& u, lst )
  {
    if ( u.layerType != "vector" )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    // open the source layer
    QgsVectorLayer* srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey );

    if ( srcLayer->isValid() )
    {
      uri.setDataSource( QString(), u.name, "geom" );
      QgsDebugMsg( "URI " + uri.uri() );
      QgsVectorLayerImport::ImportError err;
      QString importError;
      err = QgsVectorLayerImport::importLayer( srcLayer, uri.uri(), "postgres", &srcLayer->crs(), false, &importError, false, 0, progress );
      if ( err == QgsVectorLayerImport::NoError )
        importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      else
      {
        importResults.append( QString( "%1: %2" ).arg( u.name ).arg( importError ) );
        hasError = true;
      }
    }
    else
    {
      importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      hasError = true;
    }

    delete srcLayer;
  }

  delete progress;

  qApp->restoreOverrideCursor();

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to PostGIS database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ), QgsMessageOutput::MessageText );
    output->showMessage();
  }
  else
  {
    QMessageBox::information( 0, tr( "Import to PostGIS database" ), tr( "Import was successful." ) );
    refresh();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsPGLayerItem::QgsPGLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, "postgres" )
    , mLayerProperty( layerProperty )
{
  mUri = createUri();
  setState( Populated );
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QgsPGLayerItem::~QgsPGLayerItem()
{
}

QList<QAction*> QgsPGLayerItem::actions()
{
  QList<QAction*> lst;

  QString typeName = mLayerProperty.isView ? tr( "View" ) : tr( "Table" );

  QAction* actionRenameLayer = new QAction( tr( "Rename %1..." ).arg( typeName ), this );
  connect( actionRenameLayer, SIGNAL( triggered() ), this, SLOT( renameLayer() ) );
  lst.append( actionRenameLayer );

  QAction* actionDeleteLayer = new QAction( tr( "Delete %1" ).arg( typeName ), this );
  connect( actionDeleteLayer, SIGNAL( triggered() ), this, SLOT( deleteLayer() ) );
  lst.append( actionDeleteLayer );

  if ( !mLayerProperty.isView )
  {
    QAction* actionTruncateLayer = new QAction( tr( "Truncate %1" ).arg( typeName ), this );
    connect( actionTruncateLayer, SIGNAL( triggered() ), this, SLOT( truncateTable() ) );
    lst.append( actionTruncateLayer );
  }

  return lst;
}

void QgsPGLayerItem::deleteLayer()
{
  if ( QMessageBox::question( 0, QObject::tr( "Delete Table" ),
                              QObject::tr( "Are you sure you want to delete %1.%2?" ).arg( mLayerProperty.schemaName ).arg( mLayerProperty.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QString errCause;
  bool res = ::deleteLayer( mUri, errCause );
  if ( !res )
  {
    QMessageBox::warning( 0, tr( "Delete Table" ), errCause );
  }
  else
  {
    QMessageBox::information( 0, tr( "Delete Table" ), tr( "Table deleted successfully." ) );
    if ( mParent )
      mParent->refresh();
  }
}

void QgsPGLayerItem::renameLayer()
{
  QString typeName = mLayerProperty.isView ? tr( "View" ) : tr( "Table" );
  QString lowerTypeName = mLayerProperty.isView ? tr( "view" ) : tr( "table" );

  QgsNewNameDialog dlg( tr( "%1 %2.%3" ).arg( lowerTypeName ).arg( mLayerProperty.schemaName ).arg( mLayerProperty.tableName ), mLayerProperty.tableName );
  dlg.setWindowTitle( tr( "Rename %1" ).arg( typeName ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == mLayerProperty.tableName )
    return;

  QString schemaName = mLayerProperty.schemaName;
  QString tableName = mLayerProperty.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + ".";
  }
  QString oldName = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );
  QString newName = QgsPostgresConn::quotedIdentifier( dlg.name() );

  QgsDataSourceURI dsUri( mUri );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo(), false );
  if ( !conn )
  {
    QMessageBox::warning( 0, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1." ).arg( lowerTypeName ) );
    return;
  }

  //rename the layer
  QString sql;
  if ( mLayerProperty.isView )
  {
    sql = QString( "ALTER %1 VIEW %2 RENAME TO %3" ).arg( mLayerProperty.relKind == "m" ? QString( "MATERIALIZED" ) : QString() )
          .arg( oldName ).arg( newName );
  }
  else
  {
    sql = QString( "ALTER TABLE %1 RENAME TO %2" ).arg( oldName ).arg( newName );
  }

  QgsPostgresResult result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( 0, tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename %1 %2\n%3" ).arg( lowerTypeName ).arg( mName )
                          .arg( result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  if ( mParent )
    mParent->refresh();
}

void QgsPGLayerItem::truncateTable()
{
  if ( QMessageBox::question( 0, QObject::tr( "Truncate Table" ),
                              QObject::tr( "Are you sure you want to truncate %1.%2?\n\nThis will delete all data within the table." ).arg( mLayerProperty.schemaName ).arg( mLayerProperty.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceURI dsUri( mUri );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo(), false );
  if ( !conn )
  {
    QMessageBox::warning( 0, tr( "Truncate Table" ), tr( "Unable to truncate table." ) );
    return;
  }

  QString schemaName = mLayerProperty.schemaName;
  QString tableName = mLayerProperty.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + ".";
  }
  QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  QString sql = QString( "TRUNCATE TABLE %1" ).arg( tableRef );

  QgsPostgresResult result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( 0, tr( "Truncate Table" ), tr( "Unable to truncate %1\n%2" ).arg( mName )
                          .arg( result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( 0, tr( "Truncate Table" ), tr( "Table truncated successfully." ) );
}

QString QgsPGLayerItem::createUri()
{
  QString pkColName = mLayerProperty.pkCols.size() > 0 ? mLayerProperty.pkCols.at( 0 ) : QString::null;
  QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDataSourceURI uri( QgsPostgresConn::connUri( connItem->name() ).connectionInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setWkbType( mLayerProperty.types[0] );
  if ( uri.wkbType() != QGis::WKBNoGeometry )
    uri.setSrid( QString::number( mLayerProperty.srids[0] ) );
  QgsDebugMsg( QString( "layer uri: %1" ).arg( uri.uri() ) );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem* parent, QString connectionName, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
    , mConnectionName( connectionName )
{
  mIconName = "mIconDbSchema.png";
}

QgsPGSchemaItem::~QgsPGSchemaItem()
{
}

QVector<QgsDataItem*> QgsPGSchemaItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  QVector<QgsDataItem*>items;

  QgsDataSourceURI uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo() );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo() );
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
  foreach ( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    if ( layerProperty.schemaName != mName )
      continue;

    if ( !layerProperty.geometryColName.isNull() &&
         ( layerProperty.types.value( 0, QGis::WKBUnknown ) == QGis::WKBUnknown ||
           layerProperty.srids.value( 0, INT_MIN ) == INT_MIN ) )
    {
      if ( dontResolveType )
      {
        //QgsDebugMsg( QString( "skipping column %1.%2 without type constraint" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ) );
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
  return items;
}

QList<QAction *> QgsPGSchemaItem::actions()
{
  QList<QAction*> lst;

  QAction* actionRefresh = new QAction( tr( "Refresh" ), this );
  connect( actionRefresh, SIGNAL( triggered() ), this, SLOT( refresh() ) );
  lst.append( actionRefresh );

  QAction* separator = new QAction( this );
  separator->setSeparator( true );
  lst.append( separator );

  QAction* actionRename = new QAction( tr( "Rename Schema..." ), this );
  connect( actionRename, SIGNAL( triggered() ), this, SLOT( renameSchema() ) );
  lst.append( actionRename );

  QAction* actionDelete = new QAction( tr( "Delete Schema" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteSchema() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsPGSchemaItem::deleteSchema()
{
  // check if schema contains tables/views
  QgsDataSourceURI uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo(), false );
  if ( !conn )
  {
    QMessageBox::warning( 0, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
    return;
  }

  QString sql = QString( "SELECT table_name FROM information_schema.tables WHERE table_schema='%1'" ).arg( mName );
  QgsPostgresResult result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QMessageBox::warning( 0, tr( "Delete Schema" ), tr( "Unable to delete schema." ) );
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
    QString objects = childObjects.join( "\n" );
    if ( count > maxListed )
    {
      objects += QString( "\n[%1 additional objects not listed]" ).arg( count - maxListed );
    }
    if ( QMessageBox::question( 0, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( mName ).arg( objects ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      conn->unref();
      return;
    }
  }
  else
  {
    if ( QMessageBox::question( 0, QObject::tr( "Delete Schema" ),
                                QObject::tr( "Are you sure you want to delete the schema '%1'?" ).arg( mName ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  QString errCause;
  bool res = ::deleteSchema( mName, uri, errCause, count > 0 );
  if ( !res )
  {
    QMessageBox::warning( 0, tr( "Delete Schema" ), errCause );
  }
  else
  {
    QMessageBox::information( 0, tr( "Delete Schema" ), tr( "Schema deleted successfully." ) );
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
  QgsDataSourceURI uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo(), false );
  if ( !conn )
  {
    QMessageBox::warning( 0, tr( "Rename Schema" ), tr( "Unable to rename schema." ) );
    return;
  }

  //rename the schema
  QString sql = QString( "ALTER SCHEMA %1 RENAME TO %2" )
                .arg( schemaName ).arg( QgsPostgresConn::quotedIdentifier( dlg.name() ) );

  QgsPostgresResult result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QMessageBox::warning( 0, tr( "Rename Schema" ), tr( "Unable to rename schema %1\n%2" ).arg( schemaName )
                          .arg( result.PQresultErrorMessage() ) );
    conn->unref();
    return;
  }

  conn->unref();
  QMessageBox::information( 0, tr( "Rename Schema" ), tr( "Schema renamed successfully." ) );
  if ( mParent )
    mParent->refresh();
}

QgsPGLayerItem *QgsPGSchemaItem::createLayer( QgsPostgresLayerProperty layerProperty )
{
  //QgsDebugMsg( "schemaName = " + layerProperty.schemaName + " tableName = " + layerProperty.tableName + " geometryColName = " + layerProperty.geometryColName );
  QGis::WkbType wkbType = layerProperty.types[0];
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName ).arg( QgsPostgresConn::displayStringForWkbType( wkbType ) ).arg( layerProperty.srids[0] );

  QgsLayerItem::LayerType layerType;
  QgsWKBTypes::GeometryType geomType = QgsWKBTypes::geometryType(( QgsWKBTypes::Type )wkbType );
  switch ( geomType )
  {
    case QgsWKBTypes::PointGeometry:
      layerType = QgsLayerItem::Point;
      break;
    case QgsWKBTypes::LineGeometry:
      layerType = QgsLayerItem::Line;
      break;
    case QgsWKBTypes::PolygonGeometry:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( !layerProperty.geometryColName.isEmpty() )
        return 0;

      layerType = QgsLayerItem::TableLayer;
      tip = tr( "as geometryless table" );
  }

  QgsPGLayerItem *layerItem = new QgsPGLayerItem( this, layerProperty.defaultName(), mPath + "/" + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = "mIconPostgis.svg";
  populate();
}

QgsPGRootItem::~QgsPGRootItem()
{
}

QVector<QgsDataItem*> QgsPGRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach ( QString connName, QgsPostgresConn::connectionList() )
  {
    connections << new QgsPGConnectionItem( this, connName, mPath + "/" + connName );
  }
  return connections;
}

QList<QAction*> QgsPGRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsPGRootItem::paramWidget()
{
  QgsPgSourceSelect *select = new QgsPgSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsPGRootItem::connectionsChanged()
{
  refresh();
}

void QgsPGRootItem::newConnection()
{
  QgsPgNewConnection nc( NULL );
  if ( nc.exec() )
  {
    refresh();
  }
}

QMainWindow *QgsPGRootItem::sMainWindow = 0;

QGISEXTERN void registerGui( QMainWindow *mainWindow )
{
  QgsPGRootItem::sMainWindow = mainWindow;
}
