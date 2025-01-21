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

#include "qgsapplication.h"
#include "qgscolorramp.h"
#include "qgselevationmap.h"
#include "qgslogger.h"
#include "qgsmapclippingutils.h"
#include "qgsmeshlayerutils.h"
#include "qgsmessagelog.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudblockrequest.h"
#include "qgspointcloudextentrenderer.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrequest.h"
#include "qgsrendercontext.h"
#include "qgsruntimeprofiler.h"
#include "qgsvirtualpointcloudprovider.h"

#include <delaunator.hpp>


QgsPointCloudLayerRenderer::QgsPointCloudLayerRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayer( layer )
  , mLayerName( layer->name() )
  , mLayerAttributes( layer->attributes() )
  , mSubIndexes( layer && layer->dataProvider() ? layer->dataProvider()->subIndexes() : QVector<QgsPointCloudSubIndex>() )
  , mFeedback( new QgsFeedback )
  , mEnableProfile( context.flags() & Qgis::RenderContextFlag::RecordProfile )
{
  // TODO: we must not keep pointer to mLayer (it's dangerous) - we must copy anything we need for rendering
  // or use some locking to prevent read/write from multiple threads
  if ( !mLayer || !mLayer->dataProvider() || !mLayer->renderer() )
    return;

  QElapsedTimer timer;
  timer.start();

  mRenderer.reset( mLayer->renderer()->clone() );
  if ( !mSubIndexes.isEmpty() )
  {
    mSubIndexExtentRenderer.reset( new QgsPointCloudExtentRenderer() );
    mSubIndexExtentRenderer->setShowLabels( mRenderer->showLabels() );
    mSubIndexExtentRenderer->setLabelTextFormat( mRenderer->labelTextFormat() );
  }

  if ( mLayer->index() )
  {
    mScale = mLayer->index().scale();
    mOffset = mLayer->index().offset();
  }

  if ( const QgsPointCloudLayerElevationProperties *elevationProps = qobject_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() ) )
  {
    mZOffset = elevationProps->zOffset();
    mZScale = elevationProps->zScale();
  }

  mCloudExtent = mLayer->dataProvider()->polygonBounds();

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

  mReadyToCompose = false;

  mPreparationTime = timer.elapsed();
}

