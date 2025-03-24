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
#include <QTimer>
#include <QImage>
#include <QtMath>

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DExtras/QTextureMaterial>

#include "qgs3dmapsettings.h"
#include "qgschunkloader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgsgeotransform.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintexturegenerator_p.h"

///@cond PRIVATE

using namespace Qt3DCore;
using namespace Qt3DRender;


QEntity *makeGlobeMesh( double lonMin, double lonMax,
                        double latMin, double latMax,
                        int lonSliceCount, int latSliceCount,
                        QImage textureQImage,
                        QString textureDebugText )
{
  double lonRange = lonMax - lonMin;
  double latRange = latMax - latMin;
  double lonStep = lonRange / ( double )( lonSliceCount - 1 );
  double latStep = latRange / ( double )( latSliceCount - 1 );

  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( "EPSG:4326" ), QgsCoordinateReferenceSystem( "EPSG:4978" ), QgsCoordinateTransformContext() );

  std::vector<double> x, y, z;
  uint pointCount = latSliceCount * lonSliceCount;
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

  ct.transformCoords( pointCount, x.data(), y.data(), z.data() );

  int stride = ( 3 + 2 + 3 ) * sizeof( float );

  QByteArray bufferBytes;
  bufferBytes.resize( stride * pointCount );
  float *fptr = ( float * ) bufferBytes.data();
  for ( int i = 0; i < ( int )pointCount; ++i )
  {
    *fptr++ = x[i];
    *fptr++ = y[i];
    *fptr++ = z[i];

    float v = ( float )( i / lonSliceCount ) / ( float )( latSliceCount - 1 );
    float u = ( float )( i % lonSliceCount ) / ( float )( lonSliceCount - 1 );
    *fptr++ = u;
    *fptr++ = 1 - v;

    QVector3D n = QVector3D( ( float )x[i], ( float )y[i], ( float )z[i] ).normalized();
    *fptr++ = n.x();
    *fptr++ = n.y();
    *fptr++ = n.z();
  }

  int faces = ( lonSliceCount - 1 ) * ( latSliceCount - 1 ) * 2;
  qsizetype indices = faces * 3;

  QByteArray indexBytes;
  indexBytes.resize( indices * sizeof( ushort ) );

  quint16 *indexPtr = ( unsigned short * ) indexBytes.data();
  for ( short latSliceIndex = 0; latSliceIndex < latSliceCount - 1; ++latSliceIndex )
  {
    short latSliceStartIndex = latSliceIndex * ( short )lonSliceCount;
    short nextLatSliceStartIndex = ( short )lonSliceCount + latSliceStartIndex;
    for ( short lonSliceIndex = 0; lonSliceIndex < lonSliceCount - 1; ++lonSliceIndex )
    {
      indexPtr[0] = latSliceStartIndex + lonSliceIndex;
      indexPtr[1] = lonSliceIndex + latSliceStartIndex + 1;
      indexPtr[2] = nextLatSliceStartIndex + lonSliceIndex;

      indexPtr[3] = nextLatSliceStartIndex + lonSliceIndex;
      indexPtr[4] = lonSliceIndex + latSliceStartIndex + 1;
      indexPtr[5] = lonSliceIndex + nextLatSliceStartIndex + 1;

      indexPtr = indexPtr + 6;
    }
  }

  QBuffer *m_vertexBuffer = new QBuffer();
  m_vertexBuffer->setData( bufferBytes );

  QBuffer *m_indexBuffer = new QBuffer();
  m_indexBuffer->setData( indexBytes );

  QAttribute *m_positionAttribute = new QAttribute;
  m_positionAttribute = new QAttribute;
  m_positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  m_positionAttribute->setVertexBaseType( QAttribute::Float );
  m_positionAttribute->setVertexSize( 3 );
  m_positionAttribute->setAttributeType( QAttribute::VertexAttribute );
  m_positionAttribute->setBuffer( m_vertexBuffer );
  m_positionAttribute->setByteStride( stride );
  m_positionAttribute->setCount( pointCount );

  QAttribute *m_texCoordAttribute = new QAttribute;
  m_texCoordAttribute = new QAttribute;
  m_texCoordAttribute->setName( QAttribute::defaultTextureCoordinateAttributeName() );
  m_texCoordAttribute->setVertexBaseType( QAttribute::Float );
  m_texCoordAttribute->setVertexSize( 2 );
  m_texCoordAttribute->setAttributeType( QAttribute::VertexAttribute );
  m_texCoordAttribute->setBuffer( m_vertexBuffer );
  m_texCoordAttribute->setByteStride( stride );
  m_texCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  m_texCoordAttribute->setCount( pointCount );

  QAttribute *m_normalAttribute = new QAttribute;
  m_normalAttribute->setName( QAttribute::defaultNormalAttributeName() );
  m_normalAttribute->setVertexBaseType( QAttribute::Float );
  m_normalAttribute->setVertexSize( 3 );
  m_normalAttribute->setAttributeType( QAttribute::VertexAttribute );
  m_normalAttribute->setBuffer( m_vertexBuffer );
  m_normalAttribute->setByteStride( stride );
  m_normalAttribute->setByteOffset( 5 * sizeof( float ) );
  m_normalAttribute->setCount( pointCount );

  QAttribute *m_indexAttribute = new QAttribute;
  m_indexAttribute->setAttributeType( QAttribute::IndexAttribute );
  m_indexAttribute->setVertexBaseType( QAttribute::UnsignedShort );
  m_indexAttribute->setBuffer( m_indexBuffer );
  m_indexAttribute->setCount( faces * 3 );

  QGeometry *geom = new QGeometry;
  geom->addAttribute( m_positionAttribute );
  geom->addAttribute( m_texCoordAttribute );
  geom->addAttribute( m_normalAttribute );
  geom->addAttribute( m_indexAttribute );

  QGeometryRenderer *rend = new QGeometryRenderer;
  rend->setPrimitiveType( QGeometryRenderer::Triangles );
  rend->setVertexCount( faces * 3 );
  rend->setGeometry( geom );

  QgsTerrainTextureImage *textureImage = new QgsTerrainTextureImage( textureQImage, QgsRectangle( lonMin, latMin, lonMax, latMax ), textureDebugText );

  QTexture2D *texture = new QTexture2D();
  texture->addTextureImage( textureImage );
  texture->setMinificationFilter( QTexture2D::Linear );
  texture->setMagnificationFilter( QTexture2D::Linear );

  Qt3DExtras::QTextureMaterial *mat = new Qt3DExtras::QTextureMaterial;
  mat->setTexture( texture );

  QgsGeoTransform *gt = new QgsGeoTransform;

  Qt3DCore::QEntity *e = new Qt3DCore::QEntity;
  e->addComponent( mat );
  e->addComponent( rend );
  e->addComponent( gt );
  return e;
}


