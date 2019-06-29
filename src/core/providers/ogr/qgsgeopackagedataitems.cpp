/***************************************************************************
    qgsgeopackagedataitems.h
    ---------------------
    begin                : August 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgeopackagedataitems.h"
///@cond PRIVATE

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include <sqlite3.h>

#include "qgssqliteutils.h"
#include "qgsgeopackagedataitems.h"
#include "qgsogrdbconnection.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsogrprovider.h"
#include "qgsogrdataitems.h"
#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsvectorlayerexporter.h"
#include "qgsgeopackagerasterwritertask.h"
#include "qgstaskmanager.h"
#include "qgsproviderregistry.h"
#include "qgsproxyprogresstask.h"
#include "qgsprojectstorageregistry.h"
#include "qgsgeopackageprojectstorage.h"

QgsDataItem *QgsGeoPackageDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "path = " + path );
  if ( path.isEmpty() )
  {
    return new QgsGeoPackageRootItem( parentItem, QStringLiteral( "GeoPackage" ), QStringLiteral( "gpkg:" ) );
  }
  return nullptr;
}

QgsGeoPackageRootItem::QgsGeoPackageRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mGeoPackage.svg" );
  populate();
}

QVector<QgsDataItem *> QgsGeoPackageRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connList( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ) );
  for ( const QString &connName : connList )
  {
    QgsOgrDbConnection connection( connName, QStringLiteral( "GPKG" ) );
    QgsDataItem *conn = new QgsGeoPackageConnectionItem( this, connection.name(), connection.path() );

    connections.append( conn );
  }
  return connections;
}

void QgsGeoPackageRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsGeoPackageRootItem::newConnection()
{
  if ( QgsOgrDataCollectionItem::createConnection( QStringLiteral( "GeoPackage" ),  QStringLiteral( "GeoPackage Database (*.gpkg)" ),  QStringLiteral( "GPKG" ) ) )
  {
    refreshConnections();
  }
}

QgsGeoPackageCollectionItem::QgsGeoPackageCollectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
  , mPath( path )
{
  mToolTip = path;
  mCapabilities |= Collapse;
}


QVector<QgsDataItem *> QgsGeoPackageCollectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  const auto layers = QgsOgrLayerItem::subLayers( mPath, QStringLiteral( "GPKG" ) );
  for ( const QgsOgrDbLayerInfo *info : layers )
  {
    if ( info->layerType() == QgsLayerItem::LayerType::Raster )
    {
      children.append( new QgsGeoPackageRasterLayerItem( this, info->name(), info->path(), info->uri() ) );
    }
    else
    {
      children.append( new QgsGeoPackageVectorLayerItem( this, info->name(), info->path(), info->uri(), info->layerType( ) ) );
    }
  }
  qDeleteAll( layers );
  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "geopackage" );
  if ( storage )
  {
    const QStringList projectNames = storage->listProjects( mPath );
    for ( const QString &projectName : projectNames )
    {
      QgsGeoPackageProjectUri projectUri { true, mPath, projectName };
      children.append( new QgsProjectItem( this, projectName, QgsGeoPackageProjectStorage::encodeUri( projectUri ) ) );
    }
  }
  return children;
}

bool QgsGeoPackageCollectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsGeoPackageCollectionItem *o = qobject_cast<const QgsGeoPackageCollectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;

}

QWidget *QgsGeoPackageRootItem::paramWidget()
{
  return nullptr;
}

void QgsGeoPackageCollectionItem::addConnection()
{
  QgsOgrDbConnection connection( mName, QStringLiteral( "GPKG" ) );
  connection.setPath( mPath );
  connection.save();
  mParent->refreshConnections();
}

void QgsGeoPackageCollectionItem::deleteConnection()
{
  QgsOgrDbConnection::deleteConnection( name(), QStringLiteral( "GPKG" ) );
  mParent->refreshConnections();
}

bool QgsGeoPackageCollectionItem::vacuumGeoPackageDb( const QString &path, const QString &name, QString &errCause )
{
  QgsScopedProxyProgressTask task( tr( "Vacuuming %1" ).arg( name ) );

  bool result = false;
  // Better safe than sorry
  if ( ! path.isEmpty( ) )
  {
    char *errmsg = nullptr;
    sqlite3_database_unique_ptr database;
    int status = database.open_v2( path, SQLITE_OPEN_READWRITE, nullptr );
    if ( status != SQLITE_OK )
    {
      errCause = sqlite3_errmsg( database.get() );
    }
    else
    {
      ( void )sqlite3_exec(
        database.get(),                      /* An open database */
        "VACUUM",                            /* SQL to be evaluated */
        nullptr,                             /* Callback function */
        nullptr,                             /* 1st argument to callback */
        &errmsg                              /* Error msg written here */
      );
    }
    if ( status != SQLITE_OK || errmsg )
    {
      errCause = tr( "There was an error compacting (VACUUM) the database <b>%1</b>: %2" )
                 .arg( name,
                       QString::fromUtf8( errmsg ) );
    }
    else
    {
      result = true;
    }
    sqlite3_free( errmsg );
  }
  else
  {
    // This should never happen!
    errCause = tr( "Layer path is empty: layer cannot be deleted!" );
  }
  return result;
}

