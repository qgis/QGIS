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
#include "qgsgeopackageproviderconnection.h"

QString QgsGeoPackageDataItemProvider::name()
{
  return QStringLiteral( "GPKG" );
}

int QgsGeoPackageDataItemProvider::capabilities() const
{
  return QgsDataProvider::Database;
}

QgsDataItem *QgsGeoPackageDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "path = " + path, 2 );
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

bool QgsGeoPackageCollectionItem::deleteRasterLayer( const QString &layerName, QString &errCause )
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( path(), QVariantMap() ) ) );
  if ( conn )
  {
    try
    {
      conn->dropRasterTable( QString(), layerName );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      errCause = ex.what();
      return false;
    }
  }
  else
  {
    errCause = QObject::tr( "There was an error retrieving the connection %1!" ).arg( path() );
    return false;
  }
  return true;
}

bool QgsGeoPackageCollectionItem::deleteVectorLayer( const QString &layerName, QString &errCause )
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( path(), QVariantMap() ) ) );
  if ( conn )
  {
    try
    {
      conn->dropVectorTable( QString(), layerName );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      errCause = ex.what();
      return false;
    }
  }
  else
  {
    errCause = QObject::tr( "There was an error retrieving the connection %1!" ).arg( path() );
    return false;
  }
  return true;
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

bool QgsGeoPackageCollectionItem::vacuumGeoPackageDb( const QString &name, const QString &path, QString &errCause )
{
  QgsScopedProxyProgressTask task( tr( "Vacuuming %1" ).arg( name ) );
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( path, QVariantMap() ) ) );
  if ( conn )
  {
    try
    {
      conn->vacuum( QString(), QString() );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      errCause = ex.what();
      return false;
    }
  }
  else
  {
    errCause = QObject::tr( "There was an error retrieving the connection %1!" ).arg( name );
    return false;
  }
  return true;
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
  , mCollection( qobject_cast<QgsGeoPackageCollectionItem*>( parent ) )
{
  mCapabilities |= Delete;
  mToolTip = uri;
  setState( Populated ); // no children are expected
}


QStringList QgsGeoPackageAbstractLayerItem::tableNames() const
{
  QStringList names;
  // note: not using providerKey() because GPKG methods are implemented in OGR
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  QgsGeoPackageProviderConnection *conn { static_cast<QgsGeoPackageProviderConnection *>( md->findConnection( parent()->name() ) ) };
  if ( conn )
  {
    for ( const QgsGeoPackageProviderConnection::TableProperty &p : conn->tables( ) )
    {
      names.push_back( p.tableName() );
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

QgsGeoPackageCollectionItem *QgsGeoPackageAbstractLayerItem::collection() const
{
  return mCollection;
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
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( collection()->path(), QVariantMap() ) ) );
  QString tableName = name();
  if ( conn->tableExists( QString(), tableName ) )
  {
    try
    {
      conn->dropRasterTable( QString(), tableName );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      errCause = ex.what();
      return false;
    }
  }
  else
  {
    errCause = QObject::tr( "There was an error deleting '%1' on '%2'!" )
               .arg( tableName )
               .arg( collection()->path() );
    return false;
  }
  return true;
}

bool QgsGeoPackageVectorLayerItem::executeDeleteLayer( QString &errCause )
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( collection()->path(), QVariantMap() ) ) );
  QString tableName = name();
  if ( conn )
  {
    try
    {
      conn->dropVectorTable( QString(), tableName );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      errCause = ex.what();
      return false;
    }
  }
  else
  {
    errCause = QObject::tr( "There was an error deleting '%1' on '%2'!" )
               .arg( tableName )
               .arg( parent()->path() );
    return false;
  }
  return true;
}

///@endcond
