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

#include <QMessageBox>
#include <QProgressDialog>
#include <climits>

QGISEXTERN bool deleteLayer( const QString& uri, QString& errCause );

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
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo() );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugMsg( "Connection failed - " + uri.connectionInfo() );
    return items;
  }

  QVector<QgsPostgresLayerProperty> layerProperties;
  bool ok = conn->supportedLayers( layerProperties,
                                   QgsPostgresConn::geometryColumnsOnly( mName ),
                                   QgsPostgresConn::publicSchemaOnly( mName ),
                                   QgsPostgresConn::allowGeometrylessTables( mName ) );

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get schemas" ), mPath + "/error" ) );
    return items;
  }

  QSet<QString> schemaNames;
  foreach ( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    schemaNames.insert( layerProperty.schemaName );
  }

  foreach ( QString schemaName, schemaNames )
  {
    QgsPGSchemaItem * schemaItem = new QgsPGSchemaItem( this, mName, schemaName, mPath + "/" + schemaName );
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

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  QAction* actionRefresh = new QAction( tr( "Refresh" ), this );
  connect( actionRefresh, SIGNAL( triggered() ), this, SLOT( refreshConnection() ) );
  lst.append( actionRefresh );

  return lst;
}

void QgsPGConnectionItem::editConnection()
{
  QgsPgNewConnection nc( NULL, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsPGConnectionItem::deleteConnection()
{
  QgsPostgresConn::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}

void QgsPGConnectionItem::refreshConnection()
{
  // the parent should be updated
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
  mPopulated = true;
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QgsPGLayerItem::~QgsPGLayerItem()
{
}

QList<QAction*> QgsPGLayerItem::actions()
{
  QList<QAction*> lst;

  QAction* actionDeleteLayer = new QAction( tr( "Delete layer" ), this );
  connect( actionDeleteLayer, SIGNAL( triggered() ), this, SLOT( deleteLayer() ) );
  lst.append( actionDeleteLayer );

  return lst;
}

void QgsPGLayerItem::deleteLayer()
{
  QString errCause;
  bool res = ::deleteLayer( mUri, errCause );
  if ( !res )
  {
    QMessageBox::warning( 0, tr( "Delete layer" ), errCause );
  }
  else
  {
    QMessageBox::information( 0, tr( "Delete layer" ), tr( "Layer deleted successfully." ) );
    mParent->refresh();
  }
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
  uri.setSrid( QString::number( mLayerProperty.srids[0] ) );
  uri.setWkbType( mLayerProperty.types[0] );
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
                                   QgsPostgresConn::geometryColumnsOnly( mName ),
                                   QgsPostgresConn::publicSchemaOnly( mName ),
                                   QgsPostgresConn::allowGeometrylessTables( mName ) );
  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get layers" ), mPath + "/error" ) );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    return items;
  }

  bool dontResolveType = QgsPostgresConn::dontResolveType( mName );
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
        QgsDebugMsg( QString( "skipping column %1.%2 without type constraint" ).arg( layerProperty.schemaName ).arg( layerProperty.tableName ) );
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

QgsPGLayerItem *QgsPGSchemaItem::createLayer( QgsPostgresLayerProperty layerProperty )
{
  QgsDebugMsg( "schemaName = " + layerProperty.schemaName + " tableName = " + layerProperty.tableName + " geometryColName = " + layerProperty.geometryColName );
  QGis::WkbType wkbType = layerProperty.types[0];
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName ).arg( QgsPostgresConn::displayStringForWkbType( wkbType ) ).arg( layerProperty.srids[0] );

  QgsLayerItem::LayerType layerType;
  switch ( wkbType )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      layerType = QgsLayerItem::Point;
      break;
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      layerType = QgsLayerItem::Line;
      break;
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
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
