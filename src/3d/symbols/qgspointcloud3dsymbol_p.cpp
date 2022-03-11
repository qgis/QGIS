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
#include "qgspointcloudblockrequest.h"
#include "qgsfeedback.h"

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QEffect>
#include <QPointSize>
#include <QUrl>

#include <delaunator.hpp>

QgsPointCloud3DGeometry::QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mParameterAttribute( new Qt3DRender::QAttribute( this ) )
  , mColorAttribute( new Qt3DRender::QAttribute( this ) )
  , mTriangleIndexAttribute( new Qt3DRender::QAttribute( this ) )
  , mNormalsAttribute( new Qt3DRender::QAttribute( this ) )
  , mVertexBuffer( new  Qt3DRender::QBuffer( this ) )
  , mByteStride( byteStride )
{
  if ( !data.triangles.isEmpty() )
  {
    mTriangleBuffer = new  Qt3DRender::QBuffer( this );
    mTriangleIndexAttribute->setAttributeType( Qt3DRender::QAttribute::IndexAttribute );
    mTriangleIndexAttribute->setBuffer( mTriangleBuffer );
    mTriangleIndexAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedInt );
    mTriangleBuffer->setData( data.triangles );
    mTriangleIndexAttribute->setCount( data.triangles.size() / sizeof( quint32 ) );
    addAttribute( mTriangleIndexAttribute );
  }

  if ( !data.normals.isEmpty() )
  {
    mNormalsBuffer = new Qt3DRender::QBuffer( this );
    mNormalsAttribute->setName( Qt3DRender::QAttribute::defaultNormalAttributeName() );
    mNormalsAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
    mNormalsAttribute->setVertexSize( 3 );
    mNormalsAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
    mNormalsAttribute->setBuffer( mNormalsBuffer );
    mNormalsBuffer->setData( data.normals );
    mNormalsAttribute->setCount( data.normals.size() / ( 3 * sizeof( float ) ) );
    addAttribute( mNormalsAttribute );
  }
}

QgsSingleColorPointCloud3DGeometry::QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : QgsPointCloud3DGeometry( parent, data, byteStride )
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
  : QgsPointCloud3DGeometry( parent, data, byteStride )
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
  : QgsPointCloud3DGeometry( parent, data, byteStride )
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


