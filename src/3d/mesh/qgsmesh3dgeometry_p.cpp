/***************************************************************************
                         qgsmesh3dgeometry_p.cpp
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dgeometry_p.h"

#include <Qt3DRender/qattribute.h>
#include <Qt3DRender/qbuffer.h>

#include <Qt3DRender/qbufferdatagenerator.h>
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayerutils.h"


///@cond PRIVATE

using namespace Qt3DRender;

static QByteArray createTerrainVertexData( const QgsTriangularMesh &mesh,
    const QgsVector3D &origin,
    float vertScale )
{
  const int nVerts = mesh.vertices().count();

  QVector<QVector3D> normals = mesh.vertexNormals( vertScale );

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos,  vec3 normal
  const quint32 elementSize = 3 + 3;
  const quint32 stride = elementSize * sizeof( float );
  QByteArray bufferBytes;
  bufferBytes.resize( stride * nVerts );

  float *fptr = reinterpret_cast<float *>( bufferBytes.data() );

  for ( int i = 0; i < nVerts; i++ )
  {
    const QgsMeshVertex &vert = mesh.vertices().at( i );
    *fptr++ = float( vert.x() - origin.x() );
    *fptr++ = float( vert.z() - origin.z() ) * vertScale ;
    *fptr++ = float( -vert.y() + origin.y() );

    QVector3D normal = normals.at( i );
    normal = QVector3D( normal.x() * vertScale, -normal.y() * vertScale, normal.z() );
    normal.normalized();

    *fptr++ = normal.x();
    *fptr++ = normal.z();
    *fptr++ = normal.y();
  }

  return bufferBytes;
}

static QByteArray createDatasetVertexData( const QgsTriangularMesh &mesh,
    const QVector<double> verticalMagnitude,
    const QVector<double> scalarMagnitude,
    const QgsVector3D &origin,
    float vertScale,
    bool verticalRelative = false )
{
  const int nVerts = mesh.vertices().count();

  //Calculate normales with Z value equal to verticaleMagnitude
  QVector<QVector3D> normals = QgsMeshLayerUtils::calculateNormals( mesh, verticalMagnitude, verticalRelative );

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec3 normal, float magnitude
  const quint32 elementSize = 3 + 3 + 1 ;
  const quint32 stride = elementSize * sizeof( float );
  QByteArray bufferBytes;
  bufferBytes.resize( stride * nVerts );

  float *fptr = reinterpret_cast<float *>( bufferBytes.data() );

  for ( int i = 0; i < nVerts; i++ )
  {
    const QgsMeshVertex &vert = mesh.vertices().at( i );
    double vertMag = verticalMagnitude.at( i );
    double scalarMag = scalarMagnitude.at( i );
    if ( verticalRelative )
      vertMag += vert.z();

    *fptr++ = float( vert.x() - origin.x() );
    *fptr++ = float( vertMag - origin.z() ) * vertScale ;
    *fptr++ = float( -vert.y() + origin.y() );

    QVector3D normal = normals.at( i );
    normal = QVector3D( normal.x() * vertScale, -normal.y() * vertScale, normal.z() );
    normal.normalized();

    *fptr++ = normal.x();
    *fptr++ = normal.z();
    *fptr++ = normal.y();

    *fptr++ = float( scalarMag );
  }

  return bufferBytes;
}

static QByteArray createIndexData( const QgsTriangularMesh &mesh )
{
  const int faces = mesh.triangles().count();
  const quint32 indices = static_cast<quint32>( 3 * faces );
  Q_ASSERT( indices < std::numeric_limits<quint32>::max() );
  QByteArray indexBytes;
  indexBytes.resize( int( indices * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( int i = 0; i < faces; ++i )
  {
    const QgsMeshFace &face = mesh.triangles().at( i );
    for ( int i = 0; i < 3; ++i )
      *indexPtr++ = quint32( face.at( i ) );
  }

  return indexBytes;
}


static QByteArray createDatasetIndexData( const QgsTriangularMesh &mesh, const QgsMeshDataBlock &mActiveFaceFlagValues, int activeFaceCount )
{
  const int trianglesCount = mesh.triangles().count();
  const quint32 indices = static_cast<quint32>( 3 * activeFaceCount );
  QByteArray indexBytes;
  indexBytes.resize( int( indices * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( int i = 0; i < trianglesCount; ++i )
  {
    int nativeFaceIndex = mesh.trianglesToNativeFaces()[i];
    const bool isActive = mActiveFaceFlagValues.active().isEmpty() || mActiveFaceFlagValues.active( nativeFaceIndex );
    if ( !isActive )
      continue;
    const QgsMeshFace &face = mesh.triangles().at( i );
    for ( int i = 0; i < 3; ++i )
      *indexPtr++ = quint32( face.at( i ) );
  }

  return indexBytes;
}

//! Generates vertex buffer for Mesh using vertex Z value as verticale magnitude
class MeshTerrainVertexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit MeshTerrainVertexBufferFunctor( const QgsTriangularMesh &mesh,
        const QgsVector3D &origin,
        float vertScale )
      : mMesh( mesh ),
        mOrigin( origin ),
        mVertScale( vertScale )
    {}

    QByteArray operator()() final
    {
      return createTerrainVertexData( mMesh, mOrigin, mVertScale );

    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const MeshTerrainVertexBufferFunctor *otherFunctor = functor_cast<MeshTerrainVertexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh.triangles().count() == mMesh.triangles().count() &&
                 abs( otherFunctor->mVertScale - mVertScale ) < std::numeric_limits<float>::min() );
      return false;
    }

    QT3D_FUNCTOR( MeshTerrainVertexBufferFunctor )

  private:

    QgsTriangularMesh mMesh;
    QgsVector3D mOrigin;
    float mVertScale;

};

//! Generates index buffer for Mesh terrain
class MeshTerrainIndexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit MeshTerrainIndexBufferFunctor( const QgsTriangularMesh &mesh )
      : mMesh( mesh )
    {}

    QByteArray operator()() final
    {
      return createIndexData( mMesh );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const MeshTerrainIndexBufferFunctor *otherFunctor = functor_cast<MeshTerrainIndexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh.triangles().count() == mMesh.triangles().count() );
      return false;
    }

    QT3D_FUNCTOR( MeshTerrainIndexBufferFunctor )

  private:
    QgsTriangularMesh mMesh;
};

//! Generates index buffer for Mesh dataset
class MeshDatasetIndexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit MeshDatasetIndexBufferFunctor( const QgsTriangularMesh &mesh, const QgsMeshDataBlock &activeFace, int activeFaceCount )
      : mMesh( mesh ), mActiveFace( activeFace ), mActiveFaceCount( activeFaceCount )
    {}

    QByteArray operator()() final
    {
      return createDatasetIndexData( mMesh, mActiveFace, mActiveFaceCount );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const MeshDatasetIndexBufferFunctor *otherFunctor = functor_cast<MeshDatasetIndexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh.triangles().count() == mMesh.triangles().count() );
      return false;
    }

    QT3D_FUNCTOR( MeshDatasetIndexBufferFunctor )
  private:
    QgsTriangularMesh mMesh;
    QgsMeshDataBlock mActiveFace;
    int mActiveFaceCount;

};

//! Generates vertex buffer for Mesh using dataset value as verticale magnitude and for color
class MeshDatasetVertexBufferFunctor : public QBufferDataGenerator
{
  public:

    /**
     * verticalRelative parameter is true when the vertical magnitude provides from the sum of the z vertices and the scalar dataset chosen
     * for rendering the vertical component.
     */
    explicit MeshDatasetVertexBufferFunctor( const QgsTriangularMesh &mesh,
        const QVector<double> verticaleMagnitude,
        const QVector<double> scalarMagnitude,
        const QgsVector3D &origin,
        float vertScale,
        bool verticalRelative = false )
      : mMesh( mesh ),
        mVerticaleMagnitude( verticaleMagnitude ),
        mScalarMagnitude( scalarMagnitude ),
        mOrigin( origin ),
        mVertScale( vertScale ),
        mVerticalRelative( verticalRelative )
    {}

    QByteArray operator()() final
    {
      return createDatasetVertexData( mMesh,
                                      mVerticaleMagnitude,
                                      mScalarMagnitude,
                                      mOrigin,
                                      mVertScale,
                                      mVerticalRelative );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const MeshDatasetVertexBufferFunctor *otherFunctor = functor_cast<MeshDatasetVertexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh.triangles().count() == mMesh.triangles().count() &&
                 otherFunctor->mVerticaleMagnitude.count() == mVerticaleMagnitude.count() &&
                 otherFunctor->mScalarMagnitude.count() == mScalarMagnitude.count() &&
                 abs( otherFunctor->mVertScale - mVertScale ) < std::numeric_limits<float>::min() &&
                 otherFunctor->mVerticalRelative == mVerticalRelative );
      return false;
    }

    QT3D_FUNCTOR( MeshDatasetVertexBufferFunctor )

  private:
    QgsTriangularMesh mMesh;
    QVector<double> mVerticaleMagnitude;
    QVector<double> mScalarMagnitude;
    QgsVector3D mOrigin;
    float mVertScale;
    bool mVerticalRelative = false;

};



