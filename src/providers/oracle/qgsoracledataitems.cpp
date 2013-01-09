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

#include <QMessageBox>
#include <QProgressDialog>

QGISEXTERN bool deleteLayer( const QString& uri, QString& errCause );

// ---------------------------------------------------------------------------
QgsOracleConnectionItem::QgsOracleConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
    , mColumnTypeThread( 0 )
{
  mIcon = QgsApplication::getThemeIcon( "mIconConnect.png" );
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
    mColumnTypeThread = 0;
  }
}

void QgsOracleConnectionItem::refresh()
{
  QgsDebugMsg( "Entering." );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  stop();

  foreach ( QgsDataItem *child, mChildren )
  {
    QgsDebugMsg( QString( "Deleting child: %1" ).arg( child->name() ) );
    deleteChildItem( child );
  }

  foreach ( QgsDataItem *item, createChildren() )
  {
    QgsDebugMsg( QString( "Adding child %1" ).arg( item->name() ) );
    addChildItem( item, true );
  }

  QApplication::restoreOverrideCursor();

  QgsDebugMsg( "Leaving." );
}

QVector<QgsDataItem*> QgsOracleConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;
  QgsDataSourceURI uri = QgsOracleConn::connUri( mName );

  mOwnerMap.clear();

  mConn = QgsOracleConn::connectDb( uri.connectionInfo() );
  if ( !mConn )
    return children;

  QVector<QgsOracleLayerProperty> layerProperties;
  if ( !mConn->supportedLayers( layerProperties,
                                QgsOracleConn::userTablesOnly( mName ),
                                QgsOracleConn::allowGeometrylessTables( mName ) ) )
  {
    children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    return children;
  }

  if ( layerProperties.isEmpty() )
  {
    children.append( new QgsErrorItem( this, tr( "No layers found." ), mPath + "/error" ) );
    return children;
  }

  stop();

  foreach ( QgsOracleLayerProperty layerProperty, layerProperties )
  {
    QgsOracleOwnerItem *ownerItem = mOwnerMap.value( layerProperty.ownerName, 0 );
    if ( !ownerItem )
    {
      ownerItem = new QgsOracleOwnerItem( this, layerProperty.ownerName, mPath + "/" + layerProperty.ownerName );
      children.append( ownerItem );
      mOwnerMap[ layerProperty.ownerName ] = ownerItem;
    }

    if ( !layerProperty.geometryColName.isNull() )
    {
      if ( layerProperty.types.contains( QGis::WKBUnknown ) || layerProperty.srids.isEmpty() )
      {
        if ( !mColumnTypeThread )
        {
          QgsOracleConn *conn = QgsOracleConn::connectDb( uri.connectionInfo() );
          if ( conn )
          {
            mColumnTypeThread = new QgsOracleColumnTypeThread( conn, true /* use estimated metadata */ );

            connect( mColumnTypeThread, SIGNAL( setLayerType( QgsOracleLayerProperty ) ),
                     this, SLOT( setLayerType( QgsOracleLayerProperty ) ) );
            connect( this, SIGNAL( addGeometryColumn( QgsOracleLayerProperty ) ),
                     mColumnTypeThread, SLOT( addGeometryColumn( QgsOracleLayerProperty ) ) );
          }
        }

        emit addGeometryColumn( layerProperty );
      }
    }
    else
    {
      ownerItem->addLayer( layerProperty );
    }
  }

  if ( mColumnTypeThread )
    mColumnTypeThread->start();

  return children;
}

