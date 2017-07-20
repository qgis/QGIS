
#include "demterraintilegeometry.h"
#include <Qt3DRender/qattribute.h>
#include <Qt3DRender/qbuffer.h>
#include <Qt3DRender/qbufferdatagenerator.h>
#include <limits>


using namespace Qt3DRender;


QByteArray createPlaneVertexData( int res, const QByteArray &heights )
{
  Q_ASSERT( res >= 2 );
  Q_ASSERT( heights.count() == res * res * ( int )sizeof( float ) );

  const float *zBits = ( const float * ) heights.constData();

  const int nVerts = res * res;

  // Populate a buffer with the interleaved per-vertex data with
  // vec3 pos, vec2 texCoord, vec3 normal, vec4 tangent
  const quint32 elementSize = 3 + 2 + 3;
  const quint32 stride = elementSize * sizeof( float );
  QByteArray bufferBytes;
  bufferBytes.resize( stride * nVerts );
  float *fptr = reinterpret_cast<float *>( bufferBytes.data() );

  float w = 1, h = 1;
  QSize resolution( res, res );
  const float x0 = -w / 2.0f;
  const float z0 = -h / 2.0f;
  const float dx = w / ( resolution.width() - 1 );
  const float dz = h / ( resolution.height() - 1 );
  const float du = 1.0 / ( resolution.width() - 1 );
  const float dv = 1.0 / ( resolution.height() - 1 );

  // Iterate over z
  for ( int j = 0; j < resolution.height(); ++j )
  {
    const float z = z0 + static_cast<float>( j ) * dz;
    const float v = static_cast<float>( j ) * dv;

    // Iterate over x
    for ( int i = 0; i < resolution.width(); ++i )
    {
      const float x = x0 + static_cast<float>( i ) * dx;
      const float u = static_cast<float>( i ) * du;

      // position
      *fptr++ = x;
      *fptr++ = *zBits++;
      *fptr++ = z;

      // texture coordinates
      *fptr++ = u;
      *fptr++ = v;

      // normal
      *fptr++ = 0.0f;
      *fptr++ = 1.0f;
      *fptr++ = 0.0f;
    }
  }

  return bufferBytes;
}


QByteArray createPlaneIndexData( int res )
{
  QSize resolution( res, res );
  // Create the index data. 2 triangles per rectangular face
  const int faces = 2 * ( resolution.width() - 1 ) * ( resolution.height() - 1 );
  const quint32 indices = 3 * faces;
  Q_ASSERT( indices < std::numeric_limits<quint32>::max() );
  QByteArray indexBytes;
  indexBytes.resize( indices * sizeof( quint32 ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( indexBytes.data() );

  // Iterate over z
  for ( int j = 0; j < resolution.height() - 1; ++j )
  {
    const int rowStartIndex = j * resolution.width();
    const int nextRowStartIndex = ( j + 1 ) * resolution.width();

    // Iterate over x
    for ( int i = 0; i < resolution.width() - 1; ++i )
    {
      // Split quad into two triangles
      *indexPtr++ = rowStartIndex + i;
      *indexPtr++ = nextRowStartIndex + i;
      *indexPtr++ = rowStartIndex + i + 1;

      *indexPtr++ = nextRowStartIndex + i;
      *indexPtr++ = nextRowStartIndex + i + 1;
      *indexPtr++ = rowStartIndex + i + 1;
    }
  }

  return indexBytes;
}


class PlaneVertexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit PlaneVertexBufferFunctor( int resolution, const QByteArray &heightMap )
      : m_resolution( resolution )
      , m_heightMap( heightMap )
    {}

    ~PlaneVertexBufferFunctor() {}

    QByteArray operator()() Q_DECL_FINAL
    {
      return createPlaneVertexData( m_resolution, m_heightMap );
    }

    bool operator ==( const QBufferDataGenerator &other ) const Q_DECL_FINAL
    {
      const PlaneVertexBufferFunctor *otherFunctor = functor_cast<PlaneVertexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->m_resolution == m_resolution &&
                 otherFunctor->m_heightMap == m_heightMap );
      return false;
    }

    QT3D_FUNCTOR( PlaneVertexBufferFunctor )

  private:
    int m_resolution;
    QByteArray m_heightMap;
};

class PlaneIndexBufferFunctor : public QBufferDataGenerator
{
  public:
    explicit PlaneIndexBufferFunctor( int resolution )
      : m_resolution( resolution )
    {}

    ~PlaneIndexBufferFunctor() {}

    QByteArray operator()() Q_DECL_FINAL
    {
      return createPlaneIndexData( m_resolution );
    }

    bool operator ==( const QBufferDataGenerator &other ) const Q_DECL_FINAL
    {
      const PlaneIndexBufferFunctor *otherFunctor = functor_cast<PlaneIndexBufferFunctor>( &other );
      if ( otherFunctor != nullptr )
        return ( otherFunctor->m_resolution == m_resolution );
      return false;
    }