QgsMeshLayer *QgsMesh3dGeometry::meshLayer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayerRef.layer.data() );
}

QgsMesh3dGeometry::QgsMesh3dGeometry( QgsMeshLayer *layer,
                                      const QgsVector3D &origin,
                                      const QgsMesh3DSymbol &symbol,
                                      Qt3DCore::QNode *parent ):
  Qt3DRender::QGeometry( parent ),
  mOrigin( origin ),
  mVertScale( symbol.verticalScale() ),
  mLayerRef( layer )
{}

QgsMeshDataset3dGeometry::QgsMeshDataset3dGeometry(
  QgsMeshLayer *layer,
  const QgsDateTimeRange &timeRange,
  const QgsVector3D &origin,
  const QgsMesh3DSymbol &symbol,
  Qt3DCore::QNode *parent ):
  QgsMesh3dGeometry( layer, origin, symbol, parent ),
  mIsVerticalMagnitudeRelative( symbol.isVerticalMagnitudeRelative() ),
  mVerticalGroupDatasetIndex( symbol.verticalDatasetGroupIndex() ),
  mTimeRange( timeRange )
{
  init();
}

void QgsMeshDataset3dGeometry::init()
{
  QgsMeshLayer *layer = meshLayer();

  if ( !layer )
    return;

  if ( mVerticalGroupDatasetIndex < 0 )
    return;

  QVector<double> verticaleMagnitude;
  QVector<double> scalarMagnitude;
  QgsMeshDataBlock activeFaces;

  int activefaceCount = extractDataset( verticaleMagnitude, scalarMagnitude, activeFaces );
  if ( activefaceCount == 0 )
    return;

  QgsTriangularMesh triangularMesh = *layer->triangularMesh();

  if ( verticaleMagnitude.count() != triangularMesh.vertices().count()  ||
       scalarMagnitude.count() != triangularMesh.vertices().count() )
    return;

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );
  Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this );
