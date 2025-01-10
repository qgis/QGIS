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
#include "moc_qgspointcloud3dsymbol_p.cpp"

///@cond PRIVATE

#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudblockrequest.h"
#include "qgsfeedback.h"
#include "qgsaabb.h"
#include "qgsgeotransform.h"

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QParameter>
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QEffect>
#include <QPointSize>
#include <QUrl>

#include <delaunator.hpp>

// pick a point that we'll use as origin for coordinates for this node's points
static QgsVector3D originFromNodeBounds( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context )
{
  QgsBox3D bounds = pc.getNode( n ).bounds();
  double nodeOriginX = bounds.xMinimum();
  double nodeOriginY = bounds.yMinimum();
  double nodeOriginZ = bounds.zMinimum() * context.zValueScale() + context.zValueFixedOffset();
  try
  {
    context.coordinateTransform().transformInPlace( nodeOriginX, nodeOriginY, nodeOriginZ );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Error transforming node origin point" ) );
  }
  return QgsVector3D( nodeOriginX, nodeOriginY, nodeOriginZ );
}


QgsPointCloud3DGeometry::QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : Qt3DQGeometry( parent )
  , mPositionAttribute( new Qt3DQAttribute( this ) )
  , mParameterAttribute( new Qt3DQAttribute( this ) )
  , mPointSizeAttribute( new Qt3DQAttribute( this ) )
  , mColorAttribute( new Qt3DQAttribute( this ) )
  , mTriangleIndexAttribute( new Qt3DQAttribute( this ) )
  , mNormalsAttribute( new Qt3DQAttribute( this ) )
  , mVertexBuffer( new Qt3DQBuffer( this ) )
  , mByteStride( byteStride )
{
  if ( !data.triangles.isEmpty() )
  {
    mTriangleBuffer = new Qt3DQBuffer( this );
    mTriangleIndexAttribute->setAttributeType( Qt3DQAttribute::IndexAttribute );
    mTriangleIndexAttribute->setBuffer( mTriangleBuffer );
    mTriangleIndexAttribute->setVertexBaseType( Qt3DQAttribute::UnsignedInt );
    mTriangleBuffer->setData( data.triangles );
    mTriangleIndexAttribute->setCount( data.triangles.size() / sizeof( quint32 ) );
    addAttribute( mTriangleIndexAttribute );
  }

  if ( !data.normals.isEmpty() )
  {
    mNormalsBuffer = new Qt3DQBuffer( this );
    mNormalsAttribute->setName( Qt3DQAttribute::defaultNormalAttributeName() );
    mNormalsAttribute->setVertexBaseType( Qt3DQAttribute::Float );
    mNormalsAttribute->setVertexSize( 3 );
    mNormalsAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
    mNormalsAttribute->setBuffer( mNormalsBuffer );
    mNormalsBuffer->setData( data.normals );
    mNormalsAttribute->setCount( data.normals.size() / ( 3 * sizeof( float ) ) );
    addAttribute( mNormalsAttribute );
  }
}

