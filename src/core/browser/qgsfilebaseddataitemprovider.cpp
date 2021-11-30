/***************************************************************************
  qgsfilebaseddataitemprovider.cpp
  --------------------------------------
  Date                 : July 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilebaseddataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgszipitem.h"
#include "qgsogrproviderutils.h"
#include "qgsstyle.h"
#include "qgsgdalutils.h"
#include "qgsgeopackagedataitems.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsfieldsitem.h"
#include "qgsproviderutils.h"
#include "qgsmbtiles.h"
#include "qgsvectortiledataitems.h"
#include "qgsprovidermetadata.h"
#include <QUrlQuery>

//
// QgsProviderSublayerItem
//

QgsProviderSublayerItem::QgsProviderSublayerItem( QgsDataItem *parent, const QString &name,
    const QgsProviderSublayerDetails &details, const QString &filePath )
  : QgsLayerItem( parent, name, filePath.isEmpty() ? details.uri() : filePath, details.uri(), layerTypeFromSublayer( details ), details.providerKey() )
  , mDetails( details )
{
  mToolTip = details.uri();

  // no children, except for sqlite, which gets special handling because of the unusual situation with the spatialite provider
  setState( details.driverName() == QLatin1String( "SQLite" ) ? Qgis::BrowserItemState::NotPopulated : Qgis::BrowserItemState::Populated );
}

QVector<QgsDataItem *> QgsProviderSublayerItem::createChildren()
{
  QVector<QgsDataItem *> children;

  if ( mDetails.type() == QgsMapLayerType::VectorLayer )
  {
    // sqlite gets special handling because of the spatialite provider which supports the api required for a fields item.
    // TODO -- allow read only fields items to be created directly from vector layers, so that all vector layers can show field items.
    if ( mDetails.driverName() == QLatin1String( "SQLite" ) )
    {
      children.push_back( new QgsFieldsItem( this,
                                             path() + QStringLiteral( "/columns/ " ),
                                             QStringLiteral( R"(dbname="%1")" ).arg( parent()->path().replace( '"', QLatin1String( R"(\")" ) ) ),
                                             QStringLiteral( "spatialite" ), QString(), name() ) );
    }
  }
  return children;
}

Qgis::BrowserLayerType QgsProviderSublayerItem::layerTypeFromSublayer( const QgsProviderSublayerDetails &sublayer )
{
  switch ( sublayer.type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      switch ( QgsWkbTypes::geometryType( sublayer.wkbType() ) )
      {
        case QgsWkbTypes::PointGeometry:
          return Qgis::BrowserLayerType::Point;

        case QgsWkbTypes::LineGeometry:
          return Qgis::BrowserLayerType::Line;

        case QgsWkbTypes::PolygonGeometry:
          return Qgis::BrowserLayerType::Polygon;

        case QgsWkbTypes::NullGeometry:
          return Qgis::BrowserLayerType::TableLayer;

        case QgsWkbTypes::UnknownGeometry:
          return Qgis::BrowserLayerType::Vector;
      }

      break;
    }
    case QgsMapLayerType::RasterLayer:
      return Qgis::BrowserLayerType::Raster;

    case QgsMapLayerType::PluginLayer:
      return Qgis::BrowserLayerType::Plugin;

    case QgsMapLayerType::MeshLayer:
      return Qgis::BrowserLayerType::Mesh;

    case QgsMapLayerType::VectorTileLayer:
      return Qgis::BrowserLayerType::VectorTile;

    case QgsMapLayerType::PointCloudLayer:
      return Qgis::BrowserLayerType::PointCloud;

    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }
  return Qgis::BrowserLayerType::NoType;
}

QString QgsProviderSublayerItem::layerName() const
{
  return mDetails.name();
}

//
// QgsFileDataCollectionItem
//

QgsFileDataCollectionItem::QgsFileDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QList<QgsProviderSublayerDetails> &sublayers )
  : QgsDataCollectionItem( parent, name, path )
  , mSublayers( sublayers )
{
  if ( QgsProviderUtils::sublayerDetailsAreIncomplete( mSublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount ) )
    setCapabilities( Qgis::BrowserItemCapability::Fertile );
  else
    setCapabilities( Qgis::BrowserItemCapability::Fast | Qgis::BrowserItemCapability::Fertile );

  if ( !qgsVsiPrefix( path ).isEmpty() )
  {
    mIconName = QStringLiteral( "/mIconZip.svg" );
  }
}

QVector<QgsDataItem *> QgsFileDataCollectionItem::createChildren()
{
  QList< QgsProviderSublayerDetails> sublayers;
  if ( QgsProviderUtils::sublayerDetailsAreIncomplete( mSublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount )
       || mSublayers.empty() )
  {
    sublayers = QgsProviderRegistry::instance()->querySublayers( path(), Qgis::SublayerQueryFlag::ResolveGeometryType );
  }
  else
  {
    sublayers = mSublayers;
  }
  // only ever use the initial sublayers for first population -- after that we requery when asked to create children,
  // or the item won't "refresh" and update its sublayers when the actual file changes
  mSublayers.clear();
  // remove the fast flag -- after the first population we need to requery the dataset
  setCapabilities( capabilities2() & ~static_cast< int >( Qgis::BrowserItemCapability::Fast ) );

  QVector<QgsDataItem *> children;
  children.reserve( sublayers.size() );
  for ( const QgsProviderSublayerDetails &sublayer : std::as_const( sublayers ) )
  {
    QgsProviderSublayerItem *item = new QgsProviderSublayerItem( this, sublayer.name(), sublayer, QString() );
    children.append( item );
  }

  return children;
}

bool QgsFileDataCollectionItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::UriList QgsFileDataCollectionItem::mimeUris() const
{
  QgsMimeDataUtils::Uri collectionUri;
  collectionUri.uri = path();
  collectionUri.layerType = QStringLiteral( "collection" );
  collectionUri.filePath = path();
  return { collectionUri };
}

QgsAbstractDatabaseProviderConnection *QgsFileDataCollectionItem::databaseConnection() const
{
  // sqlite gets special handling because of the spatialite provider which supports the api required database connections
  const QFileInfo fi( mPath );
  if ( fi.suffix().toLower() != QLatin1String( "sqlite" )  && fi.suffix().toLower() != QLatin1String( "db" ) )
  {
    return nullptr;
  }

  QgsAbstractDatabaseProviderConnection *conn = nullptr;

  // test that file is valid with OGR
  if ( OGRGetDriverCount() == 0 )
  {
    OGRRegisterAll();
  }
  // do not print errors, but write to debug
  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  gdal::dataset_unique_ptr hDS( GDALOpenEx( path().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_READONLY, nullptr, nullptr, nullptr ) );
  CPLPopErrorHandler();

  if ( ! hDS )
  {
    QgsDebugMsgLevel( QStringLiteral( "GDALOpen error # %1 : %2 on %3" ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ).arg( path() ), 2 );
    return nullptr;
  }

  GDALDriverH hDriver = GDALGetDatasetDriver( hDS.get() );
  QString driverName = GDALGetDriverShortName( hDriver );

  if ( driverName == QLatin1String( "SQLite" ) )
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) ) };
    if ( md )
    {
      QgsDataSourceUri uri;
      uri.setDatabase( path( ) );
      conn = static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri.uri(), {} ) );
    }
  }
  return conn;
}

//
// QgsFileBasedDataItemProvider
//

QString QgsFileBasedDataItemProvider::name()
{
  return QStringLiteral( "files" );
}

int QgsFileBasedDataItemProvider::capabilities() const
{
  return QgsDataProvider::File | QgsDataProvider::Dir;
}

QgsDataItem *QgsFileBasedDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return nullptr;

  const QFileInfo info( path );
  QString suffix = info.suffix().toLower();
  const QString name = info.fileName();

  // special handling for some suffixes
  if ( suffix.compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0 )
  {
    // Geopackage is special -- it gets a dedicated collection item type
    QgsGeoPackageCollectionItem *item = new QgsGeoPackageCollectionItem( parentItem, name, path );
    item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
    return item;
  }
  else if ( suffix == QLatin1String( "txt" ) )
  {
    // never ever show .txt files as datasets in browser -- they are only used for geospatial data in extremely rare cases
    // and are predominantly just noise in the browser
    return nullptr;
  }
  // If a .tab exists, then the corresponding .map/.dat is very likely a
  // side-car file of the .tab
  else if ( suffix == QLatin1String( "map" ) || suffix == QLatin1String( "dat" ) )
  {
    if ( QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".tab" ) ) || QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".TAB" ) ) )
      return nullptr;
  }
  // .dbf and .shx should only appear if .shp is not present
  else if ( suffix == QLatin1String( "dbf" ) || suffix == QLatin1String( "shx" ) )
  {
    if ( QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".shp" ) ) || QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".SHP" ) ) )
      return nullptr;
  }
  // skip QGIS style xml files
  else if ( suffix == QLatin1String( "xml" ) && QgsStyle::isXmlStyleFile( path ) )
  {
    return nullptr;
  }
  // GDAL 3.1 Shapefile driver directly handles .shp.zip files
  else if ( path.endsWith( QLatin1String( ".shp.zip" ), Qt::CaseInsensitive ) &&
            GDALIdentifyDriverEx( path.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr ) )
  {
    suffix = QStringLiteral( "shp.zip" );
  }
  // special handling for mbtiles files
  else if ( suffix == QLatin1String( "mbtiles" ) )
  {
    QgsMbTiles reader( path );
    if ( reader.open() )
    {
      if ( reader.metadataValue( QStringLiteral( "format" ) ) == QLatin1String( "pbf" ) )
      {
        // these are vector tiles
        QUrlQuery uq;
        uq.addQueryItem( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
        uq.addQueryItem( QStringLiteral( "url" ), path );
        const QString encodedUri = uq.toString();
        QgsVectorTileLayerItem *item = new QgsVectorTileLayerItem( parentItem, name, path, encodedUri );
        item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
        return item;
      }
      else
      {
        // handled by WMS provider
        QUrlQuery uq;
        uq.addQueryItem( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
        uq.addQueryItem( QStringLiteral( "url" ), QUrl::fromLocalFile( path ).toString() );
        const QString encodedUri = uq.toString();
        QgsLayerItem *item = new QgsLayerItem( parentItem, name, path, encodedUri, Qgis::BrowserLayerType::Raster, QStringLiteral( "wms" ) );
        item->setState( Qgis::BrowserItemState::Populated );
        item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
        return item;
      }
    }
  }

  // hide blocklisted URIs, such as .aux.xml files
  if ( QgsProviderRegistry::instance()->uriIsBlocklisted( path ) )
    return nullptr;

  QgsSettings settings;

  Qgis::SublayerQueryFlags queryFlags = Qgis::SublayerQueryFlags();

  // should we fast scan only?
  if ( ( settings.value( QStringLiteral( "qgis/scanItemsInBrowser2" ),
                         "extension" ).toString() == QLatin1String( "extension" ) ) ||
       ( parentItem && settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                                       QStringList() ).toStringList().contains( parentItem->path() ) ) )
  {
    queryFlags |= Qgis::SublayerQueryFlag::FastScan;
  }

  const QList<QgsProviderSublayerDetails> sublayers = QgsProviderRegistry::instance()->querySublayers( path, queryFlags );

  if ( sublayers.size() == 1
       && ( ( ( queryFlags & Qgis::SublayerQueryFlag::FastScan ) && !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount | QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownGeometryType ) )
            || ( !( queryFlags & Qgis::SublayerQueryFlag::FastScan ) && !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount ) ) )
     )
  {
    QgsProviderSublayerItem *item = new QgsProviderSublayerItem( parentItem, name, sublayers.at( 0 ), path );
    item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
    return item;
  }
  else if ( !sublayers.empty() )
  {
    QgsFileDataCollectionItem *item = new QgsFileDataCollectionItem( parentItem, name, path, sublayers );
    item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
    return item;
  }
  else
  {
    return nullptr;
  }
}

bool QgsFileBasedDataItemProvider::handlesDirectoryPath( const QString &path )
{
  QFileInfo info( path );
  QString suffix = info.suffix().toLower();

  QStringList dirExtensions = QgsOgrProviderUtils::directoryExtensions();
  return dirExtensions.contains( suffix );
}
