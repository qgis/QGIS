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
#include "qgsauthmanager.h"
#include "qgsproviderutils.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsthreadingutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
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
#include "qgsreadwritelocker.h"
#include "qgstiledownloadmanager.h"

#include <QUrl>
#include <QIcon>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRecursiveMutex>
#include <QUrlQuery>
#include <QApplication>
#include <nlohmann/json.hpp>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "cesiumtiles" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Cesium 3D Tiles data provider" )


// This is to support a case seen with Google's tiles. Root URL is something like this:
// https://tile.googleapis.com/.../root.json?key=123
// The returned JSON contains relative links with "session" (e.g. "/.../abc.json?session=456")
// When fetching such abc.json, we have to include also "key" from the original URL!
// Then the content of abc.json contains relative links (e.g. "/.../xyz.glb") and we
// need to add both "key" and "session" (otherwise requests fail).
//
// This function simply copies any query items from the base URL to the content URI.
static QString appendQueryFromBaseUrl( const QString &contentUri, const QUrl &baseUrl )
{
  QUrlQuery contentQuery( QUrl( contentUri ).query() );
  const QList<QPair<QString, QString>> baseUrlQueryItems = QUrlQuery( baseUrl.query() ).queryItems();
  for ( const QPair<QString, QString> &kv : baseUrlQueryItems )
  {
    contentQuery.addQueryItem( kv.first, kv.second );
  }
  QUrl newContentUrl( contentUri );
  newContentUrl.setQuery( contentQuery );
  return newContentUrl.toString();
}


class QgsCesiumTiledSceneIndex final : public QgsAbstractTiledSceneIndex
{
  public:

    QgsCesiumTiledSceneIndex(
      const json &tileset,
      const QUrl &rootUrl,
      const QString &authCfg,
      const QgsHttpHeaders &headers,
      const QgsCoordinateTransformContext &transformContext );

    std::unique_ptr< QgsTiledSceneTile > tileFromJson( const json &node, const QUrl &baseUrl, const QgsTiledSceneTile *parent, Qgis::Axis gltfUpAxis );
    QgsTiledSceneNode *nodeFromJson( const json &node, const QUrl &baseUrl, QgsTiledSceneNode *parent, Qgis::Axis gltfUpAxis );
    void refineNodeFromJson( QgsTiledSceneNode *node, const QUrl &baseUrl, const json &json );

    QgsTiledSceneTile rootTile() const final;
    QgsTiledSceneTile getTile( long long id ) final;
    long long parentTileId( long long id ) const final;
    QVector< long long > childTileIds( long long id ) const final;
    QVector< long long > getTiles( const QgsTiledSceneRequest &request ) final;
    Qgis::TileChildrenAvailability childAvailability( long long id ) const final;
    bool fetchHierarchy( long long id, QgsFeedback *feedback = nullptr ) final;

  protected:

    QByteArray fetchContent( const QString &uri, QgsFeedback *feedback = nullptr ) final;

  private:

    enum class TileContentFormat
    {
      Json,
      NotJson, // TODO: refine this to actual content types when/if needed!
    };

    mutable QRecursiveMutex mLock;
    QgsCoordinateTransformContext mTransformContext;
    std::unique_ptr< QgsTiledSceneNode > mRootNode;
    QMap< long long, QgsTiledSceneNode * > mNodeMap;
    QMap< long long, TileContentFormat > mTileContentFormats;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    long long mNextTileId = 0;

};

class QgsCesiumTilesDataProviderSharedData
{
  public:
    QgsCesiumTilesDataProviderSharedData();
    void initialize( const QString &tileset,
                     const QUrl &rootUrl,
                     const QgsCoordinateTransformContext &transformContext,
                     const QString &authCfg,
                     const QgsHttpHeaders &headers );

    QgsCoordinateReferenceSystem mLayerCrs;
    QgsCoordinateReferenceSystem mSceneCrs;
    QgsTiledSceneBoundingVolume mBoundingVolume;

    QgsRectangle mExtent;
    nlohmann::json mTileset;
    QgsDoubleRange mZRange;

    QgsTiledSceneIndex mIndex;

    QgsLayerMetadata mLayerMetadata;
    QString mError;
    QReadWriteLock mReadWriteLock;

};


//
// QgsCesiumTiledSceneIndex
//

