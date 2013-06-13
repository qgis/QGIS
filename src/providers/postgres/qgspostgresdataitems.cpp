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
    , mColumnTypeThread( 0 )
{
  mIcon = QgsApplication::getThemeIcon( "mIconConnect.png" );
}

QgsPGConnectionItem::~QgsPGConnectionItem()
{
  stop();
}

void QgsPGConnectionItem::stop()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    delete mColumnTypeThread;
    mColumnTypeThread = 0;
  }
}

void QgsPGConnectionItem::refresh()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  stop();

  foreach ( QgsDataItem *child, mChildren )
  {
    deleteChildItem( child );
  }

  foreach ( QgsDataItem *item, createChildren() )
  {
    addChildItem( item, true );
  }

  QApplication::restoreOverrideCursor();
}

QVector<QgsDataItem*> QgsPGConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  mSchemaMap.clear();

  stop();

  if ( !mColumnTypeThread )
  {
    mColumnTypeThread = new QgsGeomColumnTypeThread( mName, true /* useEstimatedMetadata */, QgsPostgresConn::allowGeometrylessTables( mName ) );

    connect( mColumnTypeThread, SIGNAL( setLayerType( QgsPostgresLayerProperty ) ),
             this, SLOT( setLayerType( QgsPostgresLayerProperty ) ) );
    connect( mColumnTypeThread, SIGNAL( started() ), this, SLOT( threadStarted() ) );
    connect( mColumnTypeThread, SIGNAL( finished() ), this, SLOT( threadFinished() ) );

    if ( QgsPGRootItem::sMainWindow )
    {
      connect( mColumnTypeThread, SIGNAL( progress( int, int ) ),
               QgsPGRootItem::sMainWindow, SLOT( showProgress( int, int ) ) );
      connect( mColumnTypeThread, SIGNAL( progressMessage( QString ) ),
               QgsPGRootItem::sMainWindow, SLOT( showStatusMessage( QString ) ) );
    }
  }

  if ( mColumnTypeThread )
    mColumnTypeThread->start();

  return QVector<QgsDataItem*>();
}

void QgsPGConnectionItem::threadStarted()
{
  QgsDebugMsg( "Entering." );
  qApp->setOverrideCursor( Qt::BusyCursor );
}

void QgsPGConnectionItem::threadFinished()
{
  QgsDebugMsg( "Entering." );
  qApp->restoreOverrideCursor();
}

void QgsPGConnectionItem::setLayerType( QgsPostgresLayerProperty layerProperties )
{
  QgsDebugMsg( layerProperties.toString() );
  QgsPGSchemaItem *schemaItem = mSchemaMap.value( layerProperties.schemaName, 0 );

  for ( int i = 0; i < layerProperties.size(); i++ )
  {
    QgsPostgresLayerProperty layerProperty = layerProperties.at( i );
    if ( layerProperty.types[0] == QGis::WKBUnknown ||
         ( !layerProperty.geometryColName.isEmpty() && layerProperty.srids[0] == INT_MIN ) )
      continue;

    if ( !schemaItem )
    {
      schemaItem = new QgsPGSchemaItem( this, layerProperty.schemaName, mPath + "/" + layerProperty.schemaName );
      addChildItem( schemaItem, true );
      mSchemaMap[ layerProperty.schemaName ] = schemaItem;
    }

    schemaItem->addLayer( layerProperty );
  }
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
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconDbSchema.png" );
}

QVector<QgsDataItem*> QgsPGSchemaItem::createChildren()
{
  QgsDebugMsg( "Entering." );
  return QVector<QgsDataItem*>();
}

QgsPGSchemaItem::~QgsPGSchemaItem()
{
}

void QgsPGSchemaItem::addLayer( QgsPostgresLayerProperty layerProperty )
{
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
        return;

      layerType = QgsLayerItem::TableLayer;
      tip = tr( "as geometryless table" );
  }

  QgsPGLayerItem *layerItem = new QgsPGLayerItem( this, layerProperty.tableName, mPath + "/" + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  addChild( layerItem );
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconPostgis.png" );
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
