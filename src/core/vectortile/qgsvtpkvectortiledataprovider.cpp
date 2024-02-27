/***************************************************************************
  qgsvtpkvectortiledataprovider.cpp
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

#include "qgsvtpkvectortiledataprovider.h"
#include "qgsthreadingutils.h"
#include "qgsvtpktiles.h"
#include "qgsvectortileloader.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"

#include <QIcon>
#include <QFileInfo>

///@cond PRIVATE


QString QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY = QStringLiteral( "vtpkvectortiles" );
QString QgsVtpkVectorTileDataProvider::DATA_PROVIDER_DESCRIPTION = QObject::tr( "VTPK Vector Tiles data provider" );


QgsVtpkVectorTileDataProvider::QgsVtpkVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  const QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  QgsVtpkTiles reader( sourcePath );
  if ( !reader.open() )
  {
    QgsDebugError( QStringLiteral( "failed to open VTPK file: " ) + sourcePath );
    mIsValid = false;
    return;
  }

  const QVariantMap metadata = reader.metadata();
  const QString format = metadata.value( QStringLiteral( "tileInfo" ) ).toMap().value( QStringLiteral( "format" ) ).toString();
  if ( format != QLatin1String( "pbf" ) )
  {
    QgsDebugError( QStringLiteral( "Cannot open VTPK for vector tiles. Format = " ) + format );
    mIsValid = false;
    return;
  }

  mMatrixSet = reader.matrixSet();
  mCrs = mMatrixSet.crs();
  mExtent = reader.extent( transformContext() );
  mLayerMetadata = reader.layerMetadata();
  mStyleDefinition = reader.styleDefinition();
  mSpriteDefinition = reader.spriteDefinition();
  if ( !mSpriteDefinition.isEmpty() )
  {
    mSpriteImage = reader.spriteImage();
  }

  mIsValid = true;
}

QgsVtpkVectorTileDataProvider::QgsVtpkVectorTileDataProvider( const QgsVtpkVectorTileDataProvider &other )
  : QgsVectorTileDataProvider( other )
{
  mIsValid = other.mIsValid;
  mCrs = other.mCrs;;
  mExtent = other.mExtent;
  mMatrixSet = other.mMatrixSet;
  mLayerMetadata = other.mLayerMetadata;
  mStyleDefinition = other.mStyleDefinition;
  mSpriteDefinition = other.mSpriteDefinition;
  mSpriteImage = other.mSpriteImage;
}

Qgis::DataProviderFlags QgsVtpkVectorTileDataProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

Qgis::VectorTileProviderFlags QgsVtpkVectorTileDataProvider::providerFlags() const
{
  return Qgis::VectorTileProviderFlag::AlwaysUseTileMatrixSetFromProvider;
}

Qgis::VectorTileProviderCapabilities QgsVtpkVectorTileDataProvider::providerCapabilities() const
{
  return Qgis::VectorTileProviderCapability::ReadLayerMetadata;
}

QString QgsVtpkVectorTileDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return DATA_PROVIDER_KEY;
}

QString QgsVtpkVectorTileDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return DATA_PROVIDER_DESCRIPTION;
}

QgsVectorTileDataProvider *QgsVtpkVectorTileDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return new QgsVtpkVectorTileDataProvider( *this );
}

QString QgsVtpkVectorTileDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  return dsUri.param( QStringLiteral( "url" ) );
}

bool QgsVtpkVectorTileDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QgsCoordinateReferenceSystem QgsVtpkVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

QgsRectangle QgsVtpkVectorTileDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

QgsLayerMetadata QgsVtpkVectorTileDataProvider::layerMetadata() const
{
  return mLayerMetadata;
}

const QgsVectorTileMatrixSet &QgsVtpkVectorTileDataProvider::tileMatrixSet() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMatrixSet;
}

QVariantMap QgsVtpkVectorTileDataProvider::styleDefinition() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mStyleDefinition;
}

QVariantMap QgsVtpkVectorTileDataProvider::spriteDefinition() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSpriteDefinition;
}

QImage QgsVtpkVectorTileDataProvider::spriteImage() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSpriteImage;
}

QgsVectorTileRawData QgsVtpkVectorTileDataProvider::readTile( const QgsTileMatrixSet &, const QgsTileXYZ &id, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsVectorTileRawData data;
  if ( mShared->getCachedTileData( data, id ) )
    return data;

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  QgsVtpkTiles reader( dsUri.param( QStringLiteral( "url" ) ) );
  reader.open();
  const QgsVectorTileRawData rawData = loadFromVtpk( reader, id, feedback );
  mShared->storeCachedTileData( rawData );
  return rawData;
}

QList<QgsVectorTileRawData> QgsVtpkVectorTileDataProvider::readTiles( const QgsTileMatrixSet &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback, Qgis::RendererUsage ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  // defer actual creation of reader until we need it -- maybe everything is already present in the cache!
  std::unique_ptr< QgsVtpkTiles > reader;

  QList<QgsVectorTileRawData> rawTiles;
  QSet< QgsTileXYZ > fetchedTiles;
  rawTiles.reserve( tiles.size() );
  fetchedTiles.reserve( tiles.size() );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    if ( fetchedTiles.contains( id ) )
      continue;

    QgsVectorTileRawData data;
    if ( mShared->getCachedTileData( data, id ) )
    {
      rawTiles.append( data );
      fetchedTiles.insert( data.id );
    }
    else
    {
      if ( !reader )
      {
        reader = std::make_unique< QgsVtpkTiles >( dsUri.param( QStringLiteral( "url" ) ) );
        reader->open();
      }
      const QgsVectorTileRawData rawData = loadFromVtpk( *reader, id, feedback );
      if ( !rawData.data.isEmpty() && !fetchedTiles.contains( rawData.id ) )
      {
        rawTiles.append( rawData );
        fetchedTiles.insert( rawData.id );
        mShared->storeCachedTileData( rawData );
      }
    }
  }
  return rawTiles;
}

QString QgsVtpkVectorTileDataProvider::htmlMetadata() const
{
  QString metadata;

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  QgsVtpkTiles reader( dsUri.param( QStringLiteral( "url" ) ) );
  reader.open();

  if ( !reader.rootTileMap().isEmpty() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "VTPK storage" ) % QStringLiteral( "</td><td>" ) % tr( "Indexed VTPK (tilemap is present)" ) % QStringLiteral( "</td></tr>\n" );
  else
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "VTPK storage" ) % QStringLiteral( "</td><td>" ) % tr( "Flat VTPK (no tilemap)" ) % QStringLiteral( "</td></tr>\n" );

  if ( reader.metadata().contains( QStringLiteral( "minLOD" ) ) )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tile detail levels" ) % QStringLiteral( "</td><td>" ) % QStringLiteral( "%1 - %2" ).arg( reader.metadata().value( QStringLiteral( "minLOD" ) ).toInt() ).arg( reader.metadata().value( QStringLiteral( "maxLOD" ) ).toInt() ) % QStringLiteral( "</td></tr>\n" );
  }

  return metadata;
}

QgsVectorTileRawData QgsVtpkVectorTileDataProvider::loadFromVtpk( QgsVtpkTiles &vtpkTileReader, const QgsTileXYZ &id, QgsFeedback * )
{
  QgsTileXYZ requestedTile = id;
  QByteArray tileData = vtpkTileReader.tileData( requestedTile.zoomLevel(), requestedTile.column(), requestedTile.row() );
  // I **think** here ESRI software will detect a zero size tile and automatically fallback to lower zoom level tiles
  // I.e. they treat EVERY vtpk a bit like an indexed VTPK, but without the up-front tilemap information.
  // See https://github.com/qgis/QGIS/issues/52872
  while ( !tileData.isNull() && tileData.size() == 0 && requestedTile.zoomLevel() > vtpkTileReader.matrixSet().minimumZoom() )
  {
    requestedTile = QgsTileXYZ( requestedTile.column() / 2, requestedTile.row() / 2, requestedTile.zoomLevel() - 1 );
    tileData = vtpkTileReader.tileData( requestedTile.zoomLevel(), requestedTile.column(), requestedTile.row() );
  }

  if ( tileData.isNull() )
    return QgsVectorTileRawData();

  QgsVectorTileRawData res( id, tileData );
  res.tileGeometryId = requestedTile;
  return res;
}


//
// QgsVtpkVectorTileDataProviderMetadata
//

QgsVtpkVectorTileDataProviderMetadata::QgsVtpkVectorTileDataProviderMetadata()
  : QgsProviderMetadata( QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY, QgsVtpkVectorTileDataProvider::DATA_PROVIDER_DESCRIPTION )
{
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsVtpkVectorTileDataProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

QgsVtpkVectorTileDataProvider *QgsVtpkVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsVtpkVectorTileDataProvider( uri, options, flags );
}

QIcon QgsVtpkVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
}

QgsProviderMetadata::ProviderCapabilities QgsVtpkVectorTileDataProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QString QgsVtpkVectorTileDataProviderMetadata::filters( Qgis::FileFilterType type )
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
      return QObject::tr( "VTPK Vector Tiles" ) + QStringLiteral( " (*.vtpk *.VTPK)" );
  }
  return QString();
}

QList<QgsProviderSublayerDetails> QgsVtpkVectorTileDataProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
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

  if ( QFileInfo( fileName ).suffix().compare( QLatin1String( "vtpk" ), Qt::CaseInsensitive ) == 0 )
  {
    QVariantMap parts;
    parts.insert( QStringLiteral( "path" ), fileName );

    QgsProviderSublayerDetails details;
    details.setUri( encodeUri( parts ) );
    details.setProviderKey( key() );
    details.setType( Qgis::LayerType::VectorTile );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( fileName ) );
    return {details};
  }
  else
  {
    return {};
  }
}

int QgsVtpkVectorTileDataProviderMetadata::priorityForUri( const QString &uri ) const
{
  if ( validLayerTypesForUri( uri ).contains( Qgis::LayerType::VectorTile ) )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsVtpkVectorTileDataProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QFileInfo fi( uri );
  if ( fi.isFile() && fi.suffix().compare( QLatin1String( "vtpk" ), Qt::CaseInsensitive ) == 0 )
  {
    return { Qgis::LayerType::VectorTile };
  }

  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "path" ) ).toString().endsWith( ".vtpk", Qt::CaseSensitivity::CaseInsensitive ) )
    return { Qgis::LayerType::VectorTile };

  return {};
}

QVariantMap QgsVtpkVectorTileDataProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "type" ), QStringLiteral( "vtpk" ) );
  uriComponents.insert( QStringLiteral( "path" ), dsUri.param( QStringLiteral( "url" ) ) );

  return uriComponents;
}

QString QgsVtpkVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "vtpk" ) );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" ) ).toString() );
  return dsUri.encodedUri();
}

QString QgsVtpkVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = decodeUri( uri );

  const QString originalPath = parts.value( QStringLiteral( "path" ) ).toString();
  parts.insert( QStringLiteral( "path" ), context.pathResolver().writePath( originalPath ) );

  return encodeUri( parts );
}

QString QgsVtpkVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = decodeUri( uri );

  const QString originalPath = parts.value( QStringLiteral( "path" ) ).toString();
  parts.insert( QStringLiteral( "path" ), context.pathResolver().readPath( originalPath ) );

  return encodeUri( parts );
}

QList<Qgis::LayerType> QgsVtpkVectorTileDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}


///@endcond


