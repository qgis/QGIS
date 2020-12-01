/***************************************************************************
  qgspointcloud3dsymbol_p.cpp
  ------------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloud3dsymbol_p.h"

///@cond PRIVATE

#include "qgspointcloud3dsymbol.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"
#include "qgscolorramptexture.h"
#include "qgs3dmapsettings.h"
#include "qgspointcloudindex.h"

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QEffect>
#include <QPointSize>

QgsPointCloud3DGeometry::QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, unsigned int byteStride )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mParameterAttribute( new Qt3DRender::QAttribute( this ) )
  , mColorAttribute( new Qt3DRender::QAttribute( this ) )
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
#else
  , mVertexBuffer( new Qt3DRender::QBuffer( this ) )
#endif
  , mByteStride( byteStride )
{

}

QgsSingleColorPointCloud3DGeometry::QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : QgsPointCloud3DGeometry( parent, byteStride )
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

QgsColorRampPointCloud3DGeometry::QgsColorRampPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : QgsPointCloud3DGeometry( parent, byteStride )
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
  Q_ASSERT( data.positions.size() == data.parameter.size() );
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

QgsRGBPointCloud3DGeometry::QgsRGBPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : QgsPointCloud3DGeometry( parent, byteStride )
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
  Q_ASSERT( data.positions.size() == data.colors.size() );
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

QgsPointCloud3DSymbolHandler::QgsPointCloud3DSymbolHandler()
{
}

void QgsPointCloud3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context, QgsPointCloud3DSymbolHandler::PointData &out, bool selected )
{
  Q_UNUSED( selected )
  Q_UNUSED( context )

  if ( out.positions.empty() )
    return;

  // Geometry
  Qt3DRender::QGeometry *geom = makeGeometry( parent, out, context.symbol()->byteStride() );
  Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
  gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  gr->setVertexCount( out.positions.count() );
  gr->setGeometry( geom );

  // Transform
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;

  // Material
  Qt3DRender::QMaterial *mat = new Qt3DRender::QMaterial;
  if ( context.symbol() )
    context.symbol()->fillMaterial( mat );

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( mat );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/pointcloud.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/pointcloud.frag" ) ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( mat );
  renderPass->setShaderProgram( shaderProgram );

  Qt3DRender::QPointSize *pointSize = new Qt3DRender::QPointSize( renderPass );
  pointSize->setSizeMode( Qt3DRender::QPointSize::Programmable );  // supported since OpenGL 3.2
  pointSize->setValue( context.symbol() ? context.symbol()->pointSize() : 1.0f );
  renderPass->addRenderState( pointSize );

  // without this filter the default forward renderer would not render this
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( "forward" );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addRenderPass( renderPass );
  technique->addFilterKey( filterKey );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 1 );

  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect;
  eff->addTechnique( technique );
  mat->setEffect( eff );

  // All together
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( gr );
  entity->addComponent( tr );
  entity->addComponent( mat );
  entity->setParent( parent );
  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
}

QgsSingleColorPointCloud3DSymbolHandler::QgsSingleColorPointCloud3DSymbolHandler()
  : QgsPointCloud3DSymbolHandler()
{

}

bool QgsSingleColorPointCloud3DSymbolHandler::prepare( const QgsPointCloud3DRenderContext &context )
{
  Q_UNUSED( context )
  return true;
}

void QgsSingleColorPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context )
{
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  int count = block->pointCount();
  const std::size_t recordSize = attributes.pointRecordSize();

  const QgsVector3D scale = pc->scale();
  const QgsVector3D offset = pc->offset();

  for ( int i = 0; i < count; ++i )
  {
    qint32 ix = *( qint32 * )( ptr + i * recordSize + 0 );
    qint32 iy = *( qint32 * )( ptr + i * recordSize + 4 );
    qint32 iz = *( qint32 * )( ptr + i * recordSize + 8 );

    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
    double z = offset.z() + scale.z() * iz;
    QVector3D point( x, y, z );
    QgsVector3D p = context.map().mapToWorldCoordinates( point );
    outNormal.positions.push_back( QVector3D( p.x(), p.y(), p.z() ) );
  }
}

void QgsSingleColorPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DRender::QGeometry *QgsSingleColorPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
{
  return new QgsSingleColorPointCloud3DGeometry( parent, data, byteStride );
}

QgsColorRampPointCloud3DSymbolHandler::QgsColorRampPointCloud3DSymbolHandler()
  : QgsPointCloud3DSymbolHandler()
{

}

bool QgsColorRampPointCloud3DSymbolHandler::prepare( const QgsPointCloud3DRenderContext &context )
{
  Q_UNUSED( context )
  return true;
}

void QgsColorRampPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context )
{
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  std::unique_ptr< QgsPointCloudAttribute > parameterAttribute;
  QgsColorRampPointCloud3DSymbol *symbol = dynamic_cast<QgsColorRampPointCloud3DSymbol *>( context.symbol() );
  if ( symbol )
  {
    int offset = 0;
    QgsPointCloudAttributeCollection collection = context.attributes();
    const QgsPointCloudAttribute *attr = collection.find( symbol->renderingParameter(), offset );
    if ( attr )
    {
      parameterAttribute.reset( new QgsPointCloudAttribute( attr->name(), attr->type() ) );
      attributes.push_back( *parameterAttribute.get() );
    }
  }

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  int count = block->pointCount();
  const std::size_t recordSize = attributes.pointRecordSize();

  const QgsVector3D scale = pc->scale();
  const QgsVector3D offset = pc->offset();

  for ( int i = 0; i < count; ++i )
  {
    qint32 ix = *( qint32 * )( ptr + i * recordSize + 0 );
    qint32 iy = *( qint32 * )( ptr + i * recordSize + 4 );
    qint32 iz = *( qint32 * )( ptr + i * recordSize + 8 );
    float iParam = 0.0f;
    if ( parameterAttribute )
    {
      switch ( parameterAttribute->type() )
      {
        case QgsPointCloudAttribute::DataType::Char:
          iParam = *( char * )( ptr + i * recordSize + 12 );
          break;
        case QgsPointCloudAttribute::DataType::Short:
          iParam = *( short * )( ptr + i * recordSize + 12 );
          break;
        case QgsPointCloudAttribute::DataType::UShort:
          iParam = *( unsigned short * )( ptr + i * recordSize + 12 );
          break;
        case QgsPointCloudAttribute::DataType::Int32:
          iParam = *( qint32 * )( ptr + i * recordSize + 12 );
          break;
        case QgsPointCloudAttribute::DataType::Float:
          iParam = *( float * )( ptr + i * recordSize + 12 );
          break;
        case QgsPointCloudAttribute::DataType::Double:
          iParam = *( double * )( ptr + i * recordSize + 12 );
          break;
      }
    }

    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
    double z = offset.z() + scale.z() * iz;
    QVector3D point( x, y, z );
    QgsVector3D p = context.map().mapToWorldCoordinates( point );
    outNormal.positions.push_back( QVector3D( p.x(), p.y(), p.z() ) );
    if ( parameterAttribute && parameterAttribute->name() == "X" )
      outNormal.parameter.push_back( x );
    else if ( parameterAttribute && parameterAttribute->name() == "Y" )
      outNormal.parameter.push_back( y );
    else if ( parameterAttribute && parameterAttribute->name() == "Z" )
      outNormal.parameter.push_back( z );
    else
      outNormal.parameter.push_back( iParam );
  }
}

void QgsColorRampPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DRender::QGeometry *QgsColorRampPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
{
  return new QgsColorRampPointCloud3DGeometry( parent, data, byteStride );
}

QgsRGBPointCloud3DSymbolHandler::QgsRGBPointCloud3DSymbolHandler()
  : QgsPointCloud3DSymbolHandler()
{

}

bool QgsRGBPointCloud3DSymbolHandler::prepare( const QgsPointCloud3DRenderContext &context )
{
  Q_UNUSED( context )
  return true;
}

void QgsRGBPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context )
{
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  attributes.push_back( QgsPointCloudAttribute( "Red", QgsPointCloudAttribute::DataType::Short ) );
  attributes.push_back( QgsPointCloudAttribute( "Green", QgsPointCloudAttribute::DataType::Short ) );
  attributes.push_back( QgsPointCloudAttribute( "Blue", QgsPointCloudAttribute::DataType::Short ) );

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  int count = block->pointCount();
  const std::size_t recordSize = attributes.pointRecordSize();

  const QgsVector3D scale = pc->scale();
  const QgsVector3D offset = pc->offset();

  for ( int i = 0; i < count; ++i )
  {
    qint32 ix = *( qint32 * )( ptr + i * recordSize + 0 );
    qint32 iy = *( qint32 * )( ptr + i * recordSize + 4 );
    qint32 iz = *( qint32 * )( ptr + i * recordSize + 8 );
    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
    double z = offset.z() + scale.z() * iz;
    QVector3D point( x, y, z );
    QgsVector3D p = context.map().mapToWorldCoordinates( point );
    outNormal.positions.push_back( QVector3D( p.x(), p.y(), p.z() ) );

    QVector3D color( 0.0f, 0.0f, 0.0f );
    if ( recordSize > 10 )
    {
      short ir = *( short * )( ptr + i * recordSize + 12 );
      color.setX( ( ( float )ir ) / 256.0f );
    }
    if ( recordSize > 12 )
    {
      short ig = *( short * )( ptr + i * recordSize + 14 );
      color.setY( ( ( float )ig ) / 256.0f );
    }
    if ( recordSize > 14 )
    {
      short ib = *( short * )( ptr + i * recordSize + 16 );
      color.setZ( ( ( float )ib ) / 256.0f );
    }
    outNormal.colors.push_back( color );
  }
}

void QgsRGBPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DRender::QGeometry *QgsRGBPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
{
  return new QgsRGBPointCloud3DGeometry( parent, data, byteStride );
}

/// @endcond