void globeNodeIdToLatLon( QgsChunkNodeId n, double &latMin, double &latMax, double &lonMin, double &lonMax )
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

QgsBox3D globeNodeIdToBox3D( QgsChunkNodeId n, const QgsCoordinateTransform &globeCrsToLatLon )
{
  double latMin, latMax, lonMin, lonMax;
  globeNodeIdToLatLon( n, latMin, latMax, lonMin, lonMax );

  Q_ASSERT( latMax - latMin <= 90 && lonMax - lonMin <= 90 );  // for larger extents we would need more points than just corners

  std::vector<double> x, y, z;
  uint pointCount = 4;
  x.reserve( pointCount );
  y.reserve( pointCount );
  z.reserve( pointCount );

  x.push_back( lonMin ); y.push_back( latMin ); z.push_back( 0 );
  x.push_back( lonMin ); y.push_back( latMax ); z.push_back( 0 );
  x.push_back( lonMax ); y.push_back( latMin ); z.push_back( 0 );
  x.push_back( lonMax ); y.push_back( latMax ); z.push_back( 0 );

  globeCrsToLatLon.transformCoords( pointCount, x.data(), y.data(), z.data(), Qgis::TransformDirection::Reverse );

  QgsBox3D box( QgsVector3D( x[0], y[0], z[0] ), QgsVector3D( x[1], y[1], z[1] ) );
  box.combineWith( x[2], y[2], z[2] );
  box.combineWith( x[3], y[3], z[3] );
  return box;
}



