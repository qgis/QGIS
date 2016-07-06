/***************************************************************************
    qgsoracledataitems.cpp
    ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsoracledataitems.h"

#include "qgsoracletablemodel.h"
#include "qgsoraclenewconnection.h"
#include "qgsoraclecolumntypethread.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsapplication.h"
#include "qgsmessageoutput.h"

#include <QMessageBox>
#include <QProgressDialog>

QGISEXTERN bool deleteLayer( const QString& uri, QString& errCause );

// ---------------------------------------------------------------------------
QgsOracleConnectionItem::QgsOracleConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
    , mColumnTypeThread( nullptr )
{
  mIconName = "mIconConnect.png";
}

QgsOracleConnectionItem::~QgsOracleConnectionItem()
{
  stop();
}

void QgsOracleConnectionItem::stop()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    delete mColumnTypeThread;
    mColumnTypeThread = nullptr;
  }
}

void QgsOracleConnectionItem::refresh()
{
  stop();

  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    deleteChildItem( child );
  }

  Q_FOREACH ( QgsDataItem *item, createChildren() )
  {
    addChildItem( item, true );
  }
}

void QgsOracleConnectionItem::setAllAsPopulated()
{
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    child->setState( Populated );
  }
  setState( Populated );
}

QVector<QgsDataItem*> QgsOracleConnectionItem::createChildren()
{
  setState( Populating );

  mOwnerMap.clear();

  stop();

  if ( deferredDelete() )
    return QVector<QgsDataItem*>();

  if ( !mColumnTypeThread )
  {
    mColumnTypeThread = new QgsOracleColumnTypeThread( mName,
        /* useEstimatedMetadata */ true,
        QgsOracleConn::allowGeometrylessTables( mName ) );

    connect( mColumnTypeThread, SIGNAL( setLayerType( QgsOracleLayerProperty ) ),
             this, SLOT( setLayerType( QgsOracleLayerProperty ) ) );
    connect( mColumnTypeThread, SIGNAL( started() ), this, SLOT( threadStarted() ) );
    connect( mColumnTypeThread, SIGNAL( finished() ), this, SLOT( threadFinished() ) );

    if ( QgsOracleRootItem::sMainWindow )
    {
      connect( mColumnTypeThread, SIGNAL( progress( int, int ) ),
               QgsOracleRootItem::sMainWindow, SLOT( showProgress( int, int ) ) );
      connect( mColumnTypeThread, SIGNAL( progressMessage( QString ) ),
               QgsOracleRootItem::sMainWindow, SLOT( showStatusMessage( QString ) ) );
    }
  }

  if ( mColumnTypeThread )
  {
    mColumnTypeThread->start();
  }
  else
  {
    setAllAsPopulated();
  }

  return QVector<QgsDataItem*>();
}

void QgsOracleConnectionItem::threadStarted()
{
  QgsDebugMsgLevel( "Entering.", 3 );
}

void QgsOracleConnectionItem::threadFinished()
{
  QgsDebugMsgLevel( "Entering.", 3 );
  setAllAsPopulated();
}

void QgsOracleConnectionItem::setLayerType( QgsOracleLayerProperty layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );
  QgsOracleOwnerItem *ownerItem = mOwnerMap.value( layerProperty.ownerName, 0 );

  for ( int i = 0 ; i < layerProperty.size(); i++ )
  {
    QGis::WkbType wkbType = layerProperty.types.at( i );
    if ( wkbType == QGis::WKBUnknown )
    {
      QgsDebugMsgLevel( "skip unknown geometry type", 3 );
      continue;
    }

    if ( !ownerItem )
    {
      ownerItem = new QgsOracleOwnerItem( this, layerProperty.ownerName, mPath + "/" + layerProperty.ownerName );
      ownerItem->setState( Populating );
      QgsDebugMsgLevel( "add owner item: " + layerProperty.ownerName, 3 );
      addChildItem( ownerItem, true );
      mOwnerMap[ layerProperty.ownerName ] = ownerItem;
    }

    QgsDebugMsgLevel( "ADD LAYER", 3 );
    ownerItem->addLayer( layerProperty.at( i ) );
  }
}

bool QgsOracleConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsOracleConnectionItem *o = qobject_cast<const QgsOracleConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName && o->parent() == parent() );
}

QList<QAction*> QgsOracleConnectionItem::actions()
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

  return lst;
}

