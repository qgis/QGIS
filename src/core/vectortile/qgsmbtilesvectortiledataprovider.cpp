/***************************************************************************
  qgsmbtilesvectortiledataprovider.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmbtilesvectortiledataprovider.h"

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmbtiles.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsthreadingutils.h"
#include "qgstiles.h"
#include "qgsvectortileloader.h"
#include "qgsziputils.h"

#include <QFileInfo>
#include <QIcon>

#include "moc_qgsmbtilesvectortiledataprovider.cpp"

///@cond PRIVATE

QString QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY = u"mbtilesvectortiles"_s;
QString QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION = QObject::tr( "MBTile Vector Tiles data provider" );

QgsMbTilesVectorTileDataProvider::QgsMbTilesVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  const QString sourcePath = dsUri.param( u"url"_s );

  QgsMbTiles reader( sourcePath );
  if ( !reader.open() )
  {
    QgsDebugError( u"failed to open MBTiles file: "_s + sourcePath );
    mIsValid = false;
    return;
  }

  const QString format = reader.metadataValue( u"format"_s );
  if ( format != "pbf"_L1 )
  {
    QgsDebugError( u"Cannot open MBTiles for vector tiles. Format = "_s + format );
    mIsValid = false;
    return;
  }

  QgsDebugMsgLevel( u"name: "_s + reader.metadataValue( u"name"_s ), 2 );

  bool minZoomOk, maxZoomOk;
  const int minZoom = reader.metadataValue( u"minzoom"_s ).toInt( &minZoomOk );
  const int maxZoom = reader.metadataValue( u"maxzoom"_s ).toInt( &maxZoomOk );
  if ( minZoomOk && maxZoomOk )
  {
    mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator( minZoom, maxZoom );
  }
  else if ( minZoomOk )
  {
    mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator( minZoom, 99 );
  }
  else if ( maxZoomOk )
  {
    mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator( 0, maxZoom );
  }
  else
  {
    mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator();
  }

  QgsDebugMsgLevel( u"zoom range: %1 - %2"_s.arg( mMatrixSet.minimumZoom() ).arg( mMatrixSet.maximumZoom() ), 2 );

  QgsRectangle r = reader.extent();
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ),
                             QgsCoordinateReferenceSystem( u"EPSG:3857"_s ), transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  try
  {
    mExtent = ct.transformBoundingBox( r );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( u"Could not transform layer extent to layer CRS"_s );
  }

  mIsValid = true;
}

QgsMbTilesVectorTileDataProvider::QgsMbTilesVectorTileDataProvider( const QgsMbTilesVectorTileDataProvider &other )
  : QgsVectorTileDataProvider( other )
{
  mIsValid = other.mIsValid;
  mExtent = other.mExtent;
  mMatrixSet = other.mMatrixSet;
}

Qgis::DataProviderFlags QgsMbTilesVectorTileDataProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

QString QgsMbTilesVectorTileDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY;
}

QString QgsMbTilesVectorTileDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION;
}

QgsVectorTileDataProvider *QgsMbTilesVectorTileDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return new QgsMbTilesVectorTileDataProvider( *this );
}

QString QgsMbTilesVectorTileDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  return dsUri.param( u"url"_s );
}

bool QgsMbTilesVectorTileDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QgsRectangle QgsMbTilesVectorTileDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

QgsCoordinateReferenceSystem QgsMbTilesVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem( u"EPSG:3857"_s );
}

const QgsVectorTileMatrixSet &QgsMbTilesVectorTileDataProvider::tileMatrixSet() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMatrixSet;
}

QgsVectorTileRawData QgsMbTilesVectorTileDataProvider::readTile( const QgsTileMatrixSet &, const QgsTileXYZ &id, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsMbTiles mbReader( dsUri.param( u"url"_s ) );
  mbReader.open();
  return QgsVectorTileRawData( id, loadFromMBTiles( mbReader, id, feedback ) );
}

QList<QgsVectorTileRawData> QgsMbTilesVectorTileDataProvider::readTiles( const QgsTileMatrixSet &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback, Qgis::RendererUsage ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsMbTiles mbReader( dsUri.param( u"url"_s ) );
  mbReader.open();

  QList<QgsVectorTileRawData> rawTiles;
  rawTiles.reserve( tiles.size() );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QByteArray rawData = loadFromMBTiles( mbReader, id, feedback );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QByteArray QgsMbTilesVectorTileDataProvider::loadFromMBTiles( QgsMbTiles &mbTileReader, const QgsTileXYZ &id, QgsFeedback *feedback )
{
  // MBTiles uses TMS specs with Y starting at the bottom while XYZ uses Y starting at the top
  const int rowTMS = static_cast<int>( pow( 2, id.zoomLevel() ) - id.row() - 1 );
  QByteArray gzippedTileData = mbTileReader.tileData( id.zoomLevel(), id.column(), rowTMS );
  if ( gzippedTileData.isEmpty() )
  {
    return QByteArray();
  }

  if ( feedback && feedback->isCanceled() )
    return QByteArray();

  QByteArray data;
  if ( !QgsZipUtils::decodeGzip( gzippedTileData, data ) )
  {
    QgsDebugError( u"Failed to decompress tile "_s + id.toString() );
    return QByteArray();
  }

  QgsDebugMsgLevel( u"Tile blob size %1 -> uncompressed size %2"_s.arg( gzippedTileData.size() ).arg( data.size() ), 2 );
  return data;
}


//
// QgsMbTilesVectorTileDataProviderMetadata
//

QgsMbTilesVectorTileDataProviderMetadata::QgsMbTilesVectorTileDataProviderMetadata()
  : QgsProviderMetadata( QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY,
                         QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION )
{
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsMbTilesVectorTileDataProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

QgsMbTilesVectorTileDataProvider *QgsMbTilesVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsMbTilesVectorTileDataProvider( uri, options, flags );
}

QIcon QgsMbTilesVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconVectorTileLayer.svg"_s );
}

QgsProviderMetadata::ProviderCapabilities QgsMbTilesVectorTileDataProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QString QgsMbTilesVectorTileDataProviderMetadata::filters( Qgis::FileFilterType type )
{
  switch ( type )
  {
    case Qgis::FileFilterType::Vector:
    case Qgis::FileFilterType::Raster:
    case Qgis::FileFilterType::Mesh:
    case Qgis::FileFilterType::MeshDataset:
    case Qgis::FileFilterType::PointCloud:
    case Qgis::FileFilterType::TiledScene:
      return QString();

    case Qgis::FileFilterType::VectorTile:
      return QObject::tr( "Mbtiles Vector Tiles" ) + u" (*.mbtiles *.MBTILES)"_s;
  }
  return QString();
}

QList<QgsProviderSublayerDetails> QgsMbTilesVectorTileDataProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags, QgsFeedback * ) const
{
  QString fileName;
  const QFileInfo fi( uri );
  if ( fi.isFile() )
  {
    fileName = uri;
  }
  else
  {
    const QVariantMap parts = decodeUri( uri );
    fileName = parts.value( u"path"_s ).toString();
  }

  if ( fileName.isEmpty() )
    return {};

  if ( QFileInfo( fileName ).suffix().compare( "mbtiles"_L1, Qt::CaseInsensitive ) == 0 )
  {
    QVariantMap parts;
    parts.insert( u"path"_s, fileName );

    if ( flags & Qgis::SublayerQueryFlag::FastScan )
    {
      // fast scan -- assume vector tile are available
      QgsProviderSublayerDetails details;
      details.setUri( encodeUri( parts ) );
      details.setProviderKey( key() );
      details.setType( Qgis::LayerType::VectorTile );
      details.setSkippedContainerScan( true );
      details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( fileName ) );
      return {details};
    }
    else
    {
      // slower scan, check actual mbtiles format
      QgsMbTiles reader( fileName );
      if ( reader.open() )
      {
        if ( reader.metadataValue( "format" ) == "pbf"_L1 )
        {
          QgsProviderSublayerDetails details;
          details.setUri( encodeUri( parts ) );
          details.setProviderKey( key() );
          details.setType( Qgis::LayerType::VectorTile );
          details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( fileName ) );
          return {details};
        }
      }
    }
  }
  return {};
}

int QgsMbTilesVectorTileDataProviderMetadata::priorityForUri( const QString &uri ) const
{
  if ( validLayerTypesForUri( uri ).contains( Qgis::LayerType::VectorTile ) )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsMbTilesVectorTileDataProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QFileInfo fi( uri );
  if ( fi.isFile() && fi.suffix().compare( "mbtiles"_L1, Qt::CaseInsensitive ) == 0 )
  {
    return { Qgis::LayerType::VectorTile };
  }

  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( u"path"_s ).toString().endsWith( ".mbtiles", Qt::CaseSensitivity::CaseInsensitive ) )
    return { Qgis::LayerType::VectorTile };

  return {};
}

QVariantMap QgsMbTilesVectorTileDataProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( u"type"_s, u"mbtiles"_s );
  uriComponents.insert( u"path"_s, dsUri.param( u"url"_s ) );

  return uriComponents;
}

QString QgsMbTilesVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"type"_s, u"mbtiles"_s );
  dsUri.setParam( u"url"_s, parts.value( parts.contains( u"path"_s ) ? u"path"_s : u"url"_s ).toString() );
  return dsUri.encodedUri();
}

QString QgsMbTilesVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = decodeUri( uri );

  const QString originalPath = parts.value( u"path"_s ).toString();
  parts.insert( u"path"_s, context.pathResolver().writePath( originalPath ) );

  return encodeUri( parts );
}

QString QgsMbTilesVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = decodeUri( uri );

  const QString originalPath = parts.value( u"path"_s ).toString();
  parts.insert( u"path"_s, context.pathResolver().readPath( originalPath ) );

  return encodeUri( parts );
}

QList<Qgis::LayerType> QgsMbTilesVectorTileDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}


///@endcond


