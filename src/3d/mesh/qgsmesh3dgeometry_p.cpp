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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif


#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayerutils.h"


///@cond PRIVATE

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
    normal.normalize();

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
  const QgsMeshDataset3DGeometry::VertexData &data )
{
  const int nVerts = mesh.vertices().count();

  const QVector<double> verticalMagnitude =
    QgsMeshLayerUtils::calculateMagnitudeOnVertices( nativeMesh, data.verticalGroupMetadata, data.verticalData, data.activeFaceFlagValues );

  const QVector<double> scalarMagnitude =
    QgsMeshLayerUtils::calculateMagnitudeOnVertices( nativeMesh, data.scalarGroupMetadata, data.scalarData, data.activeFaceFlagValues );

  //Calculate normals with Z value equal to verticaleMagnitude
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
    normal.normalize();

    *fptr++ = normal.x();
    *fptr++ = normal.z();
    *fptr++ = normal.y();

    *fptr++ = float( scalarMag );
  }

  return bufferBytes;
}

static QByteArray createIndexData( const QgsTriangularMesh &mesh, const QgsRectangle &extent )
{
  const QList<int> facesInExtent = mesh.faceIndexesForRectangle( extent );
  const quint32 indices = static_cast<quint32>( 3 * facesInExtent.count() );
  Q_ASSERT( indices < std::numeric_limits<quint32>::max() );

  // count non void faces
  int nonVoidFaces = 0;
  for ( const int nativeFaceIndex : facesInExtent )
    if ( !mesh.triangles().at( nativeFaceIndex ).isEmpty() )
      nonVoidFaces++;

  QByteArray indexBytes;
  indexBytes.resize( int( nonVoidFaces * 3 * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( const int nativeFaceIndex : facesInExtent )
  {
    const QgsMeshFace &face = mesh.triangles().at( nativeFaceIndex );
    if ( face.isEmpty() )
      continue;
    for ( int j = 0; j < 3; ++j )
      *indexPtr++ = quint32( face.at( j ) );
  }

  return indexBytes;
}

static QByteArray createDatasetIndexData( const QgsTriangularMesh &mesh, const QgsMeshDataBlock &mActiveFaceFlagValues, const QgsRectangle &extent )
{
  const QList<int> facesInExtent = mesh.faceIndexesForRectangle( extent );
  int activeFaceCount = 0;

  // First we need to know about the count of active faces
  if ( mActiveFaceFlagValues.active().isEmpty() )
    activeFaceCount = facesInExtent.count();
  else
  {
    for ( const int nativeFaceIndex : facesInExtent )
    {
      if ( mActiveFaceFlagValues.active( nativeFaceIndex ) )
        activeFaceCount++;
    }
  }

  const quint32 indices = static_cast<quint32>( 3 * activeFaceCount );
  QByteArray indexBytes;
  indexBytes.resize( int( indices * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( const int nativeFaceIndex : facesInExtent )
  {
    const bool isActive = mActiveFaceFlagValues.active().isEmpty() || mActiveFaceFlagValues.active( nativeFaceIndex );
    if ( !isActive )
      continue;
    const QgsMeshFace &face = mesh.triangles().at( nativeFaceIndex );
    for ( int j = 0; j < 3; ++j )
      *indexPtr++ = quint32( face.at( j ) );
  }

  return indexBytes;
}

QgsMesh3DGeometry::QgsMesh3DGeometry( const QgsTriangularMesh &triangularMesh,
                                      const QgsVector3D &origin,
                                      const QgsRectangle &extent,
                                      double verticalScale,
                                      Qt3DCore::QNode *parent )
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  : Qt3DRender::QGeometry( parent )
#else
  : Qt3DCore::QGeometry( parent )
#endif
  , mOrigin( origin )
  , mExtent( extent )
  , mVertScale( verticalScale )
  , mTriangulaMesh( triangularMesh )
{
  mVertexBuffer = new Qt3DQBuffer( this );
  mIndexBuffer = new Qt3DQBuffer( this );
}

QgsMeshDataset3DGeometry::QgsMeshDataset3DGeometry(
  const QgsTriangularMesh &triangularMesh,
  QgsMeshLayer *layer,
  const QgsDateTimeRange &timeRange,
  const QgsVector3D &origin,
  const QgsRectangle &extent,
  const QgsMesh3DSymbol *symbol,
  Qt3DCore::QNode *parent )
  : QgsMesh3DGeometry( triangularMesh, origin, extent, symbol->verticalScale(), parent )
  , mIsVerticalMagnitudeRelative( symbol->isVerticalMagnitudeRelative() )
  , mVerticalGroupDatasetIndex( symbol->verticalDatasetGroupIndex() )
  , mTimeRange( timeRange )
  , mLayerRef( layer )
{

  const int stride = ( 3 /*position*/ +
                       3 /*normal*/ +
                       1 /*magnitude*/ ) * sizeof( float );

  prepareVerticesPositionAttribute( mVertexBuffer,  stride, 0 );
  prepareVerticesNormalAttribute( mVertexBuffer,  stride, 3 );
  prepareVerticesDatasetAttribute( mVertexBuffer,  stride, 6 );
  prepareIndexesAttribute( mIndexBuffer );

  prepareData();
}

void QgsMeshDataset3DGeometry::getData()
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

void QgsMeshDataset3DGeometry::prepareData()
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

  mBuilder = new QgsMeshDataset3DGeometryBuilder( mTriangulaMesh, nativeMesh, mOrigin, mExtent, mVertScale, data, this );
  connect( mBuilder, &QgsMeshDataset3DGeometryBuilder::dataIsReady, this, &QgsMeshDataset3DGeometry::getData );

  mBuilder->start();
}


QgsMeshTerrain3DGeometry::QgsMeshTerrain3DGeometry( const QgsTriangularMesh &triangularMesh,
    const QgsVector3D &origin,
    const QgsRectangle &extent,
    double verticalScale,
    Qt3DCore::QNode *parent )
  : QgsMesh3DGeometry( triangularMesh, origin, extent, verticalScale, parent )
{

  const int stride = ( 3 /*position*/ +
                       3 /*normal*/ ) * sizeof( float );

  prepareVerticesPositionAttribute( mVertexBuffer, stride, 0 );
  prepareVerticesNormalAttribute( mVertexBuffer,  stride, 3 );
  prepareIndexesAttribute( mIndexBuffer );

  mBuilder = new QgsMesh3DGeometryBuilder( triangularMesh, origin, extent, mVertScale, this );
  connect( mBuilder, &QgsMesh3DGeometryBuilder::dataIsReady, this, &QgsMeshTerrain3DGeometry::getData );
  mBuilder->start();
}

void QgsMesh3DGeometry::getData()
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

void QgsMesh3DGeometry::prepareVerticesPositionAttribute( Qt3DQBuffer *buffer, int stride, int offset )
{
  mPositionAttribute = new Qt3DQAttribute( this );

  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( buffer );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setByteOffset( offset * sizeof( float ) );
  mPositionAttribute->setCount( 0 );

  addAttribute( mPositionAttribute );
}

void QgsMesh3DGeometry::prepareVerticesNormalAttribute( Qt3DQBuffer *buffer, int stride, int offset )
{
  mNormalAttribute = new Qt3DQAttribute( this );

  mNormalAttribute->setName( Qt3DQAttribute::defaultNormalAttributeName() );
  mNormalAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mNormalAttribute->setVertexSize( 3 );
  mNormalAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mNormalAttribute->setBuffer( buffer );
  mNormalAttribute->setByteStride( stride );
  mNormalAttribute->setByteOffset( offset * sizeof( float ) );
  mNormalAttribute->setCount( 0 );

  addAttribute( mNormalAttribute );
}

void QgsMeshDataset3DGeometry::prepareVerticesDatasetAttribute( Qt3DQBuffer *buffer, int stride, int offset )
{
  mMagnitudeAttribute = new Qt3DQAttribute( this );

  mMagnitudeAttribute->setName( "scalarMagnitude" );
  mMagnitudeAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mMagnitudeAttribute->setVertexSize( 1 );
  mMagnitudeAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mMagnitudeAttribute->setBuffer( buffer );
  mMagnitudeAttribute->setByteStride( stride );
  mMagnitudeAttribute->setByteOffset( offset * sizeof( float ) );
  mMagnitudeAttribute->setCount( 0 );

  addAttribute( mMagnitudeAttribute );
}

QgsMeshLayer *QgsMeshDataset3DGeometry::meshLayer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayerRef.layer.data() );
}

void QgsMesh3DGeometry::prepareIndexesAttribute( Qt3DQBuffer *buffer )
{
  mIndexAttribute = new Qt3DQAttribute( this );
  mIndexAttribute->setAttributeType( Qt3DQAttribute::IndexAttribute );
  mIndexAttribute->setVertexBaseType( Qt3DQAttribute::UnsignedInt );
  mIndexAttribute->setBuffer( buffer );

  mIndexAttribute->setCount( 0 );
  addAttribute( mIndexAttribute );
}


QgsMesh3DGeometryBuilder::QgsMesh3DGeometryBuilder( const QgsTriangularMesh &mesh, const QgsVector3D &origin, const QgsRectangle &extent, float vertScale, QObject *parent ):
  QObject( parent )
  , mMesh( mesh )
  , mOrigin( origin )
  , mExtent( extent )
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
  mFutureIndex = QtConcurrent::run( createIndexData, mMesh, mExtent );
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
  const QgsRectangle &extent,
  float vertScale,
  const QgsMeshDataset3DGeometry::VertexData &vertexData,
  QObject *parent ):
  QgsMesh3DGeometryBuilder( mesh, origin, extent, vertScale, parent )
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

  mFutureIndex = QtConcurrent::run( createDatasetIndexData, mMesh, mVertexData.activeFaceFlagValues, mExtent );
  mWatcherIndex->setFuture( mFutureIndex );
}