Qgis::Axis axisFromJson( const json &json )
{
  const std::string gltfUpAxisString = json.get<std::string>();
  if ( gltfUpAxisString == "z" || gltfUpAxisString == "Z" )
  {
    return Qgis::Axis::Z;
  }
  else if ( gltfUpAxisString == "y" || gltfUpAxisString == "Y" )
  {
    return Qgis::Axis::Y;
  }
  else if ( gltfUpAxisString == "x" || gltfUpAxisString == "X" )
  {
    return Qgis::Axis::X;
  }
  QgsDebugError( QStringLiteral( "Unsupported gltfUpAxis value: %1" ).arg( QString::fromStdString( gltfUpAxisString ) ) );
  return Qgis::Axis::Y;
}

QgsCesiumTiledSceneIndex::QgsCesiumTiledSceneIndex( const json &tileset, const QUrl &rootUrl, const QString &authCfg, const QgsHttpHeaders &headers, const QgsCoordinateTransformContext &transformContext )
  : mTransformContext( transformContext )
  , mAuthCfg( authCfg )
  , mHeaders( headers )
{
  Qgis::Axis gltfUpAxis = Qgis::Axis::Y;
  if ( tileset.contains( "asset" ) )
  {
    const auto &assetJson = tileset["asset"];
    if ( assetJson.contains( "gltfUpAxis" ) )
    {
      gltfUpAxis = axisFromJson( assetJson["gltfUpAxis"] );
    }
  }

  mRootNode.reset( nodeFromJson( tileset[ "root" ], rootUrl, nullptr, gltfUpAxis ) );
}

std::unique_ptr< QgsTiledSceneTile > QgsCesiumTiledSceneIndex::tileFromJson( const json &json, const QUrl &baseUrl, const QgsTiledSceneTile *parent, Qgis::Axis gltfUpAxis )
{
  std::unique_ptr< QgsTiledSceneTile > tile = std::make_unique< QgsTiledSceneTile >( mNextTileId++ );

  tile->setBaseUrl( baseUrl );
  tile->setMetadata( {{ QStringLiteral( "gltfUpAxis" ), static_cast< int >( gltfUpAxis ) }} );

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
  QgsTiledSceneBoundingVolume volume;
  if ( boundingVolume.contains( "region" ) )
  {
    QgsBox3D rootRegion = QgsCesiumUtils::parseRegion( boundingVolume[ "region" ] );
    if ( !rootRegion.isNull() )
    {
      if ( rootRegion.width() > 20 || rootRegion.height() > 20 )
      {
        // treat very large regions as global -- these will not transform to EPSG:4978
      }
      else
      {
        // we need to transform regions from EPSG:4979 to EPSG:4978
        QVector< QgsVector3D > corners = rootRegion.corners();

        QVector< double > x;
        x.reserve( 8 );
        QVector< double > y;
        y.reserve( 8 );
        QVector< double > z;
        z.reserve( 8 );
        for ( int i = 0; i < 8; ++i )
        {
          const QgsVector3D &corner = corners[i];
          x.append( corner.x() );
          y.append( corner.y() );
          z.append( corner.z() );
        }
        QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) ),  QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) ), mTransformContext );
        ct.setBallparkTransformsAreAppropriate( true );
        try
        {
          ct.transformInPlace( x, y, z );
        }
        catch ( QgsCsException & )
        {
          QgsDebugError( QStringLiteral( "Cannot transform region bounding volume" ) );
        }

        const auto minMaxX = std::minmax_element( x.constBegin(), x.constEnd() );
        const auto minMaxY = std::minmax_element( y.constBegin(), y.constEnd() );
        const auto minMaxZ = std::minmax_element( z.constBegin(), z.constEnd() );
        volume = QgsTiledSceneBoundingVolume( QgsOrientedBox3D::fromBox3D( QgsBox3D( *minMaxX.first, *minMaxY.first, *minMaxZ.first, *minMaxX.second, *minMaxY.second, *minMaxZ.second ) ) );

        // note that matrix transforms are NOT applied to region bounding volumes!
      }
    }
  }
  else if ( boundingVolume.contains( "box" ) )
  {
    const QgsOrientedBox3D bbox = QgsCesiumUtils::parseBox( boundingVolume["box"] );
    if ( !bbox.isNull() )
    {
      volume = QgsTiledSceneBoundingVolume( bbox );
      if ( !transform.isIdentity() )
        volume.transform( transform );
    }
  }
  else if ( boundingVolume.contains( "sphere" ) )
  {
    QgsSphere sphere = QgsCesiumUtils::parseSphere( boundingVolume["sphere"] );
    if ( !sphere.isNull() )
    {
      sphere = QgsCesiumUtils::transformSphere( sphere, transform );
      volume = QgsTiledSceneBoundingVolume( QgsOrientedBox3D::fromBox3D( sphere.boundingBox() ) );
    }
  }
  else
  {
    QgsDebugError( QStringLiteral( "unsupported boundingVolume format" ) );
  }

  tile->setBoundingVolume( volume );

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
      QString relativeUri = QString::fromStdString( contentJson["uri"].get<std::string>() );
      contentUri = baseUrl.resolved( QUrl( relativeUri ) ).toString();

      if ( baseUrl.hasQuery() && QUrl( relativeUri ).isRelative() )
        contentUri = appendQueryFromBaseUrl( contentUri, baseUrl );
    }
    else if ( contentJson.contains( "url" ) && !contentJson["url"].is_null() )
    {
      QString relativeUri = QString::fromStdString( contentJson["url"].get<std::string>() );
      contentUri = baseUrl.resolved( QUrl( relativeUri ) ).toString();

      if ( baseUrl.hasQuery() && QUrl( relativeUri ).isRelative() )
        contentUri = appendQueryFromBaseUrl( contentUri, baseUrl );
    }
    if ( !contentUri.isEmpty() )
    {
      tile->setResources( {{ QStringLiteral( "content" ), contentUri } } );
    }
  }

  return tile;
}