bool QgsPointCloudLayerRenderer::render()
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( mEnableProfile )
  {
    profile = std::make_unique< QgsScopedRuntimeProfile >( mLayerName, QStringLiteral( "rendering" ), layerId() );
    if ( mPreparationTime > 0 )
      QgsApplication::profiler()->record( QObject::tr( "Create renderer" ), mPreparationTime / 1000.0, QStringLiteral( "rendering" ) );
  }

  std::unique_ptr< QgsScopedRuntimeProfile > preparingProfile;
  if ( mEnableProfile )
  {
    preparingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Preparing render" ), QStringLiteral( "rendering" ) );
  }

  QgsPointCloudRenderContext context( *renderContext(), mScale, mOffset, mZScale, mZOffset, mFeedback.get() );

  // Set up the render configuration options
  QPainter *painter = context.renderContext().painter();

  QgsScopedQPainterState painterState( painter );
  context.renderContext().setPainterFlagsUsingContext( painter );

  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), Qgis::LayerType::VectorTile, needsPainterClipPath );
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
  QgsPointCloudIndex pc = mLayer->index();
  if ( mSubIndexes.isEmpty() && ( !pc || !pc.isValid() ) )
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
       mRenderer->drawOrder2d() == Qgis::PointCloudDrawOrder::TopToBottom ||
       renderContext()->elevationMap() )
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

  QgsRectangle renderExtent;
  try
  {
    renderExtent = renderContext()->coordinateTransform().transformBoundingBox( renderContext()->mapExtent(), Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Transformation of extent failed!" ) );
  }

  preparingProfile.reset();
  std::unique_ptr< QgsScopedRuntimeProfile > renderingProfile;
  if ( mEnableProfile )
  {
    renderingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering" ), QStringLiteral( "rendering" ) );
  }

  bool canceled = false;
  if ( mSubIndexes.isEmpty() )
  {
    canceled = !renderIndex( pc );
  }
  else if ( const QgsVirtualPointCloudProvider *vpcProvider = dynamic_cast<QgsVirtualPointCloudProvider *>( mLayer->dataProvider() ) )
  {
    QVector< QgsPointCloudSubIndex > visibleIndexes;
    for ( const QgsPointCloudSubIndex &si : mSubIndexes )
    {
      if ( renderExtent.intersects( si.extent() ) )
      {
        visibleIndexes.append( si );
      }
    }
    const bool zoomedOut = renderExtent.width() > vpcProvider->averageSubIndexWidth() ||
                           renderExtent.height() > vpcProvider->averageSubIndexHeight();
    QgsPointCloudIndex overviewIndex = vpcProvider->overview();
    // if the overview of virtual point cloud exists, and we are zoomed out, we render just overview
    if ( vpcProvider->overview() && zoomedOut &&
         mRenderer->zoomOutBehavior() == Qgis::PointCloudZoomOutRenderBehavior::RenderOverview )
    {
      renderIndex( overviewIndex );
    }
    else
    {
      // if the overview of virtual point cloud exists, and we are zoomed out, but we want both overview and extents,
      // we render overview
      if ( vpcProvider->overview() && zoomedOut &&
           mRenderer->zoomOutBehavior() == Qgis::PointCloudZoomOutRenderBehavior::RenderOverviewAndExtents )
      {
        renderIndex( overviewIndex );
      }
      mSubIndexExtentRenderer->startRender( context );
      for ( const QgsPointCloudSubIndex &si : visibleIndexes )
      {
        if ( canceled )
          break;

        QgsPointCloudIndex pc = si.index();
        // if the index of point cloud is invalid, or we are zoomed out and want extents, we render the point cloud extent
        if ( !pc || !pc.isValid() || ( ( mRenderer->zoomOutBehavior() == Qgis::PointCloudZoomOutRenderBehavior::RenderExtents || mRenderer->zoomOutBehavior() == Qgis::PointCloudZoomOutRenderBehavior::RenderOverviewAndExtents ) &&
                                       zoomedOut ) )
        {
          mSubIndexExtentRenderer->renderExtent( si.polygonBounds(), context );
          if ( mSubIndexExtentRenderer->showLabels() )
          {
            mSubIndexExtentRenderer->renderLabel(
              context.renderContext().mapToPixel().transformBounds( si.extent().toRectF() ),
              si.uri().section( "/", -1 ).section( ".", 0, 0 ),
              context );
          }
        }
        // else we just render the visible point cloud
        else
        {
          canceled = !renderIndex( pc );
        }
      }
      mSubIndexExtentRenderer->stopRender( context );
    }
  }

  mRenderer->stopRender( context );
  mReadyToCompose = true;
  return !canceled;
}

