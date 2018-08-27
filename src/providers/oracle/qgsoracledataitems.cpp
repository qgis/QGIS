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
#include "qgsvectorlayer.h"
#include "qgsproxyprogresstask.h"

#include <QMessageBox>
#include <QProgressDialog>

QGISEXTERN bool deleteLayer( const QString &uri, QString &errCause );

// ---------------------------------------------------------------------------
QgsOracleConnectionItem::QgsOracleConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
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

QVector<QgsDataItem *> QgsOracleConnectionItem::createChildren()
{
  setState( Populating );

  mOwnerMap.clear();

  stop();

  if ( deferredDelete() )
    return QVector<QgsDataItem *>();

  if ( !mColumnTypeThread )
  {
    mColumnTypeThread = new QgsOracleColumnTypeThread( mName,
        QgsOracleConn::restrictToSchema( mName ),
        /* useEstimatedMetadata */ true,
        QgsOracleConn::allowGeometrylessTables( mName ) );
    mColumnTypeTask = new QgsProxyProgressTask( tr( "Scanning tables for %1" ).arg( mName ) );
    QgsApplication::taskManager()->addTask( mColumnTypeTask );

    connect( mColumnTypeThread, &QgsOracleColumnTypeThread::setLayerType,
             this, &QgsOracleConnectionItem::setLayerType );
    connect( mColumnTypeThread, &QThread::started, this, &QgsOracleConnectionItem::threadStarted );
    connect( mColumnTypeThread, &QThread::finished, this, &QgsOracleConnectionItem::threadFinished );

    if ( QgsOracleRootItem::sMainWindow )
    {
      connect( mColumnTypeThread, &QgsOracleColumnTypeThread::progress,
               mColumnTypeTask, [ = ]( int i, int n )
      {
        mColumnTypeTask->setProxyProgress( 100.0 * static_cast< double >( i ) / n );
      } );
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

  return QVector<QgsDataItem *>();
}

void QgsOracleConnectionItem::threadStarted()
{
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 3 );
}

void QgsOracleConnectionItem::threadFinished()
{
  mColumnTypeTask->finalize( true );
  mColumnTypeTask = nullptr;

  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 3 );
  setAllAsPopulated();
}