QgsTiledSceneNode *QgsCesiumTiledSceneIndex::nodeFromJson( const json &json, const QUrl &baseUrl, QgsTiledSceneNode *parent, Qgis::Axis gltfUpAxis )
{
  std::unique_ptr< QgsTiledSceneTile > tile = tileFromJson( json, baseUrl, parent ? parent->tile() : nullptr, gltfUpAxis );
  std::unique_ptr< QgsTiledSceneNode > newNode = std::make_unique< QgsTiledSceneNode >( tile.release() );
  mNodeMap.insert( newNode->tile()->id(), newNode.get() );

  if ( parent )
    parent->addChild( newNode.get() );

  if ( json.contains( "children" ) )
  {
    for ( const auto &childJson : json["children"] )
    {
      nodeFromJson( childJson, baseUrl, newNode.get(), gltfUpAxis );
    }
  }

  return newNode.release();
}

void QgsCesiumTiledSceneIndex::refineNodeFromJson( QgsTiledSceneNode *node, const QUrl &baseUrl, const json &json )
{
  const auto &rootTileJson = json["root"];

  Qgis::Axis gltfUpAxis = Qgis::Axis::Y;
  if ( json.contains( "asset" ) )
  {
    const auto &assetJson = json["asset"];
    if ( assetJson.contains( "gltfUpAxis" ) )
    {
      gltfUpAxis = axisFromJson( assetJson["gltfUpAxis"] );
    }
  }

  std::unique_ptr< QgsTiledSceneTile > newTile = tileFromJson( rootTileJson, baseUrl, node->tile(), gltfUpAxis );
  // copy just the resources from the retrieved tileset to the refined node. We assume all the rest of the tile content
  // should be the same between the node being refined and the root node of the fetched sub dataset!
  // (Ie the bounding volume, geometric error, etc).
  node->tile()->setResources( newTile->resources() );


  // root tile of the sub dataset may have transform as well, we need to bring it back
  // (actually even the referencing tile may have transform - if that's the case,
  // that transform got combined with the root tile's transform in tileFromJson)
  if ( newTile->transform() )
    node->tile()->setTransform( *newTile->transform() );

  if ( rootTileJson.contains( "children" ) )
  {
    for ( const auto &childJson : rootTileJson["children"] )
    {
      nodeFromJson( childJson, baseUrl, node, gltfUpAxis );
    }
  }
}

QgsTiledSceneTile QgsCesiumTiledSceneIndex::rootTile() const
{
  QMutexLocker locker( &mLock );
  return mRootNode ? *mRootNode->tile() : QgsTiledSceneTile();
}

