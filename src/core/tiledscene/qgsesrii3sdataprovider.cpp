/***************************************************************************
  qgsesrii3sdataprovider.cpp
  --------------------------------------
  Date                 : July 2025
  Copyright            : (C) 2025 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsesrii3sdataprovider.h"
#include "moc_qgsesrii3sdataprovider.cpp"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsreadwritelocker.h"
#include "qgsthreadingutils.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsziputils.h"

#include "qgstiledsceneboundingvolume.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenerequest.h"
#include "qgstiledscenetile.h"
#include "qgsproviderutils.h"
#include "qgsprovidersublayerdetails.h"

#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QQuaternion>

#include <nlohmann/json.hpp>


#define I3S_PROVIDER_KEY QStringLiteral( "esrii3s" )
#define I3S_PROVIDER_DESCRIPTION QStringLiteral( "ESRI I3S data provider" )


///@cond PRIVATE

/**
 * \ingroup core
 * Tiled scene index implementation for I3S
 * \since QGIS 4.0
 */
class QgsEsriI3STiledSceneIndex final : public QgsAbstractTiledSceneIndex
{
  public:

    QgsEsriI3STiledSceneIndex(
      const json &tileset,
      const QUrl &rootUrl,
      const QgsCoordinateTransformContext &transformContext );

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

    bool fetchNodePage( int nodePage, QgsFeedback *feedback = nullptr );
    void parseNodePage( const QByteArray &nodePageContent );
    void parseMesh( QgsTiledSceneTile &t, const json &meshJson );
    QVariantMap parseMaterialDefinition( const json &materialDefinitionJson );

    struct NodeDetails
    {
      long long parentNodeIndex;
      QVector<long long> childNodeIndexes;
      QgsTiledSceneTile tile;
    };

    QVector<QString> mTextureSetFormats;
    QVector<QVariantMap> mMaterialDefinitions;

    mutable QRecursiveMutex mLock;
    QUrl mRootUrl;
    QgsCoordinateTransformContext mTransformContext;
    long long mRootNodeIndex;
    int mNodesPerPage;
    bool mGlobalMode = false;
    QMap< long long, NodeDetails > mNodeMap;
    QSet<int> mCachedNodePages;

};



/**
 * \ingroup core
 * Data to be shared between multiple copies of the I3S data provider.
 * As the class may be used from multiple threads, members should be
 * accessed only while the read/write lock is locked.
 * \since QGIS 4.0
 */
class QgsEsriI3SDataProviderSharedData
{
  public:
    QgsEsriI3SDataProviderSharedData();
    void initialize( const QString &i3sVersion,
                     const json &layerJson,
                     const QUrl &rootUrl,
                     const QgsCoordinateTransformContext &transformContext );

    QString mI3sVersion;
    json mLayerJson;

    QgsCoordinateReferenceSystem mLayerCrs;
    QgsCoordinateReferenceSystem mSceneCrs;
    QgsTiledSceneBoundingVolume mBoundingVolume;

    QgsRectangle mExtent;
    QgsDoubleRange mZRange;

    QgsTiledSceneIndex mIndex;

    QString mError;
    QReadWriteLock mReadWriteLock;

};

//
// QgsEsriI3STiledSceneIndex
//

QgsEsriI3STiledSceneIndex::QgsEsriI3STiledSceneIndex(
  const json &layerJson,
  const QUrl &rootUrl,
  const QgsCoordinateTransformContext &transformContext )
  : mRootUrl( rootUrl )
  , mTransformContext( transformContext )
{
  // "spatialReference" is not required, yet the spec does not say what should
    // be the default - assuming global mode is the best we can do...
  mGlobalMode = true;
  if ( layerJson.contains( "spatialReference") && layerJson["spatialReference"].is_object() )
  {
    const json spatialReferenceJson = layerJson["spatialReference"];
    if ( spatialReferenceJson.contains( "latestWkid" ) && spatialReferenceJson["latestWkid"].is_number() )
    {
      int epsgCode = spatialReferenceJson["latestWkid"].get<int>();
      mGlobalMode = epsgCode == 4326;
    }
    else if ( spatialReferenceJson.contains( "wkid" ) && spatialReferenceJson["wkid"].is_number() )
    {
      int epsgCode = spatialReferenceJson["wkid"].get<int>();
      mGlobalMode = epsgCode == 4326;
    }
  }

  if ( layerJson.contains( "textureSetDefinitions" ) )
  {
    for ( auto textureSetDefinitionJson : layerJson["textureSetDefinitions"] )
    {
      QString formatType;
      for ( const json &formatJson : textureSetDefinitionJson["formats"] )
      {
        if ( formatJson["name"].get<std::string>() == "0" )
        {
          formatType = QString::fromStdString( formatJson["format"].get<std::string>() );
          break;
        }
      }
      mTextureSetFormats.append( formatType );
    }
  }

  if ( layerJson.contains( "materialDefinitions" ) )
  {
    for ( const json &materialDefinitionJson : layerJson["materialDefinitions"] )
    {
      QVariantMap materialDef = parseMaterialDefinition( materialDefinitionJson );
      mMaterialDefinitions.append( materialDef );
    }
  }

  json nodePagesJson = layerJson["nodePages"];
  mNodesPerPage = nodePagesJson["nodesPerPage"].get<int>();
  mRootNodeIndex = nodePagesJson.contains( "rootIndex" ) ? nodePagesJson["rootIndex"].get<long long>() : 0;

  int rootNodePage = static_cast<int>( mRootNodeIndex / mNodesPerPage );
  fetchNodePage( rootNodePage );
}

