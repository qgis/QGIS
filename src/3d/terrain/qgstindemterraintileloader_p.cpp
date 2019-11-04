/***************************************************************************
  qgistindemterraintileloader_p.h
  --------------------------------------
  Date                 : october 2019
  Copyright            : (C) 2019 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <Qt3DRender/QGeometryRenderer>


#include "qgstindemterraintileloader_p.h"
#include "qgstindemterraintilegeometry_p.h"
#include "qgsterrainentity_p.h"
#include "qgs3dmapsettings.h"
#include "qgsterraintileloader_p.h"
#include "qgschunknode_p.h"
#include "qgsterraintileentity_p.h"

QgsTriangularMeshTile::QgsTriangularMeshTile( QgsTriangularMesh triangularMesh, const QgsRectangle &extent ):
  mTriangularMesh( triangularMesh ),
  mExtent( extent )
{
  init();
}

int QgsTriangularMeshTile::faceCount() const
{
  return mFaces.count();
}

int QgsTriangularMeshTile::verticesCount() const
{
  return mVertices.count();
}

QgsMeshFace QgsTriangularMeshTile::triangle( int localTriangleIndex ) const
{
  QgsMeshFace triangle;
  for ( int i = 0; i < 3; ++i )
  {
    int globalIndex = mTriangularMesh.triangles().at( mFaces.at( localTriangleIndex ) ).at( i );
    int localIndex = mLocalIndexFromGlobalIndex[globalIndex];
    triangle.append( localIndex );
  }

  return triangle;

}

QgsMeshVertex QgsTriangularMeshTile::vertex( int localIndex ) const
{
  int globalIndex = mVertices.at( localIndex ).globalIndex;
  return mTriangularMesh.vertices().at( globalIndex );
}

bool QgsTriangularMeshTile::operator==( const QgsTriangularMeshTile &other ) const
{
  return ( other.mExtent == mExtent &&
           other.mRealExtent == mRealExtent
         );
}

QVector3D QgsTriangularMeshTile::vertexUnitNormalVector( int i ) const
{
  return mVertices.at( i ).normalVector.normalized();
}

float QgsTriangularMeshTile::zMinimum() const
{
  return mZmin;
}

float QgsTriangularMeshTile::zMaximum() const
{
  return mZmax;
}

void QgsTriangularMeshTile::init()
{
  mZmax = std::numeric_limits<float>::min();
  mZmin = std::numeric_limits<float>::max();
  mFaces = QVector<int>::fromList( mTriangularMesh.faceIndexesForRectangle( mExtent ) );
  mVertices.clear();
  mLocalIndexFromGlobalIndex.clear();

  int localIndex = 0;
  mRealExtent.setMinimal();
  for ( int f : qgis::as_const( mFaces ) )
  {

    const QgsMeshFace &face( mTriangularMesh.triangles().at( f ) );
    for ( int i = 0; i < 3; i++ )
    {
      int globalIndex( face.at( i ) );

      const QgsMeshVertex &vert( mTriangularMesh.vertices().at( globalIndex ) );
      const QgsMeshVertex &otherVert1( mTriangularMesh.vertices().at( face.at( ( i + 1 ) % 3 ) ) );
      const QgsMeshVertex &otherVert2( mTriangularMesh.vertices().at( face.at( ( i + 2 ) % 3 ) ) );

      QVector3D v1( float( otherVert1.x() - vert.x() ), float( otherVert1.y() - vert.y() ), float( otherVert1.z() - vert.z() ) );
      QVector3D v2( float( otherVert2.x() - vert.x() ), float( otherVert2.y() - vert.y() ), float( otherVert2.z() - vert.z() ) );
      QVector3D crossProduct = QVector3D::crossProduct( v2, v1 );

      if ( mLocalIndexFromGlobalIndex.contains( globalIndex ) )
      {
        LocalVertex &localVert = mVertices[mLocalIndexFromGlobalIndex.value( globalIndex )];
        localVert.normalVector += crossProduct;
      }
      else
      {
        //create a new local vertex
        mLocalIndexFromGlobalIndex[globalIndex] = localIndex;
        LocalVertex  localVert( {globalIndex, crossProduct} );
        mVertices.append( localVert );
        localIndex++;

        //adjust real extent
        if ( mRealExtent.xMinimum() > vert.x() )
          mRealExtent.setXMinimum( vert.x() );
        if ( mRealExtent.yMinimum() > vert.y() )
          mRealExtent.setYMinimum( vert.y() );
        if ( mRealExtent.xMaximum() < vert.x() )
          mRealExtent.setXMaximum( vert.x() );
        if ( mRealExtent.yMaximum() < vert.y() )
          mRealExtent.setYMaximum( vert.y() );

        if ( mZmax < float( vert.z() ) )
          mZmax = float( vert.z() );
        if ( mZmin > float( vert.z() ) )
          mZmin = float( vert.z() );
      }
    }
  }
  mRealExtent.combineExtentWith( mExtent );
}

QgsTinDemTerrainTileLoader::QgsTinDemTerrainTileLoader( QgsTerrainEntity *terrain,
    QgsChunkNode *node,
    const QgsTriangularMesh &triangularMesh ):
  QgsTerrainTileLoader( terrain, node )
{
  meshTileGenerator = new QgsTriangularMeshTileGenerator( triangularMesh,
      terrain->map3D().terrainGenerator()->tilingScheme().tileToExtent( node->tileX(),
          node->tileY(),
          node->tileZ() ) );

  connect( meshTileGenerator, &QgsTriangularMeshTileGenerator::meshTileIsReady, this, &QgsTinDemTerrainTileLoader::onMeshTileReady );

  meshTileGenerator->generate();

}

Qt3DCore::QEntity *QgsTinDemTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity( parent );

  const Qgs3DMapSettings &map = terrain()->map3D();
  QgsRectangle extent = map.terrainGenerator()->tilingScheme().tileToExtent( mNode->tileX(), mNode->tileY(), mNode->tileZ() ); //node->extent;

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsTinDemTerrainTileGeometry_p( mMeshTile, float( map.terrainVerticalScale() ), entity ) );

  createTextureComponent( entity, map.isTerrainShadingEnabled(), map.terrainShadingMaterial() );

  double x0 =  map.origin().x();
  double y0 =  -map.origin().y();

  Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );
  transform->setTranslation( QVector3D( float( -x0 ), 0, float( -y0 ) ) );

  mNode->setExactBbox( QgsAABB( float( extent.xMinimum() - x0 ),
                                mMeshTile.zMinimum()*float( map.terrainVerticalScale() ),
                                float( -extent.yMinimum() - y0 ),
                                float( extent.xMaximum() - x0 ),
                                mMeshTile.zMaximum()*float( map.terrainVerticalScale() ),
                                float( -extent.yMaximum() - y0 ) ) );

  entity->addComponent( mesh );
  entity->setEnabled( false );
  entity->setParent( parent );

  return entity;
}

void QgsTinDemTerrainTileLoader::onMeshTileReady()
{
  mMeshTile = meshTileGenerator->meshTile();
  setExtentMapCrs( terrain()->terrainToMapTransform().transformBoundingBox( mMeshTile.realTileExtent() ) );
  loadTexture();
}



static QgsTriangularMeshTile _makeMeshTile( const QgsTriangularMesh &triangularMesh, const QgsRectangle &extent )
{
  return QgsTriangularMeshTile( triangularMesh, extent );
}



QgsTriangularMeshTileGenerator::QgsTriangularMeshTileGenerator( const QgsTriangularMesh &triangularMesh, const QgsRectangle &extent, QObject *parent ):
  QObject( parent ),
  mTriangularMesh( triangularMesh ),
  mExtent( extent )
{

}

QgsTriangularMeshTile QgsTriangularMeshTileGenerator::meshTile()
{
  return mMeshTileFuture.result();
}



void QgsTriangularMeshTileGenerator::generate()
{
  QFutureWatcher<void> *watcher = new QFutureWatcher<void>( this );
  connect( watcher, &QFutureWatcher<void>::finished, this, &QgsTriangularMeshTileGenerator::meshTileIsReady );
  connect( watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater );

  mMeshTileFuture = QtConcurrent::run( _makeMeshTile, mTriangularMesh, mExtent );
  watcher->setFuture( mMeshTileFuture ) ;
}
