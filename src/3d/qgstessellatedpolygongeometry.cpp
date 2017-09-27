#include "qgstessellatedpolygongeometry.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGenerator>

#include "qgstessellator.h"

#include "qgspoint.h"
#include "qgspolygon.h"


QgsTessellatedPolygonGeometry::QgsTessellatedPolygonGeometry( QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( nullptr )
  , mNormalAttribute( nullptr )
{
  mWithNormals = true;

  mVertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );

  QgsTessellator tmpTess( 0, 0, mWithNormals );
  const int stride = tmpTess.stride();

  mPositionAttribute = new Qt3DRender::QAttribute( this );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setByteStride( stride );
  addAttribute( mPositionAttribute );

  if ( mWithNormals )
  {
    mNormalAttribute = new Qt3DRender::QAttribute( this );
    mNormalAttribute->setName( Qt3DRender::QAttribute::defaultNormalAttributeName() );
    mNormalAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
    mNormalAttribute->setVertexSize( 3 );
    mNormalAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
    mNormalAttribute->setBuffer( mVertexBuffer );
    mNormalAttribute->setByteStride( stride );
    mNormalAttribute->setByteOffset( 3 * sizeof( float ) );
    addAttribute( mNormalAttribute );
  }
}

QgsTessellatedPolygonGeometry::~QgsTessellatedPolygonGeometry()
{
  qDeleteAll( mPolygons );
}

void QgsTessellatedPolygonGeometry::setPolygons( const QList<QgsPolygonV2 *> &polygons, const QgsPointXY &origin, float extrusionHeight )
{
  qDeleteAll( mPolygons );
  mPolygons = polygons;

  int i = 0;
  QgsTessellator tesselator( origin.x(), origin.y(), mWithNormals );
  Q_FOREACH ( QgsPolygonV2 *polygon, polygons )
  {
    tesselator.addPolygon( *polygon, extrusionHeight );
    ++i;
  }

  QByteArray data( ( const char * )tesselator.data().constData(), tesselator.data().count() * sizeof( float ) );
  int nVerts = data.count() / tesselator.stride();

  mVertexBuffer->setData( data );
  mPositionAttribute->setCount( nVerts );
  if ( mNormalAttribute )
    mNormalAttribute->setCount( nVerts );
}
