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
#include "qgsabstractvectorlayer3drenderer.h" // for QgsVectorLayer3DTilingSettings

#include "qgspoint3dsymbol.h"
#include "qgsphongmaterialsettings.h"

#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"

#include <QtConcurrent>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>

///@cond PRIVATE

QgsPointCloud3DGeometry::QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data )
  : Qt3DRender::QGeometry( parent )
  , mPositionAttribute( new Qt3DRender::QAttribute( this ) )
  , mClassAttribute( new Qt3DRender::QAttribute( this ) )
  , mVertexBuffer( new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this ) )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  mPositionAttribute->setByteOffset( 0 );
  mPositionAttribute->setByteStride( 16 );

  mClassAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mClassAttribute->setBuffer( mVertexBuffer );
  mClassAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mClassAttribute->setVertexSize( 1 );
  mClassAttribute->setName( "cls" );
  mClassAttribute->setByteOffset( 12 );
  mClassAttribute->setByteStride( 16 );

  addAttribute( mPositionAttribute );
  addAttribute( mClassAttribute );


  makeVertexBuffer( data );
}

void QgsPointCloud3DGeometry::makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( data.positions.size() * 4 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  Q_ASSERT( data.positions.count() == data.classes.count() );
  for ( int i = 0; i < data.positions.size(); ++i )
  {
    rawVertexArray[idx++] = data.positions.at( i ).x();
    rawVertexArray[idx++] = data.positions.at( i ).y();
    rawVertexArray[idx++] = data.positions.at( i ).z();
    rawVertexArray[idx++] = data.classes.at( i );
  }

  mVertexCount = data.positions.size();
  mVertexBuffer->setData( vertexBufferData );
}


QgsPointCloud3DSymbolHandler::QgsPointCloud3DSymbolHandler()
{

}

bool QgsPointCloud3DSymbolHandler::prepare( const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  return true;
}

void QgsPointCloud3DSymbolHandler::processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context )
{
  const QVector<qint32> data = pc->nodePositionDataAsInt32( n );
  const QVector<char> classes = pc->nodeClassesDataAsChar( n );
  const QgsVector3D scale = pc->scale();
  const QgsVector3D offset = pc->offset();

  //qDebug() << "  node " << data.count()/3 << " points";

  // QgsRectangle mapExtent = context.map().extent()???

  const qint32 *ptr = data.constData();
  int count = data.count() / 3;
  for ( int i = 0; i < count; ++i )
  {
    qint32 ix = ptr[i * 3 + 0];
    qint32 iy = ptr[i * 3 + 1];
    qint32 iz = ptr[i * 3 + 2];

    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
    // if ( mapExtent.contains( QgsPointXY( x, y ) ) )
    // {
    double z = offset.z() + scale.z() * iz;
    QVector3D point( x, y, z );
    QgsVector3D p = context.map().mapToWorldCoordinates( point );
    outNormal.positions.push_back( QVector3D( p.x(), p.y(), p.z() ) );

    // }
  }
  outNormal.classes.append( classes );
}

void QgsPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  // makeEntity( parent, context, outSelected, true );

  // updateZRangeFromPositions( outNormal.positions );
  // updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied separately in QTransform added to sub-entities
  //float symbolHeight = mSymbol->transform().data()[13];
  //mZMin += symbolHeight;
  //mZMax += symbolHeight;
}
#include <QPointSize>
void QgsPointCloud3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsPointCloud3DSymbolHandler::PointData &out, bool selected )
{
  Q_UNUSED( selected )
  Q_UNUSED( context )

  if ( out.positions.empty() )
    return;

  // Geometry
  QgsPointCloud3DGeometry *geom = new QgsPointCloud3DGeometry( parent, out );
  Qt3DRender::QGeometryRenderer *gr = new Qt3DRender::QGeometryRenderer;
  gr->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  gr->setVertexCount( out.positions.count() );
  gr->setGeometry( geom );

  // Transform
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;
  // tr->setMatrix( symbol->transform() );
  // tr->setTranslation( position + tr->translation() );

  // Material
  Qt3DRender::QMaterial *mat = new Qt3DRender::QMaterial;

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( mat );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/pointcloud.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/pointcloud.frag" ) ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( mat );
  renderPass->setShaderProgram( shaderProgram );

  Qt3DRender::QPointSize *pointSize = new Qt3DRender::QPointSize( renderPass );
  pointSize->setSizeMode( Qt3DRender::QPointSize::Programmable );  // supported since OpenGL 3.2
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


QgsPointCloudLayerChunkLoader::QgsPointCloudLayerChunkLoader( const QgsPointCloudLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap )
    // , mSource( new QgsPointCloudLayerFeatureSource( factory->mLayer ) )
{
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsPointCloudLayerChunkLoader::finished );
    return;
  }
  qDebug() << "creating entity!";

  QgsPointCloudLayer *layer = mFactory->mLayer;
  const Qgs3DMapSettings &map = mFactory->mMap;

  QgsPointCloud3DSymbolHandler *handler = new QgsPointCloud3DSymbolHandler;
  mHandler.reset( handler );

  QgsPointCloudIndex *pc = layer->dataProvider()->index();

  //QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
  //exprContext.setFields( layer->fields() );
  //mContext.setExpressionContext( exprContext );

  //QSet<QString> attributeNames;
  //if ( !mHandler->prepare( mContext, attributeNames ) )
  // {
  //  QgsDebugMsg( QStringLiteral( "Failed to prepare 3D feature handler!" ) );
  //  return;
  //}

  // build the feature request
  // QgsFeatureRequest req;
  //req.setDestinationCrs( map.crs(), map.transformContext() );
  //req.setSubsetOfAttributes( attributeNames, layer->fields() );

  // only a subset of data to be queried
  QgsRectangle rect = Qgs3DUtils::worldToMapExtent( node->bbox(), map.origin() );
  //req.setFilterRect( rect );

  // TODO: set depth based on map units per pixel
  int depth = 3;
  QList<IndexedPointCloudNode> nodes = pc->traverseTree( rect, pc->root(), depth );

  //
  // this will be run in a background thread
  //
  QFuture<void> future = QtConcurrent::run( [pc, nodes, this]
  {
    QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "PC chunk load" ) );

    for ( const IndexedPointCloudNode &n : nodes )
    {
      if ( mCanceled )
      {
        qDebug() << "canceled";
        break;
      }
      mHandler->processNode( pc, n, mContext );
    }
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
  if ( mNode->level() < mFactory->mLeafLevel )
  {
    return new Qt3DCore::QEntity( parent );  // dummy entity
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  mHandler->finalize( entity, mContext );
  return entity;
}


///////////////


QgsPointCloudLayerChunkLoaderFactory::QgsPointCloudLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsPointCloudLayer *vl, int leafLevel )
  : mMap( map )
  , mLayer( vl )
  , mLeafLevel( leafLevel )
{
}

QgsChunkLoader *QgsPointCloudLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsPointCloudLayerChunkLoader( this, node );
}


///////////////

QgsPointCloudLayerChunkedEntity::QgsPointCloudLayerChunkedEntity( QgsPointCloudLayer *vl, double zMin, double zMax, const Qgs3DMapSettings &map )
  : QgsChunkedEntity( Qgs3DUtils::layerToWorldExtent( vl->extent(), zMin, zMax, vl->crs(), map.origin(), map.crs(), map.transformContext() ),
                      -1, // rootError (negative error means that the node does not contain anything)
                      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      0, //QgsVectorLayer3DTilingSettings().zoomLevelsCount() - 1,
                      new QgsPointCloudLayerChunkLoaderFactory( map, vl, 0 /*QgsVectorLayer3DTilingSettings().zoomLevelsCount() - 1*/ ), true )
{
  setShowBoundingBoxes( true ); //QgsVectorLayer3DTilingSettings().showBoundingBoxes() );
}

QgsPointCloudLayerChunkedEntity::~QgsPointCloudLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
