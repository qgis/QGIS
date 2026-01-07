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

#include "moc_qgsgeopackagedataitems.cpp"

///@cond PRIVATE

#include "qgsgeopackagedataitems.h"
#include "qgsprojectitem.h"
#include "qgsfieldsitem.h"
#include "qgsogrdbconnection.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsproxyprogresstask.h"
#include "qgsprojectstorageregistry.h"
#include "qgsgeopackageprojectstorage.h"
#include "qgsgeopackageproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsfielddomainsitem.h"
#include "qgsrelationshipsitem.h"
#include "qgsogrproviderutils.h"

QString QgsGeoPackageDataItemProvider::name()
{
  return u"GPKG"_s;
}

QString QgsGeoPackageDataItemProvider::dataProviderKey() const
{
  return u"ogr"_s;
}

Qgis::DataItemProviderCapabilities QgsGeoPackageDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::Databases;
}

QgsDataItem *QgsGeoPackageDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "path = " + path, 2 );
  if ( path.isEmpty() )
  {
    return new QgsGeoPackageRootItem( parentItem, u"GeoPackage"_s, u"gpkg:"_s );
  }
  return nullptr;
}

QgsGeoPackageRootItem::QgsGeoPackageRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, u"GPKG"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mGeoPackage.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsGeoPackageRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connList( QgsOgrDbConnection::connectionList( u"GPKG"_s ) );
  for ( const QString &connName : connList )
  {
    const QgsOgrDbConnection connection( connName, u"GPKG"_s );
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
  if ( QgsOgrProviderUtils::createConnection( u"GeoPackage"_s,  u"GeoPackage Database (*.gpkg)"_s,  u"GPKG"_s ) )
  {
    refreshConnections();
  }
}

QgsGeoPackageCollectionItem::QgsGeoPackageCollectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, u"GPKG"_s )
{
  mToolTip = QString( path ).remove( "gpkg:/"_L1 );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed;
}


QVector<QgsDataItem *> QgsGeoPackageCollectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QString path = mPath.remove( "gpkg:/"_L1 );
  const QList< QgsProviderSublayerDetails > sublayers = QgsProviderRegistry::instance()->querySublayers( path );
  for ( const QgsProviderSublayerDetails &sublayer : sublayers )
  {
    switch ( sublayer.type() )
    {
      case Qgis::LayerType::Vector:
      {
        Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::Vector;

        switch ( QgsWkbTypes::geometryType( sublayer.wkbType() ) )
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

          case Qgis::GeometryType::Null:
            layerType = Qgis::BrowserLayerType::TableLayer;
            break;

          case Qgis::GeometryType::Unknown:
            layerType = Qgis::BrowserLayerType::Vector;
            break;
        }

        children.append( new QgsGeoPackageVectorLayerItem( this, sublayer.name(), path, sublayer.uri(), layerType ) );
        break;
      }

      case Qgis::LayerType::Raster:
        children.append( new QgsGeoPackageRasterLayerItem( this, sublayer.name(), path, sublayer.uri() ) );
        break;

      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
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

  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
      auto domainsItem = std::make_unique< QgsFieldDomainsItem >( this, mPath + "/domains", path, u"ogr"_s );
      // force this item to appear last by setting a maximum string value for the sort key
      domainsItem->setSortKey( QString( QChar( 0x10FFFF ) ) );
      children.append( domainsItem.release() );
    }
  }
  if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RetrieveRelationships ) )
  {
    QString relationError;
    QList< QgsWeakRelation > relations;
    try
    {
      relations = conn->relationships();
    }
    catch ( QgsProviderConnectionException &ex )
    {
      relationError = ex.what();
    }

    if ( !relations.empty() || !relationError.isEmpty() )
    {
      auto relationsItem = std::make_unique< QgsRelationshipsItem >( this, mPath + "/relations", conn->uri(), u"ogr"_s );
      // force this item to appear last by setting a maximum string value for the sort key
      relationsItem->setSortKey( QString( QChar( 0x11FFFF ) ) );
      children.append( relationsItem.release() );
    }
  }

  if ( children.empty() )
  {
    // sniff database to see if it's just empty, or if something went wrong
    // note that we HAVE to use update here, or GDAL won't open an empty database
    gdal::dataset_unique_ptr hDS( GDALOpenEx( path.toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
    if ( !hDS )
    {
      QString errorMessage;
      if ( !QFile::exists( path ) )
      {
        errorMessage = tr( "The database does not contain any layers or there was an error opening the file.\nCheck file and directory permissions on\n%1" ).arg( QDir::toNativeSeparators( path ) );
      }
      else
      {
        errorMessage = tr( "Layer is not valid (%1)" ).arg( path );
      }
      children.append( new QgsErrorItem( this, errorMessage, mPath + "/error" ) );
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
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
  QgsOgrDbConnection connection( mName, u"GPKG"_s );
  connection.setPath( mPath );
  connection.save();
  mParent->refreshConnections( u"GPKG"_s );
}

void QgsGeoPackageCollectionItem::deleteConnection()
{
  QgsOgrDbConnection::deleteConnection( name() );
  mParent->refreshConnections( u"GPKG"_s );
}

bool QgsGeoPackageCollectionItem::vacuumGeoPackageDb( const QString &name, const QString &path, QString &errCause )
{
  const QgsScopedProxyProgressTask task( tr( "Vacuuming %1" ).arg( name ) );
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
  const auto mapLayers( QgsProject::instance()->mapLayers() ); // skip-keyword-check
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
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, layerType, u"ogr"_s )
{
  mCapabilities |= ( Qgis::BrowserItemCapability::Rename | Qgis::BrowserItemCapability::Fertile | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed );
  setState( Qgis::BrowserItemState::NotPopulated );
}


QVector<QgsDataItem *> QgsGeoPackageVectorLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, collection()->path() + u"/columns/ "_s, collection()->path(), providerKey(), QString(), name() ) );
  return children;
}


QgsGeoPackageRasterLayerItem::QgsGeoPackageRasterLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, Qgis::BrowserLayerType::Raster, u"gdal"_s )
{
}

bool QgsGeoPackageRasterLayerItem::executeDeleteLayer( QString &errCause )
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
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
  collectionUri.uri = path().replace( "gpkg:/"_L1, QString() );
  collectionUri.layerType = u"collection"_s;

  if ( capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {
    collectionUri.filePath = path();
  }

  return { collectionUri };
}

///@endcond
