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

  if ( !context.renderContext().zRange().isInfinite() ||
       mRenderer->drawOrder2d() == Qgis::PointCloudDrawOrder::BottomToTop ||
       mRenderer->drawOrder2d() == Qgis::PointCloudDrawOrder::TopToBottom )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  // collect attributes required by renderer
  QSet< QString > rendererAttributes = mRenderer->usedAttributes( context );


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
  if ( !context.renderContext().coordinateTransform().isShortCircuited() )
  {
    try
    {
      QgsCoordinateTransform extentTransform = context.renderContext().coordinateTransform();
      extentTransform.setBallparkTransformsAreAppropriate( true );
      rootNodeExtentMapCoords = extentTransform.transformBoundingBox( rootNodeExtentLayerCoords );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform node extent to map CRS" ) );
      rootNodeExtentMapCoords = rootNodeExtentLayerCoords;
    }
  }
  else
  {
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

  switch ( mRenderer->drawOrder2d() )
  {
    case Qgis::PointCloudDrawOrder::BottomToTop:
    case Qgis::PointCloudDrawOrder::TopToBottom:
    {
      nodesDrawn += renderNodesSorted( nodes, pc, context, request, canceled, mRenderer->drawOrder2d() );
      break;
    }
    case Qgis::PointCloudDrawOrder::Default:
    {
      switch ( pc->accessType() )
      {
        case QgsPointCloudIndex::AccessType::Local:
        {
          nodesDrawn += renderNodesSync( nodes, pc, context, request, canceled );
          break;
        }
        case QgsPointCloudIndex::AccessType::Remote:
        {
          nodesDrawn += renderNodesAsync( nodes, pc, context, request, canceled );
          break;
        }
      }
    }
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QStringLiteral( "totals: %1 nodes | %2 points | %3ms" ).arg( nodesDrawn )
                    .arg( context.pointsRendered() )
                    .arg( t.elapsed() ), 2 );
#else
  ( void )nodesDrawn;
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
      QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
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

  if ( context.feedback() && context.feedback()->isCanceled() )
    return 0;

  // Async loading of nodes
  QVector<QgsPointCloudBlockRequest *> blockRequests;
  QEventLoop loop;
  if ( context.feedback() )
    QObject::connect( context.feedback(), &QgsFeedback::canceled, &loop, &QEventLoop::quit );

  for ( int i = 0; i < nodes.size(); ++i )
  {
    const IndexedPointCloudNode &n = nodes[i];
    const QString nStr = n.toString();
    QgsPointCloudBlockRequest *blockRequest = pc->asyncNodeData( n, request );
    blockRequests.append( blockRequest );
    QObject::connect( blockRequest, &QgsPointCloudBlockRequest::finished, &loop,
                      [ this, &canceled, &nodesDrawn, &loop, &blockRequests, &context, nStr, blockRequest ]()
    {
      blockRequests.removeOne( blockRequest );

      // If all blocks are loaded, exit the event loop
      if ( blockRequests.isEmpty() )
        loop.exit();

      std::unique_ptr<QgsPointCloudBlock> block( blockRequest->block() );

      blockRequest->deleteLater();

      if ( context.feedback() && context.feedback()->isCanceled() )
      {
        canceled = true;
        return;
      }

      if ( !block )
      {
        QgsDebugMsg( QStringLiteral( "Unable to load node %1, error: %2" ).arg( nStr, blockRequest->errorStr() ) );
        return;
      }

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

    } );
  }

  // Wait for all point cloud nodes to finish loading
  loop.exec();

  // Rendering may have got canceled and the event loop exited before finished()
  // was called for all blocks, so let's clean up anything that is left
  for ( QgsPointCloudBlockRequest *blockRequest : std::as_const( blockRequests ) )
  {
    delete blockRequest->block();
    blockRequest->deleteLater();
  }

  return nodesDrawn;
}

