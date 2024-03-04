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
#include "qgsoraclenewconnection.h"
#include "qgsoraclecolumntypetask.h"
#include "qgsoracleprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsvectorlayer.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsvectorlayerexporter.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QSqlError>
#include <QStatusBar>

bool deleteLayer( const QString &uri, QString &errCause )
{
  QgsDebugMsgLevel( "deleting layer " + uri, 2 );

  QgsDataSourceUri dsUri( uri );
  QString ownerName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( ownerName != conn->currentUser() )
  {
    errCause = QObject::tr( "%1 not owner of the table %2." )
               .arg( ownerName )
               .arg( tableName );
    conn->disconnect();
    return false;
  }

  QSqlQuery qry( *conn );

  // check the geometry column count
  if ( !QgsOracleProvider::execLoggedStatic( qry, QString( "SELECT count(*)"
       " FROM user_tab_columns"
       " WHERE table_name=? AND data_type='SDO_GEOMETRY' AND data_type_owner='MDSYS'" ),
       QVariantList() << tableName, dsUri.uri(), QStringLiteral( "QgsOracleLayerItem" ), QGS_QUERY_LOG_ORIGIN )
       || !qry.next() )
  {
    errCause = QObject::tr( "Unable to determine number of geometry columns of layer %1.%2: \n%3" )
               .arg( ownerName )
               .arg( tableName )
               .arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }

  int count = qry.value( 0 ).toInt();

  QString dropTable;
  QString cleanView;
  QVariantList args;
  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    dropTable = QString( "ALTER TABLE %1 DROP COLUMN %2" )
                .arg( QgsOracleConn::quotedIdentifier( tableName ) )
                .arg( QgsOracleConn::quotedIdentifier( geometryCol ) );
    cleanView = QString( "DELETE FROM mdsys.user_sdo_geom_metadata WHERE table_name=? AND column_name=?" );
    args << tableName << geometryCol;
  }
  else
  {
    // drop the table
    dropTable = QString( "DROP TABLE %1" )
                .arg( QgsOracleConn::quotedIdentifier( tableName ) );
    cleanView = QString( "DELETE FROM mdsys.user_sdo_geom_metadata WHERE table_name=%1" );
    args << tableName;
  }

  if ( !QgsOracleProvider::execLoggedStatic( qry, dropTable, QVariantList(), dsUri.uri(), QStringLiteral( "QgsOracleLayerItem" ), QGS_QUERY_LOG_ORIGIN ) )
  {
    errCause = QObject::tr( "Unable to delete layer %1.%2: \n%3" )
               .arg( ownerName )
               .arg( tableName )
               .arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }

  if ( !QgsOracleProvider::execLoggedStatic( qry, cleanView, args, dsUri.uri(), QStringLiteral( "QgsOracleLayerItem" ), QGS_QUERY_LOG_ORIGIN ) )
  {
    errCause = QObject::tr( "Unable to clean metadata %1.%2: \n%3" )
               .arg( ownerName )
               .arg( tableName )
               .arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }

  conn->disconnect();
  return true;
}

// ---------------------------------------------------------------------------
QgsOracleConnectionItem::QgsOracleConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "ORACLE" ) )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::Fast;
}

QgsOracleConnectionItem::~QgsOracleConnectionItem()
{
  stop();
}

void QgsOracleConnectionItem::stop()
{
  if ( mColumnTypeTask )
  {
    mColumnTypeTask->cancel();
    disconnect( mColumnTypeTask, nullptr, this, nullptr );
    disconnect( mColumnTypeTask, nullptr, QgsOracleRootItem::sMainWindow, nullptr );

    // don't delete the task, taskManager takes ownership of it
    mColumnTypeTask = nullptr;
  }
}

void QgsOracleConnectionItem::refresh()
{
  stop();

  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    deleteChildItem( child );
  }

  const auto constCreateChildren = createChildren();
  for ( QgsDataItem *item : constCreateChildren )
  {
    addChildItem( item, true );
  }
}

void QgsOracleConnectionItem::setAllAsPopulated()
{
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    child->setState( Qgis::BrowserItemState::Populated );
  }
  setState( Qgis::BrowserItemState::Populated );
}

QVector<QgsDataItem *> QgsOracleConnectionItem::createChildren()
{
  setState( Qgis::BrowserItemState::Populating );

  mOwnerMap.clear();

  stop();

  if ( deferredDelete() )
    return QVector<QgsDataItem *>();

  if ( !mColumnTypeTask )
  {
    mColumnTypeTask = new QgsOracleColumnTypeTask( mName,
        QgsOracleConn::restrictToSchema( mName ),
        /* useEstimatedMetadata */ true,
        QgsOracleConn::allowGeometrylessTables( mName ) );

    connect( mColumnTypeTask, &QgsOracleColumnTypeTask::setLayerType,
             this, &QgsOracleConnectionItem::setLayerType );
    connect( mColumnTypeTask, &QgsTask::begun, this, &QgsOracleConnectionItem::taskStarted );
    connect( mColumnTypeTask, &QgsTask::taskCompleted, this, &QgsOracleConnectionItem::taskFinished );
    connect( mColumnTypeTask, &QgsTask::taskTerminated, this, &QgsOracleConnectionItem::taskFinished );

    if ( QgsOracleRootItem::sMainWindow )
    {
      connect( mColumnTypeTask, &QgsOracleColumnTypeTask::progressMessage,
               QgsOracleRootItem::sMainWindow->statusBar(), [ = ]( const QString & message )
      {
        QgsOracleRootItem::sMainWindow->statusBar()->showMessage( message );
      } );
    }

    QgsApplication::taskManager()->addTask( mColumnTypeTask );
  }

  return QVector<QgsDataItem *>();
}