#else
  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer( this );
  Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer( this );
#endif

  const int stride = ( 3 /*position*/ +
                       3 /*normale*/ +
                       1 /*magnitude*/ ) * sizeof( float );

  const uint nVerts = uint( triangularMesh.vertices().count() );

  prepareVerticesPositionAttribute( vertexBuffer, nVerts, stride, 0 );
  prepareVerticesNormalAttribute( vertexBuffer, nVerts, stride, 3 );
  prepareVerticesDatasetAttribute( vertexBuffer, nVerts, stride, 6 );

  prepareIndexesAttribute( indexBuffer, activefaceCount );


  Qt3DRender::QBufferDataGeneratorPtr vertexDataGenerator = Qt3DRender::QBufferDataGeneratorPtr(
        new MeshDatasetVertexBufferFunctor( triangularMesh, verticaleMagnitude, scalarMagnitude, mOrigin, mVertScale, mIsVerticalMagnitudeRelative ) );
  vertexBuffer->setDataGenerator( vertexDataGenerator );

  Qt3DRender::QBufferDataGeneratorPtr indexDataGenerator( new MeshDatasetIndexBufferFunctor(
        triangularMesh, activeFaces, activefaceCount ) );
  indexBuffer->setDataGenerator( indexDataGenerator );
}

int QgsMeshDataset3dGeometry::extractDataset( QVector<double> &verticalMagnitude, QVector<double> &scalarMagnitude, QgsMeshDataBlock &activeFaceFlagValues )
{
  QgsMeshLayer *layer = meshLayer();

  if ( !layer )
    return 0;

  QgsMeshDatasetIndex scalarDatasetIndex = layer->activeScalarDatasetAtTime( mTimeRange );

  if ( !scalarDatasetIndex.isValid() )
    return 0;

  if ( mVerticalGroupDatasetIndex < 0 )
    return 0;

  QgsTriangularMesh triangularMesh = *layer->triangularMesh();
  const QgsMesh nativeMesh = *layer->nativeMesh();

  //extract the scalar dataset used to render vertical magnitude of geometry
  //define the vertical magnitude datasetIndex
  QgsMeshDatasetIndex verticalMagDatasetIndex;
  verticalMagDatasetIndex = layer->datasetIndexAtTime( mTimeRange, mVerticalGroupDatasetIndex );
  if ( !verticalMagDatasetIndex.isValid() )
  {
    //if invalid (for example, static mode) use the scalar dataset index
    int vertDataSetIndex = scalarDatasetIndex.dataset();
    vertDataSetIndex = std::min( vertDataSetIndex, layer->datasetCount( mVerticalGroupDatasetIndex ) - 1 );
    verticalMagDatasetIndex = QgsMeshDatasetIndex( vertDataSetIndex, mVerticalGroupDatasetIndex );
  }
  //define the active face for vertical magnitude, the inactive faces will not be rendered
  // The active face flag values are defined based on the vertival magnitude dataset
  activeFaceFlagValues = layer->areFacesActive( verticalMagDatasetIndex, 0, nativeMesh.faces.count() );
  verticalMagnitude = QgsMeshLayerUtils::calculateMagnitudeOnVertices(
                        layer,
                        verticalMagDatasetIndex,
                        &activeFaceFlagValues );

  //count active faces
  int activeTriangularCount = 0;
  if ( activeFaceFlagValues.active().isEmpty() )
    activeTriangularCount = triangularMesh.triangles().count();
  else
    for ( int i = 0; i < triangularMesh.triangles().count(); ++i )
    {
      int nativeIndex = triangularMesh.trianglesToNativeFaces()[i];
      if ( activeFaceFlagValues.active( nativeIndex ) )
        activeTriangularCount++;
    }

  //extract the scalar dataset used to render color shading
  QgsMeshDataBlock scalarActiveFaceFlagValues =
    layer->areFacesActive( scalarDatasetIndex, 0, nativeMesh.faces.count() );
  scalarMagnitude = QgsMeshLayerUtils::calculateMagnitudeOnVertices(
                      layer,
                      scalarDatasetIndex,
                      &scalarActiveFaceFlagValues );

  return activeTriangularCount;
}


