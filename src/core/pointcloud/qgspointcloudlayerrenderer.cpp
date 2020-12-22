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

QgsPointCloudLayerRenderer::QgsPointCloudLayerRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayer( layer )
  , mLayerAttributes( layer->attributes() )
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

  if ( const QgsPointCloudLayerElevationProperties *elevationProps = dynamic_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() ) )
  {
    mZOffset = elevationProps->zOffset();
    mZScale = elevationProps->zScale();
  }

  mCloudExtent = mLayer->dataProvider()->polygonBounds();
}

bool QgsPointCloudLayerRenderer::render()
{
  QgsPointCloudRenderContext context( *renderContext(), mScale, mOffset, mZScale, mZOffset );

  // Set up the render configuration options
  QPainter *painter = context.renderContext().painter();

  QgsScopedQPainterState painterState( painter );
  context.renderContext().setPainterFlagsUsingContext( painter );

  if ( mRenderer->type() == QLatin1String( "extent" ) )
  {
    // special case for extent only renderer!
    mRenderer->startRender( context );
    static_cast< QgsPointCloudExtentRenderer * >( mRenderer.get() )->renderExtent( mCloudExtent, context );
    mRenderer->stopRender( context );
    return true;
  }

  // TODO cache!?
  QgsPointCloudIndex *pc = mLayer->dataProvider()->index();
  if ( !pc || !pc->isValid() )
    return false;

  mRenderer->startRender( context );

  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );

  // collect attributes required by renderer
  QSet< QString > rendererAttributes = mRenderer->usedAttributes( context );

  if ( !context.renderContext().zRange().isInfinite() )
    rendererAttributes.insert( QStringLiteral( "Z" ) );

  for ( const QString &attribute : qgis::as_const( rendererAttributes ) )
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

  const float maximumError = context.renderContext().convertToPainterUnits( mRenderer->maximumScreenError(), mRenderer->maximumScreenErrorUnit() );// in pixels

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

  const float rootErrorInMapCoordinates = rootNodeExtentMapCoords.width() / pc->span(); // in map coords

  double mapUnitsPerPixel = context.renderContext().mapToPixel().mapUnitsPerPixel();
  if ( ( rootErrorInMapCoordinates < 0.0 ) || ( mapUnitsPerPixel < 0.0 ) || ( maximumError < 0.0 ) )
  {
    QgsDebugMsg( QStringLiteral( "invalid screen error" ) );
    return false;
  }
  float rootErrorPixels = rootErrorInMapCoordinates / mapUnitsPerPixel; // in pixels
  const QList<IndexedPointCloudNode> nodes = traverseTree( pc, context.renderContext(), pc->root(), maximumError, rootErrorPixels );

  QgsPointCloudRequest request;
  request.setAttributes( mAttributes );

  // drawing
  int nodesDrawn = 0;
  for ( const IndexedPointCloudNode &n : nodes )
  {
    if ( context.renderContext().renderingStopped() )
    {
      QgsDebugMsgLevel( "canceled", 2 );
      break;
    }
    std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );

    if ( !block )
      continue;

    context.setAttributes( block->attributes() );

    mRenderer->renderBlock( block.get(), context );
    ++nodesDrawn;
  }

  QgsDebugMsgLevel( QStringLiteral( "totals: %1 nodes | %2 points | %3ms" ).arg( nodesDrawn )
                    .arg( context.pointsRendered() )
                    .arg( t.elapsed() ), 2 );

  mRenderer->stopRender( context );

  return true;
}

bool QgsPointCloudLayerRenderer::forceRasterRender() const
{
  // unless we are using the extent only renderer, point cloud layers should always be rasterized -- we don't want to export points as vectors
  // to formats like PDF!
  return mRenderer ? mRenderer->type() != QLatin1String( "extent" ) : false;
}

QList<IndexedPointCloudNode> QgsPointCloudLayerRenderer::traverseTree( const QgsPointCloudIndex *pc,
    const QgsRenderContext &context,
    IndexedPointCloudNode n,
    float maxErrorPixels,
    float nodeErrorPixels )
{
  QList<IndexedPointCloudNode> nodes;

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

  float childrenErrorPixels = nodeErrorPixels / 2.0f;
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