QgsTiledSceneTile QgsCesiumTiledSceneIndex::getTile( long long id )
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    return *( it.value()->tile() );
  }

  return QgsTiledSceneTile();
}

long long QgsCesiumTiledSceneIndex::parentTileId( long long id ) const
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

  return -1;
}

QVector< long long > QgsCesiumTiledSceneIndex::childTileIds( long long id ) const
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    QVector< long long > childIds;
    const QList< QgsTiledSceneNode * > children = it.value()->children();
    childIds.reserve( children.size() );
    for ( QgsTiledSceneNode *child : children )
    {
      childIds << child->tile()->id();
    }
    return childIds;
  }

  return {};
}

QVector< long long > QgsCesiumTiledSceneIndex::getTiles( const QgsTiledSceneRequest &request )
{
  QVector< long long > results;

  std::function< void( QgsTiledSceneNode * )> traverseNode;
  traverseNode = [&request, &traverseNode, &results, this]( QgsTiledSceneNode * node )
  {
    QgsTiledSceneTile *tile = node->tile();

    // check filter box first -- if the node doesn't intersect, then don't include the node and don't traverse
    // to its children
    if ( !request.filterBox().isNull() && !tile->boundingVolume().box().isNull() && !tile->boundingVolume().intersects( request.filterBox() ) )
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
  if ( request.parentTileId() < 0 )
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

Qgis::TileChildrenAvailability QgsCesiumTiledSceneIndex::childAvailability( long long id ) const
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
  const thread_local QRegularExpression isJsonRx( QStringLiteral( ".*\\.json(?:\\?.*)?$" ), QRegularExpression::PatternOption::CaseInsensitiveOption );
  if ( isJsonRx.match( contentUri ).hasMatch() )
    return Qgis::TileChildrenAvailability::NeedFetching;

  // things we know definitely CAN'T be a child tile map:
  const thread_local QRegularExpression antiCandidateRx( QStringLiteral( ".*\\.(gltf|glb|b3dm|i3dm|pnts|cmpt|bin|glbin|glbuf|png|jpeg|jpg)(?:\\?.*)?$" ), QRegularExpression::PatternOption::CaseInsensitiveOption );
  if ( antiCandidateRx.match( contentUri ).hasMatch() )
    return Qgis::TileChildrenAvailability::NoChildren;

  // here we **could** do a fetch to verify what the content actually is. But we want this method to be non-blocking,
  // so let's just report that there IS remote children available and then sort things out when we actually go to fetch those children...
  return Qgis::TileChildrenAvailability::NeedFetching;
}

bool QgsCesiumTiledSceneIndex::fetchHierarchy( long long id, QgsFeedback *feedback )
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
      refineNodeFromJson( it.value(), QUrl( contentUri ), subTileJson );
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
    // we got empty content, so the hierarchy content is probably missing,
    // so let's mark it as not JSON so that we do not try to fetch it again
    mTileContentFormats.insert( id, TileContentFormat::NotJson );
    return false;
  }
}

