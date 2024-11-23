/***************************************************************************
                         qgspointcloudlayerprofilegenerator.cpp
                         ---------------
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
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsprofilesnapping.h"
#include "qgsprofilepoint.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudblockrequest.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"

//
// QgsPointCloudLayerProfileGenerator
//

QgsPointCloudLayerProfileResults::QgsPointCloudLayerProfileResults()
{
  mPointIndex = GEOSSTRtree_create_r( QgsGeosContext::get(), ( size_t )10 );
}

QgsPointCloudLayerProfileResults::~QgsPointCloudLayerProfileResults()
{
  GEOSSTRtree_destroy_r( QgsGeosContext::get(), mPointIndex );
  mPointIndex = nullptr;
}

void QgsPointCloudLayerProfileResults::finalize( QgsFeedback *feedback )
{
  GEOSContextHandle_t geosctxt = QgsGeosContext::get();

  const std::size_t size = results.size();
  PointResult *pointData = results.data();
  for ( std::size_t i = 0; i < size; ++i, ++pointData )
  {
    if ( feedback->isCanceled() )
      break;

    geos::unique_ptr geosPoint( GEOSGeom_createPointFromXY_r( geosctxt, pointData->distanceAlongCurve, pointData->z ) );

    GEOSSTRtree_insert_r( geosctxt, mPointIndex, geosPoint.get(), pointData );
    // only required for GEOS < 3.9
  }
}

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
    res.insert( point.distanceAlongCurve, point.z );
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

QVector<QgsAbstractProfileResults::Feature> QgsPointCloudLayerProfileResults::asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback ) const
{
  QVector<  QgsAbstractProfileResults::Feature > res;
  res.reserve( static_cast< int >( results.size() ) );
  switch ( type )
  {
    case Qgis::ProfileExportType::Features3D:
    {
      for ( const PointResult &point : results )
      {
        if ( feedback && feedback->isCanceled() )
          break;
        QgsAbstractProfileResults::Feature f;
        f.layerIdentifier = mLayerId;
        f.geometry = QgsGeometry( std::make_unique< QgsPoint >( point.x, point.y, point.z ) );
        res.append( f );
      }
      break;
    }

    case Qgis::ProfileExportType::Profile2D:
    {
      for ( const PointResult &point : results )
      {
        if ( feedback && feedback->isCanceled() )
          break;
        QgsAbstractProfileResults::Feature f;
        f.layerIdentifier = mLayerId;
        f.geometry = QgsGeometry( std::make_unique< QgsPoint >( point.distanceAlongCurve, point.z ) );
        res.append( f );
      }
      break;
    }

    case Qgis::ProfileExportType::DistanceVsElevationTable:
    {
      for ( const PointResult &point : results )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        QgsAbstractProfileResults::Feature f;
        f.layerIdentifier = mLayerId;
        f.attributes =
        {
          { QStringLiteral( "distance" ),  point.distanceAlongCurve },
          { QStringLiteral( "elevation" ),  point.z }
        };
        f.geometry = QgsGeometry( std::make_unique< QgsPoint >( point.x, point.y, point.z ) );
        res << f;
      }
      break;
    }
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
    QPointF p = context.worldTransform().map( QPointF( point.distanceAlongCurve, point.z ) );
    QColor color = respectLayerColors ? point.color : pointColor;
    if ( opacityByDistanceEffect )
      color.setAlphaF( color.alphaF() * ( 1.0 - std::pow( point.distanceFromCurve / tolerance, 0.5 ) ) );

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

struct _GEOSQueryCallbackData
{
  QList< const QgsPointCloudLayerProfileResults::PointResult * > *list;
};
void _GEOSQueryCallback( void *item, void *userdata )
{
  reinterpret_cast<_GEOSQueryCallbackData *>( userdata )->list->append( reinterpret_cast<const QgsPointCloudLayerProfileResults::PointResult *>( item ) );
}

QgsProfileSnapResult QgsPointCloudLayerProfileResults::snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context )
{
  QgsProfileSnapResult result;

  GEOSContextHandle_t geosctxt = QgsGeosContext::get();

  const double minDistance = point.distance() - context.maximumPointDistanceDelta;
  const double maxDistance = point.distance() + context.maximumPointDistanceDelta;
  const double minElevation = point.elevation() - context.maximumPointElevationDelta;
  const double maxElevation = point.elevation() + context.maximumPointElevationDelta;

  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 2, 2 );
  GEOSCoordSeq_setXY_r( geosctxt, coord, 0, minDistance, minElevation );
  GEOSCoordSeq_setXY_r( geosctxt, coord, 1, maxDistance, maxElevation );
  geos::unique_ptr searchDiagonal( GEOSGeom_createLineString_r( geosctxt, coord ) );

  QList<const PointResult *> items;
  struct _GEOSQueryCallbackData callbackData;
  callbackData.list = &items;
  GEOSSTRtree_query_r( geosctxt, mPointIndex, searchDiagonal.get(), _GEOSQueryCallback, &callbackData );
  if ( items.empty() )
    return result;

  double bestMatchDistance = std::numeric_limits< double >::max();
  const PointResult *bestMatch = nullptr;
  for ( const PointResult *candidate : std::as_const( items ) )
  {
    const double distance = std::sqrt( std::pow( candidate->distanceAlongCurve - point.distance(), 2 )
                                       + std::pow( ( candidate->z - point.elevation() ) / context.displayRatioElevationVsDistance, 2 ) );
    if ( distance < bestMatchDistance )
    {
      bestMatchDistance = distance;
      bestMatch = candidate;
    }
  }
  if ( !bestMatch )
    return result;

  result.snappedPoint = QgsProfilePoint( bestMatch->distanceAlongCurve, bestMatch->z );
  return result;
}

QVector<QgsProfileIdentifyResults> QgsPointCloudLayerProfileResults::identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context )
{
  return identify( QgsDoubleRange( point.distance() - context.maximumPointDistanceDelta, point.distance() + context.maximumPointDistanceDelta ),
                   QgsDoubleRange( point.elevation() - context.maximumPointElevationDelta, point.elevation() + context.maximumPointElevationDelta ), context );
}

QVector<QgsProfileIdentifyResults> QgsPointCloudLayerProfileResults::identify( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange, const QgsProfileIdentifyContext &context )
{
  if ( !mLayer )
    return {};

  std::unique_ptr< QgsCurve > substring( mProfileCurve->curveSubstring( distanceRange.lower(), distanceRange.upper() ) );
  QgsGeos substringGeos( substring.get() );
  std::unique_ptr< QgsAbstractGeometry > searchGeometry( substringGeos.buffer( mTolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 ) );
  if ( !searchGeometry )
    return {};

  const QgsCoordinateTransform curveToLayerTransform = QgsCoordinateTransform( mCurveCrs, mLayer->crs(), context.project ? context.project->transformContext() : QgsCoordinateTransformContext() );
  try
  {
    searchGeometry->transform( curveToLayerTransform );
  }
  catch ( QgsCsException & )
  {
    return {};
  }

  // we have to adjust the elevation range to "undo" the z offset/z scale before we can hand to the provider
  const QgsDoubleRange providerElevationRange( ( elevationRange.lower() - mZOffset ) / mZScale, ( elevationRange.upper() - mZOffset ) / mZScale );

  const QgsGeometry pointCloudSearchGeometry( std::move( searchGeometry ) );
  const QVector<QVariantMap> pointAttributes = mLayer->dataProvider()->identify( mMaxErrorInLayerCoordinates, pointCloudSearchGeometry, providerElevationRange );
  if ( pointAttributes.empty() )
    return {};

  return { QgsProfileIdentifyResults( mLayer, pointAttributes )};
}

void QgsPointCloudLayerProfileResults::copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator )
{
  const QgsPointCloudLayerProfileGenerator *pcGenerator = qgis::down_cast< const QgsPointCloudLayerProfileGenerator *>( generator );
  tolerance = pcGenerator->mTolerance;
  pointSize = pcGenerator->mPointSize;
  pointSizeUnit = pcGenerator->mPointSizeUnit;
  pointSymbol = pcGenerator->mPointSymbol;
  pointColor = pcGenerator->mPointColor;
  respectLayerColors = static_cast< bool >( pcGenerator->mRenderer );
  opacityByDistanceEffect = pcGenerator->mOpacityByDistanceEffect;

  mLayer = pcGenerator->mLayer;
  mLayerId = pcGenerator->mId;
  mCurveCrs = pcGenerator->mTargetCrs;
  mProfileCurve.reset( pcGenerator->mProfileCurve->clone() );
  mTolerance = pcGenerator->mTolerance;

  mZOffset = pcGenerator->mZOffset;
  mZScale = pcGenerator->mZScale;
}

//
// QgsPointCloudLayerProfileGenerator
//

QgsPointCloudLayerProfileGenerator::QgsPointCloudLayerProfileGenerator( QgsPointCloudLayer *layer, const QgsProfileRequest &request )
  : mLayer( layer )
  , mLayerAttributes( layer->attributes() )
  , mRenderer( qgis::down_cast< QgsPointCloudLayerElevationProperties* >( layer->elevationProperties() )->respectLayerColors() && mLayer->renderer() ? mLayer->renderer()->clone() : nullptr )
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
  , mSourceCrs( layer->crs3D() )
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
  mGatheredPoints.clear();
  if ( !mLayer || !mProfileCurve || mFeedback->isCanceled() )
    return false;

  // this is not AT ALL thread safe, but it's what QgsPointCloudLayerRenderer does !
  // TODO: fix when QgsPointCloudLayerRenderer is made thread safe to use same approach

  QVector<QgsPointCloudIndex *> indexes;
  QgsPointCloudIndex *mainIndex = mLayer->dataProvider()->index();
  if ( mainIndex && mainIndex->isValid() )
    indexes.append( mainIndex );

  // Gather all relevant sub-indexes
  const QgsRectangle profileCurveBbox = mProfileCurve->boundingBox();
  for ( const QgsPointCloudSubIndex &subidx : mLayer->dataProvider()->subIndexes() )
  {
    QgsPointCloudIndex *index = subidx.index();
    if ( index && index->isValid() && subidx.polygonBounds().intersects( profileCurveBbox ) )
      indexes.append( subidx.index() );
  }

  if ( indexes.empty() )
    return false;

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

  // we need to transform the profile curve and max search extent to the layer's CRS
  QgsGeos originalCurveGeos( sourceCurve );
  originalCurveGeos.prepareGeometry();
  mSearchGeometryInLayerCrs.reset( originalCurveGeos.buffer( mTolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 ) );
  mLayerToTargetTransform = QgsCoordinateTransform( mSourceCrs, mTargetCrs, mTransformContext );

  try
  {
    mSearchGeometryInLayerCrs->transform( mLayerToTargetTransform, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Error transforming profile line to layer CRS" ) );
    return false;
  }

  if ( mFeedback->isCanceled() )
    return false;

  mSearchGeometryInLayerCrsGeometryEngine = std::make_unique< QgsGeos >( mSearchGeometryInLayerCrs.get() );
  mSearchGeometryInLayerCrsGeometryEngine->prepareGeometry();
  mMaxSearchExtentInLayerCrs = mSearchGeometryInLayerCrs->boundingBox();

  double maximumErrorPixels = context.convertDistanceToPixels( mMaximumScreenError, mMaximumScreenErrorUnit );
  if ( maximumErrorPixels < 0.0 )
  {
    QgsDebugError( QStringLiteral( "Invalid maximum error in pixels" ) );
    return false;
  }

  const double toleranceInPixels = context.convertDistanceToPixels( mTolerance, Qgis::RenderUnit::MapUnits );
  // ensure that the maximum error is compatible with the tolerance size -- otherwise if the tolerance size
  // is much smaller than the maximum error, we don't dig deep enough into the point cloud nodes to find
  // points which are inside the tolerance.
  // "4" is a magic number here, based purely on what "looks good" in the profile results!
  if ( toleranceInPixels / 4 < maximumErrorPixels )
    maximumErrorPixels = toleranceInPixels / 4;

  QgsPointCloudRequest request;
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  if ( mRenderer )
  {
    mPreparedRendererData = mRenderer->prepare();
    if ( mPreparedRendererData )
    {
      const QSet< QString > rendererAttributes = mPreparedRendererData->usedAttributes();
      for ( const QString &attribute : std::as_const( rendererAttributes ) )
      {
        if ( attributes.indexOf( attribute ) >= 0 )
          continue; // don't re-add attributes we are already going to fetch

        const int layerIndex = mLayerAttributes.indexOf( attribute );
        if ( layerIndex < 0 )
        {
          QgsMessageLog::logMessage( QObject::tr( "Required attribute %1 not found in layer" ).arg( attribute ), QObject::tr( "Point Cloud" ) );
          continue;
        }

        attributes.push_back( mLayerAttributes.at( layerIndex ) );
      }
    }
  }
  else
  {
    mPreparedRendererData.reset();
  }

  request.setAttributes( attributes );

  mResults = std::make_unique< QgsPointCloudLayerProfileResults >();
  mResults->copyPropertiesFromGenerator( this );
  mResults->mMaxErrorInLayerCoordinates = 0;

  QgsCoordinateTransform extentTransform = mLayerToTargetTransform;
  extentTransform.setBallparkTransformsAreAppropriate( true );

  const double mapUnitsPerPixel = context.mapUnitsPerDistancePixel();
  if ( mapUnitsPerPixel < 0.0 )
  {
    QgsDebugError( QStringLiteral( "Invalid map units per pixel ratio" ) );
    return false;
  }

  for ( QgsPointCloudIndex *pc : std::as_const( indexes ) )
  {
    const IndexedPointCloudNode root = pc->root();
    const QgsRectangle rootNodeExtentLayerCoords = pc->nodeMapExtent( root );
    QgsRectangle rootNodeExtentInCurveCrs;
    try
    {
      rootNodeExtentInCurveCrs = extentTransform.transformBoundingBox( rootNodeExtentLayerCoords );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Could not transform node extent to curve CRS" ) );
      rootNodeExtentInCurveCrs = rootNodeExtentLayerCoords;
    }

    const double rootErrorInMapCoordinates = rootNodeExtentInCurveCrs.width() / pc->span(); // in curve coords
    if ( rootErrorInMapCoordinates < 0.0 )
    {
      QgsDebugError( QStringLiteral( "Invalid root node error" ) );
      return false;
    }

    double rootErrorPixels = rootErrorInMapCoordinates / mapUnitsPerPixel; // in pixels
    const QVector<IndexedPointCloudNode> nodes = traverseTree( pc, pc->root(), maximumErrorPixels, rootErrorPixels, context.elevationRange() );

    const double rootErrorInLayerCoordinates = rootNodeExtentLayerCoords.width() / pc->span();
    const double maxErrorInMapCoordinates = maximumErrorPixels * mapUnitsPerPixel;

    mResults->mMaxErrorInLayerCoordinates = std::max(
        mResults->mMaxErrorInLayerCoordinates,
        maxErrorInMapCoordinates * rootErrorInLayerCoordinates / rootErrorInMapCoordinates );

    switch ( pc->accessType() )
    {
      case QgsPointCloudIndex::AccessType::Local:
      {
        visitNodesSync( nodes, pc, request, context.elevationRange() );
        break;
      }
      case QgsPointCloudIndex::AccessType::Remote:
      {
        visitNodesAsync( nodes, pc, request, context.elevationRange() );
        break;
      }
    }

    if ( mFeedback->isCanceled() )
      return false;
  }

  if ( mGatheredPoints.empty() )
  {
    mResults = nullptr;
    return false;
  }

  // convert x/y values back to distance/height values

  QString lastError;
  const QgsPointCloudLayerProfileResults::PointResult *pointData = mGatheredPoints.constData();
  const int size = mGatheredPoints.size();
  mResults->results.resize( size );
  QgsPointCloudLayerProfileResults::PointResult *destData = mResults->results.data();
  for ( int i = 0; i < size; ++i, ++pointData, ++destData )
  {
    if ( mFeedback->isCanceled() )
      return false;

    *destData = *pointData;
    destData->distanceAlongCurve = startDistanceOffset + originalCurveGeos.lineLocatePoint( destData->x, destData->y, &lastError );
    if ( mOpacityByDistanceEffect ) // don't calculate this if we don't need it
      destData->distanceFromCurve = originalCurveGeos.distance( destData->x, destData->y );

    mResults->minZ = std::min( destData->z, mResults->minZ );
    mResults->maxZ = std::max( destData->z, mResults->maxZ );
  }
  mResults->finalize( mFeedback.get() );

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

  if ( pc->nodePointCount( n ) > 0 )
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

int QgsPointCloudLayerProfileGenerator::visitNodesSync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRequest &request, const QgsDoubleRange &zRange )
{
  int nodesDrawn = 0;
  for ( const IndexedPointCloudNode &n : nodes )
  {
    if ( mFeedback->isCanceled() )
      break;

    std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );

    if ( !block )
      continue;

    visitBlock( block.get(), zRange );

    ++nodesDrawn;
  }
  return nodesDrawn;
}

int QgsPointCloudLayerProfileGenerator::visitNodesAsync( const QVector<IndexedPointCloudNode> &nodes, QgsPointCloudIndex *pc, QgsPointCloudRequest &request, const QgsDoubleRange &zRange )
{
  int nodesDrawn = 0;

  // see notes about this logic in QgsPointCloudLayerRenderer::renderNodesAsync

  // Async loading of nodes
  QVector<QgsPointCloudBlockRequest *> blockRequests;
  QEventLoop loop;
  QObject::connect( mFeedback.get(), &QgsFeedback::canceled, &loop, &QEventLoop::quit );

  for ( int i = 0; i < nodes.size(); ++i )
  {
    const IndexedPointCloudNode &n = nodes[i];
    const QString nStr = n.toString();
    QgsPointCloudBlockRequest *blockRequest = pc->asyncNodeData( n, request );
    blockRequests.append( blockRequest );
    QObject::connect( blockRequest, &QgsPointCloudBlockRequest::finished, &loop,
                      [ this, &nodesDrawn, &loop, &blockRequests, &zRange, nStr, blockRequest ]()
    {
      blockRequests.removeOne( blockRequest );

      // If all blocks are loaded, exit the event loop
      if ( blockRequests.isEmpty() )
        loop.exit();

      std::unique_ptr<QgsPointCloudBlock> block = blockRequest->takeBlock();

      blockRequest->deleteLater();

      if ( mFeedback->isCanceled() )
      {
        return;
      }

      if ( !block )
      {
        QgsDebugError( QStringLiteral( "Unable to load node %1, error: %2" ).arg( nStr, blockRequest->errorStr() ) );
        return;
      }

      visitBlock( block.get(), zRange );
      ++nodesDrawn;
    } );
  }

  // Wait for all point cloud nodes to finish loading
  loop.exec();

  // Generation may have got canceled and the event loop exited before finished()
  // was called for all blocks, so let's clean up anything that is left
  for ( QgsPointCloudBlockRequest *blockRequest : std::as_const( blockRequests ) )
  {
    std::unique_ptr<QgsPointCloudBlock> block = blockRequest->takeBlock();
    block.reset();
    blockRequest->deleteLater();
  }

  return nodesDrawn;
}

void QgsPointCloudLayerProfileGenerator::visitBlock( const QgsPointCloudBlock *block, const QgsDoubleRange &zRange )
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

  bool useRenderer = false;
  if ( mPreparedRendererData )
  {
    useRenderer = mPreparedRendererData->prepareBlock( block );
  }

  QColor color;
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
    if ( !zRange.contains( res.z ) )
      continue;

    if ( useRenderer )
    {
      color = mPreparedRendererData->pointColor( block, i, res.z );
      if ( !color.isValid() )
        continue;

      res.color = color.rgba();
    }
    else
    {
      res.color = mPointColor.rgba();
    }

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

      mGatheredPoints.append( res );
    }
  }
}