int QgsPointCloudLayerRenderer::renderNodesSorted( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled, Qgis::PointCloudDrawOrder order )
{
  int blockCount = 0;
  int pointCount = 0;

  QgsVector3D blockScale;
  QgsVector3D blockOffset;
  QgsPointCloudAttributeCollection blockAttributes;
  int recordSize = 0;

  // We'll collect byte array data from all blocks
  QByteArray allByteArrays;
  // And pairs of byte array start positions paired with their Z values for sorting
  QVector<QPair<int, double>> allPairs;

  for ( const IndexedPointCloudNode &n : nodes )
  {
    if ( context.renderContext().renderingStopped() )
    {
      QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
      canceled = true;
      break;
    }
    std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );

    if ( !block )
      continue;

    // Individual nodes may have different offset values than the root node
    // we'll calculate the differences and translate x,y,z values to use the root node's offset
    QgsVector3D offsetDifference = QgsVector3D( 0, 0, 0 );
    if ( blockCount == 0 )
    {
      blockScale = block->scale();
      blockOffset = block->offset();
      blockAttributes = block->attributes();
    }
    else
    {
      offsetDifference = blockOffset - block->offset();
    }

    const char *ptr = block->data();

    context.setScale( block->scale() );
    context.setOffset( block->offset() );
    context.setAttributes( block->attributes() );

    recordSize = context.pointRecordSize();

    for ( int i = 0; i < block->pointCount(); ++i )
    {
      allByteArrays.append( ptr + i * recordSize, recordSize );

      // Calculate the translated values only for axes that have a different offset
      if ( offsetDifference.x() != 0 )
      {
        qint32 ix = *reinterpret_cast< const qint32 * >( ptr + i * recordSize + context.xOffset() );
        ix -= std::lround( offsetDifference.x() / context.scale().x() );
        const char *xPtr = reinterpret_cast< const char * >( &ix );
        allByteArrays.replace( pointCount * recordSize + context.xOffset(), 4, QByteArray( xPtr, 4 ) );
      }
      if ( offsetDifference.y() != 0 )
      {
        qint32 iy = *reinterpret_cast< const qint32 * >( ptr + i * recordSize + context.yOffset() );
        iy -= std::lround( offsetDifference.y() / context.scale().y() );
        const char *yPtr = reinterpret_cast< const char * >( &iy );
        allByteArrays.replace( pointCount * recordSize + context.yOffset(), 4, QByteArray( yPtr, 4 ) );
      }
      // We need the Z value regardless of the node's offset
      qint32 iz = *reinterpret_cast< const qint32 * >( ptr + i * recordSize + context.zOffset() );
      if ( offsetDifference.z() != 0 )
      {
        iz -= std::lround( offsetDifference.z() / context.scale().z() );
        const char *zPtr = reinterpret_cast< const char * >( &iz );
        allByteArrays.replace( pointCount * recordSize + context.zOffset(), 4, QByteArray( zPtr, 4 ) );
      }
      allPairs.append( qMakePair( pointCount, double( iz ) + block->offset().z() ) );

      ++pointCount;
    }
    ++blockCount;
  }

  if ( pointCount == 0 )
    return 0;

  switch ( order )
  {
    case Qgis::PointCloudDrawOrder::BottomToTop:
      std::sort( allPairs.begin(), allPairs.end(), []( QPair<int, double> a, QPair<int, double> b ) { return a.second < b.second; } );
      break;
    case Qgis::PointCloudDrawOrder::TopToBottom:
      std::sort( allPairs.begin(), allPairs.end(), []( QPair<int, double> a, QPair<int, double> b ) { return a.second > b.second; } );
      break;
    case Qgis::PointCloudDrawOrder::Default:
      break;
  }

  // Now we can reconstruct a byte array sorted by Z value
  QByteArray sortedByteArray;
  sortedByteArray.reserve( allPairs.size() );
  for ( QPair<int, double> pair : allPairs )
    sortedByteArray.append( allByteArrays.mid( pair.first * recordSize, recordSize ) );

  std::unique_ptr<QgsPointCloudBlock> bigBlock { new QgsPointCloudBlock( pointCount,
        blockAttributes,
        sortedByteArray,
        blockScale,
        blockOffset ) };

  QgsVector3D contextScale = context.scale();
  QgsVector3D contextOffset = context.offset();

  context.setScale( bigBlock->scale() );
  context.setOffset( bigBlock->offset() );
  context.setAttributes( bigBlock->attributes() );

  mRenderer->renderBlock( bigBlock.get(), context );

  context.setScale( contextScale );
  context.setOffset( contextOffset );

  return blockCount;
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