    QT3D_FUNCTOR( PlaneIndexBufferFunctor )

  private:
    int m_resolution;
};




DemTerrainTileGeometry::DemTerrainTileGeometry( int resolution, const QByteArray &heightMap, DemTerrainTileGeometry::QNode *parent )
  : QGeometry( parent )
  , m_resolution( resolution )
  , m_heightMap( heightMap )
  , m_positionAttribute( nullptr )
  , m_normalAttribute( nullptr )
  , m_texCoordAttribute( nullptr )
  , m_indexAttribute( nullptr )
  , m_vertexBuffer( nullptr )
  , m_indexBuffer( nullptr )
{
  init();
}

DemTerrainTileGeometry::~DemTerrainTileGeometry()
{
}

void DemTerrainTileGeometry::setHeightMap( const QByteArray &heightMap )
{
  m_heightMap = heightMap;
  m_vertexBuffer->setDataGenerator( QSharedPointer<PlaneVertexBufferFunctor>::create( m_resolution, m_heightMap ) );
}

QAttribute *DemTerrainTileGeometry::positionAttribute() const
{
  return m_positionAttribute;
}

QAttribute *DemTerrainTileGeometry::normalAttribute() const
{
  return m_normalAttribute;
}

QAttribute *DemTerrainTileGeometry::texCoordAttribute() const
{
  return m_texCoordAttribute;
}

QAttribute *DemTerrainTileGeometry::indexAttribute() const
{
  return m_indexAttribute;
}


void DemTerrainTileGeometry::init()
{
  m_positionAttribute = new QAttribute( this );
  m_normalAttribute = new QAttribute( this );
  m_texCoordAttribute = new QAttribute( this );
  m_indexAttribute = new QAttribute( this );
  m_vertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );
  m_indexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this );

  const int nVerts = m_resolution * m_resolution;
  const int stride = ( 3 + 2 + 3 ) * sizeof( float );
  const int faces = 2 * ( m_resolution - 1 ) * ( m_resolution - 1 );

  m_positionAttribute->setName( QAttribute::defaultPositionAttributeName() );
#if QT_VERSION >= 0x050800
  m_positionAttribute->setVertexBaseType( QAttribute::Float );
  m_positionAttribute->setVertexSize( 3 );
#else
  m_positionAttribute->setDataType( QAttribute::Float );
  m_positionAttribute->setDataSize( 3 );
#endif
  m_positionAttribute->setAttributeType( QAttribute::VertexAttribute );
  m_positionAttribute->setBuffer( m_vertexBuffer );
  m_positionAttribute->setByteStride( stride );
  m_positionAttribute->setCount( nVerts );

  m_texCoordAttribute->setName( QAttribute::defaultTextureCoordinateAttributeName() );
#if QT_VERSION >= 0x050800
  m_texCoordAttribute->setVertexBaseType( QAttribute::Float );
  m_texCoordAttribute->setVertexSize( 2 );
#else
  m_texCoordAttribute->setDataType( QAttribute::Float );
  m_texCoordAttribute->setDataSize( 2 );
#endif
  m_texCoordAttribute->setAttributeType( QAttribute::VertexAttribute );
  m_texCoordAttribute->setBuffer( m_vertexBuffer );
  m_texCoordAttribute->setByteStride( stride );
  m_texCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  m_texCoordAttribute->setCount( nVerts );

  m_normalAttribute->setName( QAttribute::defaultNormalAttributeName() );
#if QT_VERSION >= 0x050800
  m_normalAttribute->setVertexBaseType( QAttribute::Float );
  m_normalAttribute->setVertexSize( 3 );
#else
  m_normalAttribute->setDataType( QAttribute::Float );
  m_normalAttribute->setDataSize( 3 );
#endif
  m_normalAttribute->setAttributeType( QAttribute::VertexAttribute );
  m_normalAttribute->setBuffer( m_vertexBuffer );
  m_normalAttribute->setByteStride( stride );
  m_normalAttribute->setByteOffset( 5 * sizeof( float ) );
  m_normalAttribute->setCount( nVerts );

  m_indexAttribute->setAttributeType( QAttribute::IndexAttribute );
#if QT_VERSION >= 0x050800
  m_indexAttribute->setVertexBaseType( QAttribute::UnsignedInt );
#else
  m_indexAttribute->setDataType( QAttribute::UnsignedInt );
#endif
  m_indexAttribute->setBuffer( m_indexBuffer );

  // Each primitive has 3 vertives
  m_indexAttribute->setCount( faces * 3 );

  m_vertexBuffer->setDataGenerator( QSharedPointer<PlaneVertexBufferFunctor>::create( m_resolution, m_heightMap ) );
  m_indexBuffer->setDataGenerator( QSharedPointer<PlaneIndexBufferFunctor>::create( m_resolution ) );

  addAttribute( m_positionAttribute );
  addAttribute( m_texCoordAttribute );
  addAttribute( m_normalAttribute );
  addAttribute( m_indexAttribute );
}
