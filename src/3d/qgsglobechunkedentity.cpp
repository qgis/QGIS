/***************************************************************************
  qgsglobechunkedentity.cpp
  --------------------------------------
  Date                 : March 2025
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

#include "qgsglobechunkedentity.h"

#include <memory>

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgseventtracing.h"
#include "qgsgeotransform.h"
#include "qgsglobematerial.h"
#include "qgsglobeutils_p.h"
#include "qgsmaterial3dhandler.h"
#include "qgsray3d.h"
#include "qgsraycastcontext.h"
#include "qgsraycastingutils.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintextureimage_p.h"

#include <QByteArray>
#include <QImage>
#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>

#include "moc_qgsglobechunkedentity.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

static Qt3DCore::QEntity *makeGlobeMesh(
  double lonMin,
  double lonMax,
  double latMin,
  double latMax,
  int lonSliceCount,
  int latSliceCount,
  const QgsCoordinateTransform &globeCrsToLatLon,
  QImage textureQImage,
  QString textureDebugText,
  const QgsMaterialContext &context
)
{
  double lonRange = lonMax - lonMin;
  double latRange = latMax - latMin;
  double lonStep = lonRange / ( double ) ( lonSliceCount - 1 );
  double latStep = latRange / ( double ) ( latSliceCount - 1 );

  std::vector<double> x, y, z;
  int pointCount = latSliceCount * lonSliceCount;
  x.reserve( pointCount );
  y.reserve( pointCount );
  z.reserve( pointCount );

  for ( int latSliceIndex = 0; latSliceIndex < latSliceCount; ++latSliceIndex )
  {
    double lat = latSliceIndex * latStep + latMin;
    for ( int lonSliceIndex = 0; lonSliceIndex < lonSliceCount; ++lonSliceIndex )
    {
      double lon = lonSliceIndex * lonStep + lonMin;
      x.push_back( lon );
      y.push_back( lat );
      z.push_back( 0 );
    }
  }

  globeCrsToLatLon.transformCoords( pointCount, x.data(), y.data(), z.data(), Qgis::TransformDirection::Reverse );

  // estimate origin of coordinates for this tile, to make the relative coordinates
  // small to avoid numerical precision issues when rendering
  // (avoids mesh jumping around when zoomed in very close to it)
  QgsVector3D meshOriginLatLon( ( lonMin + lonMax ) / 2, ( latMin + latMax ) / 2, 0 );
  QgsVector3D meshOrigin = globeCrsToLatLon.transform( meshOriginLatLon, Qgis::TransformDirection::Reverse );

  int stride = ( 3 + 2 + 3 ) * sizeof( float );

  QByteArray bufferBytes;
  bufferBytes.resize( stride * pointCount );
  float *fptr = ( float * ) bufferBytes.data();
  for ( int i = 0; i < ( int ) pointCount; ++i )
  {
    *fptr++ = static_cast<float>( x[i] - meshOrigin.x() );
    *fptr++ = static_cast<float>( y[i] - meshOrigin.y() );
    *fptr++ = static_cast<float>( z[i] - meshOrigin.z() );

    int vi = i / lonSliceCount;
    int ui = i % lonSliceCount;
    float v = static_cast<float>( vi ) / static_cast<float>( latSliceCount - 1 );
    float u = static_cast<float>( ui ) / static_cast<float>( lonSliceCount - 1 );
    *fptr++ = u;
    *fptr++ = 1 - v;

    QVector3D n = QVector3D( static_cast<float>( x[i] ), static_cast<float>( y[i] ), static_cast<float>( z[i] ) ).normalized();
    *fptr++ = n.x();
    *fptr++ = n.y();
    *fptr++ = n.z();
  }

  int faces = ( lonSliceCount - 1 ) * ( latSliceCount - 1 ) * 2;
  int indices = faces * 3;

  QByteArray indexBytes;
  indexBytes.resize( indices * static_cast<int>( sizeof( ushort ) ) );

  quint16 *indexPtr = reinterpret_cast<quint16 *>( indexBytes.data() );
  for ( int latSliceIndex = 0; latSliceIndex < latSliceCount - 1; ++latSliceIndex )
  {
    int latSliceStartIndex = latSliceIndex * lonSliceCount;
    int nextLatSliceStartIndex = lonSliceCount + latSliceStartIndex;
    for ( int lonSliceIndex = 0; lonSliceIndex < lonSliceCount - 1; ++lonSliceIndex )
    {
      indexPtr[0] = latSliceStartIndex + lonSliceIndex;
      indexPtr[1] = lonSliceIndex + latSliceStartIndex + 1;
      indexPtr[2] = nextLatSliceStartIndex + lonSliceIndex;

      indexPtr[3] = nextLatSliceStartIndex + lonSliceIndex;
      indexPtr[4] = lonSliceIndex + latSliceStartIndex + 1;
      indexPtr[5] = lonSliceIndex + nextLatSliceStartIndex + 1;

      indexPtr += 6;
    }
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  Qt3DCore::QBuffer *vertexBuffer = new Qt3DCore::QBuffer( entity );
  vertexBuffer->setData( bufferBytes );

  Qt3DCore::QBuffer *indexBuffer = new Qt3DCore::QBuffer( entity );
  indexBuffer->setData( indexBytes );

  Qt3DCore::QAttribute *positionAttribute = new Qt3DCore::QAttribute( entity );
  positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setByteStride( stride );
  positionAttribute->setCount( pointCount );

  Qt3DCore::QAttribute *texCoordAttribute = new Qt3DCore::QAttribute( entity );
  texCoordAttribute->setName( Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName() );
  texCoordAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  texCoordAttribute->setVertexSize( 2 );
  texCoordAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  texCoordAttribute->setBuffer( vertexBuffer );
  texCoordAttribute->setByteStride( stride );
  texCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  texCoordAttribute->setCount( pointCount );

  Qt3DCore::QAttribute *normalAttribute = new Qt3DCore::QAttribute( entity );
  normalAttribute->setName( Qt3DCore::QAttribute::defaultNormalAttributeName() );
  normalAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  normalAttribute->setVertexSize( 3 );
  normalAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  normalAttribute->setBuffer( vertexBuffer );
  normalAttribute->setByteStride( stride );
  normalAttribute->setByteOffset( 5 * sizeof( float ) );
  normalAttribute->setCount( pointCount );

  Qt3DCore::QAttribute *indexAttribute = new Qt3DCore::QAttribute( entity );
  indexAttribute->setAttributeType( Qt3DCore::QAttribute::IndexAttribute );
  indexAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedShort );
  indexAttribute->setBuffer( indexBuffer );
  indexAttribute->setCount( faces * 3 );

  Qt3DCore::QGeometry *geometry = new Qt3DCore::QGeometry( entity );
  geometry->addAttribute( positionAttribute );
  geometry->addAttribute( texCoordAttribute );
  geometry->addAttribute( normalAttribute );
  geometry->addAttribute( indexAttribute );

  Qt3DRender::QGeometryRenderer *geomRenderer = new Qt3DRender::QGeometryRenderer( entity );
  geomRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
  geomRenderer->setVertexCount( faces * 3 );
  geomRenderer->setGeometry( geometry );

  QgsTerrainTextureImage *textureImage = new QgsTerrainTextureImage( textureQImage, QgsRectangle( lonMin, latMin, lonMax, latMax ), textureDebugText, entity );

  Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D( entity );
  Qgs3DUtils::setTextureFiltering( texture, context );
  texture->addTextureImage( textureImage );
  texture->setFormat( Qt3DRender::QAbstractTexture::SRGB8_Alpha8 );

  QgsGlobeMaterial *material = new QgsGlobeMaterial( entity );
  material->setTexture( texture );

  QgsGeoTransform *geoTransform = new QgsGeoTransform( entity );
  geoTransform->setGeoTranslation( meshOrigin );

  entity->addComponent( material );
  entity->addComponent( geomRenderer );
  entity->addComponent( geoTransform );
  return entity;
}


// ---------------


QgsGlobeChunkLoader::QgsGlobeChunkLoader( QgsChunkNode *node, const Qgs3DRenderContext &context, QgsTerrainTextureGenerator *textureGenerator, const QgsCoordinateTransform &globeCrsToLatLon )
  : QgsChunkLoader( node )
  , mRenderContext( context )
  , mTextureGenerator( textureGenerator )
  , mGlobeCrsToLatLon( globeCrsToLatLon )
{}

void QgsGlobeChunkLoader::start()
{
  QgsChunkNode *node = chunk();

  connect( mTextureGenerator, &QgsTerrainTextureGenerator::tileReady, this, [this]( int job, const QImage &img ) {
    if ( job == mJobId )
    {
      mTexture = img;
      emit finished();
    }
  } );

  const QgsRectangle extent = QgsGlobeUtils::nodeIdToLonLatRect( node->tileId() );
  mJobId = mTextureGenerator->render( extent, node->tileId(), node->tileId().text() );
}

Qt3DCore::QEntity *QgsGlobeChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mNode->tileId() == QgsChunkNodeId( 0, 0, 0, 0 ) )
  {
    return new Qt3DCore::QEntity( parent );
  }

  const QgsRectangle extent = QgsGlobeUtils::nodeIdToLonLatRect( mNode->tileId() );

  // This is quite ad-hoc estimation how many slices we need. It could
  // be improved by basing the calculation on sagitta
  int d = mNode->tileId().d;
  int slices;
  if ( d <= 4 )
    slices = 19;
  else if ( d <= 8 )
    slices = 9;
  else if ( d <= 12 )
    slices = 5;
  else
    slices = 2;

  QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( mRenderContext );

  Qt3DCore::QEntity *e = makeGlobeMesh( extent.xMinimum(), extent.xMaximum(), extent.yMinimum(), extent.yMaximum(), slices, slices, mGlobeCrsToLatLon, mTexture, mNode->tileId().text(), materialContext );
  e->setParent( parent );
  return e;
}


// ---------------


QgsGlobeChunkLoaderFactory::QgsGlobeChunkLoaderFactory( Qgs3DMapSettings *mapSettings )
  : mMapSettings( mapSettings )
{
  mTextureGenerator = std::make_unique<QgsTerrainTextureGenerator>( *mapSettings );

  // it does not matter what kind of ellipsoid is used, this is for rough estimates
  mDistanceArea.setEllipsoid( mapSettings->crs().ellipsoidAcronym() );

  mGlobeCrsToLatLon = QgsCoordinateTransform( mapSettings->crs(), mapSettings->crs().toGeographicCrs(), mapSettings->transformContext() );

  mRadiusX = mGlobeCrsToLatLon.transform( QgsVector3D( 0, 0, 0 ), Qgis::TransformDirection::Reverse ).x();
  mRadiusY = mGlobeCrsToLatLon.transform( QgsVector3D( 90, 0, 0 ), Qgis::TransformDirection::Reverse ).y();
  mRadiusZ = mGlobeCrsToLatLon.transform( QgsVector3D( 0, 90, 0 ), Qgis::TransformDirection::Reverse ).z();
}

QgsGlobeChunkLoaderFactory::~QgsGlobeChunkLoaderFactory()
{}

QgsChunkLoader *QgsGlobeChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsGlobeChunkLoader( node, Qgs3DRenderContext::fromMapSettings( mMapSettings ), mTextureGenerator.get(), mGlobeCrsToLatLon );
}

QgsChunkNode *QgsGlobeChunkLoaderFactory::createRootNode() const
{
  const QgsChunkNodeId rootId( 0, 0, 0, 0 );
  const QgsBox3D rootNodeBox3D = QgsGlobeUtils::nodeIdToBox3D( rootId, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ );
  // use very high error to force immediate switch to level 1 (two hemispheres)
  QgsChunkNode *node = new QgsChunkNode( rootId, rootNodeBox3D, 999'999 );
  return node;
}

QVector<QgsChunkNode *> QgsGlobeChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;
  if ( node->tileId().d == 0 )
  {
    double d1 = mDistanceArea.measureLine( QgsPointXY( 0, 0 ), QgsPointXY( 90, 0 ) );
    double d2 = mDistanceArea.measureLine( QgsPointXY( 0, 0 ), QgsPointXY( 0, 90 ) );
    float error = static_cast<float>( std::max( d1, d2 ) ) / static_cast<float>( mMapSettings->terrainSettings()->mapTileResolution() );

    const QgsChunkNodeId westId( 1, 0, 0, 0 );
    const QgsChunkNodeId eastId( 1, 1, 0, 0 );

    // two children: western and eastern hemisphere
    QgsChunkNode *west = new QgsChunkNode( westId, QgsGlobeUtils::nodeIdToBox3D( westId, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ ), error, node );
    QgsChunkNode *east = new QgsChunkNode( eastId, QgsGlobeUtils::nodeIdToBox3D( eastId, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ ), error, node );
    children << west << east;
  }
  else if ( node->error() > mMapSettings->terrainSettings()->maximumGroundError() )
  {
    QgsChunkNodeId nid = node->tileId();

    const QgsRectangle extent = QgsGlobeUtils::nodeIdToLonLatRect( nid );
    const double lonMin = extent.xMinimum(), lonMax = extent.xMaximum();
    const double latMin = extent.yMinimum(), latMax = extent.yMaximum();
    QgsChunkNodeId cid1( nid.d + 1, nid.x * 2, nid.y * 2 );
    QgsChunkNodeId cid2( nid.d + 1, nid.x * 2 + 1, nid.y * 2 );
    QgsChunkNodeId cid3( nid.d + 1, nid.x * 2, nid.y * 2 + 1 );
    QgsChunkNodeId cid4( nid.d + 1, nid.x * 2 + 1, nid.y * 2 + 1 );

    double d1 = mDistanceArea.measureLine( QgsPointXY( lonMin, latMin ), QgsPointXY( lonMin + ( lonMax - lonMin ) / 2, latMin ) );
    double d2 = mDistanceArea.measureLine( QgsPointXY( lonMin, latMin ), QgsPointXY( lonMin, latMin + ( latMax - latMin ) / 2 ) );
    float error = static_cast<float>( std::max( d1, d2 ) ) / static_cast<float>( mMapSettings->terrainSettings()->mapTileResolution() );

    children
      << new QgsChunkNode( cid1, QgsGlobeUtils::nodeIdToBox3D( cid1, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ ), error, node )
      << new QgsChunkNode( cid2, QgsGlobeUtils::nodeIdToBox3D( cid2, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ ), error, node )
      << new QgsChunkNode( cid3, QgsGlobeUtils::nodeIdToBox3D( cid3, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ ), error, node )
      << new QgsChunkNode( cid4, QgsGlobeUtils::nodeIdToBox3D( cid4, mGlobeCrsToLatLon, mRadiusX, mRadiusY, mRadiusZ ), error, node );
  }

  return children;
}

// ---------------


QgsGlobeMapUpdateJob::QgsGlobeMapUpdateJob( QgsTerrainTextureGenerator *textureGenerator, QgsChunkNode *node )
  : QgsChunkQueueJob( node )
  , mTextureGenerator( textureGenerator )
{}

void QgsGlobeMapUpdateJob::start()
{
  QgsChunkNode *node = chunk();

  // extract our terrain texture image from the 3D entity
  QVector<QgsGlobeMaterial *> materials = node->entity()->componentsOfType<QgsGlobeMaterial>();
  Q_ASSERT( materials.count() == 1 );
  QVector<Qt3DRender::QAbstractTextureImage *> texImages = materials[0]->texture()->textureImages();
  Q_ASSERT( texImages.count() == 1 );
  QgsTerrainTextureImage *terrainTexImage = qobject_cast<QgsTerrainTextureImage *>( texImages[0] );
  Q_ASSERT( terrainTexImage );

  connect( mTextureGenerator, &QgsTerrainTextureGenerator::tileReady, this, [this, terrainTexImage]( int jobId, const QImage &image ) {
    if ( mJobId == jobId )
    {
      terrainTexImage->setImage( image );
      mJobId = -1;
      emit finished();
    }
  } );
  mJobId = mTextureGenerator->render( terrainTexImage->imageExtent(), node->tileId(), terrainTexImage->imageDebugText() );
}

void QgsGlobeMapUpdateJob::cancel()
{
  if ( mJobId != -1 )
    mTextureGenerator->cancelJob( mJobId );
}


// ---------------


//! Factory for map update jobs
class QgsGlobeMapUpdateJobFactory : public QgsChunkQueueJobFactory
{
  public:
    explicit QgsGlobeMapUpdateJobFactory( Qgs3DMapSettings *mapSettings ) { mTextureGenerator = new QgsTerrainTextureGenerator( *mapSettings ); }

    QgsChunkQueueJob *createJob( QgsChunkNode *chunk ) override { return new QgsGlobeMapUpdateJob( mTextureGenerator, chunk ); }

  private:
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr;
};


// ---------------


QgsGlobeEntity::QgsGlobeEntity( Qgs3DMapSettings *mapSettings )
  : QgsChunkedEntity( mapSettings, mapSettings->terrainSettings()->maximumScreenError(), new QgsGlobeChunkLoaderFactory( mapSettings ), true )
{
  mLayerWatcher = make_qobject_unique<QgsLayerStyleWatcher>( mapSettings );
  connect( mLayerWatcher.get(), &QgsLayerStyleWatcher::styleChanged, this, &QgsGlobeEntity::invalidateMapImages );

  connect( mapSettings, &Qgs3DMapSettings::showTerrainBoundingBoxesChanged, this, [this, mapSettings] {
    setShowBoundingBoxes( mapSettings->debugFlags().testFlag( Qgis::Map3DDebugFlag::ShowTerrainBoundingBoxes ) );
  } );
  connect( mapSettings, &Qgs3DMapSettings::showTerrainTilesInfoChanged, this, &QgsGlobeEntity::invalidateMapImages );
  connect( mapSettings, &Qgs3DMapSettings::showLabelsChanged, this, &QgsGlobeEntity::invalidateMapImages );
  connect( mapSettings, &Qgs3DMapSettings::backgroundColorChanged, this, &QgsGlobeEntity::invalidateMapImages );
  connect( mapSettings, &Qgs3DMapSettings::terrainMapThemeChanged, this, &QgsGlobeEntity::invalidateMapImages );

  mUpdateJobFactory = std::make_unique<QgsGlobeMapUpdateJobFactory>( mapSettings );
}

QgsGlobeEntity::~QgsGlobeEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

QList<QgsRayCastHit> QgsGlobeEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  float minDist = -1;
  QVector3D intersectionPoint;
  const QList<QgsChunkNode *> active = activeNodes();
  for ( QgsChunkNode *node : active )
  {
    const QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), mMapSettings->origin() );

    if ( node->entity() && ( minDist < 0 || nodeBbox.distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
    {
      QgsGeoTransform *nodeGeoTransform = node->entity()->findChild<QgsGeoTransform *>();
      Q_ASSERT( nodeGeoTransform );
      const QList<Qt3DRender::QGeometryRenderer *> rendLst = node->entity()->findChildren<Qt3DRender::QGeometryRenderer *>();
      for ( Qt3DRender::QGeometryRenderer *rend : rendLst )
      {
        QVector3D nodeIntPoint;
        int triangleIndex = -1;
        bool success = QgsRayCastingUtils::rayMeshIntersection( rend, ray, context.maximumDistance(), nodeGeoTransform->matrix(), nodeIntPoint, triangleIndex );
        if ( success )
        {
          float dist = ( ray.origin() - nodeIntPoint ).length();
          if ( minDist < 0 || dist < minDist )
          {
            minDist = dist;
            intersectionPoint = nodeIntPoint;
          }
        }
      }
    }
  }

  if ( minDist < 0 )
    return {};

  QgsRayCastHit hit;
  hit.setDistance( minDist );
  hit.setMapCoordinates( mMapSettings->worldToMapCoordinates( intersectionPoint ) );
  return { hit };
}

void QgsGlobeEntity::invalidateMapImages()
{
  QgsEventTracing::addEvent( QgsEventTracing::Instant, u"3D"_s, u"Invalidate textures"_s );

  // handle active nodes

  updateNodes( mActiveNodes, mUpdateJobFactory.get() );

  // handle inactive nodes afterwards

  QList<QgsChunkNode *> inactiveNodes;
  const QList<QgsChunkNode *> descendants = mRootNode->descendants();
  for ( QgsChunkNode *node : descendants )
  {
    if ( !node->entity() )
      continue;
    if ( mActiveNodes.contains( node ) )
      continue;
    if ( !node->parent() )
      continue; // skip root node because it is not proper QEntity with data
    inactiveNodes << node;
  }

  updateNodes( inactiveNodes, mUpdateJobFactory.get() );

  setNeedsUpdate( true );
}


/// @endcond