QVariantMap QgsEsriI3STiledSceneIndex::parseMaterialDefinition( const json &materialDefinitionJson )
{
  QVariantMap materialDef;

  if ( materialDefinitionJson.contains( "pbrMetallicRoughness" ) )
  {
    const json pbrJson = materialDefinitionJson["pbrMetallicRoughness"];
    if ( pbrJson.contains( "baseColorFactor" ) )
    {
      const json pbrBaseColorFactorJson = pbrJson["baseColorFactor"];
      materialDef[QStringLiteral( "pbrBaseColorFactor" )] = QVariantList
      {
        pbrBaseColorFactorJson[0].get<double>(),
        pbrBaseColorFactorJson[1].get<double>(),
        pbrBaseColorFactorJson[2].get<double>(),
        pbrBaseColorFactorJson[3].get<double>()
      };
    }
    else
    {
      materialDef[QStringLiteral( "pbrBaseColorFactor" )] = QVariantList{ 1.0, 1.0, 1.0, 1.0 };
    }
    if ( pbrJson.contains( "baseColorTexture" ) )
    {
      // but right now we only support png/jpg textures which have
      // hardcoded name "0" by the spec, and we use texture set definitions
      // only to figure out whether it is png or jpg
      const int textureSetDefinitionId = pbrJson["baseColorTexture"]["textureSetDefinitionId"].get<int>();
      if ( textureSetDefinitionId < mTextureSetFormats.count() )
      {
        materialDef[QStringLiteral( "pbrBaseColorTextureName" )] = QStringLiteral( "0" );
        materialDef[QStringLiteral( "pbrBaseColorTextureFormat" )] = mTextureSetFormats[textureSetDefinitionId];
      }
      else
      {
        QgsDebugError( QString( "referencing textureSetDefinition that does not exist! %1 " ).arg( textureSetDefinitionId ) );
      }
    }
  }
  else
  {
    materialDef[QStringLiteral( "pbrBaseColorFactor" )] = QVariantList{ 1.0, 1.0, 1.0, 1.0 };
  }

  if ( materialDefinitionJson.contains( "doubleSided" ) )
  {
    materialDef[QStringLiteral( "doubleSided" )] = materialDefinitionJson["doubleSided"].get<bool>();
  }

  // there are various other properties that can be defined in a material,
  // but we do not support them: normal texture, occlusion texture, emissive texture,
  // emissive factor, alpha mode, alpha cutoff, cull face.

  return materialDef;
}

QgsTiledSceneTile QgsEsriI3STiledSceneIndex::rootTile() const
{
  QMutexLocker locker( &mLock );
  if ( !mNodeMap.contains( mRootNodeIndex ) )
  {
    QgsDebugError( QStringLiteral( "Unable to access the root tile!" ) );
    return QgsTiledSceneTile();
  }
  return mNodeMap[mRootNodeIndex].tile;
}

QgsTiledSceneTile QgsEsriI3STiledSceneIndex::getTile( long long id )
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    return it.value().tile;
  }

  return QgsTiledSceneTile();
}

long long QgsEsriI3STiledSceneIndex::parentTileId( long long id ) const
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    return it.value().parentNodeIndex;
  }

  return -1;
}

QVector< long long > QgsEsriI3STiledSceneIndex::childTileIds( long long id ) const
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it != mNodeMap.constEnd() )
  {
    return it.value().childNodeIndexes;
  }

  return {};
}

