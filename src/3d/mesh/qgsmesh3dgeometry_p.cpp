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

#include <QFutureWatcher>
#include <QtConcurrentRun>

#include <Qt3DRender/qattribute.h>
#include <Qt3DRender/qbuffer.h>

#include <Qt3DRender/qbufferdatagenerator.h>
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayerutils.h"


///@cond PRIVATE

using namespace Qt3DRender;

static QByteArray createTerrainVertexData(
  const QgsTriangularMesh &mesh,
  const QgsVector3D &origin,
  float vertScale )
{
  const int nVerts = mesh.vertices().count();

  const QVector<QVector3D> normals = mesh.vertexNormals( vertScale );

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

static QByteArray createDatasetVertexData(
  const QgsTriangularMesh &mesh,
  const QgsMesh &nativeMesh,
  const QgsVector3D &origin,
  float vertScale,
  const QgsMeshDataset3dGeometry::VertexData &data )
{
  const int nVerts = mesh.vertices().count();

  const QVector<double> verticalMagnitude =
    QgsMeshLayerUtils::calculateMagnitudeOnVertices( nativeMesh, data.verticalGroupMetadata, data.verticalData, data.activeFaceFlagValues );

  const QVector<double> scalarMagnitude =
    QgsMeshLayerUtils::calculateMagnitudeOnVertices( nativeMesh, data.scalarGroupMetadata, data.scalarData, data.activeFaceFlagValues );

  //Calculate normales with Z value equal to verticaleMagnitude
  const QVector<QVector3D> normals = QgsMeshLayerUtils::calculateNormals( mesh, verticalMagnitude, data.isVerticalMagnitudeRelative );

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
    const double scalarMag = scalarMagnitude.at( i );
    if ( data.isVerticalMagnitudeRelative )
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

  // count non void faces
  int nonVoidFaces = 0;
  for ( int i = 0; i < faces; ++i )
    if ( !mesh.triangles().at( i ).isEmpty() )
      nonVoidFaces++;

  QByteArray indexBytes;
  indexBytes.resize( int( nonVoidFaces * 3 * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( int i = 0; i < faces; ++i )
  {
    const QgsMeshFace &face = mesh.triangles().at( i );
    if ( face.isEmpty() )
      continue;
    for ( int j = 0; j < 3; ++j )
      *indexPtr++ = quint32( face.at( j ) );
  }

  return indexBytes;
}

static QByteArray createDatasetIndexData( const QgsTriangularMesh &mesh, QgsMeshDataBlock &mActiveFaceFlagValues )
{
  int activeFaceCount = 0;

  // First we need to know about the count of active faces
  if ( mActiveFaceFlagValues.active().isEmpty() )
    activeFaceCount = mesh.triangles().count();
  else
  {
    for ( int i = 0; i < mesh.triangles().count(); ++i )
    {
      const int nativeIndex = mesh.trianglesToNativeFaces()[i];
      if ( mActiveFaceFlagValues.active( nativeIndex ) )
        activeFaceCount++;
    }
  }

  const int trianglesCount = mesh.triangles().count();
  const quint32 indices = static_cast<quint32>( 3 * activeFaceCount );
  QByteArray indexBytes;
  indexBytes.resize( int( indices * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( int i = 0; i < trianglesCount; ++i )
  {
    const int nativeFaceIndex = mesh.trianglesToNativeFaces()[i];
    const bool isActive = mActiveFaceFlagValues.active().isEmpty() || mActiveFaceFlagValues.active( nativeFaceIndex );
    if ( !isActive )
      continue;
    const QgsMeshFace &face = mesh.triangles().at( i );
    for ( int j = 0; j < 3; ++j )
      *indexPtr++ = quint32( face.at( j ) );
  }

  return indexBytes;
}

QgsMesh3dGeometry::QgsMesh3dGeometry( const QgsTriangularMesh &triangularMesh,
                                      const QgsVector3D &origin,
                                      double verticalScale,
                                      Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , mOrigin( origin )
  , mVertScale( verticalScale )
  , mTriangulaMesh( triangularMesh )
{
  mVertexBuffer = new Qt3DRender::QBuffer( this );
  mIndexBuffer = new Qt3DRender::QBuffer( this );
}

QgsMeshDataset3dGeometry::QgsMeshDataset3dGeometry(
  const QgsTriangularMesh &triangularMesh,
  QgsMeshLayer *layer,
  const QgsDateTimeRange &timeRange,
  const QgsVector3D &origin,
  const QgsMesh3DSymbol *symbol,
  Qt3DCore::QNode *parent )
  : QgsMesh3dGeometry( triangularMesh, origin, symbol->verticalScale(), parent )
  , mIsVerticalMagnitudeRelative( symbol->isVerticalMagnitudeRelative() )
  , mVerticalGroupDatasetIndex( symbol->verticalDatasetGroupIndex() )
  , mTimeRange( timeRange )
  , mLayerRef( layer )
{

  const int stride = ( 3 /*position*/ +
                       3 /*normale*/ +
                       1 /*magnitude*/ ) * sizeof( float );

  prepareVerticesPositionAttribute( mVertexBuffer,  stride, 0 );
  prepareVerticesNormalAttribute( mVertexBuffer,  stride, 3 );
  prepareVerticesDatasetAttribute( mVertexBuffer,  stride, 6 );
  prepareIndexesAttribute( mIndexBuffer );

  prepareData();
}

void QgsMeshDataset3dGeometry::getData()
{
  const QByteArray indexData = mBuilder->indexData();
  const uint activeIndexCount = indexData.size() / sizeof( qint32 );

  const uint nVerts = uint( mTriangulaMesh.vertices().count() );

  mPositionAttribute->setCount( nVerts );
  mNormalAttribute->setCount( nVerts );
  mMagnitudeAttribute->setCount( nVerts );
  mIndexAttribute->setCount( activeIndexCount );

  mVertexBuffer->setData( mBuilder->vertexData() );
  mIndexBuffer->setData( indexData );
}

void QgsMeshDataset3dGeometry::prepareData()
{
  QgsMeshLayer *layer = meshLayer();

  if ( !layer )
    return;

  const QgsMeshDatasetIndex scalarDatasetIndex = layer->activeScalarDatasetAtTime( mTimeRange );

  if ( !scalarDatasetIndex.isValid() )
    return;

  if ( mVerticalGroupDatasetIndex < 0 )
    return;

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
    verticalMagDatasetIndex = QgsMeshDatasetIndex( mVerticalGroupDatasetIndex, vertDataSetIndex );
  }

  if ( mVerticalGroupDatasetIndex < 0 )
    return;

  VertexData data;
  data.verticalGroupMetadata = layer->datasetGroupMetadata( mVerticalGroupDatasetIndex );
  const bool verticalDataOnVertices =  data.verticalGroupMetadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
  int datacount = verticalDataOnVertices ? nativeMesh.vertices.count() : nativeMesh.faces.count();
  data.verticalData = QgsMeshLayerUtils::datasetValues( layer, verticalMagDatasetIndex, 0, datacount );

  data.scalarGroupMetadata = layer->datasetGroupMetadata( layer->activeScalarDatasetAtTime( mTimeRange ) );
  const bool scalarDataOnVertices =  data.scalarGroupMetadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
  datacount = scalarDataOnVertices ? nativeMesh.vertices.count() : nativeMesh.faces.count();
  data.scalarData = QgsMeshLayerUtils::datasetValues( layer, scalarDatasetIndex, 0, datacount );

  if ( ( verticalDataOnVertices && ( data.verticalData.count() != mTriangulaMesh.vertices().count() ) )  ||
       ( scalarDataOnVertices && ( data.scalarData.count() != mTriangulaMesh.vertices().count() ) ) )
    return;

  if ( ( !verticalDataOnVertices && ( data.verticalData.count() != nativeMesh.faces.count() ) )  ||
       ( !scalarDataOnVertices && ( data.scalarData.count() != nativeMesh.faces.count() ) ) )
    return;

  data.activeFaceFlagValues = layer->areFacesActive( scalarDatasetIndex, 0, nativeMesh.faces.count() );
  data.isVerticalMagnitudeRelative = mIsVerticalMagnitudeRelative;

  mBuilder = new QgsMeshDataset3DGeometryBuilder( mTriangulaMesh, nativeMesh, mOrigin, mVertScale, data, this );
  connect( mBuilder, &QgsMeshDataset3DGeometryBuilder::dataIsReady, this, &QgsMeshDataset3dGeometry::getData );

  mBuilder->start();
}


QgsMeshTerrain3dGeometry::QgsMeshTerrain3dGeometry( const QgsTriangularMesh &triangularMesh,
    const QgsVector3D &origin,
    double verticalSacle,
    Qt3DCore::QNode *parent )
  : QgsMesh3dGeometry( triangularMesh, origin, verticalSacle, parent )
{

  const int stride = ( 3 /*position*/ +
                       3 /*normale*/ ) * sizeof( float );

  prepareVerticesPositionAttribute( mVertexBuffer, stride, 0 );
  prepareVerticesNormalAttribute( mVertexBuffer,  stride, 3 );
  prepareIndexesAttribute( mIndexBuffer );

  mBuilder = new QgsMesh3DGeometryBuilder( triangularMesh, origin, mVertScale, this );
  connect( mBuilder, &QgsMesh3DGeometryBuilder::dataIsReady, this, &QgsMeshTerrain3dGeometry::getData );
  mBuilder->start();
}

void QgsMesh3dGeometry::getData()
{
  const uint nVerts = uint( mTriangulaMesh.vertices().count() );

  const QByteArray indexData = mBuilder->indexData();
  const uint effectiveIndexCount = indexData.size() / sizeof( qint32 );

  mPositionAttribute->setCount( nVerts );
  mNormalAttribute->setCount( nVerts );
  mIndexAttribute->setCount( effectiveIndexCount );

  mVertexBuffer->setData( mBuilder->vertexData() );
  mIndexBuffer->setData( indexData );
}

void QgsMesh3dGeometry::prepareVerticesPositionAttribute( Qt3DRender::QBuffer *buffer, int stride, int offset )
{
  mPositionAttribute = new QAttribute( this );

  mPositionAttribute->setName( QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( buffer );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setByteOffset( offset * sizeof( float ) );
  mPositionAttribute->setCount( 0 );

  addAttribute( mPositionAttribute );
}

void QgsMesh3dGeometry::prepareVerticesNormalAttribute( Qt3DRender::QBuffer *buffer, int stride, int offset )
{
  mNormalAttribute = new QAttribute( this );

  mNormalAttribute->setName( QAttribute::defaultNormalAttributeName() );
  mNormalAttribute->setVertexBaseType( QAttribute::Float );
  mNormalAttribute->setVertexSize( 3 );
  mNormalAttribute->setAttributeType( QAttribute::VertexAttribute );
  mNormalAttribute->setBuffer( buffer );
  mNormalAttribute->setByteStride( stride );
  mNormalAttribute->setByteOffset( offset * sizeof( float ) );
  mNormalAttribute->setCount( 0 );

  addAttribute( mNormalAttribute );
}

void QgsMeshDataset3dGeometry::prepareVerticesDatasetAttribute( Qt3DRender::QBuffer *buffer, int stride, int offset )
{
  mMagnitudeAttribute = new QAttribute( this );

  mMagnitudeAttribute->setName( "scalarMagnitude" );
  mMagnitudeAttribute->setVertexBaseType( QAttribute::Float );
  mMagnitudeAttribute->setVertexSize( 1 );
  mMagnitudeAttribute->setAttributeType( QAttribute::VertexAttribute );
  mMagnitudeAttribute->setBuffer( buffer );
  mMagnitudeAttribute->setByteStride( stride );
  mMagnitudeAttribute->setByteOffset( offset * sizeof( float ) );
  mMagnitudeAttribute->setCount( 0 );

  addAttribute( mMagnitudeAttribute );
}

QgsMeshLayer *QgsMeshDataset3dGeometry::meshLayer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayerRef.layer.data() );
}

void QgsMesh3dGeometry::prepareIndexesAttribute( Qt3DRender::QBuffer *buffer )
{
  mIndexAttribute = new QAttribute( this );
  mIndexAttribute->setAttributeType( QAttribute::IndexAttribute );
  mIndexAttribute->setVertexBaseType( QAttribute::UnsignedInt );
  mIndexAttribute->setBuffer( buffer );

  mIndexAttribute->setCount( 0 );
  addAttribute( mIndexAttribute );
}


QgsMesh3DGeometryBuilder::QgsMesh3DGeometryBuilder( const QgsTriangularMesh &mesh, const QgsVector3D &origin, float vertScale, QObject *parent ):
  QObject( parent )
  , mMesh( mesh )
  , mOrigin( origin )
  , mVertScale( vertScale )
{}

void QgsMesh3DGeometryBuilder::start()
{
  const QMutexLocker locker( &mMutex );

  mWatcherVertex = new QFutureWatcher<QByteArray>( this );
  connect( mWatcherVertex, &QFutureWatcher<int>::finished, this, &QgsMesh3DGeometryBuilder::vertexFinished );
  mFutureVertex = QtConcurrent::run( createTerrainVertexData, mMesh, mOrigin, mVertScale );
  mWatcherVertex->setFuture( mFutureVertex );

  mWatcherIndex = new QFutureWatcher<QByteArray>( this );
  connect( mWatcherIndex, &QFutureWatcher<int>::finished, this, &QgsMesh3DGeometryBuilder::indexFinished );
  mFutureIndex = QtConcurrent::run( createIndexData, mMesh );
  mWatcherIndex->setFuture( mFutureIndex );
}

void QgsMesh3DGeometryBuilder::vertexFinished()
{
  const QMutexLocker locker( &mMutex );
  mVertexFinished = true;
  if ( mVertexFinished && mIndexFinished )
  {
    emit dataIsReady();
  }
}

void QgsMesh3DGeometryBuilder::indexFinished()
{
  const QMutexLocker locker( &mMutex );
  mIndexFinished = true;
  if ( mVertexFinished && mIndexFinished )
  {
    emit dataIsReady();
  }
}

QgsMeshDataset3DGeometryBuilder::QgsMeshDataset3DGeometryBuilder
( const QgsTriangularMesh &mesh,
  const QgsMesh &nativeMesh,
  const QgsVector3D &origin,
  float vertScale,
  const QgsMeshDataset3dGeometry::VertexData &vertexData,
  QObject *parent ):
  QgsMesh3DGeometryBuilder( mesh, origin, vertScale, parent )
  , mNativeMesh( nativeMesh )
  , mVertexData( vertexData )
{}

void QgsMeshDataset3DGeometryBuilder::start()
{
  const QMutexLocker locker( &mMutex );

  mWatcherVertex = new QFutureWatcher<QByteArray>( this );
  connect( mWatcherVertex, &QFutureWatcher<int>::finished, this, &QgsMeshDataset3DGeometryBuilder::vertexFinished );
  mFutureVertex = QtConcurrent::run( createDatasetVertexData, mMesh, mNativeMesh, mOrigin, mVertScale, mVertexData );
  mWatcherVertex->setFuture( mFutureVertex );

  mWatcherIndex = new QFutureWatcher<QByteArray>( this );
  connect( mWatcherIndex, &QFutureWatcher<int>::finished, this, &QgsMeshDataset3DGeometryBuilder::indexFinished );
  mFutureIndex = QtConcurrent::run( createDatasetIndexData, mMesh, mVertexData.activeFaceFlagValues );
  mWatcherIndex->setFuture( mFutureIndex );
}
