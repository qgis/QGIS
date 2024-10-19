/***************************************************************************
                         qgsquantizedmeshdataprovider.cpp
                         --------------------
    begin                : June 2024
    copyright            : (C) 2024 by David Koňařík
    email                : dvdkon at konarici dot cz
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquantizedmeshdataprovider.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatetransformcontext.h"
#include "qgslogger.h"
#include "qgsmatrix4x4.h"
#include "qgsorientedbox3d.h"
#include "qgsprovidermetadata.h"
#include "qgssetrequestinitiator_p.h"
#include "qgstiledownloadmanager.h"
#include "qgstiledscenedataprovider.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenerequest.h"
#include "qgstiledscenetile.h"
#include "qgstiles.h"
#include "qgsvectortileutils.h"
#include <limits>
#include <nlohmann/json.hpp>
#include <qglobal.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qstringliteral.h>
#include <qvector.h>

///@cond PRIVATE

class MissingFieldException : public std::exception
{
  public:
    MissingFieldException( const char *field ) : mField( field ) { }
    const char *what() const noexcept
    {
      return QString( "Missing field: %1" ).arg( mField ).toLocal8Bit().data();
    }
  private:
    const char *mField;
};

template <typename T>
static T jsonGet( nlohmann::json &json, const char *idx )
{
  auto &obj = json[idx];
  if ( obj.is_null() )
  {
    throw MissingFieldException( idx );
  }
  return obj.get<T>();
}


QgsQuantizedMeshMetadata::QgsQuantizedMeshMetadata(
  const QString &uri,
  const QgsCoordinateTransformContext &transformContext,
  QgsError &error )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();

  // The provided URL should be the metadata JSON's location
  QUrl metadataUrl = dsUri.param( "url" );
  QNetworkRequest requestData( metadataUrl );
  mHeaders.updateNetworkRequest( requestData );
  QgsSetRequestInitiatorClass( requestData,
                               QStringLiteral( "QgsQuantizedMeshDataProvider" ) );
  QgsBlockingNetworkRequest request;
  if ( !mAuthCfg.isEmpty() )
    request.setAuthCfg( mAuthCfg );
  auto respCode = request.get( requestData );
  if ( respCode != QgsBlockingNetworkRequest::ErrorCode::NoError )
  {
    error.append(
      QObject::tr( "Failed to retrieve quantized mesh tiles metadata: %1" )
      .arg( request.errorMessage().data() ) );
    return;
  }
  auto reply = request.reply().content();

  try
  {
    auto replyJson = nlohmann::json::parse( reply.data() );

    // The metadata is an (undocumented) variant of TileJSON
    if ( jsonGet<std::string>( replyJson, "format" ) != "quantized-mesh-1.0" )
    {
      error.append( QObject::tr( "Unexpected tile format: %1" )
                    .arg( replyJson["format"].dump().c_str() ) );
      return;
    }

    auto crsString = QString::fromStdString( jsonGet<std::string>( replyJson, "projection" ) );
    mCrs = QgsCoordinateReferenceSystem( crsString );
    if ( !mCrs.isValid() )
    {
      error.append( QObject::tr( "Invalid CRS '%1'!" ).arg( crsString ) );
      return;
    }

    try
    {
      std::vector<double> bounds = jsonGet<std::vector<double>>( replyJson, "bounds" );
      if ( bounds.size() != 4 )
      {
        error.append( QObject::tr( "Bounds array doesn't have 4 items" ) );
        return;
      }
      mExtent = QgsRectangle( bounds[0], bounds[1], bounds[2], bounds[3] );
    }
    catch ( MissingFieldException & )
    {
      mExtent = mCrs.bounds();
    }

    auto zRange = dummyZRange;
    mBoundingVolume =
      QgsOrientedBox3D::fromBox3D(
        QgsBox3D(
          mExtent.xMinimum(), mExtent.yMinimum(), zRange.lower(),
          mExtent.xMaximum(), mExtent.yMaximum(), zRange.upper() ) );

    // The TileJSON spec uses "scheme", but some real-world datasets use "schema"
    if ( replyJson.find( "scheme" ) != replyJson.end() )
      mTileScheme = QString::fromStdString( jsonGet<std::string>( replyJson, "scheme" ) );
    else if ( replyJson.find( "schema" ) != replyJson.end() )
      mTileScheme = QString::fromStdString( jsonGet<std::string>( replyJson, "schema" ) );
    else throw MissingFieldException( "scheme/schema" );

    for ( auto &aabbs : replyJson.at( "available" ) )
    {
      QVector<QgsTileRange> tileRanges;
      for ( auto &aabb : aabbs )
      {
        tileRanges.push_back(
          QgsTileRange(
            jsonGet<int>( aabb, "startX" ), jsonGet<int>( aabb, "endX" ),
            jsonGet<int>( aabb, "startY" ), jsonGet<int>( aabb, "endY" ) ) );
      }
      mAvailableTiles.push_back( tileRanges );
    }

    try
    {
      mMinZoom = jsonGet<uint8_t>( replyJson, "minzoom" );
      mMaxZoom = jsonGet<uint8_t>( replyJson, "maxzoom" );
    }
    catch ( MissingFieldException & )
    {
      mMinZoom = 0;
      mMaxZoom = mAvailableTiles.size() - 1;
    }

    QString versionStr =
      QString::fromStdString( jsonGet<std::string>( replyJson, "version" ) );
    for ( auto &urlStr : jsonGet<std::vector<std::string>>( replyJson, "tiles" ) )
    {
      QUrl url = metadataUrl.resolved( QString::fromStdString( urlStr ) );
      mTileUrls.push_back(
        url.toString( QUrl::DecodeReserved ).replace( "{version}", versionStr ) );
    }

    int rootTileCount = 1;
    if ( crsString == QLatin1String( "EPSG:4326" ) )
      rootTileCount = 2;
    else if ( crsString != QLatin1String( "EPSG:3857" ) )
      error.append( QObject::tr( "Unhandled CRS: %1" ).arg( crsString ) );

    QgsCoordinateReferenceSystem wgs84( QStringLiteral( "EPSG:4326" ) );
    // Bounds of tile schema in projected coordinates
    auto crsBounds =
      QgsCoordinateTransform( wgs84, mCrs, transformContext )
      .transform( mCrs.bounds() );
    QgsPointXY topLeft( crsBounds.xMinimum(), crsBounds.yMaximum() );
    double z0TileSize = crsBounds.height();

    mTileMatrix = QgsTileMatrix::fromCustomDef( 0, mCrs, topLeft, z0TileSize, rootTileCount, 1 );
  }
  catch ( nlohmann::json::exception &ex )
  {
    error.append( QObject::tr( "Error parsing JSON metadata: %1" ).arg( ex.what() ) );
  }
  catch ( MissingFieldException &ex )
  {
    error.append( QObject::tr( "Error parsing JSON metadata: %1" ).arg( ex.what() ) );
  }
}

const QgsDoubleRange QgsQuantizedMeshMetadata::dummyZRange = {-10000, 10000};

static QgsTileXYZ tileToTms( QgsTileXYZ &xyzTile )
{
  // Flip Y axis for TMS schema
  Q_ASSERT( xyzTile.zoomLevel() >= 0 );
  return {xyzTile.column(),
          ( 1 << xyzTile.zoomLevel() ) - xyzTile.row() - 1,
          xyzTile.zoomLevel()};
}

bool QgsQuantizedMeshMetadata::containsTile( QgsTileXYZ tile ) const
{
  if ( tile.zoomLevel() < mMinZoom || tile.zoomLevel() > mMaxZoom ||
       tile.zoomLevel() >= mAvailableTiles.size() )
    return false;
  // We operate with XYZ-style tile coordinates, but the availability array may
  // be given in TMS-style
  if ( mTileScheme == QLatin1String( "tms" ) )
    tile = tileToTms( tile );
  for ( auto &range : mAvailableTiles[tile.zoomLevel()] )
  {
    if ( range.startColumn() <= tile.column() && range.endColumn() >= tile.column() &&
         range.startRow() <= tile.row() && range.endRow() >= tile.row() )
      return true;
  }
  return false;
}

double QgsQuantizedMeshMetadata::geometricErrorAtZoom( int zoom ) const
{
  // The specification doesn't mandate any precision, we can only make a guess
  // based on each tile's maximum possible numerical precision and some
  // reasonable-looking constant.
  return 400000 / pow( 2, zoom );
}

long long QgsQuantizedMeshIndex::encodeTileId( QgsTileXYZ tile )
{
  if ( tile.zoomLevel() == -1 )
  {
    Q_ASSERT( tile.column() == 0 && tile.row() == 0 );
    return ROOT_TILE_ID;
  }
  Q_ASSERT( tile.zoomLevel() < ( 2 << 4 ) && ( tile.column() < ( 2 << 27 ) ) &&
            ( tile.row() < ( 2 << 27 ) ) );
  return tile.row() | ( ( long long )tile.column() << 28 ) |
         ( ( long long )tile.zoomLevel() << 56 ) | ( ( long long ) 1 << 61 );
}

QgsTileXYZ QgsQuantizedMeshIndex::decodeTileId( long long id )
{
  if ( id == ROOT_TILE_ID )
    return QgsTileXYZ( 0, 0, -1 );

  Q_ASSERT( id >> 61 == 1 ); // Check reserved bits
  return QgsTileXYZ(
           ( int )( ( id >> 28 ) & ( ( 2 << 27 ) - 1 ) ),
           ( int )( id & ( ( 2 << 27 ) - 1 ) ),
           ( int )( ( id >> 56 ) & ( ( 2 << 4 ) - 1 ) ) );
}

QgsTiledSceneTile QgsQuantizedMeshIndex::rootTile() const
{
  // Returns virtual tile to paper over tiling schemes which have >1 tile at zoom 0
  auto tile = QgsTiledSceneTile( ROOT_TILE_ID );
  auto bounds = mWgs84ToCrs.transform( mMetadata.mCrs.bounds() );
  tile.setBoundingVolume(
    QgsOrientedBox3D::fromBox3D(
      QgsBox3D( bounds, mMetadata.dummyZRange.lower(), mMetadata.dummyZRange.upper() ) ) );
  tile.setGeometricError( std::numeric_limits<double>::max() );
  return tile;
}
long long QgsQuantizedMeshIndex::parentTileId( long long id ) const
{
  if ( id == ROOT_TILE_ID )
    return -1;
  auto tile = decodeTileId( id );
  if ( tile.zoomLevel() == 0 )
    return ROOT_TILE_ID;
  return encodeTileId( {tile.zoomLevel() - 1, tile.column() / 2, tile.row() / 2} );
}
QVector<long long> QgsQuantizedMeshIndex::childTileIds( long long id ) const
{
  auto tile = decodeTileId( id );
  QVector<long long> children;
  auto x = tile.column(), y = tile.row(), zoom = tile.zoomLevel();

  if ( mMetadata.containsTile( {x * 2, y * 2, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2, y * 2, zoom + 1} ) );
  if ( mMetadata.containsTile( {x * 2 + 1, y * 2, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2 + 1, y * 2, zoom + 1} ) );
  if ( mMetadata.containsTile( {x * 2, y * 2 + 1, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2, y * 2 + 1, zoom + 1} ) );
  if ( mMetadata.containsTile( {x * 2 + 1, y * 2 + 1, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2 + 1, y * 2 + 1, zoom + 1} ) );

  return children;
}
QgsTiledSceneTile QgsQuantizedMeshIndex::getTile( long long id )
{
  auto xyzTile = decodeTileId( id );
  QgsTiledSceneTile sceneTile( id );

  auto zoomedMatrix = QgsTileMatrix::fromTileMatrix( xyzTile.zoomLevel(), mMetadata.mTileMatrix );
  auto tileExtent = zoomedMatrix.tileExtent( xyzTile );

  sceneTile.setBoundingVolume(
    QgsOrientedBox3D::fromBox3D(
      QgsBox3D( tileExtent, mMetadata.dummyZRange.lower(), mMetadata.dummyZRange.upper() ) ) );
  sceneTile.setGeometricError( mMetadata.geometricErrorAtZoom( xyzTile.zoomLevel() ) );

  if ( id == ROOT_TILE_ID )
    // The root tile is fictitious and has no content, don't bother pointing to any.
    return sceneTile;

  if ( mMetadata.mTileScheme == QLatin1String( "tms" ) )
    xyzTile = tileToTms( xyzTile );

  if ( mMetadata.mTileUrls.size() == 0 )
  {
    QgsDebugError( "Quantized Mesh metadata has no URLs for tiles" );
  }
  else
  {
    // TODO: Intelligently choose from alternatives. Round robin?
    auto tileUri = QgsVectorTileUtils::formatXYZUrlTemplate(
                     mMetadata.mTileUrls[0], xyzTile, zoomedMatrix );
    sceneTile.setResources( {{"content", tileUri}} );
    sceneTile.setMetadata(
    {
      {QStringLiteral( "gltfUpAxis" ), static_cast<int>( Qgis::Axis::Z )},
      {QStringLiteral( "contentFormat" ), QStringLiteral( "quantizedmesh" )},
    } );
  }

  // Tile meshes have 0.0 -- 1.0 coordinates. Rescale them to the tile's real
  // width and height in our CRS and move the tile to its position.
  QgsMatrix4x4 transform(
    tileExtent.width(), 0, 0, tileExtent.xMinimum(),
    0, tileExtent.height(), 0, tileExtent.yMinimum(),
    0, 0, 1, 0,
    0, 0, 0, 1 );
  sceneTile.setTransform( transform );

  return sceneTile;
}
QVector<long long>
QgsQuantizedMeshIndex::getTiles( const QgsTiledSceneRequest &request )
{
  uint8_t zoomLevel = mMetadata.mMinZoom;
  if ( request.requiredGeometricError() != 0 )
  {
    while ( zoomLevel < mMetadata.mMaxZoom &&
            mMetadata.geometricErrorAtZoom( zoomLevel ) > request.requiredGeometricError() )
      zoomLevel++;
  }
  auto tileMatrix = QgsTileMatrix::fromTileMatrix( zoomLevel, mMetadata.mTileMatrix );

  QVector<long long> ids;
  // We can only filter on X and Y
  auto extent = request.filterBox().extent().toRectangle();
  if ( request.parentTileId() != -1 )
  {
    auto parentTile = decodeTileId( request.parentTileId() );
    extent.intersect( tileMatrix.tileExtent( parentTile ) );
  }

  auto tileRange = tileMatrix.tileRangeFromExtent( extent );
  if ( !tileRange.isValid() )
    return {};

  for ( int col = tileRange.startColumn(); col <= tileRange.endColumn(); col++ )
    for ( int row = tileRange.startRow(); row <= tileRange.endRow(); row++ )
    {
      auto xyzTile = QgsTileXYZ( col, row, zoomLevel );
      if ( mMetadata.containsTile( xyzTile ) )
        ids.push_back( encodeTileId( xyzTile ) );
    }

  return ids;
}
Qgis::TileChildrenAvailability
QgsQuantizedMeshIndex::childAvailability( long long id ) const
{
  auto childIds = childTileIds( id );
  if ( childIds.count() == 0 )
    return Qgis::TileChildrenAvailability::NoChildren;
  return Qgis::TileChildrenAvailability::Available;
}
bool QgsQuantizedMeshIndex::fetchHierarchy( long long id, QgsFeedback *feedback )
{
  // The API was built for Cesium 3D tiles, which have tiles (actual files with
  // metadata) as nodes of a hierarchy tree, with the actual data in children
  // of those tiles. For us, tiles are represented by int IDs, so they don't
  // need to be fetched.
  Q_UNUSED( id );
  Q_UNUSED( feedback );
  return true;
}

QByteArray QgsQuantizedMeshIndex::fetchContent( const QString &uri,
    QgsFeedback *feedback )
{
  QNetworkRequest requestData( uri );
  requestData.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  requestData.setRawHeader( "Accept", "application/vnd.quantized-mesh,application/octet-stream;q=0.9" );
  mMetadata.mHeaders.updateNetworkRequest( requestData );
  if ( !mMetadata.mAuthCfg.isEmpty() )
    QgsApplication::authManager()->updateNetworkRequest( requestData, mMetadata.mAuthCfg );
  QgsSetRequestInitiatorClass( requestData,
                               QStringLiteral( "QgsQuantizedMeshIndex" ) );

  std::unique_ptr<QgsTileDownloadManagerReply> reply( QgsApplication::tileDownloadManager()->get( requestData ) );

  QEventLoop loop;
  if ( feedback )
    QObject::connect( feedback, &QgsFeedback::canceled, &loop, &QEventLoop::quit );
  QObject::connect( reply.get(), &QgsTileDownloadManagerReply::finished, &loop, &QEventLoop::quit );
  loop.exec();

  if ( reply->error() != QNetworkReply::NoError )
  {
    QgsDebugError( QStringLiteral( "Request failed (%1): %2" ).arg( uri ).arg( reply->errorString() ) );
    return {};
  }
  return reply->data();
}

QgsQuantizedMeshDataProvider::QgsQuantizedMeshDataProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
  Qgis::DataProviderReadFlags flags )
  : QgsTiledSceneDataProvider( uri, providerOptions, flags ), mUri( uri ),
    mProviderOptions( providerOptions )
{
  mMetadata = QgsQuantizedMeshMetadata( uri, transformContext(), mError );
  if ( mError.isEmpty() )
  {
    QgsCoordinateReferenceSystem wgs84( QStringLiteral( "EPSG:4326" ) );
    QgsCoordinateTransform wgs84ToCrs( wgs84, mMetadata->mCrs, transformContext() );
    mIndex.emplace( new QgsQuantizedMeshIndex( *mMetadata, wgs84ToCrs ) );
    mIsValid = true;
  }
}

Qgis::TiledSceneProviderCapabilities
QgsQuantizedMeshDataProvider::capabilities() const
{
  return Qgis::TiledSceneProviderCapabilities();
}
QgsTiledSceneDataProvider *QgsQuantizedMeshDataProvider::clone() const
{
  return new QgsQuantizedMeshDataProvider( mUri, mProviderOptions );
}
const QgsCoordinateReferenceSystem
QgsQuantizedMeshDataProvider::sceneCrs() const
{
  return mMetadata->mCrs;
}
const QgsTiledSceneBoundingVolume &
QgsQuantizedMeshDataProvider::boundingVolume() const
{
  return mMetadata->mBoundingVolume;
}

QgsTiledSceneIndex QgsQuantizedMeshDataProvider::index() const
{
  if ( !mIndex )
    // Return dummy index, this provider is likely not valid
    return QgsTiledSceneIndex( nullptr );
  return *mIndex;
}

QgsDoubleRange QgsQuantizedMeshDataProvider::zRange() const
{
  return mMetadata->dummyZRange;
}
QgsCoordinateReferenceSystem QgsQuantizedMeshDataProvider::crs() const
{
  return mMetadata->mCrs;
}
QgsRectangle QgsQuantizedMeshDataProvider::extent() const
{
  return mMetadata->mExtent;
}
bool QgsQuantizedMeshDataProvider::isValid() const { return mIsValid; }
QString QgsQuantizedMeshDataProvider::name() const { return providerName; }
QString QgsQuantizedMeshDataProvider::description() const { return providerDescription; }

const QgsQuantizedMeshMetadata &QgsQuantizedMeshDataProvider::quantizedMeshMetadata() const
{
  return *mMetadata;
}

QgsQuantizedMeshProviderMetadata::QgsQuantizedMeshProviderMetadata()
  : QgsProviderMetadata( QgsQuantizedMeshDataProvider::providerName,
                         QgsQuantizedMeshDataProvider::providerDescription ) {}

QgsDataProvider *QgsQuantizedMeshProviderMetadata::createProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
  Qgis::DataProviderReadFlags flags )
{
  return new QgsQuantizedMeshDataProvider( uri, providerOptions, flags );
}

///@endcond
