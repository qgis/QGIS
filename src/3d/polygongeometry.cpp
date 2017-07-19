#include "polygongeometry.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGenerator>

#include "tessellator.h"

#include "qgspoint.h"
#include "qgspolygon.h"


PolygonGeometry::PolygonGeometry( QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , m_positionAttribute( nullptr )
  , m_normalAttribute( nullptr )
{
  m_withNormals = true;

  m_vertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );

  Tessellator tmpTess( 0, 0, m_withNormals );
  const int stride = tmpTess.stride;

  m_positionAttribute = new Qt3DRender::QAttribute( this );
  m_positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
#if QT_VERSION >= 0x050800
  m_positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  m_positionAttribute->setVertexSize( 3 );
#else
  m_positionAttribute->setDataType( Qt3DRender::QAttribute::Float );
  m_positionAttribute->setDataSize( 3 );
#endif
  m_positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  m_positionAttribute->setBuffer( m_vertexBuffer );
  m_positionAttribute->setByteStride( stride );
  addAttribute( m_positionAttribute );

  if ( m_withNormals )
  {
    m_normalAttribute = new Qt3DRender::QAttribute( this );
    m_normalAttribute->setName( Qt3DRender::QAttribute::defaultNormalAttributeName() );
#if QT_VERSION >= 0x050800
    m_normalAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
    m_normalAttribute->setVertexSize( 3 );
#else
    m_normalAttribute->setDataType( Qt3DRender::QAttribute::Float );
    m_normalAttribute->setDataSize( 3 );
#endif
    m_normalAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
    m_normalAttribute->setBuffer( m_vertexBuffer );
    m_normalAttribute->setByteStride( stride );
    m_normalAttribute->setByteOffset( 3 * sizeof( float ) );
    addAttribute( m_normalAttribute );
  }
}

PolygonGeometry::~PolygonGeometry()
{
  qDeleteAll( mPolygons );
}

void PolygonGeometry::setPolygons( const QList<QgsPolygonV2 *> &polygons, const QgsPointXY &origin, float extrusionHeight )
{
  qDeleteAll( mPolygons );
  mPolygons = polygons;

  int i = 0;
  Tessellator tesselator( origin.x(), origin.y(), m_withNormals );
  Q_FOREACH ( QgsPolygonV2 *polygon, polygons )
  {
    tesselator.addPolygon( *polygon, extrusionHeight );
    ++i;
  }

  QByteArray data( ( const char * )tesselator.data.constData(), tesselator.data.count() * sizeof( float ) );
  int nVerts = data.count() / tesselator.stride;

  m_vertexBuffer->setData( data );
  m_positionAttribute->setCount( nVerts );
  if ( m_normalAttribute )
    m_normalAttribute->setCount( nVerts );
}