void QgsPointCloud3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context, const QgsPointCloud3DSymbolHandler::PointData &out, bool selected )
{
  Q_UNUSED( selected )

  if ( out.positions.empty() )
    return;

  // Geometry
  Qt3DRender::QGeometry *geom = makeGeometry( parent, out, context.symbol()->byteStride() );
  Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
  if ( context.symbol()->renderAsTriangles() && ! out.triangles.isEmpty() )
  {
    gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
    gr->setVertexCount( out.triangles.size() /  sizeof( quint32 ) );
  }
  else
  {
    gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
    gr->setVertexCount( out.positions.count() );
  }
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

  if ( out.triangles.isEmpty() )
  {
    Qt3DRender::QPointSize *pointSize = new Qt3DRender::QPointSize( renderPass );
    pointSize->setSizeMode( Qt3DRender::QPointSize::Programmable );  // supported since OpenGL 3.2
    pointSize->setValue( context.symbol() ? context.symbol()->pointSize() : 1.0f );
    renderPass->addRenderState( pointSize );
  }

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
  technique->addParameter( new Qt3DRender::QParameter( "triangulate", !out.triangles.isEmpty() ) );

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


std::vector<double> QgsPointCloud3DSymbolHandler::getVertices( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context, const QgsAABB &bbox )
{

  bool hasColorData = !outNormal.colors.empty();
  bool hasParameterData = !outNormal.parameter.empty();

  // first, get the points of the concerned node
  std::vector<double> vertices( outNormal.positions.size() * 2 );
  size_t idx = 0;
  for ( int i = 0; i < outNormal.positions.size(); ++i )
  {
    vertices[idx++] = outNormal.positions.at( i ).x();
    vertices[idx++] = outNormal.positions.at( i ).z();
  }

  // next, we also need all points of all parents nodes to make the triangulation (also external points)
  IndexedPointCloudNode parentNode = n.parentNode();

  int properPointsCount = outNormal.positions.count();
  while ( parentNode.d() >= 0 )
  {
    processNode( pc, parentNode, context );
    parentNode = parentNode.parentNode();
  }

  PointData filteredExtraPointData;

  double span = pc->span();
  //factor to take account of the density of the point to calculate extension of the bounding box
  // with a usual value span = 128, bounding box is extended by 12.5 % on each side.
  double extraBoxFactor = 16 / span;
  double extraX = extraBoxFactor * bbox.xExtent();
  double extraZ = extraBoxFactor * bbox.zExtent();

  // We keep all points in vertical direction to avoid odd triangulation if points are isolated on top
  const QgsAABB extendedBBox( bbox.xMin - extraX, -std::numeric_limits<float>::max(), bbox.zMin - extraZ, bbox.xMax + extraX, std::numeric_limits<float>::max(), bbox.zMax + extraZ );

  for ( int i = properPointsCount; i < outNormal.positions.count(); ++i )
  {
    const  QVector3D pos = outNormal.positions.at( i );
    if ( extendedBBox.intersects( pos.x(), pos.y(), pos.z() ) )
    {
      filteredExtraPointData.positions.append( pos );
      vertices.push_back( pos.x() );
      vertices.push_back( pos.z() );

      if ( hasColorData )
        filteredExtraPointData.colors.append( outNormal.colors.at( i ) );
      if ( hasParameterData )
        filteredExtraPointData.parameter.append( outNormal.parameter.at( i ) );
    }
  }

  outNormal.positions.resize( properPointsCount );
  if ( hasColorData )
    outNormal.colors.resize( properPointsCount );
  if ( hasParameterData )
    outNormal.parameter.resize( properPointsCount );

  outNormal.positions.append( filteredExtraPointData.positions );
  outNormal.colors.append( filteredExtraPointData.colors );
  outNormal.parameter.append( filteredExtraPointData.parameter );

  return vertices;
}

void QgsPointCloud3DSymbolHandler::calculateNormals( const std::vector<size_t> &triangles )
{
  QVector<QVector3D> normals( outNormal.positions.count(), QVector3D( 0.0, 0.0, 0.0 ) );
  for ( size_t i = 0; i < triangles.size(); i += 3 )
  {
    QVector<QVector3D> triangleVertices( 3 );
    for ( size_t j = 0; j < 3; ++j )
    {
      size_t vertIndex = triangles.at( i + j );
      triangleVertices[j] = outNormal.positions.at( vertIndex );
    }
    //calculate normals
    for ( size_t j = 0; j < 3; ++j )
      normals[triangles.at( i + j )] += QVector3D::crossProduct(
                                          triangleVertices.at( 1 ) - triangleVertices.at( 0 ),
                                          triangleVertices.at( 2 ) - triangleVertices.at( 0 ) );
  }

  // Build now normals array
  outNormal.normals.resize( ( outNormal.positions.count() ) * sizeof( float ) * 3 );
  float *normPtr = reinterpret_cast<float *>( outNormal.normals.data() );
  for ( int i = 0; i < normals.size(); ++i )
  {
    QVector3D normal = normals.at( i );
    normal = normal.normalized();

    *normPtr++ = normal.x();
    *normPtr++ = normal.y();
    *normPtr++ = normal.z();
  }
}

void QgsPointCloud3DSymbolHandler::filterTriangles( const std::vector<size_t> &triangleIndexes, const QgsPointCloud3DRenderContext &context, const QgsAABB &bbox )
{
  outNormal.triangles.resize( triangleIndexes.size() * sizeof( quint32 ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( outNormal.triangles.data() );
  size_t effective = 0;

  bool horizontalFilter = context.symbol()->horizontalTriangleFilter();
  bool verticalFilter = context.symbol()->verticalTriangleFilter();
  float horizontalThreshold =  context.symbol()->horizontalFilterThreshold();
  float verticalThreshold =  context.symbol()->verticalFilterThreshold();

  for ( size_t i = 0; i < triangleIndexes.size(); i += 3 )
  {
    bool atLeastOneInBox = false;
    bool horizontalSkip = false;
    bool verticalSkip = false;
    for ( size_t j = 0; j < 3; j++ )
    {
      QVector3D pos = outNormal.positions.at( triangleIndexes.at( i  + j ) );
      atLeastOneInBox |= bbox.intersects( pos.x(), pos.y(), pos.z() );

      if ( verticalFilter || horizontalFilter )
      {
        const QVector3D pos2 = outNormal.positions.at( triangleIndexes.at( i + ( j + 1 ) % 3 ) );

        if ( verticalFilter )
          verticalSkip |= std::fabs( pos.y() - pos2.y() ) > verticalThreshold;

        if ( horizontalFilter && ! verticalSkip )
        {
          // filter only in the horizontal plan, it is a 2.5D triangulation.
          horizontalSkip |= sqrt( std::pow( pos.x() - pos2.x(), 2 ) +
                                  std::pow( pos.z() - pos2.z(), 2 ) ) > horizontalThreshold;
        }

        if ( horizontalSkip || verticalSkip )
          break;
      }
    }
    if ( atLeastOneInBox && !horizontalSkip && !verticalSkip )
    {
      for ( size_t j = 0; j < 3; j++ )
      {
        size_t vertIndex = triangleIndexes.at( i + j );
        *indexPtr++ = quint32( vertIndex );
      }
      effective++;
    }
  }

  if ( effective != 0 )
  {
    outNormal.triangles.resize( effective * 3 * sizeof( quint32 ) );
  }
  else
  {
    outNormal.triangles.clear();
    outNormal.normals.clear();
  }
}

void QgsPointCloud3DSymbolHandler::triangulate( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context, const QgsAABB &bbox )
{
  if ( outNormal.positions.isEmpty() )
    return;

  // Triangulation happens here
  std::unique_ptr<delaunator::Delaunator> triangulation;
  try
  {
    std::vector<double> vertices = getVertices( pc, n, context, bbox );
    triangulation.reset( new delaunator::Delaunator( vertices ) );
  }
  catch ( std::exception &e )
  {
    // something went wrong, better to retrieve initial state
    QgsDebugMsgLevel( QStringLiteral( "Error with triangulation" ), 4 );
    outNormal = PointData();
    processNode( pc, n, context );
    return;
  }

  const std::vector<size_t> &triangleIndexes = triangulation->triangles;

  calculateNormals( triangleIndexes );
  filterTriangles( triangleIndexes, context, bbox );
}

QgsPointCloudBlock *QgsPointCloud3DSymbolHandler::pointCloudBlock( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloudRequest &request, const QgsPointCloud3DRenderContext &context )
{
  QgsPointCloudBlock *block = nullptr;
  if ( pc->accessType() == QgsPointCloudIndex::AccessType::Local )
  {
    block = pc->nodeData( n, request );
  }
  else if ( pc->accessType() == QgsPointCloudIndex::AccessType::Remote )
  {
    bool loopAborted = false;
    QEventLoop loop;
    QgsPointCloudBlockRequest *req = pc->asyncNodeData( n, request );
    QObject::connect( req, &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
    QObject::connect( context.feedback(), &QgsFeedback::canceled, &loop, [ & ]()
    {
      loopAborted = true;
      loop.quit();
    } );
    loop.exec();

    if ( !loopAborted )
      block = req->block();
  }
  return block;
}

//

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
  std::unique_ptr<QgsPointCloudBlock> block( pointCloudBlock( pc, n, request, context ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  const int count = block->pointCount();
  const std::size_t recordSize = block->attributes().pointRecordSize();
  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();
  const double zValueScale = context.zValueScale();
  const double zValueOffset = context.zValueFixedOffset();
  const QgsCoordinateTransform coordinateTransform = context.coordinateTransform();
  bool alreadyPrintedDebug = false;

  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * )( ptr + i * recordSize + 0 );
    const qint32 iy = *( qint32 * )( ptr + i * recordSize + 4 );
    const qint32 iz = *( qint32 * )( ptr + i * recordSize + 8 );

    double x = blockOffset.x() + blockScale.x() * ix;
    double y = blockOffset.y() + blockScale.y() * iy;
    double z = ( blockOffset.z() + blockScale.z() * iz ) * zValueScale + zValueOffset;
    try
    {
      coordinateTransform.transformInPlace( x, y, z );
    }
    catch ( QgsCsException &e )
    {
      if ( !alreadyPrintedDebug )
      {
        QgsDebugMsg( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point( x, y, z );
    const QgsVector3D p = context.map().mapToWorldCoordinates( QgsVector3D( x, y, z ) );
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
  const int xOffset = 0;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  const int yOffset = attributes.pointRecordSize();
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  const int zOffset = attributes.pointRecordSize();
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  QString attributeName;
  bool attrIsX = false;
  bool attrIsY = false;
  bool attrIsZ = false;
  QgsPointCloudAttribute::DataType attributeType = QgsPointCloudAttribute::Float;
  int attributeOffset = 0;
  const double zValueScale = context.zValueScale();
  const double zValueOffset = context.zValueFixedOffset();
  const QgsCoordinateTransform coordinateTransform = context.coordinateTransform();
  bool alreadyPrintedDebug = false;

  QgsColorRampPointCloud3DSymbol *symbol = dynamic_cast<QgsColorRampPointCloud3DSymbol *>( context.symbol() );
  if ( symbol )
  {
    int offset = 0;
    const QgsPointCloudAttributeCollection collection = context.attributes();

    if ( symbol->attribute() == QLatin1String( "X" ) )
    {
      attrIsX = true;
    }
    else if ( symbol->attribute() == QLatin1String( "Y" ) )
    {
      attrIsY = true;
    }
    else if ( symbol->attribute() == QLatin1String( "Z" ) )
    {
      attrIsZ = true;
    }
    else
    {
      const QgsPointCloudAttribute *attr = collection.find( symbol->attribute(), offset );
      if ( attr )
      {
        attributeType = attr->type();
        attributeName = attr->name();
        attributeOffset = attributes.pointRecordSize();
        attributes.push_back( *attr );
      }
    }
  }

  if ( attributeName.isEmpty() && !attrIsX && !attrIsY && !attrIsZ )
    return;

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  std::unique_ptr<QgsPointCloudBlock> block( pointCloudBlock( pc, n, request, context ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  const int count = block->pointCount();
  const std::size_t recordSize = block->attributes().pointRecordSize();

  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();

  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * )( ptr + i * recordSize + xOffset );
    const qint32 iy = *( qint32 * )( ptr + i * recordSize + yOffset );
    const qint32 iz = *( qint32 * )( ptr + i * recordSize + zOffset );

    double x = blockOffset.x() + blockScale.x() * ix;
    double y = blockOffset.y() + blockScale.y() * iy;
    double z = ( blockOffset.z() + blockScale.z() * iz ) * zValueScale + zValueOffset;
    try
    {
      coordinateTransform.transformInPlace( x, y, z );
    }
    catch ( QgsCsException & )
    {
      if ( !alreadyPrintedDebug )
      {
        QgsDebugMsg( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    QgsVector3D point( x, y, z );
    point = context.map().mapToWorldCoordinates( point );
    outNormal.positions.push_back( QVector3D( point.x(), point.y(), point.z() ) );

    if ( attrIsX )
      outNormal.parameter.push_back( x );
    else if ( attrIsY )
      outNormal.parameter.push_back( y );
    else if ( attrIsZ )
      outNormal.parameter.push_back( z );
    else
    {
      float iParam = 0.0f;
      context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, iParam );
      outNormal.parameter.push_back( iParam );
    }
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

  QgsRgbPointCloud3DSymbol *symbol = dynamic_cast<QgsRgbPointCloud3DSymbol *>( context.symbol() );

  // we have to get the RGB attributes using their real data types -- they aren't always short! (sometimes unsigned short)
  int attrOffset = 0 ;

  const int redOffset = attributes.pointRecordSize();
  const QgsPointCloudAttribute *colorAttribute = context.attributes().find( symbol->redAttribute(), attrOffset );
  attributes.push_back( *colorAttribute );
  const QgsPointCloudAttribute::DataType redType = colorAttribute->type();

  const int greenOffset = attributes.pointRecordSize();
  colorAttribute = context.attributes().find( symbol->greenAttribute(), attrOffset );
  attributes.push_back( *colorAttribute );
  const QgsPointCloudAttribute::DataType greenType = colorAttribute->type();

  const int blueOffset = attributes.pointRecordSize();
  colorAttribute = context.attributes().find( symbol->blueAttribute(), attrOffset );
  attributes.push_back( *colorAttribute );
  const QgsPointCloudAttribute::DataType blueType = colorAttribute->type();

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  std::unique_ptr<QgsPointCloudBlock> block( pointCloudBlock( pc, n, request, context ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  const int count = block->pointCount();
  const std::size_t recordSize = block->attributes().pointRecordSize();

  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();
  const double zValueScale = context.zValueScale();
  const double zValueOffset = context.zValueFixedOffset();
  const QgsCoordinateTransform coordinateTransform = context.coordinateTransform();
  bool alreadyPrintedDebug = false;

  QgsContrastEnhancement *redContrastEnhancement = symbol->redContrastEnhancement();
  QgsContrastEnhancement *greenContrastEnhancement = symbol->greenContrastEnhancement();
  QgsContrastEnhancement *blueContrastEnhancement = symbol->blueContrastEnhancement();

  const bool useRedContrastEnhancement = redContrastEnhancement && redContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  const bool useBlueContrastEnhancement = blueContrastEnhancement && blueContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;
  const bool useGreenContrastEnhancement = greenContrastEnhancement && greenContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement;

  int ir = 0;
  int ig = 0;
  int ib = 0;
  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * )( ptr + i * recordSize + 0 );
    const qint32 iy = *( qint32 * )( ptr + i * recordSize + 4 );
    const qint32 iz = *( qint32 * )( ptr + i * recordSize + 8 );
    double x = blockOffset.x() + blockScale.x() * ix;
    double y = blockOffset.y() + blockScale.y() * iy;
    double z = ( blockOffset.z() + blockScale.z() * iz ) * zValueScale + zValueOffset;
    try
    {
      coordinateTransform.transformInPlace( x, y, z );
    }
    catch ( QgsCsException & )
    {
      if ( !alreadyPrintedDebug )
      {
        QgsDebugMsg( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point( x, y, z );
    const QgsVector3D p = context.map().mapToWorldCoordinates( point );

    QVector3D color( 0.0f, 0.0f, 0.0f );

    context.getAttribute( ptr, i * recordSize + redOffset, redType, ir );
    context.getAttribute( ptr, i * recordSize + greenOffset, greenType, ig );
    context.getAttribute( ptr, i * recordSize + blueOffset, blueType, ib );

    //skip if red, green or blue not in displayable range
    if ( ( useRedContrastEnhancement && !redContrastEnhancement->isValueInDisplayableRange( ir ) )
         || ( useGreenContrastEnhancement && !greenContrastEnhancement->isValueInDisplayableRange( ig ) )
         || ( useBlueContrastEnhancement && !blueContrastEnhancement->isValueInDisplayableRange( ib ) ) )
    {
      continue;
    }

    //stretch color values
    if ( useRedContrastEnhancement )
    {
      ir = redContrastEnhancement->enhanceContrast( ir );
    }
    if ( useGreenContrastEnhancement )
    {
      ig = greenContrastEnhancement->enhanceContrast( ig );
    }
    if ( useBlueContrastEnhancement )
    {
      ib = blueContrastEnhancement->enhanceContrast( ib );
    }

    color.setX( ir / 255.0f );
    color.setY( ig / 255.0f );
    color.setZ( ib / 255.0f );

    outNormal.positions.push_back( QVector3D( p.x(), p.y(), p.z() ) );
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

QgsClassificationPointCloud3DSymbolHandler::QgsClassificationPointCloud3DSymbolHandler()
  : QgsPointCloud3DSymbolHandler()
{

}

bool QgsClassificationPointCloud3DSymbolHandler::prepare( const QgsPointCloud3DRenderContext &context )
{
  Q_UNUSED( context )
  return true;
}

void QgsClassificationPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context )
{
  QgsPointCloudAttributeCollection attributes;
  const int xOffset = 0;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  const int yOffset = attributes.pointRecordSize();
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  const int zOffset = attributes.pointRecordSize();
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  QString attributeName;
  bool attrIsX = false;
  bool attrIsY = false;
  bool attrIsZ = false;
  QgsPointCloudAttribute::DataType attributeType = QgsPointCloudAttribute::Float;
  int attributeOffset = 0;
  QgsClassificationPointCloud3DSymbol *symbol = dynamic_cast<QgsClassificationPointCloud3DSymbol *>( context.symbol() );
  if ( symbol )
  {
    int offset = 0;
    const QgsPointCloudAttributeCollection collection = context.attributes();

    if ( symbol->attribute() == QLatin1String( "X" ) )
    {
      attrIsX = true;
    }
    else if ( symbol->attribute() == QLatin1String( "Y" ) )
    {
      attrIsY = true;
    }
    else if ( symbol->attribute() == QLatin1String( "Z" ) )
    {
      attrIsZ = true;
    }
    else
    {
      const QgsPointCloudAttribute *attr = collection.find( symbol->attribute(), offset );
      if ( attr )
      {
        attributeType = attr->type();
        attributeName = attr->name();
        attributeOffset = attributes.pointRecordSize();
        attributes.push_back( *attr );
      }
    }
  }

  if ( attributeName.isEmpty() && !attrIsX && !attrIsY && !attrIsZ )
    return;

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  std::unique_ptr<QgsPointCloudBlock> block( pointCloudBlock( pc, n, request, context ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  const int count = block->pointCount();
  const std::size_t recordSize = block->attributes().pointRecordSize();

  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();
  const double zValueScale = context.zValueScale();
  const double zValueOffset = context.zValueFixedOffset();
  const QgsCoordinateTransform coordinateTransform = context.coordinateTransform();
  bool alreadyPrintedDebug = false;

  const QSet<int> filteredOutValues = context.getFilteredOutValues();
  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * )( ptr + i * recordSize + xOffset );
    const qint32 iy = *( qint32 * )( ptr + i * recordSize + yOffset );
    const qint32 iz = *( qint32 * )( ptr + i * recordSize + zOffset );

    double x = blockOffset.x() + blockScale.x() * ix;
    double y = blockOffset.y() + blockScale.y() * iy;
    double z = ( blockOffset.z() + blockScale.z() * iz ) * zValueScale + zValueOffset;
    try
    {
      coordinateTransform.transformInPlace( x, y, z );
    }
    catch ( QgsCsException & )
    {
      if ( !alreadyPrintedDebug )
      {
        QgsDebugMsg( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point( x, y, z );
    const QgsVector3D p = context.map().mapToWorldCoordinates( point );
    float iParam = 0.0f;
    if ( attrIsX )
      iParam = x;
    else if ( attrIsY )
      iParam = y;
    else if ( attrIsZ )
      iParam = z;
    else
      context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, iParam );

    if ( filteredOutValues.contains( ( int ) iParam ) )
      continue;
    outNormal.positions.push_back( QVector3D( p.x(), p.y(), p.z() ) );
    outNormal.parameter.push_back( iParam );
  }
}

void QgsClassificationPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DRender::QGeometry *QgsClassificationPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
{
  return new QgsColorRampPointCloud3DGeometry( parent, data, byteStride );
}

/// @endcond
