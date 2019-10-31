#include "qgstindemterraintilegeometry_p.h"

#include <Qt3DRender/qattribute.h>
#include <Qt3DRender/qbuffer.h>

#include <Qt3DRender/qbufferdatagenerator.h>

using namespace Qt3DRender;

static QByteArray createPlaneVertexData( QgsTriangularMeshTile mesh, float vertScale )
{
  const int nVerts = mesh.verticesCount();

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  const quint32 elementSize = 3 + 2 + 3;
  const quint32 stride = elementSize * sizeof( float );
  QByteArray bufferBytes;
  bufferBytes.resize( stride * nVerts );
  QgsRectangle extent = mesh.tileExtent();
  float w = float( extent.width() );
  float h = float( extent.height() );
  float x0 = float( extent.xMinimum() );
  float y0 = float( extent.yMaximum() );

  float *fptr = reinterpret_cast<float *>( bufferBytes.data() );


  for ( int i = 0; i < nVerts; i++ )
  {
    const QgsMeshVertex &vert = mesh.vertex( i );
    *fptr++ = float( vert.x() );
    *fptr++ = float( vert.z() ) * vertScale ;
    *fptr++ = float( -vert.y() );

    ///TODO : need to calculate texture coordinates
    *fptr++ = float( ( vert.x() - x0 ) / w );
    *fptr++ = float( ( y0 - vert.y() ) / h );


    QVector3D normal = mesh.vertexNormalVector( i );
    normal = QVector3D( normal.x(), -normal.y(), normal.z() * vertScale );
    normal.normalized();

    *fptr++ = normal.x();
    *fptr++ = normal.z();
    *fptr++ = normal.y();
  }

  qDebug() << "return buffer bytes";
  return bufferBytes;
}

static QByteArray createPlaneIndexData( QgsTriangularMeshTile mesh )
{

  const int faces = mesh.faceCount();
  const quint32 indices = static_cast<quint32>( 3 * faces );
  Q_ASSERT( indices < std::numeric_limits<quint32>::max() );
  QByteArray indexBytes;
  indexBytes.resize( int( indices * sizeof( quint32 ) ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  for ( int i = 0; i < faces; ++i )
  {

    const QgsMeshFace &face = mesh.triangle( i );
    for ( int i = 0; i < 3; ++i )
      *indexPtr++ = quint32( face.at( 2 - i ) ); //because of the negative y value, the clockwise is changed
  }

  qDebug() << "return index bytes";
  return indexBytes;
}

;

//! Generates vertex buffer for DEM terrain tiles
class TinPlaneVertexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit TinPlaneVertexBufferFunctor( QgsTriangularMeshTile mesh,  float vertScale )
      : mMesh( mesh ), mVertScale( vertScale )
    {}

    QByteArray operator()() final
    {
      qDebug() << "create plane vertex data";
      return createPlaneVertexData( mMesh, mVertScale );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const TinPlaneVertexBufferFunctor *otherFunctor = functor_cast<TinPlaneVertexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh == mMesh );
      return false;
    }

    QT3D_FUNCTOR( TinPlaneVertexBufferFunctor )

  private:

    QgsTriangularMeshTile mMesh;
    float mVertScale;

};



//! Generates index buffer for DEM terrain tiles
class TinPlaneIndexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit TinPlaneIndexBufferFunctor( QgsTriangularMeshTile mesh )
      : mMesh( mesh )
    {}

    QByteArray operator()() final
    {
      qDebug() << "create plane index data";
      return createPlaneIndexData( mMesh );
    }

    bool operator ==( const QBufferDataGenerator &other ) const final
    {
      const TinPlaneIndexBufferFunctor *otherFunctor = functor_cast<TinPlaneIndexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->mMesh == mMesh );
      return false;
    }

    QT3D_FUNCTOR( TinPlaneIndexBufferFunctor )

  private:
    QgsTriangularMeshTile mMesh;
};


QgsTinDemTerrainTileGeometry_p::QgsTinDemTerrainTileGeometry_p( QgsTriangularMeshTile meshTile, float verticaleScale, QgsTinDemTerrainTileGeometry_p::QNode *parent ):
  QGeometry( parent ),
  mTriangularMeshTile( meshTile ),
  mVertScale( verticaleScale )
{
  init();
}