void QgsOracleConnectionItem::taskStarted()
{
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 3 );
}

void QgsOracleConnectionItem::taskFinished()
{
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 3 );

  if ( mColumnTypeTask->status() == QgsTask::Complete )
    setAllAsPopulated();
  else
    setState( Qgis::BrowserItemState::NotPopulated );

  mColumnTypeTask = nullptr;
}

void QgsOracleConnectionItem::setLayerType( const QgsOracleLayerProperty &layerProperty )
{
  QgsDebugMsgLevel( layerProperty.toString(), 3 );
  QgsOracleOwnerItem *ownerItem = mOwnerMap.value( layerProperty.ownerName, nullptr );

  for ( int i = 0 ; i < layerProperty.size(); i++ )
  {
    Qgis::WkbType wkbType = layerProperty.types.at( i );
    if ( wkbType == Qgis::WkbType::Unknown )
    {
      QgsDebugMsgLevel( QStringLiteral( "skip unknown geometry type" ), 3 );
      continue;
    }

    if ( !ownerItem )
    {
      ownerItem = new QgsOracleOwnerItem( this, layerProperty.ownerName, mPath + "/" + layerProperty.ownerName );
      ownerItem->setState( Qgis::BrowserItemState::Populating );
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

  QAction *actionDelete = new QAction( tr( "Remove Connection" ), parent );
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
  if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ),
                              QObject::tr( "Are you sure you want to remove the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "oracle" ) );
  providerMetadata->deleteConnection( mName );

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
  const auto constLst = lst;
  for ( const QgsMimeDataUtils::Uri &u : constLst )
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
      QgsDebugMsgLevel( "URI " + uri.uri( false ), 3 );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( QgsVectorLayerExporterTask::withLayerOwnership( srcLayer, uri.uri( false ), QStringLiteral( "oracle" ), srcLayer->crs() ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to Oracle database" ), tr( "Import was successful." ) );
        if ( state() == Qgis::BrowserItemState::Populated )
          refresh();
        else
          populate();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( Qgis::VectorExportResult error, const QString & errorMessage )
      {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to Oracle database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        if ( state() == Qgis::BrowserItemState::Populated )
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
QgsOracleLayerItem::QgsOracleLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsOracleLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "oracle" ) )
  , mLayerProperty( layerProperty )
{
  mUri = createUri();
  setState( Qgis::BrowserItemState::Populated );
}

QList<QAction *> QgsOracleLayerItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionDeleteLayer = new QAction( tr( "Delete Table" ), parent );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsOracleLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );

  return lst;
}

bool QgsOracleLayerItem::deleteLayer()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Table" ),
                              QObject::tr( "Are you sure you want to delete %1.%2?" ).arg( mLayerProperty.ownerName, mLayerProperty.tableName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return true;

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

  return res;
}

QString QgsOracleLayerItem::createUri()
{
  Q_ASSERT( mLayerProperty.size() == 1 );
  QgsOracleConnectionItem *connItem = qobject_cast<QgsOracleConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugError( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  QgsDataSourceUri uri = QgsOracleConn::connUri( connItem->name() );
  uri.setDataSource( mLayerProperty.ownerName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, QString() );
  uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( mLayerProperty.isView && mLayerProperty.pkCols.size() > 0 )
    uri.setKeyColumn( mLayerProperty.pkCols[0] );
  QgsDebugMsgLevel( QStringLiteral( "layer uri: %1" ).arg( uri.uri( false ) ), 3 );
  return uri.uri( false );
}

// ---------------------------------------------------------------------------
QgsOracleOwnerItem::QgsOracleOwnerItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "ORACLE" ) )
{
  mIconName = QStringLiteral( "mIconDbOwner.png" );
  //not fertile, since children are created by QgsOracleConnectionItem
  mCapabilities &= ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile );
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
  Qgis::WkbType wkbType = layerProperty.types.at( 0 );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName, QgsWkbTypes::translatedDisplayString( wkbType ) ).arg( layerProperty.srids.at( 0 ) );

  Qgis::BrowserLayerType layerType;
  switch ( QgsWkbTypes::geometryType( wkbType ) )
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
      if ( wkbType == Qgis::WkbType::NoGeometry && layerProperty.geometryColName.isEmpty() )
      {
        layerType = Qgis::BrowserLayerType::TableLayer;
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
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "ORACLE" ) )
{
  mIconName = QStringLiteral( "mIconOracle.svg" );
  populate();
}

QVector<QgsDataItem *> QgsOracleRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsOracleConn::connectionList();
  for ( const QString &connName : std::as_const( list ) )
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

QgsDataItem *QgsOracleDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsOracleRootItem( parentItem, "Oracle", "oracle:" );
}

QString QgsOracleDataItemProvider::name()
{
  return QStringLiteral( "ORACLE" );
}

QString QgsOracleDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "oracle" );
}

Qgis::DataItemProviderCapabilities QgsOracleDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::Databases;
}


bool QgsOracleOwnerItem::layerCollection() const
{
  return true;
}
