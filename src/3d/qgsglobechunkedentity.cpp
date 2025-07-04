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

#include <QByteArray>
#include <QImage>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgschunkloader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgseventtracing.h"
#include "qgsgeotransform.h"
#include "qgsglobematerial.h"
#include "qgsraycastingutils_p.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintexturegenerator_p.h"


///@cond PRIVATE

static Qt3DCore::QEntity *makeGlobeMesh( double lonMin, double lonMax, double latMin, double latMax, int lonSliceCount, int latSliceCount, const QgsCoordinateTransform &globeCrsToLatLon, QImage textureQImage, QString textureDebugText )
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

  Qt3DQBuffer *vertexBuffer = new Qt3DQBuffer( entity );
  vertexBuffer->setData( bufferBytes );

  Qt3DQBuffer *indexBuffer = new Qt3DQBuffer( entity );
  indexBuffer->setData( indexBytes );

  Qt3DQAttribute *positionAttribute = new Qt3DQAttribute( entity );
  positionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setByteStride( stride );
  positionAttribute->setCount( pointCount );

  Qt3DQAttribute *texCoordAttribute = new Qt3DQAttribute( entity );
  texCoordAttribute->setName( Qt3DQAttribute::defaultTextureCoordinateAttributeName() );
  texCoordAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  texCoordAttribute->setVertexSize( 2 );
  texCoordAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  texCoordAttribute->setBuffer( vertexBuffer );
  texCoordAttribute->setByteStride( stride );
  texCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  texCoordAttribute->setCount( pointCount );

  Qt3DQAttribute *normalAttribute = new Qt3DQAttribute( entity );
  normalAttribute->setName( Qt3DQAttribute::defaultNormalAttributeName() );
  normalAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  normalAttribute->setVertexSize( 3 );
  normalAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  normalAttribute->setBuffer( vertexBuffer );
  normalAttribute->setByteStride( stride );
  normalAttribute->setByteOffset( 5 * sizeof( float ) );
  normalAttribute->setCount( pointCount );

  Qt3DQAttribute *indexAttribute = new Qt3DQAttribute( entity );
  indexAttribute->setAttributeType( Qt3DQAttribute::IndexAttribute );
  indexAttribute->setVertexBaseType( Qt3DQAttribute::UnsignedShort );
  indexAttribute->setBuffer( indexBuffer );
  indexAttribute->setCount( faces * 3 );

  Qt3DQGeometry *geometry = new Qt3DQGeometry( entity );
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
  texture->addTextureImage( textureImage );
  texture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  texture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );

  QgsGlobeMaterial *material = new QgsGlobeMaterial( entity );
  material->setTexture( texture );

  QgsGeoTransform *geoTransform = new QgsGeoTransform( entity );
  geoTransform->setGeoTranslation( meshOrigin );

  entity->addComponent( material );
  entity->addComponent( geomRenderer );
  entity->addComponent( geoTransform );
  return entity;
}


static void globeNodeIdToLatLon( QgsChunkNodeId n, double &latMin, double &latMax, double &lonMin, double &lonMax )
{
  if ( n == QgsChunkNodeId( 0, 0, 0, 0 ) )
  {
    latMin = -90;
    lonMin = -180;
    latMax = 90;
    lonMax = 180;
    return;
  }

  double tileSize = 180.0 / std::pow( 2.0, n.d - 1 );
  lonMin = n.x * tileSize - 180.0;
  latMin = n.y * tileSize - 90.0;
  lonMax = lonMin + tileSize;
  latMax = latMin + tileSize;
}


static QgsBox3D globeNodeIdToBox3D( QgsChunkNodeId n, const QgsCoordinateTransform &globeCrsToLatLon )
{
  double latMin, latMax, lonMin, lonMax;
  globeNodeIdToLatLon( n, latMin, latMax, lonMin, lonMax );

  Q_ASSERT( latMax - latMin <= 90 && lonMax - lonMin <= 90 ); // for larger extents we would need more points than just corners

  QVector<double> x, y, z;
  int pointCount = 4;
  x.reserve( pointCount );
  y.reserve( pointCount );
  z.reserve( pointCount );

  x.push_back( lonMin );
  y.push_back( latMin );
  z.push_back( 0 );
  x.push_back( lonMin );
  y.push_back( latMax );
  z.push_back( 0 );
  x.push_back( lonMax );
  y.push_back( latMin );
  z.push_back( 0 );
  x.push_back( lonMax );
  y.push_back( latMax );
  z.push_back( 0 );

  globeCrsToLatLon.transformCoords( pointCount, x.data(), y.data(), z.data(), Qgis::TransformDirection::Reverse );

  QgsBox3D box( QgsVector3D( x[0], y[0], z[0] ), QgsVector3D( x[1], y[1], z[1] ) );
  box.combineWith( x[2], y[2], z[2] );
  box.combineWith( x[3], y[3], z[3] );
  return box;
}