QVector< long long > QgsEsriI3STiledSceneIndex::getTiles( const QgsTiledSceneRequest &request )
{
  QVector< long long > results;

  std::function< void( long long )> traverseNode;
  traverseNode = [&request, &traverseNode, &results, this]( long long nodeId )
  {
    QgsTiledSceneTile t = getTile( nodeId );
    if ( !request.filterBox().isNull() && !t.boundingVolume().intersects( request.filterBox() ) )
      return;

    if ( request.requiredGeometricError() <= 0 || t.geometricError() <= 0 || t.geometricError() > request.requiredGeometricError() )
    {
      // need to go deeper, this tile does not have enough details

      if ( childAvailability( t.id() ) == Qgis::TileChildrenAvailability::NeedFetching &&
           !( request.flags() & Qgis::TiledSceneRequestFlag::NoHierarchyFetch ) )
      {
        fetchHierarchy( t.id() );
      }

      // now we should have children available (if any)
      auto it = mNodeMap.constFind( t.id() );
      for ( long long childId : it.value().childNodeIndexes )
      {
        traverseNode( childId );
      }

      if ( it.value().childNodeIndexes.isEmpty() )
      {
        // there are no children available, so we use this tile even though we want more detail
        results << t.id();
      }
    }
    else
    {
      // this tile has error sufficiently low so that we do not need to traverse
      // the node tree further down
      results << t.id();
    }
  };

  QMutexLocker locker( &mLock );
  long long startNodeId = request.parentTileId() == -1 ? mRootNodeIndex : request.parentTileId();
  traverseNode( startNodeId );

  return results;
}

Qgis::TileChildrenAvailability QgsEsriI3STiledSceneIndex::childAvailability( long long id ) const
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it == mNodeMap.constEnd() )
  {
    // we have no info about the node, so what we return is a bit arbitrary anyway
    return Qgis::TileChildrenAvailability::NoChildren;
  }

  if ( it.value().childNodeIndexes.isEmpty() )
  {
    return Qgis::TileChildrenAvailability::NoChildren;
  }

  for ( long long childId : it.value().childNodeIndexes )
  {
    if ( !mNodeMap.contains( childId ) )
    {
      // at least one child is missing from the node map
      return Qgis::TileChildrenAvailability::NeedFetching;
    }
  }
  return Qgis::TileChildrenAvailability::Available;
}

bool QgsEsriI3STiledSceneIndex::fetchHierarchy( long long id, QgsFeedback *feedback )
{
  QMutexLocker locker( &mLock );
  auto it = mNodeMap.constFind( id );
  if ( it == mNodeMap.constEnd() )
    return false;

  // gather all the missing node pages to get information about child nodes
  QSet<int> nodePagesToFetch;
  for ( long long childId : it.value().childNodeIndexes )
  {
    int nodePageIndex = static_cast<int>( childId / mNodesPerPage );
    if ( !mCachedNodePages.contains( nodePageIndex ) )
      nodePagesToFetch.insert( nodePageIndex );
  }

  bool success = true;
  for ( int nodePage : nodePagesToFetch )
  {
    if ( !fetchNodePage( nodePage, feedback ) )
    {
      success = false;
    }
  }

  return success;
}

QByteArray QgsEsriI3STiledSceneIndex::fetchContent( const QString &uri, QgsFeedback *feedback )
{
  QUrl url( uri );
  if ( url.isLocalFile() )
  {
    const QString slpkPath = mRootUrl.toLocalFile();
    if ( QFileInfo( slpkPath ).suffix().compare( QLatin1String( "slpk" ), Qt::CaseInsensitive ) == 0 )
    {
      const QString fileInSlpk = uri.mid( mRootUrl.toString().length() + 1 );

      QByteArray data;
      if ( QgsZipUtils::getFileFromZip( slpkPath, fileInSlpk, data ) )
        return data;

      return QByteArray();
    }
  }
  else
  {
    QNetworkRequest networkRequest = QNetworkRequest( url );
    QgsSetRequestInitiatorClass( networkRequest, QStringLiteral( "QgsEsriI3STiledSceneIndex" ) );
    networkRequest.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    networkRequest.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    const QgsNetworkReplyContent reply = QgsNetworkAccessManager::instance()->blockingGet(
                                           networkRequest, QString(), false, feedback );
    return reply.content();
  }

  return QByteArray();
}