void QgsOracleConnectionItem::setLayerType( QgsOracleLayerProperty layerProperty )
{
  QgsDebugMsg( layerProperty.toString() );

  QgsOracleOwnerItem *ownerItem = mOwnerMap.value( layerProperty.ownerName, 0 );
  if ( !ownerItem )
  {
    QgsDebugMsg( QString( "owner item for %1 not found." ).arg( layerProperty.ownerName ) );
    return;
  }

  for ( int i = 0 ; i < layerProperty.size(); i++ )
  {
    QGis::WkbType wkbType = layerProperty.types.at( i );
    if ( wkbType == QGis::WKBUnknown )
    {
      // skip any geometry entry
      continue;
    }

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
  return ( mPath == o->mPath && mName == o->mName && o->connection() == connection() );
}

QList<QAction*> QgsOracleConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
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
  QgsOracleConn::deleteConnection( mName );

  // the parent should be updated
  mParent->refresh();
}

bool QgsOracleConnectionItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceURI uri = QgsOracleConn::connUri( mName );

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
      uri.setDataSource( QString(), u.name.left( 30 ).toUpper(), "QGS_GEOMETRY" );
      uri.setWkbType( srcLayer->wkbType() );
      QString authid = srcLayer->crs().authid();
      if( authid.startsWith( "EPSG:", Qt::CaseInsensitive ) )
      {
        uri.setSrid( authid.mid( 5 ) );
      }
      QgsDebugMsg( "URI " + uri.uri() );
      QgsVectorLayerImport::ImportError err;
      QString importError;
      err = QgsVectorLayerImport::importLayer( srcLayer, uri.uri(), "oracle", &srcLayer->crs(), false, &importError, false, 0, progress );
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
    QMessageBox::warning( 0, tr( "Import to Oracle database" ), tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ) );
  }
  else
  {
    QMessageBox::information( 0, tr( "Import to Oracle database" ), tr( "Import was successful." ) );
  }

  refresh();

  return true;
}

// ---------------------------------------------------------------------------
QgsOracleLayerItem::QgsOracleLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsOracleLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, "oracle" )
    , mLayerProperty( layerProperty )
{
  mUri = createUri();
  mPopulated = true;
}

QgsOracleLayerItem::~QgsOracleLayerItem()
{
}

QList<QAction*> QgsOracleLayerItem::actions()
{
  QList<QAction*> lst;

  QAction* actionDeleteLayer = new QAction( tr( "Delete layer" ), this );
  connect( actionDeleteLayer, SIGNAL( triggered() ), this, SLOT( deleteLayer() ) );
  lst.append( actionDeleteLayer );

  return lst;
}

void QgsOracleLayerItem::deleteLayer()
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

QString QgsOracleLayerItem::createUri()
{
  QString pkColName = mLayerProperty.pkCols.size() > 0 ? mLayerProperty.pkCols.at( 0 ) : QString::null;

  QgsOracleConnectionItem *connItem = qobject_cast<QgsOracleConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDebugMsg( QString( "connInfo: %1" ).arg( connItem->connection()->connInfo() ) );

  QgsDataSourceURI uri( connItem->connection()->connInfo() );
  uri.setDataSource( mLayerProperty.ownerName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, QString::null );
  Q_ASSERT( mLayerProperty.size() == 1 );
  uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  QgsDebugMsg( QString( "layer uri: %1" ).arg( uri.uri() ) );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsOracleOwnerItem::QgsOracleOwnerItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconDbOwner.png" );
}

QVector<QgsDataItem*> QgsOracleOwnerItem::createChildren()
{
  QgsDebugMsg( "Entering." );
  return QVector<QgsDataItem*>();
}

QgsOracleOwnerItem::~QgsOracleOwnerItem()
{
}

void QgsOracleOwnerItem::addLayer( QgsOracleLayerProperty layerProperty )
{
  QgsDebugMsg( layerProperty.toString() );

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
  addChild( layerItem );
}

// ---------------------------------------------------------------------------
QgsOracleRootItem::QgsOracleRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconOracle.png" );
  populate();
}

QgsOracleRootItem::~QgsOracleRootItem()
{
}

QVector<QgsDataItem*> QgsOracleRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach ( QString connName, QgsOracleConn::connectionList() )
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