bool QgsGeoPackageCollectionItem::deleteGeoPackageRasterLayer( const QString &uri, QString &errCause )
{
  bool result = false;
  // Better safe than sorry
  if ( ! uri.isEmpty( ) )
  {
    QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri ) );
    QString baseUri = pieces[QStringLiteral( "path" )].toString();
    QString layerName = pieces[QStringLiteral( "layerName" )].toString();

    if ( baseUri.isEmpty() || layerName.isEmpty() )
    {
      errCause = QStringLiteral( "Layer URI is malformed: layer <b>%1</b> cannot be deleted!" ).arg( uri );
    }
    else
    {
      sqlite3_database_unique_ptr database;
      int status = database.open_v2( baseUri, SQLITE_OPEN_READWRITE, nullptr );
      if ( status != SQLITE_OK )
      {
        errCause = sqlite3_errmsg( database.get() );
      }
      else
      {
        // Remove table
        char *errmsg = nullptr;
        char *sql = sqlite3_mprintf(
                      "DROP table IF EXISTS \"%w\";"
                      "DELETE FROM gpkg_contents WHERE table_name = '%q';"
                      "DELETE FROM gpkg_tile_matrix WHERE table_name = '%q';"
                      "DELETE FROM gpkg_tile_matrix_set WHERE table_name = '%q';",
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData(),
                      layerName.toUtf8().constData() );
        status = sqlite3_exec(
                   database.get(),               /* An open database */
                   sql,                          /* SQL to be evaluated */
                   nullptr,                      /* Callback function */
                   nullptr,                      /* 1st argument to callback */
                   &errmsg                       /* Error msg written here */
                 );
        sqlite3_free( sql );
        // Remove from optional tables, may silently fail
        QStringList optionalTables;
        optionalTables << QStringLiteral( "gpkg_extensions" )
                       << QStringLiteral( "gpkg_metadata_reference" );
        for ( const QString &tableName : qgis::as_const( optionalTables ) )
        {
          char *sql = sqlite3_mprintf( "DELETE FROM %w WHERE table_name = '%q'",
                                       tableName.toUtf8().constData(),
                                       layerName.toUtf8().constData() );
          ( void )sqlite3_exec(
            database.get(),                      /* An open database */
            sql,                                 /* SQL to be evaluated */
            nullptr,                             /* Callback function */
            nullptr,                             /* 1st argument to callback */
            nullptr                              /* Error msg written here */
          );
          sqlite3_free( sql );
        }
        // Other tables, ignore errors
        {
          char *sql = sqlite3_mprintf( "DELETE FROM gpkg_2d_gridded_coverage_ancillary WHERE tile_matrix_set_name = '%q'",
                                       layerName.toUtf8().constData() );
          ( void )sqlite3_exec(
            database.get(),                      /* An open database */
            sql,                                 /* SQL to be evaluated */
            nullptr,                             /* Callback function */
            nullptr,                             /* 1st argument to callback */
            nullptr                              /* Error msg written here */
          );
          sqlite3_free( sql );
        }
        {
          char *sql = sqlite3_mprintf( "DELETE FROM gpkg_2d_gridded_tile_ancillary WHERE tpudt_name = '%q'",
                                       layerName.toUtf8().constData() );
          ( void )sqlite3_exec(
            database.get(),                      /* An open database */
            sql,                                 /* SQL to be evaluated */
            nullptr,                             /* Callback function */
            nullptr,                             /* 1st argument to callback */
            nullptr                              /* Error msg written here */
          );
          sqlite3_free( sql );
        }

        if ( status == SQLITE_OK )
        {
          result = true;
        }
        else
        {
          errCause = tr( "There was an error deleting the layer %1: %2" ).arg( layerName, QString::fromUtf8( errmsg ) );
        }
        sqlite3_free( errmsg );
      }
    }
  }
  else
  {
    // This should never happen!
    errCause = tr( "Layer URI is empty: layer cannot be deleted!" );
  }
  return result;
}