QgsSingleColorPointCloud3DGeometry::QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : QgsPointCloud3DGeometry( parent, data, byteStride )
{
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
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
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( mByteStride );
  addAttribute( mPositionAttribute );
  mParameterAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mParameterAttribute->setBuffer( mVertexBuffer );
  mParameterAttribute->setVertexBaseType( Qt3DQAttribute::Float );
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
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( mByteStride );
  addAttribute( mPositionAttribute );
  mColorAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mColorAttribute->setBuffer( mVertexBuffer );
  mColorAttribute->setVertexBaseType( Qt3DQAttribute::Float );
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

QgsClassificationPointCloud3DGeometry::QgsClassificationPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
  : QgsPointCloud3DGeometry( parent, data, byteStride )
{
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( mByteStride );
  addAttribute( mPositionAttribute );
  mParameterAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mParameterAttribute->setBuffer( mVertexBuffer );
  mParameterAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mParameterAttribute->setVertexSize( 1 );
  mParameterAttribute->setName( "vertexParameter" );
  mParameterAttribute->setByteOffset( 12 );
  mParameterAttribute->setByteStride( mByteStride );
  addAttribute( mParameterAttribute );
  mPointSizeAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPointSizeAttribute->setBuffer( mVertexBuffer );
  mPointSizeAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPointSizeAttribute->setVertexSize( 1 );
  mPointSizeAttribute->setName( "vertexSize" );
  mPointSizeAttribute->setByteOffset( 16 );
  mPointSizeAttribute->setByteStride( mByteStride );
  addAttribute( mPointSizeAttribute );
  makeVertexBuffer( data );
}

void QgsClassificationPointCloud3DGeometry::makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( data.positions.size() * mByteStride );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  Q_ASSERT( data.positions.size() == data.parameter.size() );
  Q_ASSERT( data.positions.size() == data.pointSizes.size() );
  for ( int i = 0; i < data.positions.size(); ++i )
  {
    rawVertexArray[idx++] = data.positions.at( i ).x();
    rawVertexArray[idx++] = data.positions.at( i ).y();
    rawVertexArray[idx++] = data.positions.at( i ).z();
    rawVertexArray[idx++] = data.parameter.at( i );
    rawVertexArray[idx++] = data.pointSizes.at( i );
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
  Qt3DQGeometry *geom = makeGeometry( parent, out, context.symbol()->byteStride() );
  Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
  if ( context.symbol()->renderAsTriangles() && !out.triangles.isEmpty() )
  {
    gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
    gr->setVertexCount( out.triangles.size() / sizeof( quint32 ) );
  }
  else
  {
    gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
    gr->setVertexCount( out.positions.count() );
  }
  gr->setGeometry( geom );

  // Transform: chunks are using coordinates relative to chunk origin, with X,Y,Z axes being the same
  // as map coordinates, so we need to rotate and translate entities to get them into world coordinates
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setGeoTranslation( out.positionsOrigin );

  // Material
  QgsMaterial *mat = new QgsMaterial;
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
    pointSize->setSizeMode( Qt3DRender::QPointSize::Programmable ); // supported since OpenGL 3.2
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


std::vector<double> QgsPointCloud3DSymbolHandler::getVertices( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context, const QgsBox3D &box3D )
{
  bool hasColorData = !outNormal.colors.empty();
  bool hasParameterData = !outNormal.parameter.empty();
  bool hasPointSizeData = !outNormal.pointSizes.empty();

  // first, get the points of the concerned node
  std::vector<double> vertices( outNormal.positions.size() * 2 );
  size_t idx = 0;
  for ( int i = 0; i < outNormal.positions.size(); ++i )
  {
    vertices[idx++] = outNormal.positions.at( i ).x();
    vertices[idx++] = -outNormal.positions.at( i ).y(); // flipping y to have correctly oriented triangles from delaunator
  }

  // next, we also need all points of all parents nodes to make the triangulation (also external points)
  QgsPointCloudNodeId parentNode = n.parentNode();

  double span = pc.span();
  //factor to take account of the density of the point to calculate extension of the bounding box
  // with a usual value span = 128, bounding box is extended by 12.5 % on each side.
  double extraBoxFactor = 16 / span;

  // We keep all points in vertical direction to avoid odd triangulation if points are isolated on top
  QgsRectangle rectRelativeToChunkOrigin = ( box3D - outNormal.positionsOrigin ).toRectangle();
  rectRelativeToChunkOrigin.grow( extraBoxFactor * std::max( box3D.width(), box3D.height() ) );

  PointData filteredExtraPointData;
  while ( parentNode.d() >= 0 )
  {
    PointData outputParent;
    processNode( pc, parentNode, context, &outputParent );

    // the "main" chunk and each parent chunks have their origins
    QVector3D originDifference = ( outputParent.positionsOrigin - outNormal.positionsOrigin ).toVector3D();

    for ( int i = 0; i < outputParent.positions.count(); ++i )
    {
      const QVector3D pos = outputParent.positions.at( i ) + originDifference;
      if ( rectRelativeToChunkOrigin.contains( pos.x(), pos.y() ) )
      {
        filteredExtraPointData.positions.append( pos );
        vertices.push_back( pos.x() );
        vertices.push_back( -pos.y() ); // flipping y to have correctly oriented triangles from delaunator

        if ( hasColorData )
          filteredExtraPointData.colors.append( outputParent.colors.at( i ) );
        if ( hasParameterData )
          filteredExtraPointData.parameter.append( outputParent.parameter.at( i ) );
        if ( hasPointSizeData )
          filteredExtraPointData.pointSizes.append( outputParent.pointSizes.at( i ) );
      }
    }

    parentNode = parentNode.parentNode();
  }

  outNormal.positions.append( filteredExtraPointData.positions );
  outNormal.colors.append( filteredExtraPointData.colors );
  outNormal.parameter.append( filteredExtraPointData.parameter );
  outNormal.pointSizes.append( filteredExtraPointData.pointSizes );

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
        triangleVertices.at( 2 ) - triangleVertices.at( 0 )
      );
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

void QgsPointCloud3DSymbolHandler::filterTriangles( const std::vector<size_t> &triangleIndexes, const QgsPointCloud3DRenderContext &context, const QgsBox3D &box3D )
{
  outNormal.triangles.resize( triangleIndexes.size() * sizeof( quint32 ) );
  quint32 *indexPtr = reinterpret_cast<quint32 *>( outNormal.triangles.data() );
  size_t effective = 0;

  bool horizontalFilter = context.symbol()->horizontalTriangleFilter();
  bool verticalFilter = context.symbol()->verticalTriangleFilter();
  float horizontalThreshold = context.symbol()->horizontalFilterThreshold();
  float verticalThreshold = context.symbol()->verticalFilterThreshold();

  QgsBox3D boxRelativeToChunkOrigin = box3D - outNormal.positionsOrigin;

  for ( size_t i = 0; i < triangleIndexes.size(); i += 3 )
  {
    bool atLeastOneInBox = false;
    bool horizontalSkip = false;
    bool verticalSkip = false;
    for ( size_t j = 0; j < 3; j++ )
    {
      QVector3D pos = outNormal.positions.at( triangleIndexes.at( i + j ) );
      atLeastOneInBox |= boxRelativeToChunkOrigin.contains( pos.x(), pos.y(), pos.z() );

      if ( verticalFilter || horizontalFilter )
      {
        const QVector3D pos2 = outNormal.positions.at( triangleIndexes.at( i + ( j + 1 ) % 3 ) );

        if ( verticalFilter )
          verticalSkip |= std::fabs( pos.z() - pos2.z() ) > verticalThreshold;

        if ( horizontalFilter && !verticalSkip )
        {
          // filter only in the horizontal plan, it is a 2.5D triangulation.
          horizontalSkip |= sqrt( std::pow( pos.x() - pos2.x(), 2 ) + std::pow( pos.y() - pos2.y(), 2 ) ) > horizontalThreshold;
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

void QgsPointCloud3DSymbolHandler::triangulate( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context, const QgsBox3D &box3D )
{
  if ( outNormal.positions.isEmpty() )
    return;

  // Triangulation happens here
  std::unique_ptr<delaunator::Delaunator> triangulation;
  try
  {
    std::vector<double> vertices = getVertices( pc, n, context, box3D );
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
  filterTriangles( triangleIndexes, context, box3D );
}

std::unique_ptr<QgsPointCloudBlock> QgsPointCloud3DSymbolHandler::pointCloudBlock( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request, const QgsPointCloud3DRenderContext &context )
{
  std::unique_ptr<QgsPointCloudBlock> block;
  if ( pc.accessType() == Qgis::PointCloudAccessType::Local )
  {
    block = pc.nodeData( n, request );
  }
  else if ( pc.accessType() == Qgis::PointCloudAccessType::Remote )
  {
    QgsPointCloudNode node = pc.getNode( n );
    if ( node.pointCount() < 1 )
      return block;

    bool loopAborted = false;
    QEventLoop loop;
    QgsPointCloudBlockRequest *req = pc.asyncNodeData( n, request );
    QObject::connect( req, &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
    QObject::connect( context.feedback(), &QgsFeedback::canceled, &loop, [&]() {
      loopAborted = true;
      loop.quit();
    } );
    loop.exec();

    if ( !loopAborted )
      block = req->takeBlock();
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

void QgsSingleColorPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context, PointData *output )
{
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  request.setFilterRect( context.layerExtent() );
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

  if ( !output )
    output = &outNormal;

  output->positionsOrigin = originFromNodeBounds( pc, n, context );

  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * ) ( ptr + i * recordSize + 0 );
    const qint32 iy = *( qint32 * ) ( ptr + i * recordSize + 4 );
    const qint32 iz = *( qint32 * ) ( ptr + i * recordSize + 8 );

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
        QgsDebugError( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point = QgsVector3D( x, y, z ) - output->positionsOrigin;
    output->positions.push_back( point.toVector3D() );
  }
}

void QgsSingleColorPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DQGeometry *QgsSingleColorPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
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

void QgsColorRampPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context, PointData *output )
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
  request.setFilterRect( context.layerExtent() );
  std::unique_ptr<QgsPointCloudBlock> block( pointCloudBlock( pc, n, request, context ) );
  if ( !block )
    return;

  const char *ptr = block->data();
  const int count = block->pointCount();
  const std::size_t recordSize = block->attributes().pointRecordSize();

  const QgsVector3D blockScale = block->scale();
  const QgsVector3D blockOffset = block->offset();

  if ( !output )
    output = &outNormal;

  output->positionsOrigin = originFromNodeBounds( pc, n, context );

  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * ) ( ptr + i * recordSize + xOffset );
    const qint32 iy = *( qint32 * ) ( ptr + i * recordSize + yOffset );
    const qint32 iz = *( qint32 * ) ( ptr + i * recordSize + zOffset );

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
        QgsDebugError( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point = QgsVector3D( x, y, z ) - output->positionsOrigin;
    output->positions.push_back( point.toVector3D() );

    if ( attrIsX )
      output->parameter.push_back( x );
    else if ( attrIsY )
      output->parameter.push_back( y );
    else if ( attrIsZ )
      output->parameter.push_back( z );
    else
    {
      float iParam = 0.0f;
      context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, iParam );
      output->parameter.push_back( iParam );
    }
  }
}

void QgsColorRampPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DQGeometry *QgsColorRampPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
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

void QgsRGBPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context, PointData *output )
{
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  QgsRgbPointCloud3DSymbol *symbol = qgis::down_cast<QgsRgbPointCloud3DSymbol *>( context.symbol() );

  // we have to get the RGB attributes using their real data types -- they aren't always short! (sometimes unsigned short)
  int attrOffset = 0;

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
  request.setFilterRect( context.layerExtent() );
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

  if ( !output )
    output = &outNormal;

  output->positionsOrigin = originFromNodeBounds( pc, n, context );

  int ir = 0;
  int ig = 0;
  int ib = 0;
  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * ) ( ptr + i * recordSize + 0 );
    const qint32 iy = *( qint32 * ) ( ptr + i * recordSize + 4 );
    const qint32 iz = *( qint32 * ) ( ptr + i * recordSize + 8 );
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
        QgsDebugError( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point = QgsVector3D( x, y, z ) - output->positionsOrigin;

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

    output->positions.push_back( point.toVector3D() );
    output->colors.push_back( color );
  }
}

void QgsRGBPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DQGeometry *QgsRGBPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
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

void QgsClassificationPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex &pc, const QgsPointCloudNodeId &n, const QgsPointCloud3DRenderContext &context, PointData *output )
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
  if ( !symbol )
    return;

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

  if ( attributeName.isEmpty() && !attrIsX && !attrIsY && !attrIsZ )
    return;

  QgsPointCloudRequest request;
  request.setAttributes( attributes );
  request.setFilterRect( context.layerExtent() );
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

  QList<QgsPointCloudCategory> categoriesList = symbol->categoriesList();
  QVector<int> categoriesValues;
  QHash<int, double> categoriesPointSizes;
  for ( QgsPointCloudCategory &c : categoriesList )
  {
    categoriesValues.push_back( c.value() );
    categoriesPointSizes.insert( c.value(), c.pointSize() > 0 ? c.pointSize() : context.symbol() ? context.symbol()->pointSize()
                                                                                                 : 1.0 );
  }

  if ( !output )
    output = &outNormal;

  output->positionsOrigin = originFromNodeBounds( pc, n, context );

  const QSet<int> filteredOutValues = context.getFilteredOutValues();
  for ( int i = 0; i < count; ++i )
  {
    if ( context.isCanceled() )
      break;

    const qint32 ix = *( qint32 * ) ( ptr + i * recordSize + xOffset );
    const qint32 iy = *( qint32 * ) ( ptr + i * recordSize + yOffset );
    const qint32 iz = *( qint32 * ) ( ptr + i * recordSize + zOffset );

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
        QgsDebugError( QStringLiteral( "Error transforming point coordinate" ) );
        alreadyPrintedDebug = true;
      }
    }
    const QgsVector3D point = QgsVector3D( x, y, z ) - output->positionsOrigin;
    float iParam = 0.0f;
    if ( attrIsX )
      iParam = x;
    else if ( attrIsY )
      iParam = y;
    else if ( attrIsZ )
      iParam = z;
    else
      context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, iParam );

    if ( filteredOutValues.contains( ( int ) iParam ) || !categoriesValues.contains( ( int ) iParam ) )
      continue;
    output->positions.push_back( point.toVector3D() );

    // find iParam actual index in the categories list
    float iParam2 = categoriesValues.indexOf( ( int ) iParam ) + 1;
    output->parameter.push_back( iParam2 );
    output->pointSizes.push_back( categoriesPointSizes.value( ( int ) iParam ) );
  }
}

void QgsClassificationPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

Qt3DQGeometry *QgsClassificationPointCloud3DSymbolHandler::makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride )
{
  return new QgsClassificationPointCloud3DGeometry( parent, data, byteStride );
}

/// @endcond