void QgsOracleConnectionItem::setLayerType( const QgsOracleLayerProperty &layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );
  QgsOracleOwnerItem *ownerItem = mOwnerMap.value( layerProperty.ownerName, nullptr );

  for ( int i = 0 ; i < layerProperty.size(); i++ )
  {
    QgsWkbTypes::Type wkbType = layerProperty.types.at( i );
    if ( wkbType == QgsWkbTypes::Unknown )
    {
      QgsDebugMsgLevel( QStringLiteral( "skip unknown geometry type" ), 3 );
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

    QgsDebugMsgLevel( QStringLiteral( "ADD LAYER" ), 3 );
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

QList<QAction *> QgsOracleConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionRefresh = new QAction( tr( "Refresh" ), parent );
  connect( actionRefresh, &QAction::triggered, this, &QgsOracleConnectionItem::refreshConnection );
  lst.append( actionRefresh );

  QAction *separator = new QAction( parent );
  separator->setSeparator( true );
  lst.append( separator );

  QAction *actionEdit = new QAction( tr( "Edit Connection…" ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsOracleConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete Connection" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsOracleConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsOracleConnectionItem::editConnection()
{
  QgsOracleNewConnection nc( nullptr, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refreshConnections();
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
    mParent->refreshConnections();
}

void QgsOracleConnectionItem::refreshConnection()
{
  // the parent should be updated
  refresh();
}

bool QgsOracleConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceUri uri = QgsOracleConn::connUri( mName );

  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Q_FOREACH ( const QgsMimeDataUtils::Uri &u, lst )
  {
    if ( u.layerType != QLatin1String( "vector" ) )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    // open the source layer
    QgsVectorLayer *srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey );

    if ( srcLayer->isValid() )
    {
      uri.setDataSource( QString(), u.name.left( 30 ).toUpper(), QStringLiteral( "GEOM" ) );
      uri.setWkbType( srcLayer->wkbType() );
      QString authid = srcLayer->crs().authid();
      if ( authid.startsWith( QStringLiteral( "EPSG:" ), Qt::CaseInsensitive ) )
      {
        uri.setSrid( authid.mid( 5 ) );
      }
      QgsDebugMsgLevel( "URI " + uri.uri(), 3 );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( QgsVectorLayerExporterTask::withLayerOwnership( srcLayer, uri.uri(), QStringLiteral( "oracle" ), srcLayer->crs() ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to Oracle database" ), tr( "Import was successful." ) );
        if ( state() == Populated )
          refresh();
        else
          populate();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
      {
        if ( error != QgsVectorLayerExporter::ErrUserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to Oracle database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        if ( state() == Populated )
          refresh();
        else
          populate();
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
    output->setTitle( tr( "Import to Oracle database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( '\n' ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsOracleLayerItem::QgsOracleLayerItem( QgsDataItem *parent, const QString &name, const QString &path, QgsLayerItem::LayerType layerType, const QgsOracleLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "oracle" ) )
  , mLayerProperty( layerProperty )
{
  mUri = createUri();
  setState( Populated );
}

QList<QAction *> QgsOracleLayerItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionDeleteLayer = new QAction( tr( "Delete Table" ), parent );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsOracleLayerItem::deleteLayer );
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
    QMessageBox::warning( nullptr, tr( "Delete Table" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Delete Table" ), tr( "Table deleted successfully." ) );
    deleteLater();
  }
}

QString QgsOracleLayerItem::createUri()
{
  Q_ASSERT( mLayerProperty.size() == 1 );
  QgsOracleConnectionItem *connItem = qobject_cast<QgsOracleConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  QgsDataSourceUri uri = QgsOracleConn::connUri( connItem->name() );
  uri.setDataSource( mLayerProperty.ownerName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, QString() );
  uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( mLayerProperty.isView && mLayerProperty.pkCols.size() > 0 )
    uri.setKeyColumn( mLayerProperty.pkCols[0] );
  QgsDebugMsgLevel( QString( QStringLiteral( "layer uri: %1" ) ).arg( uri.uri() ), 3 );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsOracleOwnerItem::QgsOracleOwnerItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconDbOwner.png" );
  //not fertile, since children are created by QgsOracleConnectionItem
  mCapabilities &= ~( Fertile );
}

QVector<QgsDataItem *> QgsOracleOwnerItem::createChildren()
{
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 3 );
  return QVector<QgsDataItem *>();
}

void QgsOracleOwnerItem::addLayer( const QgsOracleLayerProperty &layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );

  Q_ASSERT( layerProperty.size() == 1 );
  QgsWkbTypes::Type wkbType = layerProperty.types.at( 0 );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName, QgsOracleConn::displayStringForWkbType( wkbType ) ).arg( layerProperty.srids.at( 0 ) );

  QgsLayerItem::LayerType layerType;
  switch ( wkbType )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      layerType = QgsLayerItem::Point;
      break;
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
      layerType = QgsLayerItem::Line;
      break;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( wkbType == QgsWkbTypes::NoGeometry && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else
      {
        return;
      }
  }

  QgsOracleLayerItem *layerItem = new QgsOracleLayerItem( this, layerProperty.tableName, mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  addChildItem( layerItem, true );
}

// ---------------------------------------------------------------------------
QgsOracleRootItem::QgsOracleRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconOracle.svg" );
  populate();
}

QVector<QgsDataItem *> QgsOracleRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  Q_FOREACH ( QString connName, QgsOracleConn::connectionList() )
  {
    connections << new QgsOracleConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

QList<QAction *> QgsOracleRootItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsOracleRootItem::newConnection );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsOracleRootItem::paramWidget()
{
  QgsOracleSourceSelect *select = new QgsOracleSourceSelect();
  connect( select, &QgsAbstractDataSourceWidget::connectionsChanged, this, &QgsOracleRootItem::connectionsChanged );
  return select;
}

void QgsOracleRootItem::connectionsChanged()
{
  refresh();
}

void QgsOracleRootItem::newConnection()
{
  QgsOracleNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    refreshConnections();
  }
}

QMainWindow *QgsOracleRootItem::sMainWindow = nullptr;

QGISEXTERN void registerGui( QMainWindow *mainWindow )
{
  QgsOracleRootItem::sMainWindow = mainWindow;
}
