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
#include "qgsthreadingutils.h"
#include "qgsmbtiles.h"
#include "qgstiles.h"
#include "qgsvectortileloader.h"
#include "qgsziputils.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsproviderutils.h"
#include "qgsprovidersublayerdetails.h"

#include <QIcon>
#include <QFileInfo>

///@cond PRIVATE

QString QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY = QStringLiteral( "mbtilesvectortiles" );
QString QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION = QObject::tr( "MBTile Vector Tiles data provider" );

QgsMbTilesVectorTileDataProvider::QgsMbTilesVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  const QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  QgsMbTiles reader( sourcePath );
  if ( !reader.open() )
  {
    QgsDebugError( QStringLiteral( "failed to open MBTiles file: " ) + sourcePath );
    mIsValid = false;
    return;
  }

  const QString format = reader.metadataValue( QStringLiteral( "format" ) );
  if ( format != QLatin1String( "pbf" ) )
  {
    QgsDebugError( QStringLiteral( "Cannot open MBTiles for vector tiles. Format = " ) + format );
    mIsValid = false;
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "name: " ) + reader.metadataValue( QStringLiteral( "name" ) ), 2 );

  mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator();

  bool minZoomOk, maxZoomOk;
  const int minZoom = reader.metadataValue( QStringLiteral( "minzoom" ) ).toInt( &minZoomOk );
  const int maxZoom = reader.metadataValue( QStringLiteral( "maxzoom" ) ).toInt( &maxZoomOk );
  if ( minZoomOk )
    mMatrixSet.dropMatricesOutsideZoomRange( minZoom, 99 );
  if ( maxZoomOk )
    mMatrixSet.dropMatricesOutsideZoomRange( 0, maxZoom );
  QgsDebugMsgLevel( QStringLiteral( "zoom range: %1 - %2" ).arg( mMatrixSet.minimumZoom() ).arg( mMatrixSet.maximumZoom() ), 2 );

  QgsRectangle r = reader.extent();
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                             QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  try
  {
    mExtent = ct.transformBoundingBox( r );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Could not transform layer extent to layer CRS" ) );
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
  return dsUri.param( QStringLiteral( "url" ) );
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

  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
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

  QgsMbTiles mbReader( dsUri.param( QStringLiteral( "url" ) ) );
  mbReader.open();
  return QgsVectorTileRawData( id, loadFromMBTiles( mbReader, id, feedback ) );
}

QList<QgsVectorTileRawData> QgsMbTilesVectorTileDataProvider::readTiles( const QgsTileMatrixSet &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback, Qgis::RendererUsage ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsMbTiles mbReader( dsUri.param( QStringLiteral( "url" ) ) );
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
    QgsDebugError( QStringLiteral( "Failed to decompress tile " ) + id.toString() );
    return QByteArray();
  }

  QgsDebugMsgLevel( QStringLiteral( "Tile blob size %1 -> uncompressed size %2" ).arg( gzippedTileData.size() ).arg( data.size() ), 2 );
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

QgsMbTilesVectorTileDataProvider *QgsMbTilesVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsMbTilesVectorTileDataProvider( uri, options, flags );
}

QIcon QgsMbTilesVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
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
      return QObject::tr( "Mbtiles Vector Tiles" ) + QStringLiteral( " (*.mbtiles *.MBTILES)" );
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
    fileName = parts.value( QStringLiteral( "path" ) ).toString();
  }

  if ( fileName.isEmpty() )
    return {};

  if ( QFileInfo( fileName ).suffix().compare( QLatin1String( "mbtiles" ), Qt::CaseInsensitive ) == 0 )
  {
    QVariantMap parts;
    parts.insert( QStringLiteral( "path" ), fileName );

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
        if ( reader.metadataValue( "format" ) == QLatin1String( "pbf" ) )
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
  if ( fi.isFile() && fi.suffix().compare( QLatin1String( "mbtiles" ), Qt::CaseInsensitive ) == 0 )
  {
    return { Qgis::LayerType::VectorTile };
  }

  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "path" ) ).toString().endsWith( ".mbtiles", Qt::CaseSensitivity::CaseInsensitive ) )
    return { Qgis::LayerType::VectorTile };

  return {};
}

QVariantMap QgsMbTilesVectorTileDataProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  uriComponents.insert( QStringLiteral( "path" ), dsUri.param( QStringLiteral( "url" ) ) );

  return uriComponents;
}

QString QgsMbTilesVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" ) ).toString() );
  return dsUri.encodedUri();
}

QString QgsMbTilesVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = decodeUri( uri );

  const QString originalPath = parts.value( QStringLiteral( "path" ) ).toString();
  parts.insert( QStringLiteral( "path" ), context.pathResolver().writePath( originalPath ) );

  return encodeUri( parts );
}

QString QgsMbTilesVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = decodeUri( uri );

  const QString originalPath = parts.value( QStringLiteral( "path" ) ).toString();
  parts.insert( QStringLiteral( "path" ), context.pathResolver().readPath( originalPath ) );

  return encodeUri( parts );
}

QList<Qgis::LayerType> QgsMbTilesVectorTileDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}


///@endcond