QByteArray QgsCesiumTiledSceneIndex::fetchContent( const QString &uri, QgsFeedback *feedback )
{
  QUrl url( uri );
  // TODO -- error reporting?
  if ( uri.startsWith( "http" ) )
  {
    QNetworkRequest networkRequest = QNetworkRequest( url );
    QgsSetRequestInitiatorClass( networkRequest, QStringLiteral( "QgsCesiumTiledSceneIndex" ) );
    networkRequest.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    networkRequest.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    mHeaders.updateNetworkRequest( networkRequest );

    if ( QThread::currentThread() == QApplication::instance()->thread() )
    {
      // running on main thread, use a blocking get to handle authcfg and SSL errors ok.
      const QgsNetworkReplyContent reply = QgsNetworkAccessManager::instance()->blockingGet(
                                             networkRequest, mAuthCfg, false, feedback );
      return reply.content();
    }
    else
    {
      // running on background thread, use tile download manager for efficient network handling
      if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( networkRequest, mAuthCfg ) )
      {
        // TODO -- report error
        return QByteArray();
      }
      std::unique_ptr< QgsTileDownloadManagerReply > reply( QgsApplication::tileDownloadManager()->get( networkRequest ) );

      QEventLoop loop;
      if ( feedback )
        QObject::connect( feedback, &QgsFeedback::canceled, &loop, &QEventLoop::quit );

      QObject::connect( reply.get(), &QgsTileDownloadManagerReply::finished, &loop, &QEventLoop::quit );
      loop.exec();

      return reply->data();
    }
  }
  else if ( url.isLocalFile() && QFile::exists( url.toLocalFile() ) )
  {
    QFile file( url.toLocalFile() );
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

void QgsCesiumTilesDataProviderSharedData::initialize( const QString &tileset, const QUrl &rootUrl, const QgsCoordinateTransformContext &transformContext, const QString &authCfg, const QgsHttpHeaders &headers )
{
  mTileset = json::parse( tileset.toStdString() );
  if ( !mTileset.contains( "root" ) )
  {
    mError = QObject::tr( "JSON is not a valid Cesium 3D Tiles source (does not contain \"root\" value)" );
    return;
  }

  mLayerMetadata.setType( QStringLiteral( "dataset" ) );

  if ( mTileset.contains( "asset" ) )
  {
    const auto &asset = mTileset[ "asset" ];
    if ( asset.contains( "tilesetVersion" ) )
    {
      try
      {
        const QString tilesetVersion = QString::fromStdString( asset["tilesetVersion"].get<std::string>() );
        mLayerMetadata.setIdentifier( tilesetVersion );
      }
      catch ( json::type_error & )
      {
        QgsDebugError( QStringLiteral( "Error when parsing tilesetVersion value" ) );
      }
    }
  }

  mIndex = QgsTiledSceneIndex(
             new QgsCesiumTiledSceneIndex(
               mTileset,
               rootUrl,
               authCfg,
               headers,
               transformContext
             )
           );

  // parse root
  {
    const auto &root = mTileset[ "root" ];
    // parse root bounding volume

    // TODO -- read crs from metadata tags. Need to find real world examples of this. And can metadata crs override
    // the EPSG:4979 requirement from a region bounding volume??

    {
      // TODO -- on some datasets there is a "boundingVolume" present on the tileset itself, i.e. not the root node.
      // what does this mean? Should we use it instead of the root node bounding volume if it's present?

      QgsLayerMetadata::SpatialExtent spatialExtent;

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
          mBoundingVolume = QgsTiledSceneBoundingVolume( QgsOrientedBox3D::fromBox3D( rootRegion ) );

          // only set z range for datasets which aren't too large (ie global datasets)
          if ( !mIndex.rootTile().boundingVolume().box().isNull() )
          {
            mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );
          }
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mSceneCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );

          mLayerMetadata.setCrs( mSceneCrs );
          mExtent = rootRegion.toRectangle();
          spatialExtent.extentCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          spatialExtent.bounds = rootRegion;
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
          mLayerMetadata.setCrs( mSceneCrs );

          mBoundingVolume = QgsTiledSceneBoundingVolume( bbox );
          mBoundingVolume.transform( rootTransform );
          try
          {
            QgsCoordinateTransform ct( mSceneCrs, mLayerCrs, transformContext );
            ct.setBallparkTransformsAreAppropriate( true );
            const QgsBox3D rootRegion = mBoundingVolume.bounds( ct );
            // only set z range for datasets which aren't too large (ie global datasets)
            if ( !mIndex.rootTile().boundingVolume().box().isNull() )
            {
              mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );
            }

            std::unique_ptr< QgsAbstractGeometry > extent2D( mBoundingVolume.as2DGeometry( ct ) );
            mExtent = extent2D->boundingBox();
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Caught transform exception when transforming boundingVolume" ) );
          }

          spatialExtent.extentCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );
          spatialExtent.bounds = mBoundingVolume.bounds();
        }
      }
      else if ( rootBoundingVolume.contains( "sphere" ) )
      {
        QgsSphere sphere = QgsCesiumUtils::parseSphere( rootBoundingVolume["sphere"] );
        if ( !sphere.isNull() )
        {
          // layer must advertise as EPSG:4979, as the various QgsMapLayer
          // methods which utilize QgsMapLayer::crs() (such as layer extent transformation)
          // are all purely 2D and can't handle the cesium data source z value
          // range in EPSG:4978 !
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mSceneCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );
          mLayerMetadata.setCrs( mSceneCrs );

          sphere = QgsCesiumUtils::transformSphere( sphere, rootTransform );

          mBoundingVolume = QgsTiledSceneBoundingVolume( QgsOrientedBox3D::fromBox3D( sphere.boundingBox() ) );
          try
          {
            QgsCoordinateTransform ct( mSceneCrs, mLayerCrs, transformContext );
            ct.setBallparkTransformsAreAppropriate( true );
            const QgsBox3D rootRegion = mBoundingVolume.bounds( ct );
            // only set z range for datasets which aren't too large (ie global datasets)
            if ( !mIndex.rootTile().boundingVolume().box().isNull() )
            {
              mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );
            }

            std::unique_ptr< QgsAbstractGeometry > extent2D( mBoundingVolume.as2DGeometry( ct ) );
            mExtent = extent2D->boundingBox();
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Caught transform exception when transforming boundingVolume" ) );
          }

          spatialExtent.extentCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );
          spatialExtent.bounds = mBoundingVolume.bounds();
        }
      }
      else
      {
        mError = QObject::tr( "JSON is not a valid Cesium 3D Tiles source (unsupported boundingVolume format)" );
        return;
      }

      QgsLayerMetadata::Extent layerExtent;
      layerExtent.setSpatialExtents( {spatialExtent } );
      mLayerMetadata.setExtent( layerExtent );
    }
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
{
  QgsReadWriteLocker locker( other.mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  mShared = other.mShared;
}

Qgis::DataProviderFlags QgsCesiumTilesDataProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

Qgis::TiledSceneProviderCapabilities QgsCesiumTilesDataProvider::capabilities() const
{
  return Qgis::TiledSceneProviderCapability::ReadLayerMetadata;
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

  QString tileSetUri;
  const QString uri = dataSourceUri();

  if ( uri.startsWith( QLatin1String( "ion://" ) ) )
  {
    QUrl url( uri );
    const QString assetId = QUrlQuery( url ).queryItemValue( QStringLiteral( "assetId" ) );
    const QString accessToken = QUrlQuery( url ).queryItemValue( QStringLiteral( "accessToken" ) );

    const QString CESIUM_ION_URL = QStringLiteral( "https://api.cesium.com/" );

    // get asset info
    {
      const QString assetInfoEndpoint = CESIUM_ION_URL + QStringLiteral( "v1/assets/%1" ).arg( assetId );
      QNetworkRequest request = QNetworkRequest( assetInfoEndpoint );
      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsCesiumTilesDataProvider" ) )
      mHeaders.updateNetworkRequest( request );
      if ( !accessToken.isEmpty() )
        request.setRawHeader( "Authorization", QStringLiteral( "Bearer %1" ).arg( accessToken ).toLocal8Bit() );

      QgsBlockingNetworkRequest networkRequest;
      if ( accessToken.isEmpty() )
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
      const json assetInfoJson  = json::parse( content.content().toStdString() );
      if ( assetInfoJson["type"] != "3DTILES" )
      {
        appendError( QgsErrorMessage( tr( "Only ion 3D Tiles content can be accessed, not %1" ).arg( QString::fromStdString( assetInfoJson["type"].get<std::string>() ) ) ) );
        return false;
      }

      mShared->mLayerMetadata.setTitle( QString::fromStdString( assetInfoJson["name"].get<std::string>() ) );
      mShared->mLayerMetadata.setAbstract( QString::fromStdString( assetInfoJson["description"].get<std::string>() ) );
      const QString attribution = QString::fromStdString( assetInfoJson["attribution"].get<std::string>() );
      if ( !attribution.isEmpty() )
        mShared->mLayerMetadata.setRights( { attribution } );

      mShared->mLayerMetadata.setDateTime( Qgis::MetadataDateType::Created, QDateTime::fromString( QString::fromStdString( assetInfoJson["dateAdded"].get<std::string>() ), Qt::DateFormat::ISODate ) );
    }

    // get tileset access details
    {
      const QString tileAccessEndpoint = CESIUM_ION_URL + QStringLiteral( "v1/assets/%1/endpoint" ).arg( assetId );
      QNetworkRequest request = QNetworkRequest( tileAccessEndpoint );
      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsCesiumTilesDataProvider" ) )
      mHeaders.updateNetworkRequest( request );
      if ( !accessToken.isEmpty() )
        request.setRawHeader( "Authorization", QStringLiteral( "Bearer %1" ).arg( accessToken ).toLocal8Bit() );

      QgsBlockingNetworkRequest networkRequest;
      if ( accessToken.isEmpty() )
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
      const json tileAccessJson = json::parse( content.content().toStdString() );

      if ( tileAccessJson.contains( "url" ) )
      {
        tileSetUri = QString::fromStdString( tileAccessJson["url"].get<std::string>() );
      }
      else if ( tileAccessJson.contains( "options" ) )
      {
        const auto &optionsJson = tileAccessJson["options"];
        if ( optionsJson.contains( "url" ) )
        {
          tileSetUri = QString::fromStdString( optionsJson["url"].get<std::string>() );
        }
      }

      if ( tileAccessJson.contains( "accessToken" ) )
      {
        // The tileset accessToken is NOT the same as the token we use to access the asset details -- ie we can't
        // use the same authentication as we got from the providers auth cfg!
        mHeaders.insert( QStringLiteral( "Authorization" ),
                         QStringLiteral( "Bearer %1" ).arg( QString::fromStdString( tileAccessJson["accessToken"].get<std::string>() ) ) );
      }
      mAuthCfg.clear();
    }
  }
  else
  {
    QgsDataSourceUri dsUri;
    dsUri.setEncodedUri( uri );
    tileSetUri = dsUri.param( QStringLiteral( "url" ) );
  }

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

    mShared->initialize( content.content(), tileSetUri, transformContext(), mAuthCfg, mHeaders );

    mShared->mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), QStringLiteral( "WWW:LINK" ), tileSetUri ) );
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
        mShared->initialize( raw, QUrl::fromLocalFile( dataSourceUri() ), transformContext(), mAuthCfg, mHeaders );
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

  if ( !mShared->mIndex.isValid() )
  {
    appendError( mShared->mError );
    return false;
  }
  return true;
}