QgsGeoPackageConnectionItem::QgsGeoPackageConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsGeoPackageCollectionItem( parent, name, path )
{

}

bool QgsGeoPackageConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsGeoPackageConnectionItem *o = qobject_cast<const QgsGeoPackageConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;

}

QgsGeoPackageAbstractLayerItem::QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, QgsLayerItem::LayerType layerType, const QString &providerKey )
  : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
{
  mCapabilities |= Delete;
  mToolTip = uri;
  setState( Populated ); // no children are expected
}

bool QgsGeoPackageAbstractLayerItem::executeDeleteLayer( QString &errCause )
{
  errCause = QObject::tr( "The layer <b>%1</b> cannot be deleted because this feature is not yet implemented for this kind of layers." ).arg( mName );
  return false;
}

static int collect_strings( void *names, int, char **argv, char ** )
{
  *static_cast<QList<QString>*>( names ) << QString::fromUtf8( argv[ 0 ] );
  return 0;
}

QStringList QgsGeoPackageAbstractLayerItem::tableNames()
{
  QStringList names;
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( providerKey(), mUri ) );
  QString baseUri = pieces[QStringLiteral( "path" )].toString();
  if ( !baseUri.isEmpty() )
  {
    char *errmsg = nullptr;
    sqlite3_database_unique_ptr database;
    int status = database.open_v2( baseUri, SQLITE_OPEN_READONLY, nullptr );
    if ( status == SQLITE_OK )
    {
      char *sql = sqlite3_mprintf( "SELECT table_name FROM gpkg_contents;" );
      status = sqlite3_exec(
                 database.get(),              /* An open database */
                 sql,                         /* SQL to be evaluated */
                 collect_strings,             /* Callback function */
                 &names,                      /* 1st argument to callback */
                 &errmsg                      /* Error msg written here */
               );
      sqlite3_free( sql );
      if ( status != SQLITE_OK )
      {
        QgsDebugMsg( QStringLiteral( "There was an error reading tables from GPKG layer %1: %2" ).arg( mUri, QString::fromUtf8( errmsg ) ) );
      }
      sqlite3_free( errmsg );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "There was an error opening GPKG %1" ).arg( mUri ) );
    }
  }
  return  names;
}


QList<QgsMapLayer *> QgsGeoPackageAbstractLayerItem::layersInProject() const
{
  // Check if the layer(s) are in the registry
  QList<QgsMapLayer *> layersList;
  const auto mapLayers( QgsProject::instance()->mapLayers() );
  for ( QgsMapLayer *layer :  mapLayers )
  {
    if ( layer->publicSource() == mUri )
    {
      layersList << layer;
    }
  }
  return layersList;
}

QgsGeoPackageVectorLayerItem::QgsGeoPackageVectorLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{
  mCapabilities |= Rename;
}


QgsGeoPackageRasterLayerItem::QgsGeoPackageRasterLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, QgsLayerItem::LayerType::Raster, QStringLiteral( "gdal" ) )
{

}

bool QgsGeoPackageRasterLayerItem::executeDeleteLayer( QString &errCause )
{
  return QgsGeoPackageCollectionItem::deleteGeoPackageRasterLayer( mUri, errCause );
}

bool QgsGeoPackageVectorLayerItem::executeDeleteLayer( QString &errCause )
{
  return QgsOgrProviderUtils::deleteLayer( mUri, errCause );
}

///@endcond