bool QgsPointCloudLayerRenderer::renderIndex( QgsPointCloudIndex &pc )
{
  QgsPointCloudRenderContext context( *renderContext(),
                                      pc.scale(),
                                      pc.offset(),
                                      mZScale,
                                      mZOffset,
                                      mFeedback.get() );


#ifdef QGISDEBUG
  QElapsedTimer t;
  t.start();
#endif

  const QgsPointCloudNodeId root = pc.root();

  const double maximumError = context.renderContext().convertToPainterUnits( mRenderer->maximumScreenError(), mRenderer->maximumScreenErrorUnit() );// in pixels

  const QgsPointCloudNode rootNode = pc.getNode( root );
  const QgsRectangle rootNodeExtentLayerCoords = pc.extent();
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
      QgsDebugError( QStringLiteral( "Could not transform node extent to map CRS" ) );
      rootNodeExtentMapCoords = rootNodeExtentLayerCoords;
    }
  }
  else
  {
    rootNodeExtentMapCoords = rootNodeExtentLayerCoords;
  }

  const double rootErrorInMapCoordinates = rootNodeExtentMapCoords.width() / pc.span(); // in map coords

  double mapUnitsPerPixel = context.renderContext().mapToPixel().mapUnitsPerPixel();
  if ( ( rootErrorInMapCoordinates < 0.0 ) || ( mapUnitsPerPixel < 0.0 ) || ( maximumError < 0.0 ) )
  {
    QgsDebugError( QStringLiteral( "invalid screen error" ) );
    return false;
  }
  double rootErrorPixels = rootErrorInMapCoordinates / mapUnitsPerPixel; // in pixels
  const QVector<QgsPointCloudNodeId> nodes = traverseTree( pc, context.renderContext(), pc.root(), maximumError, rootErrorPixels );

  QgsPointCloudRequest request;
  request.setAttributes( mAttributes );

  // drawing
  int nodesDrawn = 0;
  bool canceled = false;

  Qgis::PointCloudDrawOrder drawOrder = mRenderer->drawOrder2d();
  if ( mRenderer->renderAsTriangles() )
  {
    // Ordered rendering is ignored when drawing as surface, because all points are used for triangulation.
    // We would need to have a way to detect if a point is occluded by some other points, which may be costly.
    drawOrder = Qgis::PointCloudDrawOrder::Default;
  }

  switch ( drawOrder )
  {
    case Qgis::PointCloudDrawOrder::BottomToTop:
    case Qgis::PointCloudDrawOrder::TopToBottom:
    {
      nodesDrawn += renderNodesSorted( nodes, pc, context, request, canceled, mRenderer->drawOrder2d() );
      break;
    }
    case Qgis::PointCloudDrawOrder::Default:
    {
      switch ( pc.accessType() )
      {
        case Qgis::PointCloudAccessType::Local:
        {
          nodesDrawn += renderNodesSync( nodes, pc, context, request, canceled );
          break;
        }
        case Qgis::PointCloudAccessType::Remote:
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

  return !canceled;
}

int QgsPointCloudLayerRenderer::renderNodesSync( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled )
{
  QPainter *finalPainter = context.renderContext().painter();
  if ( mRenderer->renderAsTriangles() && context.renderContext().previewRenderPainter() )
  {
    // swap out the destination painter for the preview render painter to render points
    // until the actual triangles are ready to be rendered
    context.renderContext().setPainter( context.renderContext().previewRenderPainter() );
  }

  int nodesDrawn = 0;
  for ( const QgsPointCloudNodeId &n : nodes )
  {
    if ( context.renderContext().renderingStopped() )
    {
      QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
      canceled = true;
      break;
    }
    std::unique_ptr<QgsPointCloudBlock> block( pc.nodeData( n, request ) );

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

  if ( mRenderer->renderAsTriangles() )
  {
    // Switch back from the preview painter to the destination painter to render the triangles
    context.renderContext().setPainter( finalPainter );
    renderTriangulatedSurface( context );
  }

  return nodesDrawn;
}

int QgsPointCloudLayerRenderer::renderNodesAsync( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled )
{
  if ( context.feedback() && context.feedback()->isCanceled() )
    return 0;

  QPainter *finalPainter = context.renderContext().painter();
  if ( mRenderer->renderAsTriangles() && context.renderContext().previewRenderPainter() )
  {
    // swap out the destination painter for the preview render painter to render points
    // until the actual triangles are ready to be rendered
    context.renderContext().setPainter( context.renderContext().previewRenderPainter() );
  }

  int nodesDrawn = 0;

  // Async loading of nodes
  QVector<QgsPointCloudBlockRequest *> blockRequests;
  QEventLoop loop;
  if ( context.feedback() )
    QObject::connect( context.feedback(), &QgsFeedback::canceled, &loop, &QEventLoop::quit );

  for ( int i = 0; i < nodes.size(); ++i )
  {
    const QgsPointCloudNodeId &n = nodes[i];
    const QString nStr = n.toString();
    QgsPointCloudBlockRequest *blockRequest = pc.asyncNodeData( n, request );
    blockRequests.append( blockRequest );
    QObject::connect( blockRequest, &QgsPointCloudBlockRequest::finished, &loop,
                      [ this, &canceled, &nodesDrawn, &loop, &blockRequests, &context, nStr, blockRequest ]()
    {
      blockRequests.removeOne( blockRequest );

      // If all blocks are loaded, exit the event loop
      if ( blockRequests.isEmpty() )
        loop.exit();

      std::unique_ptr<QgsPointCloudBlock> block( blockRequest->takeBlock() );

      blockRequest->deleteLater();

      if ( context.feedback() && context.feedback()->isCanceled() )
      {
        canceled = true;
        return;
      }

      if ( !block )
      {
        QgsDebugError( QStringLiteral( "Unable to load node %1, error: %2" ).arg( nStr, blockRequest->errorStr() ) );
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
    std::unique_ptr<QgsPointCloudBlock> block = blockRequest->takeBlock();
    block.reset();

    blockRequest->deleteLater();
  }

  if ( mRenderer->renderAsTriangles() )
  {
    // Switch back from the preview painter to the destination painter to render the triangles
    context.renderContext().setPainter( finalPainter );
    renderTriangulatedSurface( context );
  }

  return nodesDrawn;
}

int QgsPointCloudLayerRenderer::renderNodesSorted( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled, Qgis::PointCloudDrawOrder order )
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

  for ( const QgsPointCloudNodeId &n : nodes )
  {
    if ( context.renderContext().renderingStopped() )
    {
      QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
      canceled = true;
      break;
    }
    std::unique_ptr<QgsPointCloudBlock> block( pc.nodeData( n, request ) );

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

inline bool isEdgeTooLong( const QPointF &p1, const QPointF &p2, float length )
{
  QPointF p = p1 - p2;
  return p.x() * p.x() + p.y() * p.y() > length;
}

static void renderTriangle( QImage &img, QPointF *pts, QRgb c0, QRgb c1, QRgb c2, float horizontalFilter, float *elev, QgsElevationMap *elevationMap )
{
  if ( horizontalFilter > 0 )
  {
    float filterThreshold2 = horizontalFilter * horizontalFilter;
    if ( isEdgeTooLong( pts[0], pts[1], filterThreshold2 ) ||
         isEdgeTooLong( pts[1], pts[2], filterThreshold2 ) ||
         isEdgeTooLong( pts[2], pts[0], filterThreshold2 ) )
      return;
  }

  QgsRectangle screenBBox = QgsMeshLayerUtils::triangleBoundingBox( pts[0], pts[1], pts[2] );

  QSize outputSize = img.size();

  int topLim = std::max( int( screenBBox.yMinimum() ), 0 );
  int bottomLim = std::min( int( screenBBox.yMaximum() ), outputSize.height() - 1 );
  int leftLim = std::max( int( screenBBox.xMinimum() ), 0 );
  int rightLim = std::min( int( screenBBox.xMaximum() ), outputSize.width() - 1 );

  int red0 = qRed( c0 ), green0 = qGreen( c0 ), blue0 = qBlue( c0 );
  int red1 = qRed( c1 ), green1 = qGreen( c1 ), blue1 = qBlue( c1 );
  int red2 = qRed( c2 ), green2 = qGreen( c2 ), blue2 = qBlue( c2 );

  QRgb *elevData = elevationMap ? elevationMap->rawElevationImageData() : nullptr;

  for ( int j = topLim; j <= bottomLim; j++ )
  {
    QRgb *scanLine = ( QRgb * ) img.scanLine( j );
    QRgb *elevScanLine = elevData ? elevData + static_cast<size_t>( outputSize.width() * j ) : nullptr;
    for ( int k = leftLim; k <= rightLim; k++ )
    {
      QPointF pt( k, j );
      double lam1, lam2, lam3;
      if ( !QgsMeshLayerUtils::calculateBarycentricCoordinates( pts[0], pts[1], pts[2], pt, lam3, lam2, lam1 ) )
        continue;

      // interpolate color
      int r = static_cast<int>( red0 * lam1 + red1 * lam2 + red2 * lam3 );
      int g = static_cast<int>( green0 * lam1 + green1 * lam2 + green2 * lam3 );
      int b = static_cast<int>( blue0 * lam1 + blue1 * lam2 + blue2 * lam3 );
      scanLine[k] = qRgb( r, g, b );

      // interpolate elevation - in case we are doing global map shading
      if ( elevScanLine )
      {
        float z = static_cast<float>( elev[0] * lam1 + elev[1] * lam2 + elev[2] * lam3 );
        elevScanLine[k] = QgsElevationMap::encodeElevation( z );
      }
    }
  }
}

void QgsPointCloudLayerRenderer::renderTriangulatedSurface( QgsPointCloudRenderContext &context )
{
  const QgsPointCloudRenderContext::TriangulationData &triangulation = context.triangulationData();
  const std::vector<double> &points = triangulation.points;

  // Delaunator would crash if it gets less than three points
  if ( points.size() < 3 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Need at least 3 points to triangulate" ), 4 );
    return;
  }

  std::unique_ptr<delaunator::Delaunator> delaunator;
  try
  {
    delaunator.reset( new delaunator::Delaunator( points ) );
  }
  catch ( std::exception & )
  {
    // something went wrong, better to retrieve initial state
    QgsDebugMsgLevel( QStringLiteral( "Error with triangulation" ), 4 );
    return;
  }

  float horizontalFilter = 0;
  if ( mRenderer->horizontalTriangleFilter() )
  {
    horizontalFilter = static_cast<float>( renderContext()->convertToPainterUnits(
        mRenderer->horizontalTriangleFilterThreshold(), mRenderer->horizontalTriangleFilterUnit() ) );
  }

  QImage img( context.renderContext().deviceOutputSize(), QImage::Format_ARGB32_Premultiplied );
  img.setDevicePixelRatio( context.renderContext().devicePixelRatio() );
  img.fill( 0 );

  const std::vector<size_t> &triangleIndexes = delaunator->triangles;
  QPainter *painter = context.renderContext().painter();
  QgsElevationMap *elevationMap = context.renderContext().elevationMap();
  QPointF triangle[3];
  float elev[3] {0, 0, 0};
  for ( size_t i = 0; i < triangleIndexes.size(); i += 3 )
  {
    size_t v0 = triangleIndexes[i], v1 = triangleIndexes[i + 1], v2 = triangleIndexes[i + 2];
    triangle[0].rx() = points[v0 * 2];
    triangle[0].ry() = points[v0 * 2 + 1];
    triangle[1].rx() = points[v1 * 2];
    triangle[1].ry() = points[v1 * 2 + 1];
    triangle[2].rx() = points[v2 * 2];
    triangle[2].ry() = points[v2 * 2 + 1];

    if ( elevationMap )
    {
      elev[0] = triangulation.elevations[v0];
      elev[1] = triangulation.elevations[v1];
      elev[2] = triangulation.elevations[v2];
    }

    QRgb c0 = triangulation.colors[v0], c1 = triangulation.colors[v1], c2 = triangulation.colors[v2];
    renderTriangle( img, triangle, c0, c1, c2, horizontalFilter, elev, elevationMap );
  }

  painter->drawImage( 0, 0, img );
}

Qgis::MapLayerRendererFlags QgsPointCloudLayerRenderer::flags() const
{
  // when rendering as triangles we still want to show temporary incremental renders as points until
  // the final triangulated surface is ready, which may be slow
  // So we request here a preview render image for the temporary incremental updates:
  if ( mRenderer->renderAsTriangles() )
    return Qgis::MapLayerRendererFlag::RenderPartialOutputs | Qgis::MapLayerRendererFlag::RenderPartialOutputOverPreviousCachedImage;

  return Qgis::MapLayerRendererFlags();
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

QVector<QgsPointCloudNodeId> QgsPointCloudLayerRenderer::traverseTree( const QgsPointCloudIndex &pc, const QgsRenderContext &context, QgsPointCloudNodeId n, double maxErrorPixels, double nodeErrorPixels )
{
  QVector<QgsPointCloudNodeId> nodes;

  if ( context.renderingStopped() )
  {
    QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
    return nodes;
  }

  QgsPointCloudNode node = pc.getNode( n );
  QgsBox3D nodeExtent = node.bounds();

  if ( !context.extent().intersects( nodeExtent.toRectangle() ) )
    return nodes;

  const QgsDoubleRange nodeZRange( nodeExtent.zMinimum(), nodeExtent.zMaximum() );
  const QgsDoubleRange adjustedNodeZRange = QgsDoubleRange( nodeZRange.lower() + mZOffset, nodeZRange.upper() + mZOffset );
  if ( !context.zRange().isInfinite() && !context.zRange().overlaps( adjustedNodeZRange ) )
    return nodes;

  if ( node.pointCount() > 0 )
    nodes.append( n );

  double childrenErrorPixels = nodeErrorPixels / 2.0;
  if ( childrenErrorPixels < maxErrorPixels )
    return nodes;

  for ( const QgsPointCloudNodeId &nn : node.children() )
  {
    nodes += traverseTree( pc, context, nn, maxErrorPixels, childrenErrorPixels );
  }

  return nodes;
}

QgsPointCloudLayerRenderer::~QgsPointCloudLayerRenderer() = default;
