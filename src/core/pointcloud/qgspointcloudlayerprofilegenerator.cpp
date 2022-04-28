/***************************************************************************
                         qgspointcloudlayerprofilegenerator.cpp
                         ---------------fun!
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspointcloudlayerprofilegenerator.h"
#include "qgsprofilerequest.h"
#include "qgscurve.h"
#include "qgspointcloudlayer.h"
#include "qgscoordinatetransform.h"
#include "qgsgeos.h"
#include "qgsterrainprovider.h"
#include "qgslinesymbol.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsprofilesnapping.h"
#include "qgsprofilepoint.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudblockrequest.h"
#include "qgsmarkersymbol.h"

//
// QgsPointCloudLayerProfileGenerator
//

QString QgsPointCloudLayerProfileResults::type() const
{
  return QStringLiteral( "pointcloud" );
}

QMap<double, double> QgsPointCloudLayerProfileResults::distanceToHeightMap() const
{
  // TODO -- cache?
  QMap< double, double > res;
  for ( const PointResult &point : results )
  {
    res.insert( point.distance, point.z );
  }
  return res;
}

QgsPointSequence QgsPointCloudLayerProfileResults::sampledPoints() const
{
  // TODO -- cache?
  QgsPointSequence res;
  res.reserve( results.size() );
  for ( const PointResult &point : results )
  {
    res.append( QgsPoint( point.x, point.y, point.z ) );
  }
  return res;
}

QVector<QgsGeometry> QgsPointCloudLayerProfileResults::asGeometries() const
{
  // TODO -- cache?
  QVector< QgsGeometry > res;
  res.reserve( results.size() );
  for ( const PointResult &point : results )
  {
    res.append( QgsGeometry( new QgsPoint( point.x, point.y, point.z ) ) );
  }
  return res;
}

QgsDoubleRange QgsPointCloudLayerProfileResults::zRange() const
{
  return QgsDoubleRange( minZ, maxZ );
}

void QgsPointCloudLayerProfileResults::renderResults( QgsProfileRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  if ( !painter )
    return;

  const QgsScopedQPainterState painterState( painter );

  painter->setBrush( Qt::NoBrush );
  painter->setPen( Qt::NoPen );

  switch ( pointSymbol )
  {
    case Qgis::PointCloudSymbol::Square:
      // for square point we always disable antialiasing -- it's not critical here and we benefit from the performance boost disabling it gives
      context.renderContext().painter()->setRenderHint( QPainter::Antialiasing, false );
      break;

    case Qgis::PointCloudSymbol::Circle:
      break;
  }

  const double minDistance = context.distanceRange().lower();
  const double maxDistance = context.distanceRange().upper();
  const double minZ = context.elevationRange().lower();
  const double maxZ = context.elevationRange().upper();

  const QRectF visibleRegion( minDistance, minZ, maxDistance - minDistance, maxZ - minZ );
  QPainterPath clipPath;
  clipPath.addPolygon( context.worldTransform().map( visibleRegion ) );
  painter->setClipPath( clipPath, Qt::ClipOperation::IntersectClip );

  const double penWidth = context.renderContext().convertToPainterUnits( pointSize, pointSizeUnit );

  for ( const PointResult &point : std::as_const( results ) )
  {
    QPointF p = context.worldTransform().map( QPointF( point.distance, point.z ) );
    QColor color = pointColor;
    if ( opacityByDistanceEffect )
      color.setAlphaF( color.alphaF() * ( 1.0 - std::pow( point.curveDistance / tolerance, 0.5 ) ) );

    switch ( pointSymbol )
    {
      case Qgis::PointCloudSymbol::Square:
        painter->fillRect( QRectF( p.x() - penWidth * 0.5,
                                   p.y() - penWidth * 0.5,
                                   penWidth, penWidth ), color );
        break;

      case Qgis::PointCloudSymbol::Circle:
        painter->setBrush( QBrush( color ) );
        painter->setPen( Qt::NoPen );
        painter->drawEllipse( QRectF( p.x() - penWidth * 0.5,
                                      p.y() - penWidth * 0.5,
                                      penWidth, penWidth ) );
        break;
    }
  }
}

QgsProfileSnapResult QgsPointCloudLayerProfileResults::snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context )
{
  QgsProfileSnapResult result;
  Q_UNUSED( point )
  Q_UNUSED( context )
#if 0
  // TODO -- index
  double prevDistance = std::numeric_limits< double >::max();
  double prevElevation = 0;
  for ( const PointResult &point : results )
  {
    // find segment which corresponds to the given distance along curve
    if ( it != results.constBegin() && prevDistance <= point.distance() && it.key() >= point.distance() )
    {
      const double dx = it.key() - prevDistance;
      const double dy = it.value() - prevElevation;
      const double snappedZ = ( dy / dx ) * ( point.distance() - prevDistance ) + prevElevation;

      if ( std::fabs( point.elevation() - snappedZ ) > context.maximumElevationDelta )
        return QgsProfileSnapResult();

      result.snappedPoint = QgsProfilePoint( point.distance(), snappedZ );
      break;
    }

    prevDistance = it.key();
    prevElevation = it.value();
  }
#endif
  return result;
}

void QgsPointCloudLayerProfileResults::copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator )
{
  const QgsPointCloudLayerProfileGenerator *pcGenerator = qgis::down_cast< const QgsPointCloudLayerProfileGenerator *>( generator );
  tolerance = pcGenerator->mTolerance;
  pointSize = pcGenerator->mPointSize;
  pointSizeUnit = pcGenerator->mPointSizeUnit;
  pointSymbol = pcGenerator->mPointSymbol;
  pointColor = pcGenerator->mPointColor;
  opacityByDistanceEffect = pcGenerator->mOpacityByDistanceEffect;
}

//
// QgsPointCloudLayerProfileGenerator
//

QgsPointCloudLayerProfileGenerator::QgsPointCloudLayerProfileGenerator( QgsPointCloudLayer *layer, const QgsProfileRequest &request )
  : mLayer( layer )
  , mRenderer( mLayer->renderer() ? mLayer->renderer()->clone() : nullptr )
  , mMaximumScreenError( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->maximumScreenError() )
  , mMaximumScreenErrorUnit( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->maximumScreenErrorUnit() )
  , mPointSize( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->pointSize() )
  , mPointSizeUnit( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->pointSizeUnit() )
  , mPointSymbol( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->pointSymbol() )
  , mPointColor( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->pointColor() )
  , mOpacityByDistanceEffect( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->applyOpacityByDistanceEffect() )
  , mId( layer->id() )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mTolerance( request.tolerance() )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mZOffset( layer->elevationProperties()->zOffset() )
  , mZScale( layer->elevationProperties()->zScale() )
  , mStepDistance( request.stepDistance() )
{
  if ( mLayer->dataProvider()->index() )
  {
    mScale = mLayer->dataProvider()->index()->scale();
    mOffset = mLayer->dataProvider()->index()->offset();
  }
}

QString QgsPointCloudLayerProfileGenerator::sourceId() const
{
  return mId;
}

Qgis::ProfileGeneratorFlags QgsPointCloudLayerProfileGenerator::flags() const
{
  return Qgis::ProfileGeneratorFlag::RespectsDistanceRange | Qgis::ProfileGeneratorFlag::RespectsMaximumErrorMapUnit;
}

QgsPointCloudLayerProfileGenerator::~QgsPointCloudLayerProfileGenerator() = default;

bool QgsPointCloudLayerProfileGenerator::generateProfile( const QgsProfileGenerationContext &context )
{
  if ( !mLayer || !mProfileCurve || mFeedback->isCanceled() )
    return false;

  // this is not AT ALL thread safe, but it's what QgsPointCloudLayerRenderer does !
  // TODO: fix when QgsPointCloudLayerRenderer is made thread safe to use same approach

  QgsPointCloudIndex *pc = mLayer->dataProvider()->index();
  if ( !pc || !pc->isValid() )
  {
    return false;
  }

  const double startDistanceOffset = std::max( !context.distanceRange().isInfinite() ? context.distanceRange().lower() : 0, 0.0 );
  const double endDistance = context.distanceRange().upper();

  std::unique_ptr< QgsCurve > trimmedCurve;
  QgsCurve *sourceCurve = nullptr;
  if ( startDistanceOffset > 0 || endDistance < mProfileCurve->length() )
  {
    trimmedCurve.reset( mProfileCurve->curveSubstring( startDistanceOffset, endDistance ) );
    sourceCurve = trimmedCurve.get();
  }
  else
  {
    sourceCurve = mProfileCurve.get();
  }

  const QgsRectangle maxSearchExtentInCurveCrs = sourceCurve->boundingBox().buffered( mTolerance );

  // we need to transform the profile curve and max search extent to the layer's CRS
  QgsGeos originalCurveGeos( sourceCurve );
  originalCurveGeos.prepareGeometry();
  mSearchGeometryInLayerCrs.reset( originalCurveGeos.buffer( mTolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 ) );
  mLayerToTargetTransform = QgsCoordinateTransform( mSourceCrs, mTargetCrs, mTransformContext );

  QgsRectangle maxSearchExtentInLayerCrs;
  try
  {
    mSearchGeometryInLayerCrs->transform( mLayerToTargetTransform, Qgis::TransformDirection::Reverse );
    QgsCoordinateTransform layerToTargetBallparkTransform( mLayerToTargetTransform );
    layerToTargetBallparkTransform.setBallparkTransformsAreAppropriate( true );
    maxSearchExtentInLayerCrs = layerToTargetBallparkTransform.transformBoundingBox( maxSearchExtentInCurveCrs, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming profile line to layer CRS" ) );
    return false;
  }

  if ( mFeedback->isCanceled() )
    return false;

  mSearchGeometryInLayerCrsGeometryEngine = std::make_unique< QgsGeos >( mSearchGeometryInLayerCrs.get() );
  mSearchGeometryInLayerCrsGeometryEngine->prepareGeometry();
  mMaxSearchExtentInLayerCrs = mSearchGeometryInLayerCrs->boundingBox();

  const IndexedPointCloudNode root = pc->root();

  const double maximumErrorPixels = context.convertDistanceToPixels( mMaximumScreenError, mMaximumScreenErrorUnit );

  const QgsRectangle rootNodeExtentLayerCoords = pc->nodeMapExtent( root );
  QgsRectangle rootNodeExtentInCurveCrs;
  try
  {
    QgsCoordinateTransform extentTransform = mLayerToTargetTransform;
    extentTransform.setBallparkTransformsAreAppropriate( true );
    rootNodeExtentInCurveCrs = extentTransform.transformBoundingBox( rootNodeExtentLayerCoords );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Could not transform node extent to curve CRS" ) );
    rootNodeExtentInCurveCrs = rootNodeExtentLayerCoords;
  }

  const double rootErrorInMapCoordinates = rootNodeExtentInCurveCrs.width() / pc->span(); // in curve coords

  const double mapUnitsPerPixel = context.mapUnitsPerDistancePixel();
  if ( ( rootErrorInMapCoordinates < 0.0 ) || ( mapUnitsPerPixel < 0.0 ) || ( maximumErrorPixels < 0.0 ) )
  {
    QgsDebugMsg( QStringLiteral( "invalid screen error" ) );
    return false;
  }
  double rootErrorPixels = rootErrorInMapCoordinates / mapUnitsPerPixel; // in pixels
  const QVector<IndexedPointCloudNode> nodes = traverseTree( pc, pc->root(), maximumErrorPixels, rootErrorPixels, context.elevationRange() );

  mResults = std::make_unique< QgsPointCloudLayerProfileResults >();
  mResults->copyPropertiesFromGenerator( this );

  QgsPointCloudRequest request;
  QgsPointCloudAttributeCollection attributes;
  // TODO -- add renderer attributes
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );
  request.setAttributes( attributes );

  switch ( pc->accessType() )
  {
    case QgsPointCloudIndex::AccessType::Local:
    {
      visitNodesSync( nodes, pc, request );
      break;
    }
    case QgsPointCloudIndex::AccessType::Remote:
    {
      visitNodesAsync( nodes, pc, request );
      break;
    }
  }

  if ( mFeedback->isCanceled() )
    return false;

  // convert x/y values back to distance/height values

  QString lastError;
  QgsPointCloudLayerProfileResults::PointResult *pointData = mResults->results.data();
  const int size = mResults->results.size();
  for ( int i = 0; i < size; ++i, ++pointData )
  {
    if ( mFeedback->isCanceled() )
      return false;

    pointData->distance = startDistanceOffset + originalCurveGeos.lineLocatePoint( pointData->x, pointData->y, &lastError );
    pointData->curveDistance = originalCurveGeos.distance( pointData->x, pointData->y );

    mResults->minZ = std::min( pointData->z, mResults->minZ );
    mResults->maxZ = std::max( pointData->z, mResults->maxZ );
  }

  return true;
}

QgsAbstractProfileResults *QgsPointCloudLayerProfileGenerator::takeResults()
{
  return mResults.release();
}

QgsFeedback *QgsPointCloudLayerProfileGenerator::feedback() const
{
  return mFeedback.get();
}

QVector<IndexedPointCloudNode> QgsPointCloudLayerProfileGenerator::traverseTree( const QgsPointCloudIndex *pc, IndexedPointCloudNode n, double maxErrorPixels, double nodeErrorPixels, const QgsDoubleRange &zRange )
{
  QVector<IndexedPointCloudNode> nodes;

  if ( mFeedback->isCanceled() )
  {
    return nodes;
  }

  const QgsDoubleRange nodeZRange = pc->nodeZRange( n );
  const QgsDoubleRange adjustedNodeZRange = QgsDoubleRange( nodeZRange.lower() * mZScale + mZOffset, nodeZRange.upper() * mZScale + mZOffset );
  if ( !zRange.isInfinite() && !zRange.overlaps( adjustedNodeZRange ) )
    return nodes;

  const QgsRectangle nodeMapExtent = pc->nodeMapExtent( n );
  if ( !mMaxSearchExtentInLayerCrs.intersects( nodeMapExtent ) )
    return nodes;

  const QgsGeometry nodeMapGeometry = QgsGeometry::fromRect( nodeMapExtent );
  if ( !mSearchGeometryInLayerCrsGeometryEngine->intersects( nodeMapGeometry.constGet() ) )
    return nodes;

  nodes.append( n );

  double childrenErrorPixels = nodeErrorPixels / 2.0;
  if ( childrenErrorPixels < maxErrorPixels )
    return nodes;

  const QList<IndexedPointCloudNode> children = pc->nodeChildren( n );
  for ( const IndexedPointCloudNode &nn : children )
  {
    nodes += traverseTree( pc, nn, maxErrorPixels, childrenErrorPixels, zRange );
  }

  return nodes;
}

int QgsPointCloudLayerProfileGenerator::visitNodesSync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRequest &request )
{
  int nodesDrawn = 0;
  for ( const IndexedPointCloudNode &n : nodes )
  {
    if ( mFeedback->isCanceled() )
      break;

    std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );

    if ( !block )
      continue;

    visitBlock( block.get() );

    ++nodesDrawn;
  }
  return nodesDrawn;
}

int QgsPointCloudLayerProfileGenerator::visitNodesAsync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRequest &request )
{
  int nodesDrawn = 0;

  // see notes about this logic in QgsPointCloudLayerRenderer::renderNodesAsync

  const int groupSize = 4;
  for ( int groupIndex = 0; groupIndex < nodes.size(); groupIndex += groupSize )
  {
    if ( mFeedback->isCanceled() )
      break;
    // Async loading of nodes
    const int currentGroupSize = std::min< size_t >( std::max< size_t >( nodes.size() - groupIndex, 0 ), groupSize );
    QVector<QgsPointCloudBlockRequest *> blockRequests( currentGroupSize, nullptr );
    QVector<bool> finishedLoadingBlock( currentGroupSize, false );
    QEventLoop loop;

    QObject::connect( mFeedback.get(), &QgsFeedback::canceled, &loop, &QEventLoop::quit );
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

    if ( !mFeedback->isCanceled() )
    {
      // Render all the point cloud blocks sequentially
      for ( int i = 0; i < blockRequests.size(); ++i )
      {
        if ( mFeedback->isCanceled() )
        {
          break;
        }

        if ( !blockRequests[ i ]->block() )
          continue;

        visitBlock( blockRequests[ i ]->block() );

        ++nodesDrawn;
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

void QgsPointCloudLayerProfileGenerator::visitBlock( const QgsPointCloudBlock *block )
{
  const char *ptr = block->data();
  int count = block->pointCount();

  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();

  const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
  int xOffset = 0, yOffset = 0, zOffset = 0;
  const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
  const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
  const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();

  const bool reproject = !mLayerToTargetTransform.isShortCircuited();
  for ( int i = 0; i < count; ++i )
  {
    if ( mFeedback->isCanceled() )
    {
      break;
    }

    QgsPointCloudLayerProfileResults::PointResult res;
    QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, block->scale(), block->offset(), res.x, res.y, res.z );

    res.z = res.z * mZScale + mZOffset;

    if ( mSearchGeometryInLayerCrsGeometryEngine->contains( res.x, res.y ) )
    {
      if ( reproject )
      {
        try
        {
          mLayerToTargetTransform.transformInPlace( res.x, res.y, res.z );
        }
        catch ( QgsCsException & )
        {
          continue;
        }
      }

      res.color = mPointColor.rgba();

      // TODO we may hit the limit of QVector size?
      mResults->results.append( res );
    }
  }
}


