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
#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
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
#include <climits>
#include <limits>
#include <nlohmann/json.hpp>
#include <qglobal.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qstringliteral.h>
#include <qvector.h>

///@cond PRIVATE

constexpr const char *providerName = "quantizedmesh";
constexpr const char *providerDescription = "Cesium Quantized Mesh tiles";

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

class QgsQuantizedMeshIndex : public QgsAbstractTiledSceneIndex
{
  public:
    QgsQuantizedMeshIndex( QgsQuantizedMeshDataProvider::Metadata metadata,
                           QgsTileMatrix tileMatrix )
      : mMetadata( metadata )
      , mTileMatrix( tileMatrix ) {}
    QgsTiledSceneTile rootTile() const override;
    long long parentTileId( long long id ) const override;
    QVector< long long > childTileIds( long long id ) const override;
    QgsTiledSceneTile getTile( long long id ) override;
    QVector< long long > getTiles( const QgsTiledSceneRequest &request ) override;
    Qgis::TileChildrenAvailability childAvailability( long long id ) const override;
    bool fetchHierarchy( long long id, QgsFeedback *feedback = nullptr ) override;
  protected:
    QByteArray fetchContent( const QString &uri, QgsFeedback *feedback = nullptr ) override;

    // Tile ID coding scheme:
    // From MSb, 3 bits reserved, 5 bits zoom, 28 bits X, 28 bits Y
    static long long encodeTileId( QgsTileXYZ tile );
    static QgsTileXYZ decodeTileId( long long id );
    bool containsTile( QgsTileXYZ tile ) const;
    double geometricErrorAtZoom( uint8_t zoom ) const;

    QgsQuantizedMeshDataProvider::Metadata mMetadata;
    QgsTileMatrix mTileMatrix;

    static constexpr long long ROOT_TILE_ID = std::numeric_limits<long long>::max();
    // We need to have non-empty 3D bounding boxes, but don't have any info
    // about the real heights inside the tiles. We just give some reasonable
    // lower/upper bounds instead.
    static constexpr double DUMMY_HEIGHT_MIN = -1000;
    static constexpr double DUMMY_HEIGHT_MAX = 10000;
};

long long QgsQuantizedMeshIndex::encodeTileId( QgsTileXYZ tile )
{
  Q_ASSERT( tile.zoomLevel() < ( 2 << 4 ) && ( tile.column() < ( 2 << 27 ) ) &&
            ( tile.row() < ( 2 << 27 ) ) );
  return tile.row() | ( ( long long )tile.column() << 28 ) |
         ( ( long long )tile.zoomLevel() << 56 );
}

QgsTileXYZ QgsQuantizedMeshIndex::decodeTileId( long long id )
{
  if ( id == ROOT_TILE_ID )
    return QgsTileXYZ( 0, 0, -1 );

  Q_ASSERT( id >> 61 == 0 ); // Reserved bits are zero for regular tiles
  return QgsTileXYZ(
           ( int )( ( id >> 28 ) & ( ( 2 << 27 ) - 1 ) ),
           ( int )( id & ( ( 2 << 27 ) - 1 ) ),
           ( int )( ( id >> 56 ) & ( ( 2 << 4 ) - 1 ) ) );
}

double QgsQuantizedMeshIndex::geometricErrorAtZoom( uint8_t zoom ) const
{
  // The specification doesn't mandate any precision, we can only make a guess
  // based on each tile's maximum possible numerical precision and some
  // reasonable-looking constant.
  return 400000 / pow( 2, zoom );
}

static QgsTileXYZ tileToTms( QgsTileXYZ &xyzTile )
{
  // Flip Y axis for TMS schema
  return {xyzTile.column(),
          ( 1 << xyzTile.zoomLevel() ) - xyzTile.row() - 1,
          xyzTile.zoomLevel()};
}

bool QgsQuantizedMeshIndex::containsTile( QgsTileXYZ tile ) const
{
  if ( tile.zoomLevel() < mMetadata.mMinZoom || tile.zoomLevel() > mMetadata.mMaxZoom ||
       tile.zoomLevel() >= mMetadata.mAvailableTiles.size() )
    return false;
  // We operate with XYZ-style tile coordinates, but the availability array may
  // be given in TMS-style
  if ( mMetadata.mTileScheme == QLatin1String( "tms" ) )
    tile = tileToTms( tile );
  for ( auto &range : mMetadata.mAvailableTiles[tile.zoomLevel()] )
  {
    if ( range.startColumn() <= tile.column() && range.endColumn() >= tile.column() &&
         range.startRow() <= tile.row() && range.endRow() >= tile.row() )
      return true;
  }
  return false;
}