QgsCoordinateReferenceSystem QgsCesiumTilesDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mLayerCrs;
}

QgsRectangle QgsCesiumTilesDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
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

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
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
      try
      {
        const QString tilesetVersion = QString::fromStdString( asset["tilesetVersion"].get<std::string>() );
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( tilesetVersion ) % QStringLiteral( "</td></tr>\n" );
      }
      catch ( json::type_error & )
      {
        QgsDebugError( QStringLiteral( "Error when parsing tilesetVersion value" ) );
      }
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
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Extensions Required" ) % QStringLiteral( "</td><td><ul><li>%1</li></ul></a>" ).arg( extensions.join( QLatin1String( "</li><li>" ) ) ) % QStringLiteral( "</td></tr>\n" );
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
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Extensions Used" ) % QStringLiteral( "</td><td><ul><li>%1</li></ul></a>" ).arg( extensions.join( QLatin1String( "</li><li>" ) ) ) % QStringLiteral( "</td></tr>\n" );
    }
  }

  if ( !mShared->mZRange.isInfinite() )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Z Range" ) % QStringLiteral( "</td><td>%1 - %2</a>" ).arg( QLocale().toString( mShared->mZRange.lower() ), QLocale().toString( mShared->mZRange.upper() ) ) % QStringLiteral( "</td></tr>\n" );
  }

  return metadata;
}

QgsLayerMetadata QgsCesiumTilesDataProvider::layerMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsLayerMetadata();

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mLayerMetadata;
}

const QgsCoordinateReferenceSystem QgsCesiumTilesDataProvider::sceneCrs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsCoordinateReferenceSystem();

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mSceneCrs ;
}

const QgsTiledSceneBoundingVolume &QgsCesiumTilesDataProvider::boundingVolume() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  static QgsTiledSceneBoundingVolume nullVolume;
  if ( !mShared )
    return nullVolume;

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared ? mShared->mBoundingVolume : nullVolume;
}

QgsTiledSceneIndex QgsCesiumTilesDataProvider::index() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsTiledSceneIndex( nullptr );

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mIndex;
}

QgsDoubleRange QgsCesiumTilesDataProvider::zRange() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsDoubleRange();

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mZRange;
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
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconCesium3dTiles.svg" ) );
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