bool QgsEsriI3STiledSceneIndex::fetchNodePage( int nodePage, QgsFeedback *feedback )
{
  QByteArray nodePageContent;
  if ( !mRootUrl.isLocalFile() )
  {
    const QString uri = QStringLiteral( "%1/layers/0/nodepages/%2" ).arg( mRootUrl.toString() ).arg( nodePage );
    nodePageContent = fetchContent( uri, feedback );
  }
  else
  {
    const QString uri = QStringLiteral( "%1/nodepages/%2.json.gz" ).arg( mRootUrl.toString() ).arg( nodePage );
    nodePageContent = fetchContent( uri, feedback );

    if ( nodePageContent.isEmpty() )
    {
      QgsDebugError( QStringLiteral( "Failed to read node page content: " ) + uri );
      return false;
    }
  }

  try
  {
    parseNodePage( nodePageContent );
  }
  catch ( json::exception &error )
  {
    QgsDebugError( QStringLiteral( "Error reading node page %1: %2" ).arg( nodePage ).arg( error.what() ) );
    return false;
  }

  mCachedNodePages.insert( nodePage );
  return true;
}

static QgsOrientedBox3D parseBox( const json &box )
{
  try
  {
    json center = box["center"];
    json halfSize = box["halfSize"];
    json quaternion = box["quaternion"];  // order is x, y, z, w

    return QgsOrientedBox3D(
             QgsVector3D( center[0].get<double>(),
                          center[1].get<double>(),
                          center[2].get<double>() ),
             QgsVector3D( halfSize[0].get<double>(),
                          halfSize[1].get<double>(),
                          halfSize[2].get<double>() ),
             QQuaternion( static_cast<float>( quaternion[3].get<double>() ),
                          static_cast<float>( quaternion[0].get<double>() ),
                          static_cast<float>( quaternion[1].get<double>() ),
                          static_cast<float>( quaternion[2].get<double>() ) ) );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsOrientedBox3D();
  }
}

void QgsEsriI3STiledSceneIndex::parseMesh( QgsTiledSceneTile &t, const json &meshJson )
{
  if ( !meshJson.contains( "geometry" ) || !meshJson.contains( "material" ) )
    return;

  int geometryResource = meshJson["geometry"]["resource"].get<int>();
  QString geometryUri;
  if ( mRootUrl.isLocalFile() )
    geometryUri = QStringLiteral( "%1/nodes/%2/geometries/1.bin.gz" ).arg( mRootUrl.toString() ).arg( geometryResource );
  else
    geometryUri = QStringLiteral( "%1/layers/0/nodes/%2/geometries/1" ).arg( mRootUrl.toString() ).arg( geometryResource );

  // parse material and related textures
  const json materialJson = meshJson["material"];
  int materialIndex = materialJson["definition"].get<int>();
  QVariantMap materialInfo;
  if ( materialIndex >= 0 && materialIndex < mMaterialDefinitions.count() )
  {
    materialInfo = mMaterialDefinitions[materialIndex];
    if ( materialInfo.contains( QStringLiteral( "pbrBaseColorTextureName" ) ) )
    {
      const QString textureName = materialInfo[QStringLiteral( "pbrBaseColorTextureName" )].toString();
      const QString textureFormat = materialInfo[QStringLiteral( "pbrBaseColorTextureFormat" )].toString();
      materialInfo.remove( QStringLiteral( "pbrBaseColorTextureName" ) );
      materialInfo.remove( QStringLiteral( "pbrBaseColorTextureFormat" ) );

      const int textureResource = materialJson["resource"].get<int>();
      QString textureUri;
      if ( mRootUrl.isLocalFile() )
        textureUri = QStringLiteral( "%1/nodes/%2/textures/%3.%4" ).arg( mRootUrl.toString() ).arg( textureResource ).arg( textureName, textureFormat );
      else
        textureUri = QStringLiteral( "%1/layers/0/nodes/%2/textures/%3" ).arg( mRootUrl.toString() ).arg( textureResource ).arg( textureName );
      materialInfo[QStringLiteral( "pbrBaseColorTexture" )] = textureUri;
    }
  }

  t.setResources( { { QStringLiteral( "content" ), geometryUri } } );

  QVariantMap metadata =
  {
    { QStringLiteral( "gltfUpAxis" ), static_cast< int >( Qgis::Axis::Z ) },
    { QStringLiteral( "contentFormat" ), QStringLiteral( "draco" ) },
    { QStringLiteral( "material" ), materialInfo }
  };
  t.setMetadata( metadata );
}