QgsTiledSceneTile QgsQuantizedMeshIndex::rootTile() const
{
  // Returns virtual tile to paper over tiling schemes which have >1 tile at zoom 0
  auto tile = QgsTiledSceneTile( ROOT_TILE_ID );
  tile.setBoundingVolume( QgsOrientedBox3D::fromBox3D(
                            QgsBox3D( mMetadata.mCrs.bounds(), DUMMY_HEIGHT_MIN, DUMMY_HEIGHT_MAX ) ) );
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

  if ( containsTile( {x * 2, y * 2, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2, y * 2, zoom + 1} ) );
  if ( containsTile( {x * 2 + 1, y * 2, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2 + 1, y * 2, zoom + 1} ) );
  if ( containsTile( {x * 2, y * 2 + 1, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2, y * 2 + 1, zoom + 1} ) );
  if ( containsTile( {x * 2 + 1, y * 2 + 1, zoom + 1} ) )
    children.push_back( encodeTileId( {x * 2 + 1, y * 2 + 1, zoom + 1} ) );

  return children;
}
QgsTiledSceneTile QgsQuantizedMeshIndex::getTile( long long id )
{
  auto xyzTile = decodeTileId( id );
  QgsTiledSceneTile sceneTile( id );

  auto zoomedMatrix = QgsTileMatrix::fromTileMatrix( xyzTile.zoomLevel(), mTileMatrix );
  auto tileExtent = zoomedMatrix.tileExtent( xyzTile );
  sceneTile.setBoundingVolume(
    QgsOrientedBox3D::fromBox3D(
      QgsBox3D( tileExtent, DUMMY_HEIGHT_MIN, DUMMY_HEIGHT_MAX ) ) );
  sceneTile.setGeometricError( geometricErrorAtZoom( xyzTile.zoomLevel() ) );

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
            geometricErrorAtZoom( zoomLevel ) > request.requiredGeometricError() )
      zoomLevel++;
  }
  auto tileMatrix = QgsTileMatrix::fromTileMatrix( zoomLevel, mTileMatrix );

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
      if ( containsTile( xyzTile ) )
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
    QgsDebugError( QStringLiteral( "Request failed: %1" ).arg( uri ) );
    return {};
  }
  return reply->data();
}

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

