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

#include "qgssqliteutils.h"
#include "qgsgeopackagedataitems.h"
#include "qgsprojectitem.h"
#include "qgsfieldsitem.h"
#include "qgsogrdbconnection.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsogrprovider.h"
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
#include "qgsprovidermetadata.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsfielddomainsitem.h"

QString QgsGeoPackageDataItemProvider::name()
{
  return QStringLiteral( "GPKG" );
}

QString QgsGeoPackageDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "ogr" );
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
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "GPKG" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mGeoPackage.svg" );
  populate();
}

QVector<QgsDataItem *> QgsGeoPackageRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connList( QgsOgrDbConnection::connectionList( QStringLiteral( "GPKG" ) ) );
  for ( const QString &connName : connList )
  {
    const QgsOgrDbConnection connection( connName, QStringLiteral( "GPKG" ) );
    QgsDataItem *conn = new QgsGeoPackageConnectionItem( this, connection.name(), mPath + '/' + connection.path() );

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
  if ( QgsOgrProviderUtils::createConnection( QStringLiteral( "GeoPackage" ),  QStringLiteral( "GeoPackage Database (*.gpkg)" ),  QStringLiteral( "GPKG" ) ) )
  {
    refreshConnections();
  }
}

QgsGeoPackageCollectionItem::QgsGeoPackageCollectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "GPKG" ) )
{
  mToolTip = QString( path ).remove( QLatin1String( "gpkg:/" ) );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed;
}


QVector<QgsDataItem *> QgsGeoPackageCollectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QString path = mPath.remove( QLatin1String( "gpkg:/" ) );
  const QList< QgsProviderSublayerDetails > sublayers = QgsProviderRegistry::instance()->querySublayers( path );
  for ( const QgsProviderSublayerDetails &sublayer : sublayers )
  {
    switch ( sublayer.type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::Vector;

        switch ( QgsWkbTypes::geometryType( sublayer.wkbType() ) )
        {
          case QgsWkbTypes::PointGeometry:
            layerType = Qgis::BrowserLayerType::Point;
            break;

          case QgsWkbTypes::LineGeometry:
            layerType = Qgis::BrowserLayerType::Line;
            break;

          case QgsWkbTypes::PolygonGeometry:
            layerType = Qgis::BrowserLayerType::Polygon;
            break;

          case QgsWkbTypes::NullGeometry:
            layerType = Qgis::BrowserLayerType::TableLayer;
            break;

          case QgsWkbTypes::UnknownGeometry:
            layerType = Qgis::BrowserLayerType::Vector;
            break;
        }

        children.append( new QgsGeoPackageVectorLayerItem( this, sublayer.name(), path, sublayer.uri(), layerType ) );
        break;
      }

      case QgsMapLayerType::RasterLayer:
        children.append( new QgsGeoPackageRasterLayerItem( this, sublayer.name(), path, sublayer.uri() ) );
        break;

      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "geopackage" );
  if ( storage )
  {
    const QStringList projectNames = storage->listProjects( mPath );
    for ( const QString &projectName : projectNames )
    {
      const QgsGeoPackageProjectUri projectUri { true, mPath, projectName };
      children.append( new QgsProjectItem( this, projectName, QgsGeoPackageProjectStorage::encodeUri( projectUri ) ) );
    }
  }

  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( path, QVariantMap() ) ) );
  if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::ListFieldDomains ) )
  {
    QString domainError;
    QStringList fieldDomains;
    try
    {
      fieldDomains = conn->fieldDomainNames();
    }
    catch ( QgsProviderConnectionException &ex )
    {
      domainError = ex.what();
    }

    if ( !fieldDomains.empty() || !domainError.isEmpty() )
    {
      std::unique_ptr< QgsFieldDomainsItem > domainsItem = std::make_unique< QgsFieldDomainsItem >( this, mPath + "/domains", path, QStringLiteral( "ogr" ) );
      // force this item to appear last by setting a maximum string value for the sort key
      domainsItem->setSortKey( QString( QChar( 0x10FFFF ) ) );
      children.append( domainsItem.release() );
    }
  }

  if ( children.empty() )
  {
    QString errorMessage;
    if ( QFile::exists( path ) )
    {
      errorMessage = tr( "The file does not contain any layer or there was an error opening the file.\nCheck file and directory permissions on\n%1" ).arg( QDir::toNativeSeparators( path ) );
    }
    else
    {
      errorMessage = tr( "Layer is not valid (%1)" ).arg( path );
    }
    children.append( new QgsErrorItem( this, errorMessage, mPath + "/error" ) );
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
  mParent->refreshConnections( QStringLiteral( "GPKG" ) );
}

void QgsGeoPackageCollectionItem::deleteConnection()
{
  QgsOgrDbConnection::deleteConnection( name() );
  mParent->refreshConnections( QStringLiteral( "GPKG" ) );
}

bool QgsGeoPackageCollectionItem::vacuumGeoPackageDb( const QString &name, const QString &path, QString &errCause )
{
  const QgsScopedProxyProgressTask task( tr( "Vacuuming %1" ).arg( name ) );
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

QgsGeoPackageAbstractLayerItem::QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType, const QString &providerKey )
  : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
  , mCollection( qobject_cast<QgsGeoPackageCollectionItem*>( parent ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete;
  mToolTip = uri;
  setState( Qgis::BrowserItemState::Populated ); // no children are expected
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

QgsGeoPackageVectorLayerItem::QgsGeoPackageVectorLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{
  mCapabilities |= ( Qgis::BrowserItemCapability::Rename | Qgis::BrowserItemCapability::Fertile | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed );
  setState( Qgis::BrowserItemState::NotPopulated );
}


QVector<QgsDataItem *> QgsGeoPackageVectorLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, collection()->path() + QStringLiteral( "/columns/ " ), collection()->path(), providerKey(), QString(), name() ) );
  return children;
}


QgsGeoPackageRasterLayerItem::QgsGeoPackageRasterLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, Qgis::BrowserLayerType::Raster, QStringLiteral( "gdal" ) )
{
}

bool QgsGeoPackageRasterLayerItem::executeDeleteLayer( QString &errCause )
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  std::unique_ptr<QgsGeoPackageProviderConnection> conn( static_cast<QgsGeoPackageProviderConnection *>( md->createConnection( collection()->path(), QVariantMap() ) ) );
  const QString tableName = name();
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
  const QString tableName = name();
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
               .arg( tableName, parent()->path() );
    return false;
  }
  return true;
}


bool QgsGeoPackageCollectionItem::layerCollection() const
{
  return true;
}

bool QgsGeoPackageCollectionItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::UriList QgsGeoPackageCollectionItem::mimeUris() const
{
  QgsMimeDataUtils::Uri collectionUri;
  collectionUri.uri = path().replace( QLatin1String( "gpkg:/" ), QString() );
  collectionUri.layerType = QStringLiteral( "collection" );

  if ( capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {
    collectionUri.filePath = path();
  }

  return { collectionUri };
}

///@endcond
