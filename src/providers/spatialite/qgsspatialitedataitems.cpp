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

#include "qgsapplication.h"
#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnection.h"
#include "qgsfieldsitem.h"

#ifdef HAVE_GUI
#include "qgsspatialitesourceselect.h"
#endif

#include "qgslogger.h"
#include "qgsvectorlayerexporter.h"
#include "qgsvectorlayer.h"

#include <QDir>
#include <QFileInfo>


bool SpatiaLiteUtils::deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause )
{
  QgsDebugMsg( "deleting layer " + tableName );

  QgsSqliteHandle *hndl = QgsSqliteHandle::openDb( dbPath );
  if ( !hndl )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }
  sqlite3 *sqlite_handle = hndl->handle();
  int ret;
  if ( !gaiaDropTable( sqlite_handle, tableName.toUtf8().constData() ) )
  {
    // unexpected error
    errCause = QObject::tr( "Unable to delete table %1\n" ).arg( tableName );
    QgsSqliteHandle::closeDb( hndl );
    return false;
  }

  // TODO: remove spatial indexes?
  // run VACUUM to free unused space and compact the database
  ret = sqlite3_exec( sqlite_handle, "VACUUM", nullptr, nullptr, nullptr );
  if ( ret != SQLITE_OK )
  {
    QgsDebugMsg( "Failed to run VACUUM after deleting table on database " + dbPath );
  }

  QgsSqliteHandle::closeDb( hndl );

  return true;
}

QgsSLLayerItem::QgsSLLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType )
  : QgsLayerItem( parent, name, path, uri, layerType, QStringLiteral( "spatialite" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete;
  setState( Qgis::BrowserItemState::NotPopulated );
}

// ------

QgsSLConnectionItem::QgsSLConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "spatialite" ) )
{
  mDbPath = QgsSpatiaLiteConnection::connectionPath( name );
  mToolTip = mDbPath;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

static Qgis::BrowserLayerType _layerTypeFromDb( const QString &dbType )
{
  if ( dbType == QLatin1String( "POINT" ) || dbType == QLatin1String( "MULTIPOINT" ) )
  {
    return Qgis::BrowserLayerType::Point;
  }
  else if ( dbType == QLatin1String( "LINESTRING" ) || dbType == QLatin1String( "MULTILINESTRING" ) )
  {
    return Qgis::BrowserLayerType::Line;
  }
  else if ( dbType == QLatin1String( "POLYGON" ) || dbType == QLatin1String( "MULTIPOLYGON" ) )
  {
    return Qgis::BrowserLayerType::Polygon;
  }
  else if ( dbType == QLatin1String( "qgis_table" ) )
  {
    return Qgis::BrowserLayerType::Table;
  }
  else
  {
    return Qgis::BrowserLayerType::NoType;
  }
}

QVector<QgsDataItem *> QgsSLConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QgsSpatiaLiteConnection connection( mName );

  const QgsSpatiaLiteConnection::Error err = connection.fetchTables( true );
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
    const QString msgDetails = connection.errorMessage();
    if ( !msgDetails.isEmpty() )
      msg = QStringLiteral( "%1 (%2)" ).arg( msg, msgDetails );
    children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
    return children;
  }

  const QString connectionInfo = QStringLiteral( "dbname='%1'" ).arg( QString( connection.path() ).replace( '\'', QLatin1String( "\\'" ) ) );
  QgsDataSourceUri uri( connectionInfo );

  const auto constTables = connection.tables();
  for ( const QgsSpatiaLiteConnection::TableEntry &entry : constTables )
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


// ---------------------------------------------------------------------------

QgsSLRootItem::QgsSLRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "spatialite" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconSpatialite.svg" );
  populate();
}

QVector<QgsDataItem *> QgsSLRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsSpatiaLiteConnection::connectionList();
  for ( const QString &connName : list )
  {
    QgsDataItem *conn = new QgsSLConnectionItem( this, connName, mPath + '/' + connName );
    connections.push_back( conn );
  }
  return connections;
}

#ifdef HAVE_GUI

QWidget *QgsSLRootItem::paramWidget()
{
  QgsSpatiaLiteSourceSelect *select = new QgsSpatiaLiteSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsSpatiaLiteSourceSelect::connectionsChanged, this, &QgsSLRootItem::onConnectionsChanged );
  return select;
}

void QgsSLRootItem::onConnectionsChanged()
{
  refresh();
}

#endif

static bool initializeSpatialMetadata( sqlite3 *sqlite_handle, QString &errCause )
{
  // attempting to perform self-initialization for a newly created DB
  if ( !sqlite_handle )
    return false;

  // checking if this DB is really empty
  char **results = nullptr;
  int rows, columns;
  int ret = sqlite3_get_table( sqlite_handle, "select count(*) from sqlite_master", &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return false;

  int count = 0;
  if ( rows >= 1 )
  {
    for ( int i = 1; i <= rows; i++ )
      count = atoi( results[( i * columns ) + 0] );
  }

  sqlite3_free_table( results );

  if ( count > 0 )
    return false;

  const bool above41 = QgsSpatiaLiteProvider::versionIsAbove( sqlite_handle, 4, 1 );

  // all right, it's empty: proceeding to initialize
  char *errMsg = nullptr;
  ret = sqlite3_exec( sqlite_handle, above41 ? "SELECT InitSpatialMetadata(1)" : "SELECT InitSpatialMetadata()", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to initialize SpatialMetadata:\n" );
    errCause += QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    return false;
  }
  spatial_ref_sys_init( sqlite_handle, 0 );
  return true;
}

bool SpatiaLiteUtils::createDb( const QString &dbPath, QString &errCause )
{
  QgsDebugMsg( QStringLiteral( "creating a new db" ) );

  const QFileInfo fullPath = QFileInfo( dbPath );
  const QDir path = fullPath.dir();
  QgsDebugMsg( QStringLiteral( "making this dir: %1" ).arg( path.absolutePath() ) );

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath() );

  // creating/opening the new database
  spatialite_database_unique_ptr database;
  int ret = database.open_v2( dbPath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( ret )
  {
    // an error occurred
    errCause = QObject::tr( "Could not create a new database\n" );
    errCause += database.errorMessage();
    return false;
  }
  // activating Foreign Key constraints
  char *errMsg = nullptr;
  ret = sqlite3_exec( database.get(), "PRAGMA foreign_keys = 1", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to activate FOREIGN_KEY constraints [%1]" ).arg( errMsg );
    sqlite3_free( errMsg );
    return false;
  }
  const bool init_res = ::initializeSpatialMetadata( database.get(), errCause );

  return init_res;
}

// ---------------------------------------------------------------------------

QString QgsSpatiaLiteDataItemProvider::name()
{
  return QStringLiteral( "spatialite" );
}

QString QgsSpatiaLiteDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "spatialite" );
}

int QgsSpatiaLiteDataItemProvider::capabilities() const
{
  return QgsDataProvider::Database;
}

QgsDataItem *QgsSpatiaLiteDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsSLRootItem( parentItem, QStringLiteral( "SpatiaLite" ), QStringLiteral( "spatialite:" ) );
}


bool QgsSLConnectionItem::layerCollection() const
{
  return true;
}

QVector<QgsDataItem *> QgsSLLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this,
                                         path() + QStringLiteral( "/columns/ " ),
                                         uri(),
                                         QStringLiteral( "spatialite" ), QString(), name() ) );
  return children;
}