void QgsTinDemTerrainTileGeometry_p::init()
{
  mPositionAttribute = new QAttribute( this );
  mNormalAttribute = new QAttribute( this );
  mTexCoordAttribute = new QAttribute( this );
  mIndexAttribute = new QAttribute( this );
  mVertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );
  mIndexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this );


  const int stride = ( 3 + 2 + 3 ) * sizeof( float );
  const uint nVerts = uint( mTriangularMeshTile.verticesCount() );

  mPositionAttribute->setName( QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setByteStride( stride );
  mPositionAttribute->setCount( nVerts );

  mTexCoordAttribute->setName( QAttribute::defaultTextureCoordinateAttributeName() );
  mTexCoordAttribute->setVertexBaseType( QAttribute::Float );
  mTexCoordAttribute->setVertexSize( 2 );
  mTexCoordAttribute->setAttributeType( QAttribute::VertexAttribute );
  mTexCoordAttribute->setBuffer( mVertexBuffer );
  mTexCoordAttribute->setByteStride( stride );
  mTexCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  mTexCoordAttribute->setCount( nVerts );

  mNormalAttribute->setName( QAttribute::defaultNormalAttributeName() );
  mNormalAttribute->setVertexBaseType( QAttribute::Float );
  mNormalAttribute->setVertexSize( 3 );
  mNormalAttribute->setAttributeType( QAttribute::VertexAttribute );
  mNormalAttribute->setBuffer( mVertexBuffer );
  mNormalAttribute->setByteStride( stride );
  mNormalAttribute->setByteOffset( 5 * sizeof( float ) );
  mNormalAttribute->setCount( nVerts );

  mIndexAttribute->setAttributeType( QAttribute::IndexAttribute );
  mIndexAttribute->setVertexBaseType( QAttribute::UnsignedInt );
  mIndexAttribute->setBuffer( mIndexBuffer );

  // Each primitive has 3 vertives
  mIndexAttribute->setCount( uint( mTriangularMeshTile.faceCount() ) * 3 );

  // switched to setting data instead of just setting data generators because we also need the buffers
  // available for ray-mesh intersections and we can't access the private copy of data in Qt (if there is any)
  mVertexBuffer->setData( TinPlaneVertexBufferFunctor( mTriangularMeshTile, mVertScale )() );
  mIndexBuffer->setData( TinPlaneIndexBufferFunctor( mTriangularMeshTile )() );

  addAttribute( mPositionAttribute );
  addAttribute( mTexCoordAttribute );
  addAttribute( mNormalAttribute );
  addAttribute( mIndexAttribute );
}



QgsTriangularMeshTile::QgsTriangularMeshTile( QgsTriangularMesh *triangularMesh, const QgsRectangle &extent ):
  mTriangularMesh( triangularMesh ), mExtent( extent )
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
    int globalIndex = mTriangularMesh->triangles().at( mFaces.at( localTriangleIndex ) ).at( i );
    int localIndex = mLocalIndexFromGlobalIndex[globalIndex];
    triangle.append( localIndex );
  }

  return triangle;

}

QgsMeshVertex QgsTriangularMeshTile::vertex( int localIndex ) const
{
  int globalIndex = mVertices.at( localIndex ).globalIndex;
  return mTriangularMesh->vertices().at( globalIndex );
}

bool QgsTriangularMeshTile::operator==( const QgsTriangularMeshTile &other ) const
{
  return ( other.mTriangularMesh == mTriangularMesh &&
           mExtent == mExtent &&
           mRealExtent == mRealExtent
         );
}

QVector3D QgsTriangularMeshTile::vertexNormalVector( int i ) const
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
  mFaces = QVector<int>::fromList( mTriangularMesh->faceIndexesForRectangle( mExtent ) );
  mVertices.clear();
  mLocalIndexFromGlobalIndex.clear();

  int localIndex = 0;
  mRealExtent.setMinimal();
  for ( auto f : mFaces )
  {

    const QgsMeshFace &face( mTriangularMesh->triangles().at( f ) );
    for ( int i = 0; i < 3; i++ )
    {
      int globalIndex( face.at( i ) );

      const QgsMeshVertex &vert( mTriangularMesh->vertices().at( globalIndex ) );
      const QgsMeshVertex &otherVert1( mTriangularMesh->vertices().at( face.at( ( i + 1 ) % 3 ) ) );
      const QgsMeshVertex &otherVert2( mTriangularMesh->vertices().at( face.at( ( i + 2 ) % 3 ) ) );

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
        if ( mRealExtent.xMinimum() < vert.x() )
          mRealExtent.setXMinimum( vert.x() );
        if ( mRealExtent.yMinimum() < vert.y() )
          mRealExtent.setYMinimum( vert.y() );
        if ( mRealExtent.xMaximum() > vert.x() )
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
}
