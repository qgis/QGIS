/***************************************************************************
    qgsspatialitedataitems.cpp
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
#include "qgsspatialitedataitems.h"

#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnection.h"

#ifdef HAVE_GUI
#include "qgsspatialitesourceselect.h"
#endif

#include "qgslogger.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerexporter.h"
#include "qgsmessageoutput.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

QGISEXTERN bool deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause );

QgsSLLayerItem::QgsSLLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType )
  : QgsLayerItem( parent, name, path, uri, layerType, QStringLiteral( "spatialite" ) )
{
  mCapabilities |= Delete;
  setState( Populated ); // no children are expected
}

#ifdef HAVE_GUI
bool QgsSLLayerItem::deleteLayer()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Object" ),
                              QObject::tr( "Are you sure you want to delete %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return true;

  QgsDataSourceUri uri( mUri );
  QString errCause;
  bool res = ::deleteLayer( uri.database(), uri.table(), errCause );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Delete Layer" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Delete Layer" ), tr( "Layer deleted successfully." ) );
    mParent->refresh();
  }
  return true;
}
#endif

// ------

QgsSLConnectionItem::QgsSLConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mDbPath = QgsSpatiaLiteConnection::connectionPath( name );
  mToolTip = mDbPath;
  mCapabilities |= Collapse;
}

static QgsLayerItem::LayerType _layerTypeFromDb( const QString &dbType )
{
  if ( dbType == QLatin1String( "POINT" ) || dbType == QLatin1String( "MULTIPOINT" ) )
  {
    return QgsLayerItem::Point;
  }
  else if ( dbType == QLatin1String( "LINESTRING" ) || dbType == QLatin1String( "MULTILINESTRING" ) )
  {
    return QgsLayerItem::Line;
  }
  else if ( dbType == QLatin1String( "POLYGON" ) || dbType == QLatin1String( "MULTIPOLYGON" ) )
  {
    return QgsLayerItem::Polygon;
  }
  else if ( dbType == QLatin1String( "qgis_table" ) )
  {
    return QgsLayerItem::Table;
  }
  else
  {
    return QgsLayerItem::NoType;
  }
}

QVector<QgsDataItem *> QgsSLConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QgsSpatiaLiteConnection connection( mName );

  QgsSpatiaLiteConnection::Error err = connection.fetchTables( true );
  if ( err != QgsSpatiaLiteConnection::NoError )
  {
    QString msg;
    switch ( err )
    {
      case QgsSpatiaLiteConnection::NotExists:
        msg = tr( "Database does not exist" );
        break;
      case QgsSpatiaLiteConnection::FailedToOpen:
        msg = tr( "Failed to open database" );
        break;
      case QgsSpatiaLiteConnection::FailedToCheckMetadata:
        msg = tr( "Failed to check metadata" );
        break;
      case QgsSpatiaLiteConnection::FailedToGetTables:
        msg = tr( "Failed to get list of tables" );
        break;
      default:
        msg = tr( "Unknown error" );
        break;
    }
    QString msgDetails = connection.errorMessage();
    if ( !msgDetails.isEmpty() )
      msg = QStringLiteral( "%1 (%2)" ).arg( msg, msgDetails );
    children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
    return children;
  }

  QString connectionInfo = QStringLiteral( "dbname='%1'" ).arg( QString( connection.path() ).replace( '\'', QLatin1String( "\\'" ) ) );
  QgsDataSourceUri uri( connectionInfo );

  Q_FOREACH ( const QgsSpatiaLiteConnection::TableEntry &entry, connection.tables() )
  {
    uri.setDataSource( QString(), entry.tableName, entry.column, QString(), QString() );
    QgsSLLayerItem *layer = new QgsSLLayerItem( this, entry.tableName, mPath + '/' + entry.tableName, uri.uri(), _layerTypeFromDb( entry.type ) );
    children.append( layer );
  }
  return children;
}

bool QgsSLConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsSLConnectionItem *o = dynamic_cast<const QgsSLConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;
}

#ifdef HAVE_GUI
QList<QAction *> QgsSLConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  //QAction* actionEdit = new QAction( tr( "Edit…" ), parent );
  //connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  //lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsSLConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsSLConnectionItem::editConnection()
{
}

void QgsSLConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsSpatiaLiteConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refreshConnections();
}
#endif

bool QgsSLConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc

  QgsDataSourceUri destUri;
  destUri.setDatabase( mDbPath );

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
      destUri.setDataSource( QString(), u.name, srcLayer->geometryType() != QgsWkbTypes::NullGeometry ? QStringLiteral( "geom" ) : QString() );
      QgsDebugMsg( "URI " + destUri.uri() );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( new QgsVectorLayerExporterTask( srcLayer, destUri.uri(), QStringLiteral( "spatialite" ), srcLayer->crs(), QVariantMap(), owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to SpatiaLite database" ), tr( "Import was successful." ) );
        refresh();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
      {
        if ( error != QgsVectorLayerExporter::ErrUserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to SpatiaLite database" ) );
          output->setMessage( tr( "Failed to import layer!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        refresh();
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
    output->setTitle( tr( "Import to SpatiaLite database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}


// ---------------------------------------------------------------------------

QgsSLRootItem::QgsSLRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconSpatialite.svg" );
  populate();
}

QVector<QgsDataItem *> QgsSLRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  Q_FOREACH ( const QString &connName, QgsSpatiaLiteConnection::connectionList() )
  {
    QgsDataItem *conn = new QgsSLConnectionItem( this, connName, mPath + '/' + connName );
    connections.push_back( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsSLRootItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsSLRootItem::newConnection );
  lst.append( actionNew );

  QAction *actionCreateDatabase = new QAction( tr( "Create Database…" ), parent );
  connect( actionCreateDatabase, &QAction::triggered, this, &QgsSLRootItem::createDatabase );
  lst.append( actionCreateDatabase );

  return lst;
}

QWidget *QgsSLRootItem::paramWidget()
{
  QgsSpatiaLiteSourceSelect *select = new QgsSpatiaLiteSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsSpatiaLiteSourceSelect::connectionsChanged, this, &QgsSLRootItem::onConnectionsChanged );
  return select;
}

void QgsSLRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsSLRootItem::newConnection()
{
  if ( QgsSpatiaLiteSourceSelect::newConnection( nullptr ) )
  {
    refreshConnections();
  }
}
#endif

QGISEXTERN bool createDb( const QString &dbPath, QString &errCause );

void QgsSLRootItem::createDatabase()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::homePath() ).toString();

  QString filename = QFileDialog::getSaveFileName( nullptr, tr( "New SpatiaLite Database File" ),
                     lastUsedDir,
                     tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)" );
  if ( filename.isEmpty() )
    return;

  QString errCause;
  if ( ::createDb( filename, errCause ) )
  {
    // add connection
    settings.setValue( "/SpatiaLite/connections/" + QFileInfo( filename ).fileName() + "/sqlitepath", filename );

    refresh();
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Create SpatiaLite database" ), tr( "Failed to create the database:\n" ) + errCause );
  }
}

// ---------------------------------------------------------------------------

#ifdef HAVE_GUI
QGISEXTERN QgsSpatiaLiteSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  // TODO: this should be somewhere else
  return new QgsSpatiaLiteSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  Q_UNUSED( path );
  return new QgsSLRootItem( parentItem, QStringLiteral( "SpatiaLite" ), QStringLiteral( "spatialite:" ) );
}