// ---------------


class QgsGlobeChunkLoader : public QgsChunkLoader
{
  public:
    QgsGlobeChunkLoader( QgsChunkNode *node, QgsTerrainTextureGenerator *textureGenerator, const QgsCoordinateTransform &globeCrsToLatLon )
      : QgsChunkLoader( node )
      , mTextureGenerator( textureGenerator )
      , mGlobeCrsToLatLon( globeCrsToLatLon )
    {
      connect( mTextureGenerator, &QgsTerrainTextureGenerator::tileReady, this, [this]( int job, const QImage &img ) {
        if ( job == mJobId )
        {
          mTexture = img;
          emit finished();
        }
      } );

      double latMin, latMax, lonMin, lonMax;
      globeNodeIdToLatLon( node->tileId(), latMin, latMax, lonMin, lonMax );
      QgsRectangle extent( lonMin, latMin, lonMax, latMax );
      mJobId = mTextureGenerator->render( extent, node->tileId(), node->tileId().text() );
    }

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override
    {
      if ( mNode->tileId() == QgsChunkNodeId( 0, 0, 0, 0 ) )
      {
        return new Qt3DCore::QEntity( parent );
      }

      double latMin, latMax, lonMin, lonMax;
      globeNodeIdToLatLon( mNode->tileId(), latMin, latMax, lonMin, lonMax );

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

      Qt3DCore::QEntity *e = makeGlobeMesh( lonMin, lonMax, latMin, latMax, slices, slices, mGlobeCrsToLatLon, mTexture, mNode->tileId().text() );
      e->setParent( parent );
      return e;
    }

  private:
    QgsTerrainTextureGenerator *mTextureGenerator;
    QgsCoordinateTransform mGlobeCrsToLatLon;
    int mJobId;
    QImage mTexture;
};


// ---------------


class QgsGlobeChunkLoaderFactory : public QgsChunkLoaderFactory
{
  public:
    QgsGlobeChunkLoaderFactory( Qgs3DMapSettings *mapSettings )
      : mMapSettings( mapSettings )
    {
      mTextureGenerator = new QgsTerrainTextureGenerator( *mapSettings );

      // it does not matter what kind of ellipsoid is used, this is for rough estimates
      mDistanceArea.setEllipsoid( mapSettings->crs().ellipsoidAcronym() );

      mGlobeCrsToLatLon = QgsCoordinateTransform( mapSettings->crs(), mapSettings->crs().toGeographicCrs(), mapSettings->transformContext() );

      mRadiusX = mGlobeCrsToLatLon.transform( QgsVector3D( 0, 0, 0 ), Qgis::TransformDirection::Reverse ).x();
      mRadiusY = mGlobeCrsToLatLon.transform( QgsVector3D( 90, 0, 0 ), Qgis::TransformDirection::Reverse ).y();
      mRadiusZ = mGlobeCrsToLatLon.transform( QgsVector3D( 0, 90, 0 ), Qgis::TransformDirection::Reverse ).z();
    }

    ~QgsGlobeChunkLoaderFactory()
    {
      delete mTextureGenerator;
    }

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override
    {
      return new QgsGlobeChunkLoader( node, mTextureGenerator, mGlobeCrsToLatLon );
    }