class QgsGlobeChunkLoader : public QgsChunkLoader
{
  public:
    QgsGlobeChunkLoader( QgsChunkNode *node, QgsTerrainTextureGenerator *tg )
      : QgsChunkLoader( node )
      , mTG( tg )
    {
      connect( mTG, &QgsTerrainTextureGenerator::tileReady, this, [ = ]( int job, const QImage & img )
      {
        if ( job == mJobId )
        {
          mTexture = img;
          emit finished();
        }
      } );

      double latMin, latMax, lonMin, lonMax;
      globeNodeIdToLatLon( node->tileId(), latMin, latMax, lonMin, lonMax );
      QgsRectangle extent( lonMin, latMin, lonMax, latMax );
      mJobId = mTG->render( extent, node->tileId(), node->tileId().text() );
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

      Qt3DCore::QEntity *e = makeGlobeMesh( lonMin, lonMax, latMin, latMax, slices, slices, mTexture, mNode->tileId().text() );
      e->setParent( parent );
      return e;
    }

    QgsTerrainTextureGenerator *mTG;
    int mJobId;
    QImage mTexture;
};


class QgsGlobeChunkLoaderFactory : public QgsChunkLoaderFactory
{
  public:
    Qgs3DMapSettings *mMapSettings = nullptr;
    QgsGlobeEntity *mGlobeEntity = nullptr;

    QgsDistanceArea mDistanceArea;

    QgsCoordinateTransform mGlobeCrsToLatLon;

    double mRadiusX, mRadiusY, mRadiusZ;

    QgsGlobeChunkLoaderFactory( Qgs3DMapSettings *mapSettings, QgsGlobeEntity *globeEntity )
      : mMapSettings( mapSettings )
      , mGlobeEntity( globeEntity )
    {
      // it does not matter what kind of ellipsoid is used, this is for rough estimates
      mDistanceArea.setEllipsoid( mapSettings->crs().ellipsoidAcronym() );

      mGlobeCrsToLatLon = QgsCoordinateTransform( mapSettings->crs(), mapSettings->crs().toGeographicCrs(), mapSettings->transformContext() );

      mRadiusX = mGlobeCrsToLatLon.transform( QgsVector3D( 0, 0, 0 ), Qgis::TransformDirection::Reverse ).x();
      mRadiusY = mGlobeCrsToLatLon.transform( QgsVector3D( 90, 0, 0 ), Qgis::TransformDirection::Reverse ).y();
      mRadiusZ = mGlobeCrsToLatLon.transform( QgsVector3D( 0, 90, 0 ), Qgis::TransformDirection::Reverse ).z();
    }

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override
    {
      return new QgsGlobeChunkLoader( node, mGlobeEntity->mTextureGenerator );
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
        float error = std::max( d1, d2 ) / 256;

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
        float error = std::max( d1, d2 ) / 256;

        children << new QgsChunkNode( cid1, globeNodeIdToBox3D( cid1, mGlobeCrsToLatLon ), error, node )
                 << new QgsChunkNode( cid2, globeNodeIdToBox3D( cid2, mGlobeCrsToLatLon ), error, node )
                 << new QgsChunkNode( cid3, globeNodeIdToBox3D( cid3, mGlobeCrsToLatLon ), error, node )
                 << new QgsChunkNode( cid4, globeNodeIdToBox3D( cid4, mGlobeCrsToLatLon ), error, node );
      }

      return children;
    }

};


// ---------------


QgsGlobeEntity::QgsGlobeEntity( Qgs3DMapSettings *mapSettings, float maximumScreenSpaceError )
  : QgsChunkedEntity( mapSettings, maximumScreenSpaceError, new QgsGlobeChunkLoaderFactory( mapSettings, this ), true )
{

  mTextureGenerator = new QgsTerrainTextureGenerator( *mapSettings );

  setShowBoundingBoxes( true );
}

QgsGlobeEntity::~QgsGlobeEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();

  delete mTextureGenerator;
}

/// @endcond