void QgsEsriI3STiledSceneIndex::parseNodePage( const QByteArray &nodePageContent )
{
  const json nodePageJson = json::parse( nodePageContent.toStdString() );
  for ( const json &nodeJson : nodePageJson["nodes"] )
  {
    long long nodeIndex = nodeJson["index"].get<long long>();
    long long parentNodeIndex = nodeJson.contains( "parentIndex" ) ? nodeJson["parentIndex"].get<long long>() : -1;

    QgsOrientedBox3D obb = parseBox( nodeJson["obb"] );

    // OBB in global scene layers should be constructed in ECEF and its values are defined like this:
    // - center - X,Y - lon/lat in degrees, Z - elevation in meters
    // - half-size - in meters
    // - quaternion - in reference to ECEF coordinate system

    // OBB in local scene layers should be constructed in the CRS of the layer
    // - center, half-size - in units of the CRS
    // - quaternion - in reference to CRS of the layer

    if ( mGlobalMode )
    {
      QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) ),
                                 QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) ),
                                 mTransformContext );
      QgsVector3D obbCenterEcef = ct.transform( obb.center() );
      obb = QgsOrientedBox3D( { obbCenterEcef.x(), obbCenterEcef.y(), obbCenterEcef.z() }, obb.halfAxesList() );
    }

    double threshold = -1;
    if ( nodeJson.contains( "lodThreshold" ) )
    {
      double maxScreenThresholdSquared = nodeJson["lodThreshold"].get<double>();

      // This conversion from "maxScreenThresholdSQ" to geometry error is copied from CesiumJS
      // implementation of I3S (the only difference is Cesium uses longest OBB axis length
      threshold = obb.longestSide() / sqrt( maxScreenThresholdSquared / ( M_PI / 4 ) ) * 16;
    }
    QVector<long long> childNodeIds;
    if ( nodeJson.contains( "children" ) )
    {
      for ( const json &childJson : nodeJson["children"] )
      {
        childNodeIds << childJson.get<long long>();
      }
    }

    QgsTiledSceneTile t( nodeIndex );
    t.setBoundingVolume( obb );
    t.setGeometricError( threshold );

    QgsMatrix4x4 transform;
    transform.translate( obb.center() );
    t.setTransform( transform );

    if ( nodeJson.contains( "mesh" ) )
    {
      // parse geometry and material
      parseMesh( t, nodeJson["mesh"] );
    }

    mNodeMap.insert( nodeIndex, { parentNodeIndex, childNodeIds, t } );
  }
}


//
// QgsEsriI3SDataProviderSharedData
//

QgsEsriI3SDataProviderSharedData::QgsEsriI3SDataProviderSharedData()
  : mIndex( QgsTiledSceneIndex( nullptr ) )
{
}

void QgsEsriI3SDataProviderSharedData::initialize(
  const QString &i3sVersion,
  const json &layerJson,
  const QUrl &rootUrl,
  const QgsCoordinateTransformContext &transformContext )
{
  mI3sVersion = i3sVersion;
  mLayerJson = layerJson;

  int epsgCode = 0;

  // ESRI spatialReference https://developers.arcgis.com/web-map-specification/objects/spatialReference/
  // defines latestWkid as optional, wkid as mandatory
  // so we first check for latestWkid and use wkid as fallback
  if ( layerJson.contains( "spatialReference" ) && layerJson["spatialReference"].is_object() )
  {
    const json spatialReferenceJson = layerJson["spatialReference"];
    if ( spatialReferenceJson.contains( "latestWkid" ) && spatialReferenceJson["latestWkid"].is_number() )
    {
      epsgCode = spatialReferenceJson["latestWkid"].get<int>();
    }
    else if ( spatialReferenceJson.contains( "wkid" ) && spatialReferenceJson["wkid"].is_number() )
    {
      epsgCode = spatialReferenceJson["wkid"].get<int>();
    }
  }

  if ( epsgCode == 4326 )
  {
    // "global" mode

    // TODO: elevation can be ellipsoidal or gravity-based!
    mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
    mSceneCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );
  }
  else
  {
    // "local" mode - using a projected CRS
    mLayerCrs = QgsCoordinateReferenceSystem( QString( "EPSG:%1" ).arg( epsgCode ) );
    mSceneCrs = mLayerCrs;
  }

  mIndex = QgsTiledSceneIndex(
             new QgsEsriI3STiledSceneIndex(
               layerJson,
               rootUrl,
               transformContext
             )
           );

  if ( layerJson.contains( "fullExtent" ) )
  {
    const json fullExtentJson = layerJson["fullExtent"];
    mExtent = QgsRectangle(
                fullExtentJson["xmin"].get<double>(),
                fullExtentJson["ymin"].get<double>(),
                fullExtentJson["xmax"].get<double>(),
                fullExtentJson["ymax"].get<double>() );
    mZRange = QgsDoubleRange(
                fullExtentJson["zmin"].get<double>(),
                fullExtentJson["zmax"].get<double>() );
  }
  else
  {
    QgsBox3D box = mIndex.rootTile().boundingVolume().bounds( QgsCoordinateTransform( mSceneCrs, mLayerCrs, transformContext ) );
    mExtent = box.toRectangle();
    mZRange = QgsDoubleRange( box.zMinimum(), box.zMaximum() );
  }

  mBoundingVolume = mIndex.rootTile().boundingVolume();
}

