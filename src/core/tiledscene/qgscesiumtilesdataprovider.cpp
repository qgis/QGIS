/***************************************************************************
                         qgscesiumtilesdataprovider.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscesiumtilesdataprovider.h"
#include "qgsproviderutils.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsthreadingutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscesiumutils.h"
#include "qgssphere.h"
#include "qgslogger.h"
#include "qgsorientedbox3d.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgscoordinatetransform.h"
#include "qgstiledscenenode.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenerequest.h"
#include "qgstiledscenetile.h"

#include <QUrl>
#include <QIcon>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRecursiveMutex>
#include <nlohmann/json.hpp>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "cesiumtiles" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Cesium 3D Tiles data provider" )


class QgsCesiumTiledSceneIndex final : public QgsAbstractTiledSceneIndex
{
  public:

    QgsCesiumTiledSceneIndex(
      const json &tileset,
      const QString &rootPath,
      const QString &authCfg,
      const QgsHttpHeaders &headers );

    std::unique_ptr< QgsTiledSceneTile > tileFromJson( const json &node, const QgsTiledSceneTile *parent );
    QgsTiledSceneNode *nodeFromJson( const json &node, QgsTiledSceneNode *parent );
    void refineNodeFromJson( QgsTiledSceneNode *node, const json &json );

    QgsTiledSceneTile rootTile() const final;
    QgsTiledSceneTile getTile( const QString &id ) final;
    QString parentTileId( const QString &id ) const final;
    QStringList childTileIds( const QString &id ) const final;
    QStringList getTiles( const QgsTiledSceneRequest &request ) final;
    Qgis::TileChildrenAvailability childAvailability( const QString &id ) const final;
    bool fetchHierarchy( const QString &id, QgsFeedback *feedback = nullptr ) final;

  protected:

    QByteArray fetchContent( const QString &uri, QgsFeedback *feedback = nullptr ) final;

  private:

    enum class TileContentFormat
    {
      Json,
      NotJson, // TODO: refine this to actual content types when/if needed!
    };

    mutable QRecursiveMutex mLock;
    QString mRootPath;
    std::unique_ptr< QgsTiledSceneNode > mRootNode;
    QMap< QString, QgsTiledSceneNode * > mNodeMap;
    QMap< QString, TileContentFormat > mTileContentFormats;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;

};

class QgsCesiumTilesDataProviderSharedData
{
  public:
    QgsCesiumTilesDataProviderSharedData();
    void initialize( const QString &tileset,
                     const QString &rootPath,
                     const QgsCoordinateTransformContext &transformContext,
                     const QString &authCfg,
                     const QgsHttpHeaders &headers );

    QgsCoordinateReferenceSystem mLayerCrs;
    QgsCoordinateReferenceSystem mSceneCrs;
    std::unique_ptr< QgsAbstractTiledSceneBoundingVolume > mBoundingVolume;

    QgsRectangle mExtent;
    nlohmann::json mTileset;
    QgsDoubleRange mZRange;

    QgsTiledSceneIndex mIndex;

    QReadWriteLock mMutex;


};


//
// QgsCesiumTiledSceneIndex
//

QgsCesiumTiledSceneIndex::QgsCesiumTiledSceneIndex( const json &tileset, const QString &rootPath, const QString &authCfg, const QgsHttpHeaders &headers )
  : mRootPath( rootPath )
  , mAuthCfg( authCfg )
  , mHeaders( headers )
{
  mRootNode.reset( nodeFromJson( tileset[ "root" ], nullptr ) );
}

std::unique_ptr< QgsTiledSceneTile > QgsCesiumTiledSceneIndex::tileFromJson( const json &json, const QgsTiledSceneTile *parent )
{
  std::unique_ptr< QgsTiledSceneTile > tile = std::make_unique< QgsTiledSceneTile >( QUuid::createUuid().toString() );

  QgsMatrix4x4 transform;
  if ( json.contains( "transform" ) && !json["transform"].is_null() )
  {
    const auto &transformJson = json["transform"];
    double *ptr = transform.data();
    for ( int i = 0; i < 16; ++i )
      ptr[i] = transformJson[i].get<double>();

    if ( parent && parent->transform() )
    {
      transform = *parent->transform() * transform;
    }
  }
  else if ( parent && parent->transform() )
  {
    transform = *parent->transform();
  }
  if ( !transform.isIdentity() )
    tile->setTransform( transform );

  const auto &boundingVolume = json[ "boundingVolume" ];
  std::unique_ptr< QgsAbstractTiledSceneBoundingVolume > volume;
  if ( boundingVolume.contains( "region" ) )
  {
    const QgsBox3D rootRegion = QgsCesiumUtils::parseRegion( boundingVolume[ "region" ] );
    if ( !rootRegion.isNull() )
    {
      volume = std::make_unique< QgsTiledSceneBoundingVolumeRegion >( rootRegion );
    }
  }
  else if ( boundingVolume.contains( "box" ) )
  {
    const QgsOrientedBox3D bbox = QgsCesiumUtils::parseBox( boundingVolume["box"] );
    if ( !bbox.isNull() )
    {
      volume = std::make_unique< QgsTiledSceneBoundingVolumeBox >( bbox );
    }
  }
  else if ( boundingVolume.contains( "sphere" ) )
  {
    const QgsSphere sphere = QgsCesiumUtils::parseSphere( boundingVolume["sphere"] );
    if ( !sphere.isNull() )
    {
      volume = std::make_unique< QgsTiledSceneBoundingVolumeSphere >( sphere );
    }
  }
  else
  {
    QgsDebugError( QStringLiteral( "unsupported boundingVolume format" ) );
  }

  if ( volume )
  {
    if ( !transform.isIdentity() )
      volume->transform( transform );
    tile->setBoundingVolume( volume.release() );
  }

  if ( json.contains( "geometricError" ) )
    tile->setGeometricError( json["geometricError"].get< double >() );
  if ( json.contains( "refine" ) )
  {
    if ( json["refine"] == "ADD" )
      tile->setRefinementProcess( Qgis::TileRefinementProcess::Additive );
    else if ( json["refine"] == "REPLACE" )
      tile->setRefinementProcess( Qgis::TileRefinementProcess::Replacement );
  }
  else if ( parent )
  {
    // children inherit the parent refinement if not explicitly set -- see https://github.com/CesiumGS/cesium-native/blob/172ac5ddcce602c8b268ad342639554dea2f6004/Cesium3DTilesSelection/src/TilesetJsonLoader.cpp#L440C5-L440C40
    tile->setRefinementProcess( parent->refinementProcess() );
  }

  if ( json.contains( "content" ) && !json["content"].is_null() )
  {
    const auto &contentJson = json["content"];

    // sometimes URI, sometimes URL...
    QString contentUri;
    if ( contentJson.contains( "uri" ) && !contentJson["uri"].is_null() )
    {
      contentUri = mRootPath + '/' + QString::fromStdString( contentJson["uri"].get<std::string>() );
    }
    else if ( contentJson.contains( "url" ) && !contentJson["url"].is_null() )
    {
      contentUri = mRootPath + '/' + QString::fromStdString( contentJson["url"].get<std::string>() );
    }
    if ( !contentUri.isEmpty() )
    {
      tile->setResources( {{ QStringLiteral( "content" ), contentUri } } );
    }
  }

  return tile;
}

QgsTiledSceneNode *QgsCesiumTiledSceneIndex::nodeFromJson( const json &json, QgsTiledSceneNode *parent )
{
  std::unique_ptr< QgsTiledSceneTile > tile = tileFromJson( json, parent ? parent->tile() : nullptr );
  std::unique_ptr< QgsTiledSceneNode > newNode = std::make_unique< QgsTiledSceneNode >( tile.release() );
  mNodeMap.insert( newNode->tile()->id(), newNode.get() );

  if ( parent )
    parent->addChild( newNode.get() );

  if ( json.contains( "children" ) )
  {
    for ( const auto &childJson : json["children"] )
    {
      nodeFromJson( childJson, newNode.get() );
    }
  }

  return newNode.release();
}

void QgsCesiumTiledSceneIndex::refineNodeFromJson( QgsTiledSceneNode *node, const json &json )
{
  std::unique_ptr< QgsTiledSceneTile > newTile = tileFromJson( json, node->parentNode() ? node->parentNode()->tile() : nullptr );
  // copy just the resources from the retrieved tileset to the refined node. We assume all the rest of the tile content
  // should be the same between the node being refined and the root node of the fetched sub dataset!
  // (Ie the bounding volume, geometric error, etc).
  node->tile()->setResources( newTile->resources() );

  if ( json.contains( "children" ) )
  {
    for ( const auto &childJson : json["children"] )
    {
      nodeFromJson( childJson, node );
    }
  }
}

QgsTiledSceneTile QgsCesiumTiledSceneIndex::rootTile() const
{
  QMutexLocker locker( &mLock );
  return mRootNode ? *mRootNode->tile() : QgsTiledSceneTile();
}

QgsTiledSceneTile QgsCesiumTiledSceneIndex::getTile( const QString &id )
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    return *( it.value()->tile() );
  }

  return QgsTiledSceneTile();
}

QString QgsCesiumTiledSceneIndex::parentTileId( const QString &id ) const
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    if ( QgsTiledSceneNode *parent = it.value()->parentNode() )
    {
      return parent->tile()->id();
    }
  }

  return QString();
}

QStringList QgsCesiumTiledSceneIndex::childTileIds( const QString &id ) const
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    QStringList childIds;
    const QList< QgsTiledSceneNode * > children = it.value()->children();
    childIds.reserve( children.size() );
    for ( QgsTiledSceneNode *child : children )
    {
      childIds << child->tile()->id();
    }
    return childIds;
  }

  return QStringList();
}

QStringList QgsCesiumTiledSceneIndex::getTiles( const QgsTiledSceneRequest &request )
{
  QStringList results;

  std::function< void( QgsTiledSceneNode * )> traverseNode;
  traverseNode = [&request, &traverseNode, &results, this]( QgsTiledSceneNode * node )
  {
    QgsTiledSceneTile *tile = node->tile();

    // check filter box first -- if the node doesn't intersect, then don't include the node and don't traverse
    // to its children
    if ( !request.filterBox().isNull() && !tile->boundingVolume()->intersects( request.filterBox() ) )
      return;

    // TODO -- option to filter out nodes without content

    if ( request.requiredGeometricError() <= 0 || tile->geometricError() <= 0 || tile->geometricError() > request.requiredGeometricError() )
    {
      // haven't traversed deep enough down this node, we need to explore children

      // are children available?
      QList< QgsTiledSceneNode * > children = node->children();
      if ( children.empty() )
      {
        switch ( childAvailability( tile->id() ) )
        {
          case Qgis::TileChildrenAvailability::NoChildren:
          case Qgis::TileChildrenAvailability::Available:
            break;
          case Qgis::TileChildrenAvailability::NeedFetching:
          {
            if ( !( request.flags() & Qgis::TiledSceneRequestFlag::NoHierarchyFetch ) )
            {
              // do a blocking fetch of children
              if ( fetchHierarchy( tile->id() ), request.feedback() )
              {
                children = node->children();
              }
            }
            break;
          }
        }
      }

      for ( QgsTiledSceneNode *child : std::as_const( children ) )
      {
        if ( request.feedback() && request.feedback()->isCanceled() )
          break;

        traverseNode( child );
      }

      switch ( tile->refinementProcess() )
      {
        case Qgis::TileRefinementProcess::Additive:
          // child add to parent content, so we must also include the parent
          results << tile->id();
          break;

        case Qgis::TileRefinementProcess::Replacement:
          // children replace the parent, so we skip the parent if we found children
          if ( children.empty() )
            results << tile->id();
          break;
      }
    }
    else
    {
      results << tile->id();
    }

  };

  QMutexLocker locker( &mLock );
  if ( request.parentTileId().isEmpty() )
  {
    if ( mRootNode )
      traverseNode( mRootNode.get() );
  }
  else
  {
    auto it = mNodeMap.constFind( request.parentTileId() );
    if ( it != mNodeMap.constEnd() )
    {
      traverseNode( it.value() );
    }
  }

  return results;
}

Qgis::TileChildrenAvailability QgsCesiumTiledSceneIndex::childAvailability( const QString &id ) const
{
  QString contentUri;
  QMutexLocker locker( &mLock );
  {
    auto it = mNodeMap.constFind( id );
    if ( it == mNodeMap.constEnd() )
      return Qgis::TileChildrenAvailability::NoChildren;

    if ( !it.value()->children().isEmpty() )
      return Qgis::TileChildrenAvailability::Available;

    contentUri = it.value()->tile()->resources().value( QStringLiteral( "content" ) ).toString();
  }
  {
    // maybe we already retrieved content for this node and know the answer:
    auto it = mTileContentFormats.constFind( id );
    if ( it != mTileContentFormats.constEnd() )
    {
      switch ( it.value() )
      {
        case TileContentFormat::NotJson:
          return Qgis::TileChildrenAvailability::NoChildren;
        case TileContentFormat::Json:
          return Qgis::TileChildrenAvailability::NeedFetching;
      }
    }
  }
  locker.unlock();

  if ( contentUri.isEmpty() )
    return Qgis::TileChildrenAvailability::NoChildren;

  // https://github.com/CesiumGS/3d-tiles/tree/main/specification#tile-json says:
  // "A file extension is not required for content.uri. A content’s tile format can
  // be identified by the magic field in its header, or else as an external tileset if the content is JSON."
  // This is rather annoying... it means we have to do a network request in order to determine whether
  // a tile has children or geometry content!

  // let's avoid this request if we can get away with it:
  if ( contentUri.endsWith( QLatin1String( ".json" ), Qt::CaseInsensitive ) )
    return Qgis::TileChildrenAvailability::NeedFetching;

  // things we know definitely CAN'T be a child tile map:
  const thread_local QRegularExpression antiCandidateRx( QStringLiteral( ".*\\.(gltf|glb|b3dm|i3dm|pnts|cmpt|bin|glbin|glbuf|png|jpeg|jpg)$" ), QRegularExpression::PatternOption::CaseInsensitiveOption );
  if ( antiCandidateRx.match( contentUri ).hasMatch() )
    return Qgis::TileChildrenAvailability::NoChildren;

  // here we **could** do a fetch to verify what the content actually is. But we want this method to be non-blocking,
  // so let's just report that there IS remote children available and then sort things out when we actually go to fetch those children...
  return Qgis::TileChildrenAvailability::NeedFetching;
}

bool QgsCesiumTiledSceneIndex::fetchHierarchy( const QString &id, QgsFeedback *feedback )
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it == mNodeMap.constEnd() )
    return false;

  {
    // maybe we already know what content type this tile has. If so, and it's not json, then
    // don't try to fetch it as a hierarchy
    auto it = mTileContentFormats.constFind( id );
    if ( it != mTileContentFormats.constEnd() )
    {
      switch ( it.value() )
      {
        case TileContentFormat::NotJson:
          return false;
        case TileContentFormat::Json:
          break;
      }
    }
  }

  const QString contentUri = it.value()->tile()->resources().value( QStringLiteral( "content" ) ).toString();
  locker.unlock();

  if ( contentUri.isEmpty() )
    return false;

  // if node has content json, fetch it now and parse
  const QByteArray subTile = retrieveContent( contentUri, feedback );
  if ( !subTile.isEmpty() )
  {
    // we don't know for certain that the content IS  json -- from https://github.com/CesiumGS/3d-tiles/tree/main/specification#tile-json says:
    // "A file extension is not required for content.uri. A content’s tile format can
    // be identified by the magic field in its header, or else as an external tileset if the content is JSON."
    try
    {
      const auto subTileJson = json::parse( subTile.toStdString() );
      QMutexLocker locker( &mLock );
      refineNodeFromJson( it.value(), subTileJson["root"] );
      mTileContentFormats.insert( id, TileContentFormat::Json );
      return true;
    }
    catch ( json::parse_error & )
    {
      QMutexLocker locker( &mLock );
      mTileContentFormats.insert( id, TileContentFormat::NotJson );
      return false;
    }
  }
  else
  {
    return false;
  }
}

QByteArray QgsCesiumTiledSceneIndex::fetchContent( const QString &uri, QgsFeedback *feedback )
{
  // TODO -- error reporting?
  if ( uri.startsWith( "http" ) )
  {
    QNetworkRequest networkRequest = QNetworkRequest( QUrl( uri ) );
    mHeaders.updateNetworkRequest( networkRequest );
    const QgsNetworkReplyContent reply = QgsNetworkAccessManager::instance()->blockingGet(
                                           networkRequest, mAuthCfg, false, feedback );
    return reply.content();
  }
  else if ( QFile::exists( uri ) )
  {
    QFile file( uri );
    if ( file.open( QIODevice::ReadOnly ) )
    {
      return file.readAll();
    }
  }
  return QByteArray();
}


//
// QgsCesiumTilesDataProviderSharedData
//

QgsCesiumTilesDataProviderSharedData::QgsCesiumTilesDataProviderSharedData()
  : mIndex( QgsTiledSceneIndex( nullptr ) )
{

}

void QgsCesiumTilesDataProviderSharedData::initialize( const QString &tileset, const QString &rootPath, const QgsCoordinateTransformContext &transformContext, const QString &authCfg, const QgsHttpHeaders &headers )
{
  mTileset = json::parse( tileset.toStdString() );

  // parse root
  {
    const auto &root = mTileset[ "root" ];
    // parse root bounding volume

    // TODO -- read crs from metadata tags. Need to find real world examples of this. And can metadata crs override
    // the EPSG:4979 requirement from a region bounding volume??

    {
      // TODO -- on some datasets there is a "boundingVolume" present on the tileset itself, i.e. not the root node.
      // what does this mean? Should we use it instead of the root node bounding volume if it's present?

      const auto &rootBoundingVolume = root[ "boundingVolume" ];

      QgsMatrix4x4 rootTransform;
      if ( root.contains( "transform" ) && !root["transform"].is_null() )
      {
        const auto &transformJson = root["transform"];
        double *ptr = rootTransform.data();
        for ( int i = 0; i < 16; ++i )
          ptr[i] = transformJson[i].get<double>();
      }

      if ( rootBoundingVolume.contains( "region" ) )
      {
        const QgsBox3D rootRegion = QgsCesiumUtils::parseRegion( rootBoundingVolume[ "region" ] );
        if ( !rootRegion.isNull() )
        {
          mBoundingVolume = std::make_unique< QgsTiledSceneBoundingVolumeRegion >( rootRegion );
          mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mSceneCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );
          mExtent = rootRegion.toRectangle();
        }
      }
      else if ( rootBoundingVolume.contains( "box" ) )
      {
        const QgsOrientedBox3D bbox = QgsCesiumUtils::parseBox( rootBoundingVolume["box"] );
        if ( !bbox.isNull() )
        {
          // layer must advertise as EPSG:4979, as the various QgsMapLayer
          // methods which utilize QgsMapLayer::crs() (such as layer extent transformation)
          // are all purely 2D and can't handle the cesium data source z value
          // range in EPSG:4978 !
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mSceneCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );

          const QgsCoordinateTransform transform( mSceneCrs, mLayerCrs, transformContext );

          mBoundingVolume = std::make_unique< QgsTiledSceneBoundingVolumeBox >( bbox );
          mBoundingVolume->transform( rootTransform );
          try
          {
            const QgsBox3D rootRegion = mBoundingVolume->bounds( transform );
            mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );

            std::unique_ptr< QgsAbstractGeometry > extent2D( mBoundingVolume->as2DGeometry( transform ) );
            mExtent = extent2D->boundingBox();
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Caught transform exception when transforming boundingVolume" ) );
          }
        }
      }
      else if ( rootBoundingVolume.contains( "sphere" ) )
      {
        const QgsSphere sphere = QgsCesiumUtils::parseSphere( rootBoundingVolume["sphere"] );
        if ( !sphere.isNull() )
        {
          // layer must advertise as EPSG:4979, as the various QgsMapLayer
          // methods which utilize QgsMapLayer::crs() (such as layer extent transformation)
          // are all purely 2D and can't handle the cesium data source z value
          // range in EPSG:4978 !
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mSceneCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );

          const QgsCoordinateTransform transform( mSceneCrs, mLayerCrs, transformContext );

          mBoundingVolume = std::make_unique< QgsTiledSceneBoundingVolumeSphere >( sphere );
          mBoundingVolume->transform( rootTransform );
          try
          {
            const QgsBox3D rootRegion = mBoundingVolume->bounds( transform );
            mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );

            std::unique_ptr< QgsAbstractGeometry > extent2D( mBoundingVolume->as2DGeometry( transform ) );
            mExtent = extent2D->boundingBox();
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Caught transform exception when transforming boundingVolume" ) );
          }
        }
      }
      else
      {
        QgsDebugError( QStringLiteral( "unsupported boundingVolume format" ) );
      }
    }

    mIndex = QgsTiledSceneIndex(
               new QgsCesiumTiledSceneIndex(
                 mTileset,
                 rootPath,
                 authCfg,
                 headers
               )
             );
  }
}


//
// QgsCesiumTilesDataProvider
//

QgsCesiumTilesDataProvider::QgsCesiumTilesDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsTiledSceneDataProvider( uri, providerOptions, flags )
  , mShared( std::make_shared< QgsCesiumTilesDataProviderSharedData >() )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();

  mIsValid = init();
}

QgsCesiumTilesDataProvider::QgsCesiumTilesDataProvider( const QgsCesiumTilesDataProvider &other )
  : QgsTiledSceneDataProvider( other )
  , mIsValid( other.mIsValid )
  , mAuthCfg( other.mAuthCfg )
  , mHeaders( other.mHeaders )
  , mShared( other.mShared )
{
}

QgsCesiumTilesDataProvider::~QgsCesiumTilesDataProvider() = default;

QgsCesiumTilesDataProvider *QgsCesiumTilesDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return new QgsCesiumTilesDataProvider( *this );
}

bool QgsCesiumTilesDataProvider::init()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  const QString tileSetUri = dsUri.param( QStringLiteral( "url" ) );
  if ( !tileSetUri.isEmpty() )
  {
    const QUrl url( tileSetUri );

    QNetworkRequest request = QNetworkRequest( url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsCesiumTilesDataProvider" ) )
    mHeaders.updateNetworkRequest( request );

    QgsBlockingNetworkRequest networkRequest;
    networkRequest.setAuthCfg( mAuthCfg );

    switch ( networkRequest.get( request ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        // TODO -- error reporting
        return false;
    }

    const QgsNetworkReplyContent content = networkRequest.reply();

    const QString base = tileSetUri.left( tileSetUri.lastIndexOf( '/' ) );
    mShared->initialize( content.content(), base, transformContext(), mAuthCfg, mHeaders );
  }
  else
  {
    // try uri as a local file
    const QFileInfo fi( dataSourceUri() );
    if ( fi.exists() )
    {
      QFile file( dataSourceUri( ) );
      if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        const QByteArray raw = file.readAll();
        mShared->initialize( raw, fi.path(), transformContext(), mAuthCfg, mHeaders );
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  return true;
}

QgsCoordinateReferenceSystem QgsCesiumTilesDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared->mLayerCrs;
}

QgsRectangle QgsCesiumTilesDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared->mExtent;
}

bool QgsCesiumTilesDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QString QgsCesiumTilesDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return PROVIDER_KEY;
}

QString QgsCesiumTilesDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QObject::tr( "Cesium 3D Tiles" );
}

QString QgsCesiumTilesDataProvider::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString metadata;

  if ( mShared->mTileset.contains( "asset" ) )
  {
    const auto &asset = mShared->mTileset[ "asset" ];
    if ( asset.contains( "version" ) )
    {
      const QString version = QString::fromStdString( asset["version"].get<std::string>() );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "3D Tiles Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( version ) % QStringLiteral( "</td></tr>\n" );
    }

    if ( asset.contains( "tilesetVersion" ) )
    {
      const QString tilesetVersion = QString::fromStdString( asset["tilesetVersion"].get<std::string>() );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( tilesetVersion ) % QStringLiteral( "</td></tr>\n" );
    }

    if ( asset.contains( "generator" ) )
    {
      const QString generator = QString::fromStdString( asset["generator"].get<std::string>() );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Generator" ) % QStringLiteral( "</td><td>%1</a>" ).arg( generator ) % QStringLiteral( "</td></tr>\n" );
    }
  }
  if ( mShared->mTileset.contains( "extensionsRequired" ) )
  {
    QStringList extensions;
    for ( const auto &item : mShared->mTileset["extensionsRequired"] )
    {
      extensions << QString::fromStdString( item.get<std::string>() );
    }
    if ( !extensions.isEmpty() )
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Extensions Required" ) % QStringLiteral( "</td><td><ul><li>%1</li></ul></a>" ).arg( extensions.join( QStringLiteral( "</li><li>" ) ) ) % QStringLiteral( "</td></tr>\n" );
    }
  }
  if ( mShared->mTileset.contains( "extensionsUsed" ) )
  {
    QStringList extensions;
    for ( const auto &item : mShared->mTileset["extensionsUsed"] )
    {
      extensions << QString::fromStdString( item.get<std::string>() );
    }
    if ( !extensions.isEmpty() )
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Extensions Used" ) % QStringLiteral( "</td><td><ul><li>%1</li></ul></a>" ).arg( extensions.join( QStringLiteral( "</li><li>" ) ) ) % QStringLiteral( "</td></tr>\n" );
    }
  }

  if ( !mShared->mZRange.isInfinite() )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Z Range" ) % QStringLiteral( "</td><td>%1 - %2</a>" ).arg( QLocale().toString( mShared->mZRange.lower() ), QLocale().toString( mShared->mZRange.upper() ) ) % QStringLiteral( "</td></tr>\n" );
  }

  return metadata;
}

const QgsCoordinateReferenceSystem QgsCesiumTilesDataProvider::sceneCrs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared ? mShared->mSceneCrs : QgsCoordinateReferenceSystem();
}

const QgsAbstractTiledSceneBoundingVolume *QgsCesiumTilesDataProvider::boundingVolume() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared ? mShared->mBoundingVolume.get() : nullptr;
}

QgsTiledSceneIndex QgsCesiumTilesDataProvider::index() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared ? mShared->mIndex : QgsTiledSceneIndex( nullptr );
}


//
// QgsCesiumTilesProviderMetadata
//

QgsCesiumTilesProviderMetadata::QgsCesiumTilesProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsCesiumTilesProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "xxxcesiumxxx.svg" ) );
}

QgsCesiumTilesDataProvider *QgsCesiumTilesProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsCesiumTilesDataProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsCesiumTilesProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "tileset.json" ), Qt::CaseInsensitive ) == 0 )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( PROVIDER_KEY );
    details.setType( Qgis::LayerType::TiledScene );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( uri ) );
    return {details};
  }
  else
  {
    return {};
  }
}

int QgsCesiumTilesProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "tileset.json" ), Qt::CaseInsensitive ) == 0 )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsCesiumTilesProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "tileset.json" ), Qt::CaseInsensitive ) == 0 )
    return QList< Qgis::LayerType>() << Qgis::LayerType::TiledScene;

  return QList< Qgis::LayerType>();
}

QVariantMap QgsCesiumTilesProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;
  QUrl url = QUrl::fromUserInput( uri );
  uriComponents.insert( QStringLiteral( "file-name" ), url.fileName() );
  uriComponents.insert( QStringLiteral( "path" ), uri );
  return uriComponents;
}

QString QgsCesiumTilesProviderMetadata::filters( Qgis::FileFilterType type )
{
  switch ( type )
  {
    case Qgis::FileFilterType::Vector:
    case Qgis::FileFilterType::Raster:
    case Qgis::FileFilterType::Mesh:
    case Qgis::FileFilterType::MeshDataset:
    case Qgis::FileFilterType::VectorTile:
    case Qgis::FileFilterType::PointCloud:
      return QString();

    case Qgis::FileFilterType::TiledScene:
      return QObject::tr( "Cesium 3D Tiles" ) + QStringLiteral( " (tileset.json TILESET.JSON)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsCesiumTilesProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsCesiumTilesProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::TiledScene };
}

QString QgsCesiumTilesProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsCesiumTilesProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

///@endcond