    QgsChunkNode *createRootNode() const override
    {
      QgsBox3D rootNodeBox3D( -mRadiusX, -mRadiusY, -mRadiusZ, +mRadiusX, +mRadiusY, +mRadiusZ );
      // use very high error to force immediate switch to level 1 (two hemispheres)
      QgsChunkNode *node = new QgsChunkNode( QgsChunkNodeId( 0, 0, 0, 0 ), rootNodeBox3D, 999'999 );
      return node;
    }

    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override
    {
      QVector<QgsChunkNode *> children;
      if ( node->tileId().d == 0 )
      {
        double d1 = mDistanceArea.measureLine( QgsPointXY( 0, 0 ), QgsPointXY( 90, 0 ) );
        double d2 = mDistanceArea.measureLine( QgsPointXY( 0, 0 ), QgsPointXY( 0, 90 ) );
        float error = static_cast<float>( std::max( d1, d2 ) ) / static_cast<float>( mMapSettings->terrainSettings()->mapTileResolution() );

        QgsBox3D boxWest( -mRadiusX, -mRadiusY, -mRadiusZ, +mRadiusX, 0, +mRadiusZ );
        QgsBox3D boxEast( -mRadiusX, 0, -mRadiusY, +mRadiusX, +mRadiusY, +mRadiusZ );

        // two children: western and eastern hemisphere
        QgsChunkNode *west = new QgsChunkNode( QgsChunkNodeId( 1, 0, 0, 0 ), boxWest, error, node );
        QgsChunkNode *east = new QgsChunkNode( QgsChunkNodeId( 1, 1, 0, 0 ), boxEast, error, node );
        children << west << east;
      }
      else if ( node->error() > mMapSettings->terrainSettings()->maximumGroundError() )
      {
        QgsChunkNodeId nid = node->tileId();

        double latMin, latMax, lonMin, lonMax;
        globeNodeIdToLatLon( nid, latMin, latMax, lonMin, lonMax );
        QgsChunkNodeId cid1( nid.d + 1, nid.x * 2, nid.y * 2 );
        QgsChunkNodeId cid2( nid.d + 1, nid.x * 2 + 1, nid.y * 2 );
        QgsChunkNodeId cid3( nid.d + 1, nid.x * 2, nid.y * 2 + 1 );
        QgsChunkNodeId cid4( nid.d + 1, nid.x * 2 + 1, nid.y * 2 + 1 );

        double d1 = mDistanceArea.measureLine( QgsPointXY( lonMin, latMin ), QgsPointXY( lonMin + ( lonMax - lonMin ) / 2, latMin ) );
        double d2 = mDistanceArea.measureLine( QgsPointXY( lonMin, latMin ), QgsPointXY( lonMin, latMin + ( latMax - latMin ) / 2 ) );
        float error = static_cast<float>( std::max( d1, d2 ) ) / static_cast<float>( mMapSettings->terrainSettings()->mapTileResolution() );

        children << new QgsChunkNode( cid1, globeNodeIdToBox3D( cid1, mGlobeCrsToLatLon ), error, node )
                 << new QgsChunkNode( cid2, globeNodeIdToBox3D( cid2, mGlobeCrsToLatLon ), error, node )
                 << new QgsChunkNode( cid3, globeNodeIdToBox3D( cid3, mGlobeCrsToLatLon ), error, node )
                 << new QgsChunkNode( cid4, globeNodeIdToBox3D( cid4, mGlobeCrsToLatLon ), error, node );
      }

      return children;
    }

  private:
    Qgs3DMapSettings *mMapSettings = nullptr;
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr; // owned by the factory
    QgsDistanceArea mDistanceArea;
    QgsCoordinateTransform mGlobeCrsToLatLon;
    double mRadiusX, mRadiusY, mRadiusZ;
};


// ---------------


//! Handles asynchronous updates of globe's map images when layers change
class QgsGlobeMapUpdateJob : public QgsChunkQueueJob
{
  public:
    QgsGlobeMapUpdateJob( QgsTerrainTextureGenerator *textureGenerator, QgsChunkNode *node )
      : QgsChunkQueueJob( node )
      , mTextureGenerator( textureGenerator )
    {
      // extract our terrain texture image from the 3D entity
      QVector<QgsGlobeMaterial *> materials = node->entity()->componentsOfType<QgsGlobeMaterial>();
      Q_ASSERT( materials.count() == 1 );
      QVector<Qt3DRender::QAbstractTextureImage *> texImages = materials[0]->texture()->textureImages();
      Q_ASSERT( texImages.count() == 1 );
      QgsTerrainTextureImage *terrainTexImage = qobject_cast<QgsTerrainTextureImage *>( texImages[0] );
      Q_ASSERT( terrainTexImage );

      connect( textureGenerator, &QgsTerrainTextureGenerator::tileReady, this, [this, terrainTexImage]( int jobId, const QImage &image ) {
        if ( mJobId == jobId )
        {
          terrainTexImage->setImage( image );
          mJobId = -1;
          emit finished();
        }
      } );
      mJobId = textureGenerator->render( terrainTexImage->imageExtent(), node->tileId(), terrainTexImage->imageDebugText() );
    }