QgsMeshTerrain3dGeometry::QgsMeshTerrain3dGeometry(
  QgsMeshLayer *layer,
  const QgsVector3D &origin,
  const QgsMesh3DSymbol &symbol,
  Qt3DCore::QNode *parent ):
  QgsMesh3dGeometry( layer, origin, symbol, parent )
{
  init();
}

void QgsMeshTerrain3dGeometry::init()
{
  QgsMeshLayer *layer = meshLayer();

  if ( !layer )
    return;

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );
  Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this );
#else
  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer( this );
  Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer( this );
#endif

  QgsTriangularMesh triangularMesh = *layer->triangularMesh();

  const int stride = ( 3 /*position*/ +
                       3 /*normale*/ ) * sizeof( float );

  const uint nVerts = uint( triangularMesh.vertices().count() );

  prepareVerticesPositionAttribute( vertexBuffer, nVerts, stride, 0 );
  prepareVerticesNormalAttribute( vertexBuffer, nVerts, stride, 3 );
  prepareIndexesAttribute( indexBuffer, triangularMesh.triangles().count() );

  Qt3DRender::QBufferDataGeneratorPtr vertexDataGenerator =
    Qt3DRender::QBufferDataGeneratorPtr( new MeshTerrainVertexBufferFunctor( triangularMesh,  mOrigin, mVertScale ) );
  vertexBuffer->setDataGenerator( vertexDataGenerator );

  Qt3DRender::QBufferDataGeneratorPtr indexDataGenerator( new MeshTerrainIndexBufferFunctor( triangularMesh ) );
  indexBuffer->setDataGenerator( indexDataGenerator );

}

