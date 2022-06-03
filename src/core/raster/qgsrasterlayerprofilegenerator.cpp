/***************************************************************************
                         qgsrasterlayerprofilegenerator.cpp
                         ---------------
    begin                : March 2022
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
#include "qgsrasterlayerprofilegenerator.h"
#include "qgsprofilerequest.h"
#include "qgscurve.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsrasteriterator.h"
#include "qgsgeometryengine.h"
#include "qgsgeos.h"
#include "qgslinesymbol.h"
#include "qgsgeometryutils.h"
#include "qgsprofilepoint.h"
#include "qgsfillsymbol.h"

#include <QPolygonF>

//
// QgsRasterLayerProfileResults
//

QString QgsRasterLayerProfileResults::type() const
{
  return QStringLiteral( "raster" );
}

QVector<QgsProfileIdentifyResults> QgsRasterLayerProfileResults::identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context )
{
  const QVector<QgsProfileIdentifyResults> noLayerResults = QgsAbstractProfileSurfaceResults::identify( point, context );

  // we have to make a new list, with the correct layer reference set
  QVector<QgsProfileIdentifyResults> res;
  res.reserve( noLayerResults.size() );
  for ( const QgsProfileIdentifyResults &result : noLayerResults )
  {
    res.append( QgsProfileIdentifyResults( mLayer, result.results() ) );
  }
  return res;
}



//
// QgsRasterLayerProfileGenerator
//

QgsRasterLayerProfileGenerator::QgsRasterLayerProfileGenerator( QgsRasterLayer *layer, const QgsProfileRequest &request )
  : mId( layer->id() )
  , mFeedback( std::make_unique< QgsRasterBlockFeedback >() )
  , mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mOffset( layer->elevationProperties()->zOffset() )
  , mScale( layer->elevationProperties()->zScale() )
  , mLayer( layer )
  , mBand( qgis::down_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() )->bandNumber() )
  , mRasterUnitsPerPixelX( layer->rasterUnitsPerPixelX() )
  , mRasterUnitsPerPixelY( layer->rasterUnitsPerPixelY() )
  , mStepDistance( request.stepDistance() )
{
  mRasterProvider.reset( layer->dataProvider()->clone() );

  mSymbology = qgis::down_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() )->profileSymbology();
  mLineSymbol.reset( qgis::down_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() )->profileLineSymbol()->clone() );
  mFillSymbol.reset( qgis::down_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() )->profileFillSymbol()->clone() );
}

QString QgsRasterLayerProfileGenerator::sourceId() const
{
  return mId;
}

Qgis::ProfileGeneratorFlags QgsRasterLayerProfileGenerator::flags() const
{
  return Qgis::ProfileGeneratorFlag::RespectsDistanceRange | Qgis::ProfileGeneratorFlag::RespectsMaximumErrorMapUnit;
}

QgsRasterLayerProfileGenerator::~QgsRasterLayerProfileGenerator() = default;

bool QgsRasterLayerProfileGenerator::generateProfile( const QgsProfileGenerationContext &context )
{
  if ( !mProfileCurve || mFeedback->isCanceled() )
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

  // we need to transform the profile curve to the raster's CRS
  std::unique_ptr< QgsCurve > transformedCurve( sourceCurve->clone() );
  const QgsCoordinateTransform rasterToTargetTransform( mSourceCrs, mTargetCrs, mTransformContext );
  try
  {
    transformedCurve->transform( rasterToTargetTransform, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming profile line to raster CRS" ) );
    return false;
  }

  if ( mFeedback->isCanceled() )
    return false;

  const QgsRectangle profileCurveBoundingBox = transformedCurve->boundingBox();
  if ( !profileCurveBoundingBox.intersects( mRasterProvider->extent() ) )
    return false;

  if ( mFeedback->isCanceled() )
    return false;

  mResults = std::make_unique< QgsRasterLayerProfileResults >();
  mResults->mLayer = mLayer;
  mResults->copyPropertiesFromGenerator( this );

  std::unique_ptr< QgsGeometryEngine > curveEngine( QgsGeometry::createGeometryEngine( transformedCurve.get() ) );
  curveEngine->prepareGeometry();

  if ( mFeedback->isCanceled() )
    return false;

  double stepDistance = mStepDistance;
  if ( !std::isnan( context.maximumErrorMapUnits() ) )
  {
    // convert the maximum error in curve units to a step distance
    // TODO -- there's no point in this being << pixel size!
    if ( std::isnan( stepDistance ) || context.maximumErrorMapUnits() > stepDistance )
    {
      stepDistance = context.maximumErrorMapUnits();
    }
  }

  QSet< QgsPointXY > profilePoints;
  if ( !std::isnan( stepDistance ) )
  {
    // if specific step distance specified, use this to generate points along the curve
    QgsGeometry densifiedCurve( sourceCurve->clone() );
    densifiedCurve = densifiedCurve.densifyByDistance( stepDistance );
    densifiedCurve.transform( rasterToTargetTransform, Qgis::TransformDirection::Reverse );
    profilePoints.reserve( densifiedCurve.constGet()->nCoordinates() );
    for ( auto it = densifiedCurve.vertices_begin(); it != densifiedCurve.vertices_end(); ++it )
    {
      profilePoints.insert( *it );
    }
  }

  if ( mFeedback->isCanceled() )
    return false;

  // calculate the portion of the raster which actually covers the curve
  int subRegionWidth = 0;
  int subRegionHeight = 0;
  int subRegionLeft = 0;
  int subRegionTop = 0;
  const QgsRectangle rasterSubRegion = mRasterProvider->xSize() > 0 && mRasterProvider->ySize() > 0 ?
                                       QgsRasterIterator::subRegion(
                                         mRasterProvider->extent(),
                                         mRasterProvider->xSize(),
                                         mRasterProvider->ySize(),
                                         transformedCurve->boundingBox(),
                                         subRegionWidth,
                                         subRegionHeight,
                                         subRegionLeft,
                                         subRegionTop ) : transformedCurve->boundingBox();

  if ( mRasterProvider->xSize() == 0 || mRasterProvider->ySize() == 0 )
  {
    // e.g. XYZ tile source -- this is a rough hack for https://github.com/qgis/QGIS/issues/48806, which results
    // in pretty poor curves ;)
    const double curveLengthInPixels = sourceCurve->length() / context.mapUnitsPerDistancePixel();
    const double conversionFactor = curveLengthInPixels / transformedCurve->length();
    subRegionWidth = 2 * conversionFactor * rasterSubRegion.width();
    subRegionHeight = 2 * conversionFactor * rasterSubRegion.height();
  }

  // iterate over the raster blocks, throwing away any which don't intersect the profile curve
  QgsRasterIterator it( mRasterProvider.get() );
  // we use smaller tile sizes vs the default, as we will be skipping over tiles which don't intersect the curve at all,
  // and we expect that to be the VAST majority of the tiles in the raster.
  // => Smaller tile sizes = more regions we can shortcut over = less pixels to iterate over = faster runtime
  it.setMaximumTileHeight( 64 );
  it.setMaximumTileWidth( 64 );

  it.startRasterRead( mBand, subRegionWidth, subRegionHeight, rasterSubRegion );

  const double halfPixelSizeX = mRasterUnitsPerPixelX / 2.0;
  const double halfPixelSizeY = mRasterUnitsPerPixelY / 2.0;
  int blockColumns = 0;
  int blockRows = 0;
  int blockTopLeftColumn = 0;
  int blockTopLeftRow = 0;
  QgsRectangle blockExtent;

  while ( it.next( mBand, blockColumns, blockRows, blockTopLeftColumn, blockTopLeftRow, blockExtent ) )
  {
    if ( mFeedback->isCanceled() )
      return false;

    const QgsGeometry blockExtentGeom = QgsGeometry::fromRect( blockExtent );
    if ( !curveEngine->intersects( blockExtentGeom.constGet() ) )
      continue;

    std::unique_ptr< QgsRasterBlock > block( mRasterProvider->block( mBand, blockExtent, blockColumns, blockRows, mFeedback.get() ) );
    if ( mFeedback->isCanceled() )
      return false;

    if ( !block )
      continue;

    bool isNoData = false;

    // there's two potential code paths we use here, depending on if we want to sample at every pixel, or if we only want to
    // sample at specific points
    if ( !std::isnan( stepDistance ) )
    {
      auto it = profilePoints.begin();
      while ( it != profilePoints.end() )
      {
        if ( mFeedback->isCanceled() )
          return false;

        // convert point to a pixel and sample, if it's in this block
        if ( blockExtent.contains( *it ) )
        {
          const int row = std::clamp( static_cast< int >( std::round( ( blockExtent.yMaximum() - it->y() ) / mRasterUnitsPerPixelY ) ), 0, blockRows - 1 );
          const int col = std::clamp( static_cast< int >( std::round( ( it->x() - blockExtent.xMinimum() ) / mRasterUnitsPerPixelX ) ),  0, blockColumns - 1 );
          double val = block->valueAndNoData( row, col, isNoData );
          if ( !isNoData )
          {
            val = val * mScale + mOffset;
          }
          else
          {
            val = std::numeric_limits<double>::quiet_NaN();
          }

          QgsPoint pixel( it->x(), it->y(), val );
          try
          {
            pixel.transform( rasterToTargetTransform );
          }
          catch ( QgsCsException & )
          {
            continue;
          }
          mResults->mRawPoints.append( pixel );

          it = profilePoints.erase( it );
        }
        else
        {
          it++;
        }
      }

      if ( profilePoints.isEmpty() )
        break; // all done!
    }
    else
    {
      double currentY = blockExtent.yMaximum() - 0.5 * mRasterUnitsPerPixelY;
      for ( int row = 0; row < blockRows; ++row )
      {
        if ( mFeedback->isCanceled() )
          return false;

        double currentX = blockExtent.xMinimum() + 0.5 * mRasterUnitsPerPixelX;
        for ( int col = 0; col < blockColumns; ++col, currentX += mRasterUnitsPerPixelX )
        {
          const double val = block->valueAndNoData( row, col, isNoData );

          // does pixel intersect curve?
          QgsGeometry pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - halfPixelSizeX,
                                          currentY - halfPixelSizeY,
                                          currentX + halfPixelSizeX,
                                          currentY + halfPixelSizeY ) );
          if ( !curveEngine->intersects( pixelRectGeometry.constGet() ) )
            continue;

          QgsPoint pixel( currentX, currentY, isNoData ? std::numeric_limits<double>::quiet_NaN() : val * mScale + mOffset );
          try
          {
            pixel.transform( rasterToTargetTransform );
          }
          catch ( QgsCsException & )
          {
            continue;
          }
          mResults->mRawPoints.append( pixel );
        }
        currentY -= mRasterUnitsPerPixelY;
      }
    }
  }

  if ( mFeedback->isCanceled() )
    return false;

  // convert x/y values back to distance/height values
  QgsGeos originalCurveGeos( sourceCurve );
  originalCurveGeos.prepareGeometry();
  QString lastError;
  for ( const QgsPoint &pixel : std::as_const( mResults->mRawPoints ) )
  {
    if ( mFeedback->isCanceled() )
      return false;

    const double distance = originalCurveGeos.lineLocatePoint( pixel, &lastError ) + startDistanceOffset;

    if ( !std::isnan( pixel.z() ) )
    {
      mResults->minZ = std::min( pixel.z(), mResults->minZ );
      mResults->maxZ = std::max( pixel.z(), mResults->maxZ );
    }
    mResults->mDistanceToHeightMap.insert( distance, pixel.z() );
  }

  return true;
}

QgsAbstractProfileResults *QgsRasterLayerProfileGenerator::takeResults()
{
  return mResults.release();
}

QgsFeedback *QgsRasterLayerProfileGenerator::feedback() const
{
  return mFeedback.get();
}
