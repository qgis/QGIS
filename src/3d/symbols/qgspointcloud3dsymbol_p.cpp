#include "qgspointcloud3dsymbol_p.h"

///@cond PRIVATE

QgsPointCloud3DGeometry::QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, QgsPointCloud3DSymbol *symbol )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mParameterAttribute( new Qt3DRender::QAttribute( this ) )
  , mColorAttribute( new Qt3DRender::QAttribute( this ) )
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
#else
  , mVertexBuffer( new Qt3DRender::QBuffer( this ) )
#endif
  , mByteStride( symbol->byteStride() )
{

}

QgsSingleColorPointCloud3DGeometry::QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol )
  : QgsPointCloud3DGeometry( parent, symbol )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( mByteStride );
  addAttribute( mPositionAttribute );
  makeVertexBuffer( data );
}

void QgsSingleColorPointCloud3DGeometry::makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( data.positions.size() * mByteStride );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( int i = 0; i < data.positions.size(); ++i )
  {
    rawVertexArray[idx++] = data.positions.at( i ).x();
    rawVertexArray[idx++] = data.positions.at( i ).y();
    rawVertexArray[idx++] = data.positions.at( i ).z();
  }

  mVertexCount = data.positions.size();
  mVertexBuffer->setData( vertexBufferData );
}

QgsColorRampPointCloud3DGeometry::QgsColorRampPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol )
  : QgsPointCloud3DGeometry( parent, symbol )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( mByteStride );
  addAttribute( mPositionAttribute );
  mParameterAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mParameterAttribute->setBuffer( mVertexBuffer );
  mParameterAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mParameterAttribute->setVertexSize( 1 );
  mParameterAttribute->setName( "vertexParameter" );
  mParameterAttribute->setByteOffset( 12 );
  mParameterAttribute->setByteStride( mByteStride );
  addAttribute( mParameterAttribute );
  makeVertexBuffer( data );
}

void QgsColorRampPointCloud3DGeometry::makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( data.positions.size() * mByteStride );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( int i = 0; i < data.positions.size(); ++i )
  {
    rawVertexArray[idx++] = data.positions.at( i ).x();
    rawVertexArray[idx++] = data.positions.at( i ).y();
    rawVertexArray[idx++] = data.positions.at( i ).z();
    rawVertexArray[idx++] = data.parameter.at( i );
  }

  mVertexCount = data.positions.size();
  mVertexBuffer->setData( vertexBufferData );
}

QgsRGBPointCloud3DGeometry::QgsRGBPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol )
  : QgsPointCloud3DGeometry( parent, symbol )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( mByteStride );
  addAttribute( mPositionAttribute );
  mColorAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mColorAttribute->setBuffer( mVertexBuffer );
  mColorAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mColorAttribute->setVertexSize( 3 );
  mColorAttribute->setName( QStringLiteral( "vertexColor" ) );
  mColorAttribute->setByteOffset( 12 );
  mColorAttribute->setByteStride( mByteStride );
  addAttribute( mColorAttribute );
  makeVertexBuffer( data );
}

void QgsRGBPointCloud3DGeometry::makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( data.positions.size() * mByteStride );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( int i = 0; i < data.positions.size(); ++i )
  {
    rawVertexArray[idx++] = data.positions.at( i ).x();
    rawVertexArray[idx++] = data.positions.at( i ).y();
    rawVertexArray[idx++] = data.positions.at( i ).z();
    rawVertexArray[idx++] = data.colors.at( i ).x();
    rawVertexArray[idx++] = data.colors.at( i ).y();
    rawVertexArray[idx++] = data.colors.at( i ).z();
  }
  mVertexCount = data.positions.size();
  mVertexBuffer->setData( vertexBufferData );
}

/// @endcond