QgsQuantizedMeshDataProvider::QgsQuantizedMeshDataProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
  Qgis::DataProviderReadFlags flags )
  : QgsTiledSceneDataProvider( uri, providerOptions, flags ), mUri( uri ),
    mProviderOptions( providerOptions )
{
  // The provided URI should be the metadata JSON's location
  QNetworkRequest requestData( uri );
  QgsSetRequestInitiatorClass( requestData,
                               QStringLiteral( "QgsQuantizedMeshDataProvider" ) );
  QgsBlockingNetworkRequest request;
  auto respCode = request.get( requestData );
  if ( respCode != QgsBlockingNetworkRequest::ErrorCode::NoError )
  {
    appendError(
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
      appendError( QObject::tr( "Unexpected tile format: %1" )
                   .arg( replyJson["format"].dump().c_str() ) );
      return;
    }

    auto crsString = QString::fromStdString( jsonGet<std::string>( replyJson, "projection" ) );
    mMetadata.mCrs = QgsCoordinateReferenceSystem( crsString );
    if ( !mMetadata.mCrs.isValid() )
    {
      appendError( QObject::tr( "Invalid CRS '%1'!" ).arg( crsString ) );
      return;
    }

    try
    {
      std::vector<double> bounds = jsonGet<std::vector<double>>( replyJson, "bounds" );
      if ( bounds.size() != 4 )
      {
        appendError( QObject::tr( "Bounds array doesn't have 4 items" ) );
        return;
      }
      mMetadata.mExtent = QgsRectangle( bounds[0], bounds[1], bounds[2], bounds[3] );
    }
    catch ( MissingFieldException & )
    {
      mMetadata.mExtent = mMetadata.mCrs.bounds();
    }

    // Call our class' version explicitly to tell clang-tidy we don't need virtual dispatch here
    auto zRange = QgsQuantizedMeshDataProvider::zRange();
    mMetadata.mBoundingVolume =
      QgsOrientedBox3D::fromBox3D(
        QgsBox3D(
          mMetadata.mExtent.xMinimum(), mMetadata.mExtent.yMinimum(), zRange.lower(),
          mMetadata.mExtent.xMaximum(), mMetadata.mExtent.yMaximum(), zRange.upper() ) );

    // The TileJSON spec uses "scheme", but some real-world datasets use "schema"
    if ( replyJson.find( "scheme" ) != replyJson.end() )
      mMetadata.mTileScheme = QString::fromStdString( jsonGet<std::string>( replyJson, "scheme" ) );
    else if ( replyJson.find( "schema" ) != replyJson.end() )
      mMetadata.mTileScheme = QString::fromStdString( jsonGet<std::string>( replyJson, "schema" ) );
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
      mMetadata.mAvailableTiles.push_back( tileRanges );
    }

    try
    {
      mMetadata.mMinZoom = jsonGet<uint8_t>( replyJson, "minzoom" );
      mMetadata.mMaxZoom = jsonGet<uint8_t>( replyJson, "maxzoom" );
    }
    catch ( MissingFieldException & )
    {
      mMetadata.mMinZoom = 0;
      mMetadata.mMaxZoom = mMetadata.mAvailableTiles.size() - 1;
    }

    QString versionStr =
      QString::fromStdString( jsonGet<std::string>( replyJson, "version" ) );
    QUrl metadataUrl = uri;
    for ( auto &urlStr : jsonGet<std::vector<std::string>>( replyJson, "tiles" ) )
    {
      QUrl url = metadataUrl.resolved( QString::fromStdString( urlStr ) );
      mMetadata.mTileUrls.push_back(
        url.toString( QUrl::DecodeReserved ).replace( "{version}", versionStr ) );
    }

    int rootTileCount = 1;
    if ( crsString == QLatin1String( "EPSG:4326" ) )
      rootTileCount = 2;
    else if ( crsString != QLatin1String( "EPSG:3857" ) )
      appendError( QObject::tr( "Unhandled CRS: %1" ).arg( crsString ) );

    QgsCoordinateReferenceSystem wgs84( QStringLiteral( "EPSG:4326" ) );
    // Bounds of tile schema in projected coordinates
    auto crsBounds =
      QgsCoordinateTransform( wgs84, mMetadata.mCrs, transformContext() )
      .transform( mMetadata.mCrs.bounds() );
    QgsPointXY topLeft( crsBounds.xMinimum(), crsBounds.yMaximum() );
    double z0TileSize = crsBounds.height();

    QgsTileMatrix tileMatrix;
    tileMatrix = QgsTileMatrix::fromCustomDef( 0, mMetadata.mCrs, topLeft,
                 z0TileSize, rootTileCount, 1 );

    mIndex.emplace( new QgsQuantizedMeshIndex( mMetadata, tileMatrix ) );
  }
  catch ( nlohmann::json::exception &ex )
  {
    appendError( QObject::tr( "Error parsing JSON metadata: %1" ).arg( ex.what() ) );
    return;
  }
  catch ( MissingFieldException &ex )
  {
    appendError( QObject::tr( "Error parsing JSON metadata: %1" ).arg( ex.what() ) );
    return;
  }

  mIsValid = true;
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
  return mMetadata.mCrs;
}
const QgsTiledSceneBoundingVolume &
QgsQuantizedMeshDataProvider::boundingVolume() const
{
  return mMetadata.mBoundingVolume;
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
  // The metadata JSON doesn't declare this, each tile sets its own
  // altitude range. For now some "reasonable defaults" are in place.
  return {-10000, 10000};
}
QgsCoordinateReferenceSystem QgsQuantizedMeshDataProvider::crs() const
{
  return mMetadata.mCrs;
}
QgsRectangle QgsQuantizedMeshDataProvider::extent() const
{
  return mMetadata.mExtent;
}
bool QgsQuantizedMeshDataProvider::isValid() const { return mIsValid; }
QString QgsQuantizedMeshDataProvider::name() const { return providerName; }
QString QgsQuantizedMeshDataProvider::description() const { return providerDescription; }

QgsQuantizedMeshProviderMetadata::QgsQuantizedMeshProviderMetadata()
  : QgsProviderMetadata( providerName, providerDescription ) {}

QgsDataProvider *QgsQuantizedMeshProviderMetadata::createProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
  Qgis::DataProviderReadFlags flags )
{
  return new QgsQuantizedMeshDataProvider( uri, providerOptions, flags );
}

///@endcond
