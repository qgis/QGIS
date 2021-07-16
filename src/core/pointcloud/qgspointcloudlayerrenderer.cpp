/***************************************************************************
                         qgspointcloudlayerrenderer.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QElapsedTimer>
#include <QPointer>

#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudlayer.h"
#include "qgsrendercontext.h"
#include "qgspointcloudindex.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudextentrenderer.h"
#include "qgslogger.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsmessagelog.h"
#include "qgscircle.h"
#include "qgsmapclippingutils.h"
#include "qgspointcloudblockrequest.h"

QgsPointCloudLayerRenderer::QgsPointCloudLayerRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayer( layer )
  , mLayerAttributes( layer->attributes() )
  , mFeedback( new QgsFeedback )
{
  // TODO: we must not keep pointer to mLayer (it's dangerous) - we must copy anything we need for rendering
  // or use some locking to prevent read/write from multiple threads
  if ( !mLayer || !mLayer->dataProvider() || !mLayer->renderer() )
    return;

  mRenderer.reset( mLayer->renderer()->clone() );

  if ( mLayer->dataProvider()->index() )
  {
    mScale = mLayer->dataProvider()->index()->scale();
    mOffset = mLayer->dataProvider()->index()->offset();
  }

  if ( const QgsPointCloudLayerElevationProperties *elevationProps = qobject_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() ) )
  {
    mZOffset = elevationProps->zOffset();
    mZScale = elevationProps->zScale();
  }

  mCloudExtent = mLayer->dataProvider()->polygonBounds();

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

  mReadyToCompose = false;
}

bool QgsPointCloudLayerRenderer::render()
{
  QgsPointCloudRenderContext context( *renderContext(), mScale, mOffset, mZScale, mZOffset, mFeedback.get() );

  // Set up the render configuration options
  QPainter *painter = context.renderContext().painter();

  QgsScopedQPainterState painterState( painter );
  context.renderContext().setPainterFlagsUsingContext( painter );

  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), QgsMapLayerType::VectorTileLayer, needsPainterClipPath );
    if ( needsPainterClipPath )
      renderContext()->painter()->setClipPath( path, Qt::IntersectClip );
  }

  if ( mRenderer->type() == QLatin1String( "extent" ) )
  {
    // special case for extent only renderer!
    mRenderer->startRender( context );
    static_cast< QgsPointCloudExtentRenderer * >( mRenderer.get() )->renderExtent( mCloudExtent, context );
    mRenderer->stopRender( context );
    mReadyToCompose = true;
    return true;
  }

  // TODO cache!?
  QgsPointCloudIndex *pc = mLayer->dataProvider()->index();
  if ( !pc || !pc->isValid() )
  {
    mReadyToCompose = true;
    return false;
  }

  // if the previous layer render was relatively quick (e.g. less than 3 seconds), the we show any previously
  // cached version of the layer during rendering instead of the usual progressive updates
  if ( mRenderTimeHint > 0 && mRenderTimeHint <= MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
  {
    mBlockRenderUpdates = true;
    mElapsedTimer.start();
  }

  mRenderer->startRender( context );

  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );

  // collect attributes required by renderer
  QSet< QString > rendererAttributes = mRenderer->usedAttributes( context );

  if ( !context.renderContext().zRange().isInfinite() )
    rendererAttributes.insert( QStringLiteral( "Z" ) );

  for ( const QString &attribute : std::as_const( rendererAttributes ) )
  {
    if ( mAttributes.indexOf( attribute ) >= 0 )
      continue; // don't re-add attributes we are already going to fetch

    const int layerIndex = mLayerAttributes.indexOf( attribute );
    if ( layerIndex < 0 )
    {
      QgsMessageLog::logMessage( QObject::tr( "Required attribute %1 not found in layer" ).arg( attribute ), QObject::tr( "Point Cloud" ) );
      continue;
    }

    mAttributes.push_back( mLayerAttributes.at( layerIndex ) );
  }

  QgsPointCloudDataBounds db;

#ifdef QGISDEBUG
  QElapsedTimer t;
  t.start();
#endif

  const IndexedPointCloudNode root = pc->root();

  const double maximumError = context.renderContext().convertToPainterUnits( mRenderer->maximumScreenError(), mRenderer->maximumScreenErrorUnit() );// in pixels

  const QgsRectangle rootNodeExtentLayerCoords = pc->nodeMapExtent( root );
  QgsRectangle rootNodeExtentMapCoords;
  try
  {
    rootNodeExtentMapCoords = context.renderContext().coordinateTransform().transformBoundingBox( rootNodeExtentLayerCoords );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Could not transform node extent to map CRS" ) );
    rootNodeExtentMapCoords = rootNodeExtentLayerCoords;
  }

  const double rootErrorInMapCoordinates = rootNodeExtentMapCoords.width() / pc->span(); // in map coords

  double mapUnitsPerPixel = context.renderContext().mapToPixel().mapUnitsPerPixel();
  if ( ( rootErrorInMapCoordinates < 0.0 ) || ( mapUnitsPerPixel < 0.0 ) || ( maximumError < 0.0 ) )
  {
    QgsDebugMsg( QStringLiteral( "invalid screen error" ) );
    mReadyToCompose = true;
    return false;
  }
  double rootErrorPixels = rootErrorInMapCoordinates / mapUnitsPerPixel; // in pixels
  const QVector<IndexedPointCloudNode> nodes = traverseTree( pc, context.renderContext(), pc->root(), maximumError, rootErrorPixels );

  QgsPointCloudRequest request;
  request.setAttributes( mAttributes );

  // drawing
  int nodesDrawn = 0;
  bool canceled = false;

  if ( pc->accessType() == QgsPointCloudIndex::AccessType::Local )
  {
    nodesDrawn += renderNodesSync( nodes, pc, context, request, canceled );
  }
  else if ( pc->accessType() == QgsPointCloudIndex::AccessType::Remote )
  {
    nodesDrawn += renderNodesAsync( nodes, pc, context, request, canceled );
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QStringLiteral( "totals: %1 nodes | %2 points | %3ms" ).arg( nodesDrawn )
                    .arg( context.pointsRendered() )
                    .arg( t.elapsed() ), 2 );
#endif

  mRenderer->stopRender( context );

  mReadyToCompose = true;
  return !canceled;
}

int QgsPointCloudLayerRenderer::renderNodesSync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled )
{
  int nodesDrawn = 0;
  for ( const IndexedPointCloudNode &n : nodes )
  {
    if ( context.renderContext().renderingStopped() )
    {
      QgsDebugMsgLevel( "canceled", 2 );
      canceled = true;
      break;
    }
    std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );

    if ( !block )
      continue;

    QgsVector3D contextScale = context.scale();
    QgsVector3D contextOffset = context.offset();

    context.setScale( block->scale() );
    context.setOffset( block->offset() );

    context.setAttributes( block->attributes() );

    mRenderer->renderBlock( block.get(), context );

    context.setScale( contextScale );
    context.setOffset( contextOffset );

    ++nodesDrawn;

    // as soon as first block is rendered, we can start showing layer updates.
    // but if we are blocking render updates (so that a previously cached image is being shown), we wait
    // at most e.g. 3 seconds before we start forcing progressive updates.
    if ( !mBlockRenderUpdates || mElapsedTimer.elapsed() > MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
    {
      mReadyToCompose = true;
    }
  }
  return nodesDrawn;
}

int QgsPointCloudLayerRenderer::renderNodesAsync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled )
{
  int nodesDrawn = 0;

  QElapsedTimer downloadTimer;
  downloadTimer.start();

  // Instead of loading all point blocks in parallel and then rendering the one by one,
  // we split the processing into groups of size groupSize where we load the blocks of the group
  // in parallel and then render the group's blocks sequentially.
  // This way helps QGIS stay responsive if the nodes vector size is big
  const int groupSize = 4;
  for ( int groupIndex = 0; groupIndex < nodes.size(); groupIndex += groupSize )
  {
    if ( context.feedback() && context.feedback()->isCanceled() )
      break;
    // Async loading of nodes
    const int currentGroupSize = std::min< size_t >( std::max< size_t >( nodes.size() - groupIndex, 0 ), groupSize );
    QVector<QgsPointCloudBlockRequest *> blockRequests( currentGroupSize, nullptr );
    QVector<bool> finishedLoadingBlock( currentGroupSize, false );
    QEventLoop loop;
    if ( context.feedback() )
      QObject::connect( context.feedback(), &QgsFeedback::canceled, &loop, &QEventLoop::quit );
    // Note: All capture by reference warnings here shouldn't be an issue since we have an event loop, so locals won't be deallocated
    for ( int i = 0; i < blockRequests.size(); ++i )
    {
      int nodeIndex = groupIndex + i;
      const IndexedPointCloudNode &n = nodes[nodeIndex];
      const QString nStr = n.toString();
      QgsPointCloudBlockRequest *blockRequest = pc->asyncNodeData( n, request );
      blockRequests[ i ] = blockRequest;
      QObject::connect( blockRequest, &QgsPointCloudBlockRequest::finished, &loop, [ &, i, nStr, blockRequest ]()
      {
        if ( !blockRequest->block() )
        {
          QgsDebugMsg( QStringLiteral( "Unable to load node %1, error: %2" ).arg( nStr, blockRequest->errorStr() ) );
        }
        finishedLoadingBlock[ i ] = true;
        // If all blocks are loaded, exit the event loop
        if ( !finishedLoadingBlock.contains( false ) ) loop.exit();
      } );
    }
    // Wait for all point cloud nodes to finish loading
    loop.exec();

    QgsDebugMsg( QStringLiteral( "Downloaded in : %1ms" ).arg( downloadTimer.elapsed() ) );
    if ( !context.feedback()->isCanceled() )
    {
      // Render all the point cloud blocks sequentially
      for ( int i = 0; i < blockRequests.size(); ++i )
      {
        if ( context.renderContext().renderingStopped() )
        {
          QgsDebugMsgLevel( "canceled", 2 );
          canceled = true;
          break;
        }

        if ( !blockRequests[ i ]->block() )
          continue;

        QgsVector3D contextScale = context.scale();
        QgsVector3D contextOffset = context.offset();

        context.setScale( blockRequests[ i ]->block()->scale() );
        context.setOffset( blockRequests[ i ]->block()->offset() );

        context.setAttributes( blockRequests[ i ]->block()->attributes() );

        mRenderer->renderBlock( blockRequests[ i ]->block(), context );

        context.setScale( contextScale );
        context.setOffset( contextOffset );

        ++nodesDrawn;

        // as soon as first block is rendered, we can start showing layer updates.
        // but if we are blocking render updates (so that a previously cached image is being shown), we wait
        // at most e.g. 3 seconds before we start forcing progressive updates.
        if ( !mBlockRenderUpdates || mElapsedTimer.elapsed() > MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
        {
          mReadyToCompose = true;
        }
      }
    }

    for ( int i = 0; i < blockRequests.size(); ++i )
    {
      if ( blockRequests[ i ] )
      {
        if ( blockRequests[ i ]->block() )
          delete blockRequests[ i ]->block();
        blockRequests[ i ]->deleteLater();
      }
    }
  }

  return nodesDrawn;
}

bool QgsPointCloudLayerRenderer::forceRasterRender() const
{
  // unless we are using the extent only renderer, point cloud layers should always be rasterized -- we don't want to export points as vectors
  // to formats like PDF!
  return mRenderer ? mRenderer->type() != QLatin1String( "extent" ) : false;
}

void QgsPointCloudLayerRenderer::setLayerRenderingTimeHint( int time )
{
  mRenderTimeHint = time;
}

QVector<IndexedPointCloudNode> QgsPointCloudLayerRenderer::traverseTree( const QgsPointCloudIndex *pc,
    const QgsRenderContext &context,
    IndexedPointCloudNode n,
    double maxErrorPixels,
    double nodeErrorPixels )
{
  QVector<IndexedPointCloudNode> nodes;

  if ( context.renderingStopped() )
  {
    QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
    return nodes;
  }

  if ( !context.extent().intersects( pc->nodeMapExtent( n ) ) )
    return nodes;

  const QgsDoubleRange nodeZRange = pc->nodeZRange( n );
  const QgsDoubleRange adjustedNodeZRange = QgsDoubleRange( nodeZRange.lower() + mZOffset, nodeZRange.upper() + mZOffset );
  if ( !context.zRange().isInfinite() && !context.zRange().overlaps( adjustedNodeZRange ) )
    return nodes;

  nodes.append( n );

  double childrenErrorPixels = nodeErrorPixels / 2.0;
  if ( childrenErrorPixels < maxErrorPixels )
    return nodes;

  const QList<IndexedPointCloudNode> children = pc->nodeChildren( n );
  for ( const IndexedPointCloudNode &nn : children )
  {
    nodes += traverseTree( pc, context, nn, maxErrorPixels, childrenErrorPixels );
  }

  return nodes;
}

QgsPointCloudLayerRenderer::~QgsPointCloudLayerRenderer() = default;
