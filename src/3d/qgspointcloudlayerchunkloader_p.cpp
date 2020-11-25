/***************************************************************************
  qgspointcloudlayerchunkloader_p.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerchunkloader_p.h"

#include "qgs3dutils.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgschunknode_p.h"
#include "qgslogger.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgseventtracing.h"

#include "qgspoint3dsymbol.h"
#include "qgsphongmaterialsettings.h"

#include "qgspointcloud3dsymbol.h"

#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"

#include "qgspointcloud3dsymbol.h"
#include "qgscolorramptexture.h"

#include <QtConcurrent>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QPointSize>

///@cond PRIVATE

QgsPointCloud3DGeometry::QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mParameterAttribute( new Qt3DRender::QAttribute( this ) )
  , mColorAttribute( new Qt3DRender::QAttribute( this ) )
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
#else
  , mVertexBuffer( new Qt3DRender::QBuffer( this ) )
#endif
  , mRenderingStyle( symbol->renderingStyle() )
{
  unsigned int byte_stride = mRenderingStyle == QgsPointCloud3DSymbol::RenderingStyle::RGBRendering ? 28 : 16;
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( byte_stride );

  mParameterAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mParameterAttribute->setBuffer( mVertexBuffer );
  mParameterAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mParameterAttribute->setVertexSize( 1 );
  mParameterAttribute->setName( "vertexParameter" );
  mParameterAttribute->setByteOffset( 12 );
  mParameterAttribute->setByteStride( byte_stride );

  addAttribute( mPositionAttribute );
  addAttribute( mParameterAttribute );

  if ( mRenderingStyle == QgsPointCloud3DSymbol::RenderingStyle::RGBRendering )
  {
    mColorAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
    mColorAttribute->setBuffer( mVertexBuffer );
    mColorAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
    mColorAttribute->setVertexSize( 3 );
    mColorAttribute->setName( QStringLiteral( "vertexColor" ) );
    mColorAttribute->setByteOffset( 16 );
    mColorAttribute->setByteStride( byte_stride );
    addAttribute( mColorAttribute );
  }

  makeVertexBuffer( data );
}

void QgsPointCloud3DGeometry::makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data )
{
  QByteArray vertexBufferData;
  if ( mRenderingStyle == QgsPointCloud3DSymbol::RenderingStyle::RGBRendering )
    vertexBufferData.resize( data.positions.size() * 7 * sizeof( float ) );
  else
    vertexBufferData.resize( data.positions.size() * 4 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  Q_ASSERT( data.positions.count() == data.parameter.count() );
  for ( int i = 0; i < data.positions.size(); ++i )
  {
    rawVertexArray[idx++] = data.positions.at( i ).x();
    rawVertexArray[idx++] = data.positions.at( i ).y();
    rawVertexArray[idx++] = data.positions.at( i ).z();
    rawVertexArray[idx++] = data.parameter.at( i );
    if ( mRenderingStyle == QgsPointCloud3DSymbol::RenderingStyle::RGBRendering )
    {
      rawVertexArray[idx++] = data.colors.at( i ).x();
      rawVertexArray[idx++] = data.colors.at( i ).y();
      rawVertexArray[idx++] = data.colors.at( i ).z();
    }
  }

  mVertexCount = data.positions.size();
  mVertexBuffer->setData( vertexBufferData );
}


QgsPointCloud3DSymbolHandler::QgsPointCloud3DSymbolHandler( QgsPointCloud3DSymbol *symbol )
{
  mSymbol.reset( symbol ) ;
}

bool QgsPointCloud3DSymbolHandler::prepare( const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  return true;
}

void QgsPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context )
{
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  std::unique_ptr< QgsPointCloudAttribute > parameterAttribute;
  if ( mSymbol.get()->renderingStyle() == QgsPointCloud3DSymbol::ColorRamp )
  {
    QgsColorRampPointCloud3DSymbol *symbol = dynamic_cast<QgsColorRampPointCloud3DSymbol *>( mSymbol.get() );
    if ( symbol && symbol->layer() )
    {
      int offset = 0;
      const QgsPointCloudAttribute *attr = symbol->layer()->attributes().find( symbol->renderingParameter(), offset );
      if ( attr )
      {
        parameterAttribute.reset( new QgsPointCloudAttribute( attr->name(), attr->type() ) );
        attributes.push_back( *parameterAttribute.get() );
      }
    }
  }

  if ( mSymbol.get()->renderingStyle() == QgsPointCloud3DSymbol::RGBRendering )
  {
    attributes.push_back( QgsPointCloudAttribute( "Red", QgsPointCloudAttribute::DataType::Short ) );
    attributes.push_back( QgsPointCloudAttribute( "Green", QgsPointCloudAttribute::DataType::Short ) );
    attributes.push_back( QgsPointCloudAttribute( "Blue", QgsPointCloudAttribute::DataType::Short ) );
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

    if ( mSymbol.get()->renderingStyle() == QgsPointCloud3DSymbol::RGBRendering )
    {
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
}

void QgsPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
}

void QgsPointCloud3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsPointCloud3DSymbolHandler::PointData &out, bool selected )
{
  Q_UNUSED( selected )
  Q_UNUSED( context )

  if ( out.positions.empty() )
    return;

  // Geometry
  QgsPointCloud3DGeometry *geom = new QgsPointCloud3DGeometry( parent, out, mSymbol.get() );
  Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
  gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  gr->setVertexCount( out.positions.count() );
  gr->setGeometry( geom );

  // Transform
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;

  // Material
  Qt3DRender::QMaterial *mat = nullptr;
  switch ( mSymbol->renderingStyle() )
  {
    case QgsPointCloud3DSymbol::RenderingStyle::NoRendering:
      mat = constructMaterial( dynamic_cast<QgsNoRenderingPointCloud3DSymbol *>( mSymbol.get() ) );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
      mat = constructMaterial( dynamic_cast<QgsSingleColorPointCloud3DSymbol *>( mSymbol.get() ) );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
      mat = constructMaterial( dynamic_cast<QgsColorRampPointCloud3DSymbol *>( mSymbol.get() ) );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::RGBRendering:
      mat = constructMaterial( dynamic_cast<QgsRGBPointCloud3DSymbol *>( mSymbol.get() ) );
      break;
  }

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( mat );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/pointcloud.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/pointcloud.frag" ) ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( mat );
  renderPass->setShaderProgram( shaderProgram );

  Qt3DRender::QPointSize *pointSize = new Qt3DRender::QPointSize( renderPass );
  pointSize->setSizeMode( Qt3DRender::QPointSize::Programmable );  // supported since OpenGL 3.2
  switch ( mSymbol->renderingStyle() )
  {
    case QgsPointCloud3DSymbol::RenderingStyle::NoRendering:
      // Do Nothing since there is no rendering
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
      pointSize->setValue( dynamic_cast<QgsSingleColorPointCloud3DSymbol *>( mSymbol.get() )->pointSize() );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
      pointSize->setValue( dynamic_cast<QgsColorRampPointCloud3DSymbol *>( mSymbol.get() )->pointSize() );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::RGBRendering:
      pointSize->setValue( dynamic_cast<QgsRGBPointCloud3DSymbol *>( mSymbol.get() )->pointSize() );
      break;
  }
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

Qt3DRender::QMaterial *QgsPointCloud3DSymbolHandler::constructMaterial( QgsNoRenderingPointCloud3DSymbol *symbol )
{
  Qt3DRender::QMaterial *mat = new Qt3DRender::QMaterial;
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", symbol->renderingStyle() );
  mat->addParameter( renderingStyle );
  return mat;
}

Qt3DRender::QMaterial *QgsPointCloud3DSymbolHandler::constructMaterial( QgsSingleColorPointCloud3DSymbol *symbol )
{
  Qt3DRender::QMaterial *mat = new Qt3DRender::QMaterial;
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", symbol->renderingStyle() );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( symbol->pointSize() ) );
  mat->addParameter( pointSizeParameter );
  QColor singleColor = symbol->singleColor();
  Qt3DRender::QParameter *singleColorParameter = new Qt3DRender::QParameter( "u_singleColor", QVector3D( singleColor.redF(), singleColor.greenF(), singleColor.blueF() ) );
  mat->addParameter( singleColorParameter );
  return mat;
}

Qt3DRender::QMaterial *QgsPointCloud3DSymbolHandler::constructMaterial( QgsColorRampPointCloud3DSymbol *symbol )
{
  Qt3DRender::QMaterial *mat = new Qt3DRender::QMaterial;
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", symbol->renderingStyle() );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( symbol->pointSize() ) );
  mat->addParameter( pointSizeParameter );
  QgsColorRampShader colorRampShader = symbol->colorRampShader();

  // Create the texture to pass the color ramp
  Qt3DRender::QTexture1D *colorRampTexture = nullptr;
  if ( colorRampShader.colorRampItemList().count() > 0 )
  {
    colorRampTexture = new Qt3DRender::QTexture1D( mat );
    colorRampTexture->addTextureImage( new QgsColorRampTexture( colorRampShader, 1 ) );
    colorRampTexture->setMinificationFilter( Qt3DRender::QTexture1D::Linear );
    colorRampTexture->setMagnificationFilter( Qt3DRender::QTexture1D::Linear );
  }

  // Parameters
  Qt3DRender::QParameter *colorRampTextureParameter = new Qt3DRender::QParameter( "u_colorRampTexture", colorRampTexture );
  mat->addParameter( colorRampTextureParameter );
  Qt3DRender::QParameter *colorRampCountParameter = new Qt3DRender::QParameter( "u_colorRampCount", colorRampShader.colorRampItemList().count() );
  mat->addParameter( colorRampCountParameter );
  int colorRampType = colorRampShader.colorRampType();
  Qt3DRender::QParameter *colorRampTypeParameter = new Qt3DRender::QParameter( "u_colorRampType", colorRampType );
  mat->addParameter( colorRampTypeParameter );
  return mat;
}

Qt3DRender::QMaterial *QgsPointCloud3DSymbolHandler::constructMaterial( QgsRGBPointCloud3DSymbol *symbol )
{
  Qt3DRender::QMaterial *mat = new Qt3DRender::QMaterial;
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", symbol->renderingStyle() );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( symbol->pointSize() ) );
  mat->addParameter( pointSizeParameter );
  return mat;
}

QgsPointCloudLayerChunkLoader::QgsPointCloudLayerChunkLoader( const QgsPointCloudLayerChunkLoaderFactory *factory, QgsChunkNode *node, QgsPointCloud3DSymbol *symbol )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap )
{
  QgsPointCloudIndex *pc = mFactory->mPointCloudIndex;
  QgsChunkNodeId nodeId = node->tileId();
  IndexedPointCloudNode pcNode( nodeId.d, nodeId.x, nodeId.y, nodeId.z );

  Q_ASSERT( pc->hasNode( pcNode ) );

  QgsDebugMsgLevel( QStringLiteral( "loading entity %1" ).arg( node->tileId().text() ), 2 );

  mHandler.reset( new QgsPointCloud3DSymbolHandler( symbol ) );

  //
  // this will be run in a background thread
  //
  QFuture<void> future = QtConcurrent::run( [pc, pcNode, this]
  {
    QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "PC chunk load" ) );

    if ( mCanceled )
    {
      QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
      return;
    }
    mHandler->processNode( pc, pcNode, mContext );
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher = new QFutureWatcher<void>( this );
  mFutureWatcher->setFuture( future );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

}

QgsPointCloudLayerChunkLoader::~QgsPointCloudLayerChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsPointCloudLayerChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsPointCloudLayerChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsPointCloudIndex *pc = mFactory->mPointCloudIndex;
  QgsChunkNodeId nodeId = mNode->tileId();
  IndexedPointCloudNode pcNode( nodeId.d, nodeId.x, nodeId.y, nodeId.z );
  Q_ASSERT( pc->hasNode( pcNode ) );

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  mHandler->finalize( entity, mContext );
  return entity;
}


///////////////


QgsPointCloudLayerChunkLoaderFactory::QgsPointCloudLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsPointCloudIndex *pc, QgsPointCloud3DSymbol *symbol )
  : mMap( map )
  , mPointCloudIndex( pc )
{
  mSymbol.reset( symbol );
}

QgsChunkLoader *QgsPointCloudLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  QgsChunkNodeId id = node->tileId();
  Q_ASSERT( mPointCloudIndex->hasNode( IndexedPointCloudNode( id.d, id.x, id.y, id.z ) ) );
  return new QgsPointCloudLayerChunkLoader( this, node, dynamic_cast<QgsPointCloud3DSymbol *>( mSymbol->clone() ) );
}

QgsAABB nodeBoundsToAABB( QgsPointCloudDataBounds nodeBounds, QgsVector3D offset, QgsVector3D scale, const Qgs3DMapSettings &map );

QgsChunkNode *QgsPointCloudLayerChunkLoaderFactory::createRootNode() const
{
  QgsAABB bbox = nodeBoundsToAABB( mPointCloudIndex->nodeBounds( IndexedPointCloudNode( 0, 0, 0, 0 ) ), mPointCloudIndex->offset(), mPointCloudIndex->scale(), mMap );
  float error = mPointCloudIndex->nodeError( IndexedPointCloudNode( 0, 0, 0, 0 ) );
  return new QgsChunkNode( QgsChunkNodeId( 0, 0, 0, 0 ), bbox, error );
}

QVector<QgsChunkNode *> QgsPointCloudLayerChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;
  QgsChunkNodeId nodeId = node->tileId();
  QgsAABB bbox = node->bbox();
  float childError = node->error() / 2;
  float xc = bbox.xCenter(), yc = bbox.yCenter(), zc = bbox.zCenter();

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy, nodeId.z * 2 + dz );

    if ( !mPointCloudIndex->hasNode( IndexedPointCloudNode( childId.d, childId.x, childId.y, childId.z ) ) )
      continue;

    // the Y and Z coordinates below are intentionally flipped, because
    // in chunk node IDs the X,Y axes define horizontal plane,
    // while in our 3D scene the X,Z axes define the horizontal plane
    float chXMin = dx ? xc : bbox.xMin;
    float chXMax = dx ? bbox.xMax : xc;
    // Z axis: values are increasing to the south
    float chZMin = !dy ? zc : bbox.zMin;
    float chZMax = !dy ? bbox.zMax : zc;
    float chYMin = dz ? yc : bbox.yMin;
    float chYMax = dz ? bbox.yMax : yc;
    children << new QgsChunkNode( childId, QgsAABB( chXMin, chYMin, chZMin, chXMax, chYMax, chZMax ), childError, node );
  }
  return children;
}

///////////////


QgsAABB nodeBoundsToAABB( QgsPointCloudDataBounds nodeBounds, QgsVector3D offset, QgsVector3D scale, const Qgs3DMapSettings &map )
{
  // TODO: reprojection from layer to map coordinates if needed
  QgsVector3D extentMin3D( nodeBounds.xMin() * scale.x() + offset.x(), nodeBounds.yMin() * scale.y() + offset.y(), nodeBounds.zMin() * scale.z() + offset.z() );
  QgsVector3D extentMax3D( nodeBounds.xMax() * scale.x() + offset.x(), nodeBounds.yMax() * scale.y() + offset.y(), nodeBounds.zMax() * scale.z() + offset.z() );
  QgsVector3D worldExtentMin3D = Qgs3DUtils::mapToWorldCoordinates( extentMin3D, map.origin() );
  QgsVector3D worldExtentMax3D = Qgs3DUtils::mapToWorldCoordinates( extentMax3D, map.origin() );
  QgsAABB rootBbox( worldExtentMin3D.x(), worldExtentMin3D.y(), worldExtentMin3D.z(),
                    worldExtentMax3D.x(), worldExtentMax3D.y(), worldExtentMax3D.z() );
  return rootBbox;
}


QgsPointCloudLayerChunkedEntity::QgsPointCloudLayerChunkedEntity( QgsPointCloudIndex *pc, const Qgs3DMapSettings &map, QgsPointCloud3DSymbol *symbol )
  : QgsChunkedEntity( 5, // max. allowed screen error (in pixels)  -- // TODO
                      new QgsPointCloudLayerChunkLoaderFactory( map, pc, symbol ), true )
{
  setUsingAdditiveStrategy( true );
  setShowBoundingBoxes( false );
}

QgsPointCloudLayerChunkedEntity::~QgsPointCloudLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