void QgsOracleConnectionItem::editConnection()
{
  QgsOracleNewConnection nc( NULL, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsOracleConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOracleConn::deleteConnection( mName );

  // the parent should be updated
  if ( mParent )
    mParent->refresh();
}

void QgsOracleConnectionItem::refreshConnection()
{
  // the parent should be updated
  refresh();
}

bool QgsOracleConnectionItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceURI uri = QgsOracleConn::connUri( mName );

  qApp->setOverrideCursor( Qt::WaitCursor );

  QProgressDialog *progress = new QProgressDialog( tr( "Copying features..." ), tr( "Abort" ), 0, 0, nullptr );
  progress->setWindowTitle( tr( "Import layer" ) );
  progress->setWindowModality( Qt::WindowModal );
  progress->show();

  QStringList importResults;
  bool hasError = false;
  bool cancelled = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Q_FOREACH ( const QgsMimeDataUtils::Uri& u, lst )
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
      uri.setDataSource( QString(), u.name.left( 30 ).toUpper(), "GEOM" );
      uri.setWkbType( QGis::fromOldWkbType( srcLayer->wkbType() ) );
      QString authid = srcLayer->crs().authid();
      if ( authid.startsWith( "EPSG:", Qt::CaseInsensitive ) )
      {
        uri.setSrid( authid.mid( 5 ) );
      }
      QgsDebugMsgLevel( "URI " + uri.uri(), 3 );
      QgsVectorLayerImport::ImportError err;
      QString importError;
      err = QgsVectorLayerImport::importLayer( srcLayer, uri.uri(), "oracle", &srcLayer->crs(), false, &importError, false, nullptr, progress );
      if ( err == QgsVectorLayerImport::NoError )
        importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      else if ( err == QgsVectorLayerImport::ErrUserCancelled )
        cancelled = true;
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

  if ( cancelled )
  {
    QMessageBox::information( nullptr, tr( "Import to Oracle database" ), tr( "Import cancelled." ) );
    refresh();
  }
  else if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to Oracle database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ), QgsMessageOutput::MessageText );
    output->showMessage();
  }
  else
  {
    QMessageBox::information( 0, tr( "Import to Oracle database" ), tr( "Import was successful." ) );
    refresh();
  }

  if ( state() == Populated )
    refresh();
  else
    populate();

  return true;
}

// ---------------------------------------------------------------------------
QgsOracleLayerItem::QgsOracleLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsOracleLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, "oracle" )
    , mLayerProperty( layerProperty )
{
  mUri = createUri();
  setState( Populated );
}

QgsOracleLayerItem::~QgsOracleLayerItem()
{
}

QList<QAction*> QgsOracleLayerItem::actions()
{
  QList<QAction*> lst;

  QAction* actionDeleteLayer = new QAction( tr( "Delete Table" ), this );
  connect( actionDeleteLayer, SIGNAL( triggered() ), this, SLOT( deleteLayer() ) );
  lst.append( actionDeleteLayer );

  return lst;
}

void QgsOracleLayerItem::deleteLayer()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Table" ),
                              QObject::tr( "Are you sure you want to delete %1.%2?" ).arg( mLayerProperty.ownerName, mLayerProperty.tableName ),
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
    deleteLater();
  }
}

QString QgsOracleLayerItem::createUri()
{
  Q_ASSERT( mLayerProperty.size() == 1 );
  QgsOracleConnectionItem *connItem = qobject_cast<QgsOracleConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDataSourceURI uri = QgsOracleConn::connUri( connItem->name() );
  uri.setDataSource( mLayerProperty.ownerName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, QString::null );
  uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );
  uri.setWkbType( QGis::fromOldWkbType( mLayerProperty.types.at( 0 ) ) );
  if ( mLayerProperty.isView && mLayerProperty.pkCols.size() > 0 )
    uri.setKeyColumn( mLayerProperty.pkCols[0] );
  QgsDebugMsgLevel( QString( "layer uri: %1" ).arg( uri.uri() ), 3 );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsOracleOwnerItem::QgsOracleOwnerItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconDbOwner.png";
  //not fertile, since children are created by QgsOracleConnectionItem
  mCapabilities &= ~( Fertile );
}

QVector<QgsDataItem*> QgsOracleOwnerItem::createChildren()
{
  QgsDebugMsgLevel( "Entering.", 3 );
  return QVector<QgsDataItem*>();
}

QgsOracleOwnerItem::~QgsOracleOwnerItem()
{
}

void QgsOracleOwnerItem::addLayer( QgsOracleLayerProperty layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );

  Q_ASSERT( layerProperty.size() == 1 );
  QGis::WkbType wkbType = layerProperty.types.at( 0 );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName ).arg( QgsOracleConn::displayStringForWkbType( wkbType ) ).arg( layerProperty.srids.at( 0 ) );

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
      if ( wkbType == QGis::WKBNoGeometry && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else
      {
        return;
      }
  }

  QgsOracleLayerItem *layerItem = new QgsOracleLayerItem( this, layerProperty.tableName, mPath + "/" + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  addChildItem( layerItem, true );
}

// ---------------------------------------------------------------------------
QgsOracleRootItem::QgsOracleRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconOracle.svg";
  populate();
}

QgsOracleRootItem::~QgsOracleRootItem()
{
}

QVector<QgsDataItem*> QgsOracleRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  Q_FOREACH ( QString connName, QgsOracleConn::connectionList() )
  {
    connections << new QgsOracleConnectionItem( this, connName, mPath + "/" + connName );
  }
  return connections;
}

QList<QAction*> QgsOracleRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsOracleRootItem::paramWidget()
{
  QgsOracleSourceSelect *select = new QgsOracleSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsOracleRootItem::connectionsChanged()
{
  refresh();
}

void QgsOracleRootItem::newConnection()
{
  QgsOracleNewConnection nc( NULL );
  if ( nc.exec() )
  {
    refresh();
  }
}

QMainWindow *QgsOracleRootItem::sMainWindow = 0;

QGISEXTERN void registerGui( QMainWindow *mainWindow )
{
  QgsOracleRootItem::sMainWindow = mainWindow;
}