void QgsMesh3dGeometry::prepareVerticesPositionAttribute( Qt3DRender::QBuffer *buffer, int count, int stride, int offset )
{
  Qt3DRender::QAttribute *positionAttribute = new QAttribute( this );

  positionAttribute->setName( QAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( QAttribute::VertexAttribute );
  positionAttribute->setBuffer( buffer );
  positionAttribute->setByteStride( stride );
  positionAttribute->setByteOffset( offset * sizeof( float ) );
  positionAttribute->setCount( count );

  addAttribute( positionAttribute );
}

void QgsMesh3dGeometry::prepareVerticesNormalAttribute( Qt3DRender::QBuffer *buffer, int count, int stride, int offset )
{
  Qt3DRender::QAttribute *normalAttribute = new QAttribute( this );

  normalAttribute->setName( QAttribute::defaultNormalAttributeName() );
  normalAttribute->setVertexBaseType( QAttribute::Float );
  normalAttribute->setVertexSize( 3 );
  normalAttribute->setAttributeType( QAttribute::VertexAttribute );
  normalAttribute->setBuffer( buffer );
  normalAttribute->setByteStride( stride );
  normalAttribute->setByteOffset( offset * sizeof( float ) );
  normalAttribute->setCount( count );

  addAttribute( normalAttribute );
}

void QgsMeshDataset3dGeometry::prepareVerticesDatasetAttribute( Qt3DRender::QBuffer *buffer, int count, int stride, int offset )
{
  Qt3DRender::QAttribute *magnitudeAttribute = new QAttribute( this );

  magnitudeAttribute->setName( "scalarMagnitude" );
  magnitudeAttribute->setVertexBaseType( QAttribute::Float );
  magnitudeAttribute->setVertexSize( 1 );
  magnitudeAttribute->setAttributeType( QAttribute::VertexAttribute );
  magnitudeAttribute->setBuffer( buffer );
  magnitudeAttribute->setByteStride( stride );
  magnitudeAttribute->setByteOffset( offset * sizeof( float ) );
  magnitudeAttribute->setCount( count );

  addAttribute( magnitudeAttribute );
}

void QgsMesh3dGeometry::prepareIndexesAttribute( Qt3DRender::QBuffer *buffer, int trianglesCount )
{

  Qt3DRender::QAttribute *indexAttribute = new QAttribute( this );
  indexAttribute->setAttributeType( QAttribute::IndexAttribute );
  indexAttribute->setVertexBaseType( QAttribute::UnsignedInt );
  indexAttribute->setBuffer( buffer );

  // Each primitive has 3 vertives
  indexAttribute->setCount( trianglesCount * 3 );

  addAttribute( indexAttribute );
}