    void cancel() override
    {
      if ( mJobId != -1 )
        mTextureGenerator->cancelJob( mJobId );
    }

  private:
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr;
    int mJobId;
};


// ---------------


//! Factory for map update jobs
class QgsGlobeMapUpdateJobFactory : public QgsChunkQueueJobFactory
{
  public:
    explicit QgsGlobeMapUpdateJobFactory( Qgs3DMapSettings *mapSettings )
    {
      mTextureGenerator = new QgsTerrainTextureGenerator( *mapSettings );
    }

    QgsChunkQueueJob *createJob( QgsChunkNode *chunk ) override
    {
      return new QgsGlobeMapUpdateJob( mTextureGenerator, chunk );
    }

  private:
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr;
};


// ---------------


QgsGlobeEntity::QgsGlobeEntity( Qgs3DMapSettings *mapSettings )
  : QgsChunkedEntity( mapSettings, mapSettings->terrainSettings()->maximumScreenError(), new QgsGlobeChunkLoaderFactory( mapSettings ), true )
{
  connect( mapSettings, &Qgs3DMapSettings::showTerrainBoundingBoxesChanged, this, [this, mapSettings] {
    setShowBoundingBoxes( mapSettings->showTerrainBoundingBoxes() );
  } );
  connect( mapSettings, &Qgs3DMapSettings::showTerrainTilesInfoChanged, this, &QgsGlobeEntity::invalidateMapImages );
  connect( mapSettings, &Qgs3DMapSettings::showLabelsChanged, this, &QgsGlobeEntity::invalidateMapImages );
  connect( mapSettings, &Qgs3DMapSettings::layersChanged, this, &QgsGlobeEntity::onLayersChanged );
  connect( mapSettings, &Qgs3DMapSettings::backgroundColorChanged, this, &QgsGlobeEntity::invalidateMapImages );
  connect( mapSettings, &Qgs3DMapSettings::terrainMapThemeChanged, this, &QgsGlobeEntity::invalidateMapImages );

  connectToLayersRepaintRequest();

  mUpdateJobFactory.reset( new QgsGlobeMapUpdateJobFactory( mapSettings ) );
}

QgsGlobeEntity::~QgsGlobeEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

QVector<QgsRayCastingUtils::RayHit> QgsGlobeEntity::rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QgsRayCastingUtils::RayCastContext &context ) const
{
  Q_UNUSED( context );

  float minDist = -1;
  QVector3D intersectionPoint;
  const QList<QgsChunkNode *> active = activeNodes();
  for ( QgsChunkNode *node : active )
  {
    QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), mMapSettings->origin() );

    if ( node->entity() && ( minDist < 0 || nodeBbox.distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
    {
      QgsGeoTransform *nodeGeoTransform = node->entity()->findChild<QgsGeoTransform *>();
      Q_ASSERT( nodeGeoTransform );
      const QList<Qt3DRender::QGeometryRenderer *> rendLst = node->entity()->findChildren<Qt3DRender::QGeometryRenderer *>();
      for ( Qt3DRender::QGeometryRenderer *rend : rendLst )
      {
        QVector3D nodeIntPoint;
        int triangleIndex = -1;
        bool success = QgsRayCastingUtils::rayMeshIntersection( rend, ray, nodeGeoTransform->matrix(), nodeIntPoint, triangleIndex );
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

  QVector<QgsRayCastingUtils::RayHit> result;
  if ( minDist >= 0 )
  {
    result.append( QgsRayCastingUtils::RayHit( minDist, intersectionPoint ) );
  }
  return result;
}


void QgsGlobeEntity::invalidateMapImages()
{
  QgsEventTracing::addEvent( QgsEventTracing::Instant, QStringLiteral( "3D" ), QStringLiteral( "Invalidate textures" ) );

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

void QgsGlobeEntity::onLayersChanged()
{
  connectToLayersRepaintRequest();
  invalidateMapImages();
}

void QgsGlobeEntity::connectToLayersRepaintRequest()
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsGlobeEntity::invalidateMapImages );
  }

  mLayers = mMapSettings->layers();

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsGlobeEntity::invalidateMapImages );
  }
}

/// @endcond