//
// QgsEsriI3SDataProvider
//


QgsEsriI3SDataProvider::QgsEsriI3SDataProvider( const QString &uri,
    const QgsDataProvider::ProviderOptions &providerOptions,
    Qgis::DataProviderReadFlags flags )
  : QgsTiledSceneDataProvider( uri, providerOptions, flags )
  , mShared( std::make_shared< QgsEsriI3SDataProviderSharedData >() )
{
  QgsDataSourceUri dataSource( dataSourceUri() );
  QString sourcePath = dataSource.param( QStringLiteral( "url" ) );

  if ( sourcePath.isEmpty() )
  {
    sourcePath = uri;
  }

  QUrl rootUrl;
  if ( sourcePath.startsWith( QLatin1String( "http" ) ) || sourcePath.startsWith( QLatin1String( "file" ) ) )
  {
    rootUrl = sourcePath;
  }
  else
  {
    // when saved in project as relative path, we then get just the path... (TODO?)
    rootUrl = QUrl::fromLocalFile( sourcePath );
  }

  QString i3sVersion;
  json layerJson;
  if ( sourcePath.startsWith( QLatin1String( "http" ) ) )
  {
    if ( !loadFromRestService( sourcePath, layerJson, i3sVersion ) )
      return;
  }
  else
  {
    if ( !loadFromSlpk( sourcePath, layerJson, i3sVersion ) )
      return;
  }

  if ( !layerJson.contains( "layerType" ) )
  {
    appendError( QgsErrorMessage( tr( "Invalid I3S source: missing layer type." ), QStringLiteral( "I3S" ) ) );
    return;
  }

  if ( !layerJson.contains( "nodePages" ) )
  {
    appendError( QgsErrorMessage( tr( "Missing 'nodePages' attribute (should be available in I3S >= 1.7)" ), QStringLiteral( "I3S" ) ) );
    return;
  }

  QString layerType = QString::fromStdString( layerJson["layerType"].get<std::string>() );
  if ( layerType != QLatin1String( "3DObject" ) && layerType != QLatin1String( "IntegratedMesh" ) )
  {
    appendError( QgsErrorMessage( tr( "Unsupported layer type: " ) + layerType, QStringLiteral( "I3S" ) ) );
    return;
  }

  mShared->initialize( i3sVersion, layerJson, rootUrl, transformContext() );

  if ( !mShared->mIndex.isValid() )
  {
    appendError( mShared->mError );
    return;
  }

  mIsValid = true;
}


