#include "terrainboundsentity.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPhongMaterial>

#include "aabb.h"


class LineMeshGeometry : public Qt3DRender::QGeometry
{
  public:
    LineMeshGeometry( Qt3DCore::QNode *parent = nullptr );

    int vertexCount()
    {
      return _vertices.size();
    }

    void setVertices( QList<QVector3D> vertices );

  private:
    Qt3DRender::QAttribute *_positionAttribute;
    Qt3DRender::QBuffer *_vertexBuffer;
    QList<QVector3D> _vertices;

};

LineMeshGeometry::LineMeshGeometry( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometry( parent )
  , _positionAttribute( new Qt3DRender::QAttribute( this ) )
  , _vertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
{
  _positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  _positionAttribute->setBuffer( _vertexBuffer );
#if QT_VERSION >= 0x050800
  _positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  _positionAttribute->setVertexSize( 3 );
#else
  _positionAttribute->setDataType( Qt3DRender::QAttribute::Float );
  _positionAttribute->setDataSize( 3 );
#endif
  _positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );

  addAttribute( _positionAttribute );
}

void LineMeshGeometry::setVertices( QList<QVector3D> vertices )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const auto &v : vertices )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
    _vertices.append( v );
  }

  _vertexBuffer->setData( vertexBufferData );
}


// ----------------


class AABBMesh : public Qt3DRender::QGeometryRenderer
{
  public:
    AABBMesh( Qt3DCore::QNode *parent = nullptr );

    void setBoxes( const QList<AABB> &bboxes );

  private:
    LineMeshGeometry *_lineMeshGeo;
};

AABBMesh::AABBMesh( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometryRenderer( parent )
  , _lineMeshGeo( nullptr )
{
  setInstanceCount( 1 );
  setIndexOffset( 0 );
  setFirstInstance( 0 );
  setPrimitiveType( Qt3DRender::QGeometryRenderer::Lines );

  _lineMeshGeo = new LineMeshGeometry( this );
  setGeometry( _lineMeshGeo );
}

void AABBMesh::setBoxes( const QList<AABB> &bboxes )
{
  QList<QVector3D> vertices;
  Q_FOREACH ( const AABB &bbox, bboxes )
    vertices << bbox.verticesForLines();
  _lineMeshGeo->setVertices( vertices );
  setVertexCount( _lineMeshGeo->vertexCount() );
}


// ----------------


TerrainBoundsEntity::TerrainBoundsEntity( Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  aabbMesh = new AABBMesh;
  addComponent( aabbMesh );

  Qt3DExtras::QPhongMaterial *bboxesMaterial = new Qt3DExtras::QPhongMaterial;
  bboxesMaterial->setAmbient( Qt::red );
  addComponent( bboxesMaterial );
}

void TerrainBoundsEntity::setBoxes( const QList<AABB> &bboxes )
{
  aabbMesh->setBoxes( bboxes );
}