bool QgsEsriI3SDataProvider::loadFromRestService( const QString &uri, json &layerJson, QString &i3sVersion )
{
  QNetworkRequest networkRequest = QNetworkRequest( QUrl( uri ) );
  QgsSetRequestInitiatorClass( networkRequest, QStringLiteral( "QgsEsriI3SDataProvider" ) );
  networkRequest.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  networkRequest.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  const QgsNetworkReplyContent reply = QgsNetworkAccessManager::instance()->blockingGet( networkRequest );
  if ( reply.error() != QNetworkReply::NoError )
  {
    appendError( QgsErrorMessage( tr( "Failed to fetch layer metadata: " ) + networkRequest.url().toString(), QStringLiteral( "I3S" ) ) );
    return false;
  }
  QByteArray sceneLayerContent = reply.content();

  json serviceJson;
  try
  {
    serviceJson = json::parse( sceneLayerContent.toStdString() );
  }
  catch ( const json::parse_error & )
  {
    appendError( QgsErrorMessage( tr( "Unable to parse JSON: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }

  if ( !serviceJson.contains( "serviceVersion" ) )
  {
    appendError( QgsErrorMessage( tr( "Missing I3S version: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }
  i3sVersion = QString::fromStdString( serviceJson["serviceVersion"].get<std::string>() );
  if ( !checkI3SVersion( i3sVersion ) )
    return false;

  if ( !serviceJson.contains( "layers" ) || !serviceJson["layers"].is_array() || serviceJson["layers"].size() < 1 )
  {
    appendError( QgsErrorMessage( tr( "Unable to get layer info: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }

  layerJson = serviceJson["layers"][0];
  return true;
}

bool QgsEsriI3SDataProvider::loadFromSlpk( const QString &uri, json &layerJson, QString &i3sVersion )
{
  QByteArray metadataContent;
  QString metadataPath = QStringLiteral( "metadata.json" );
  if ( !QgsZipUtils::getFileFromZip( uri, metadataPath, metadataContent ) )
  {
    appendError( QgsErrorMessage( tr( "Failed to read layer metadata: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }

  json metadataJson;
  try
  {
    metadataJson = json::parse( metadataContent.toStdString() );
  }
  catch ( const json::parse_error & )
  {
    appendError( QgsErrorMessage( tr( "Unable to parse metadata JSON: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }

  if ( !metadataJson.contains( "I3SVersion" ) )
  {
    appendError( QgsErrorMessage( tr( "Missing I3S version: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }
  i3sVersion = QString::fromStdString( metadataJson["I3SVersion"].get<std::string>() );
  if ( !checkI3SVersion( i3sVersion ) )
    return false;

  QByteArray sceneLayerContent;
  const QString sceneLayerContentPath = QStringLiteral( "3dSceneLayer.json.gz" );
  if ( !QgsZipUtils::getFileFromZip( uri, sceneLayerContentPath, sceneLayerContent ) )
  {
    appendError( QgsErrorMessage( tr( "Failed to decode layer metadata: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }

  try
  {
    layerJson = json::parse( sceneLayerContent.toStdString() );
  }
  catch ( const json::parse_error & )
  {
    appendError( QgsErrorMessage( tr( "Unable to parse JSON: " ) + uri, QStringLiteral( "I3S" ) ) );
    return false;
  }

  return true;
}

bool QgsEsriI3SDataProvider::checkI3SVersion( const QString &i3sVersion )
{
  // We support I3S version >= 1.7 released in 2019. Earlier versions
  // of the spec are much less efficient to work with (e.g. they do not
  // support node pages, no Draco compression of geometries)

  // Note: for more confusion, OGC has different versioning of I3S.
  // ESRI I3S version 1.7 should be equivalent to OGC I3S version 1.3.
  // Fortunately OGC versioning is not really used anywhere (apart from OGC docs)
  // so we can ignore OGC I3S versions and use ESRI I3S version.

  QStringList i3sVersionComponents = i3sVersion.split( '.' );
  if ( i3sVersionComponents.size() != 2 )
  {
    appendError( QgsErrorMessage( tr( "Unexpected I3S version format: " ) + i3sVersion, QStringLiteral( "I3S" ) ) );
    return false;
  }
  int i3sVersionMajor = i3sVersionComponents[0].toInt();
  int i3sVersionMinor = i3sVersionComponents[1].toInt();
  if ( i3sVersionMajor != 1 || ( i3sVersionMajor == 1 && i3sVersionMinor < 7 ) )
  {
    appendError( QgsErrorMessage( tr( "Unsupported I3S version: " ) + i3sVersion, QStringLiteral( "I3S" ) ) );
    return false;
  }
  return true;
}


QgsEsriI3SDataProvider::QgsEsriI3SDataProvider( const QgsEsriI3SDataProvider &other )
  : QgsTiledSceneDataProvider( other )
  , mIsValid( other.mIsValid )
{
  QgsReadWriteLocker locker( other.mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  mShared = other.mShared;
}

QgsEsriI3SDataProvider::~QgsEsriI3SDataProvider() = default;

Qgis::DataProviderFlags QgsEsriI3SDataProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

Qgis::TiledSceneProviderCapabilities QgsEsriI3SDataProvider::capabilities() const
{
  return Qgis::TiledSceneProviderCapabilities();
}

QgsEsriI3SDataProvider *QgsEsriI3SDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return new QgsEsriI3SDataProvider( *this );
}

QgsCoordinateReferenceSystem QgsEsriI3SDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mLayerCrs;
}

QgsRectangle QgsEsriI3SDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mExtent;
}

bool QgsEsriI3SDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QString QgsEsriI3SDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return I3S_PROVIDER_KEY;
}

QString QgsEsriI3SDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QObject::tr( "ESRI I3S" );
}

QString QgsEsriI3SDataProvider::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString metadata;

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "I3S Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( mShared->mI3sVersion ) % QStringLiteral( "</td></tr>\n" );

  QString layerType = QString::fromStdString( mShared->mLayerJson["layerType"].get<std::string>() );
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Layer Type" ) % QStringLiteral( "</td><td>%1</a>" ).arg( layerType ) % QStringLiteral( "</td></tr>\n" );

  if ( mShared->mLayerJson.contains( "version" ) )
  {
    // [required] "The ID of the last update session in which any resource belonging to this layer has been updated."
    // (even though marked as required, not all datasets provide it)
    QString version = QString::fromStdString( mShared->mLayerJson["version"].get<std::string>() );
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( version ) % QStringLiteral( "</td></tr>\n" );
  }

  // [optional] "The name of this layer."
  if ( mShared->mLayerJson.contains( "name" ) )
  {
    QString name = QString::fromStdString( mShared->mLayerJson["name"].get<std::string>() );
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Name" ) % QStringLiteral( "</td><td>%1</a>" ).arg( name ) % QStringLiteral( "</td></tr>\n" );
  }
  // [optional] "The display alias to be used for this layer."
  if ( mShared->mLayerJson.contains( "alias" ) )
  {
    QString alias = QString::fromStdString( mShared->mLayerJson["alias"].get<std::string>() );
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Alias" ) % QStringLiteral( "</td><td>%1</a>" ).arg( alias ) % QStringLiteral( "</td></tr>\n" );
  }
  // [optional] "Description string for this layer."
  if ( mShared->mLayerJson.contains( "description" ) )
  {
    QString description = QString::fromStdString( mShared->mLayerJson["description"].get<std::string>() );
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Description" ) % QStringLiteral( "</td><td>%1</a>" ).arg( description ) % QStringLiteral( "</td></tr>\n" );
  }

  if ( !mShared->mZRange.isInfinite() )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Z Range" ) % QStringLiteral( "</td><td>%1 - %2</a>" ).arg( QLocale().toString( mShared->mZRange.lower() ), QLocale().toString( mShared->mZRange.upper() ) ) % QStringLiteral( "</td></tr>\n" );
  }

  return metadata;
}

const QgsCoordinateReferenceSystem QgsEsriI3SDataProvider::sceneCrs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsCoordinateReferenceSystem();

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mSceneCrs;
}

const QgsTiledSceneBoundingVolume &QgsEsriI3SDataProvider::boundingVolume() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  static QgsTiledSceneBoundingVolume nullVolume;
  if ( !mShared )
    return nullVolume;

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared ? mShared->mBoundingVolume : nullVolume;
}

QgsTiledSceneIndex QgsEsriI3SDataProvider::index() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsTiledSceneIndex( nullptr );

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mIndex;
}

QgsDoubleRange QgsEsriI3SDataProvider::zRange() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mShared )
    return QgsDoubleRange();

  QgsReadWriteLocker locker( mShared->mReadWriteLock, QgsReadWriteLocker::Read );
  return mShared->mZRange;
}



//
// QgsEsriI3SProviderMetadata
//


QgsEsriI3SProviderMetadata::QgsEsriI3SProviderMetadata():
  QgsProviderMetadata( I3S_PROVIDER_KEY, I3S_PROVIDER_DESCRIPTION )
{
}

QIcon QgsEsriI3SProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconEsriI3s.svg" ) );
}

QgsEsriI3SDataProvider *QgsEsriI3SProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsEsriI3SDataProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsEsriI3SProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
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

  if ( QFileInfo( fileName ).suffix().compare( QLatin1String( "slpk" ), Qt::CaseInsensitive ) == 0 )
  {
    QVariantMap parts;
    parts.insert( QStringLiteral( "path" ), fileName );

    QgsProviderSublayerDetails details;
    details.setUri( encodeUri( parts ) );
    details.setProviderKey( key() );
    details.setType( Qgis::LayerType::TiledScene );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( fileName ) );
    return {details};
  }
  return {};
}

QVariantMap QgsEsriI3SProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri( uri );

  QVariantMap uriComponents;
  QString path = dsUri.param( QStringLiteral( "url" ) );
  if ( path.isEmpty() && !uri.isEmpty() )
  {
    path = uri;
  }
  uriComponents.insert( QStringLiteral( "path" ), path );

  return uriComponents;
}

QList< Qgis::LayerType > QgsEsriI3SProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );

  if ( filePath.endsWith( QStringLiteral( ".slpk" ), Qt::CaseSensitivity::CaseInsensitive ) )
    return { Qgis::LayerType::TiledScene };

  return {};
}

QString QgsEsriI3SProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  const QString partsKey = parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( partsKey ).toString() );
  return dsUri.encodedUri();
}

QString QgsEsriI3SProviderMetadata::filters( Qgis::FileFilterType type )
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
      return QObject::tr( "ESRI Scene layer package" ) + QStringLiteral( " (*.slpk *.SLPK)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsEsriI3SProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsEsriI3SProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::TiledScene };
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsEsriI3SProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::PriorityForUri;
}

int QgsEsriI3SProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );

  if ( filePath.endsWith( QStringLiteral( ".slpk" ), Qt::CaseSensitivity::CaseInsensitive ) )
    return 100;

  return 0;
}

///@endcond
