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

#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include <QPolygonF>

//
// QgsRasterLayerProfileResults
//

QString QgsRasterLayerProfileResults::type() const
{
  return QStringLiteral( "raster" );
}

QMap<double, double> QgsRasterLayerProfileResults::distanceToHeightMap() const
{
  QMap<double, double> res;
  for ( const Result &r : results )
  {
    res.insert( r.distance, r.height );
  }
  return res;
}

QgsPointSequence QgsRasterLayerProfileResults::sampledPoints() const
{
  return rawPoints;
}

QVector<QgsGeometry> QgsRasterLayerProfileResults::asGeometries() const
{
  QVector<QgsGeometry> res;
  res.reserve( rawPoints.size() );
  for ( const QgsPoint &point : rawPoints )
    res.append( QgsGeometry( point.clone() ) );

  return res;
}

void QgsRasterLayerProfileResults::renderResults( QgsProfileRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  if ( !painter )
    return;

  painter->save();
  painter->setBrush( Qt::NoBrush );
  QPen pen( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor() );
  pen.setWidthF( 3 );
  painter->setPen( pen );

  const QMap< double, double > distMap = distanceToHeightMap();
  QPolygonF currentLine;
  for ( auto pointIt = distMap.begin(); pointIt != distMap.end(); ++pointIt )
  {
    if ( std::isnan( pointIt.value() ) )
    {
      if ( currentLine.length() > 1 )
        painter->drawPolyline( currentLine );
      currentLine.clear();
    }
    currentLine.append( context.worldTransform().map( QPointF( pointIt.key(), pointIt.value() ) ) );
  }
  if ( currentLine.length() > 1 )
    painter->drawPolyline( currentLine );

  painter->restore();
}


//
// QgsRasterLayerProfileGenerator
//

QgsRasterLayerProfileGenerator::QgsRasterLayerProfileGenerator( QgsRasterLayer *layer, const QgsProfileRequest &request )
  : mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mOffset( layer->elevationProperties()->zOffset() )
  , mScale( layer->elevationProperties()->zScale() )
  , mRasterUnitsPerPixelX( layer->rasterUnitsPerPixelX() )
  , mRasterUnitsPerPixelY( layer->rasterUnitsPerPixelY() )
  , mStepDistance( request.stepDistance() )
{
  mRasterProvider.reset( layer->dataProvider()->clone() );
}

QgsRasterLayerProfileGenerator::~QgsRasterLayerProfileGenerator() = default;

bool QgsRasterLayerProfileGenerator::generateProfile()
{
  if ( !mProfileCurve )
    return false;

  // we need to transform the profile curve to the raster's CRS
  std::unique_ptr< QgsCurve > transformedCurve( mProfileCurve->clone() );
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

  const QgsRectangle profileCurveBoundingBox = transformedCurve->boundingBox();
  if ( !profileCurveBoundingBox.intersects( mRasterProvider->extent() ) )
    return false;

  mResults = std::make_unique< QgsRasterLayerProfileResults >();

  std::unique_ptr< QgsGeometryEngine > curveEngine( QgsGeometry::createGeometryEngine( transformedCurve.get() ) );
  curveEngine->prepareGeometry();

  QSet< QgsPointXY > profilePoints;
  if ( !std::isnan( mStepDistance ) )
  {
    // if specific step distance specified, use this to generate points along the curve
    QgsGeometry densifiedCurve( mProfileCurve->clone() );
    densifiedCurve = densifiedCurve.densifyByDistance( mStepDistance );
    densifiedCurve.transform( rasterToTargetTransform, Qgis::TransformDirection::Reverse );
    profilePoints.reserve( densifiedCurve.constGet()->nCoordinates() );
    for ( auto it = densifiedCurve.vertices_begin(); it != densifiedCurve.vertices_end(); ++it )
    {
      profilePoints.insert( *it );
    }
  }

  // calculate the portion of the raster which actually covers the curve
  int subRegionWidth = 0;
  int subRegionHeight = 0;
  int subRegionLeft = 0;
  int subRegionTop = 0;
  const QgsRectangle rasterSubRegion = QgsRasterIterator::subRegion(
                                         mRasterProvider->extent(),
                                         mRasterProvider->xSize(),
                                         mRasterProvider->ySize(),
                                         transformedCurve->boundingBox(),
                                         subRegionWidth,
                                         subRegionHeight,
                                         subRegionLeft,
                                         subRegionTop );

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
    const QgsGeometry blockExtentGeom = QgsGeometry::fromRect( blockExtent );
    if ( !curveEngine->intersects( blockExtentGeom.constGet() ) )
      continue;

    // TODO -- add feedback argument!
    std::unique_ptr< QgsRasterBlock > block( mRasterProvider->block( mBand, blockExtent, blockColumns, blockRows ) );
    if ( !block )
      continue;

    bool isNoData = false;

    // there's two potential code paths we use here, depending on if we want to sample at every pixel, or if we only want to
    // sample at specific points
    if ( !std::isnan( mStepDistance ) )
    {
      auto it = profilePoints.begin();
      while ( it != profilePoints.end() )
      {
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
          mResults->rawPoints.append( pixel );

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
          mResults->rawPoints.append( pixel );
        }
        currentY -= mRasterUnitsPerPixelY;
      }
    }
  }

  // convert x/y values back to distance/height values
  QgsGeos originalCurveGeos( mProfileCurve.get() );
  originalCurveGeos.prepareGeometry();
  mResults->results.reserve( mResults->rawPoints.size() );
  QString lastError;
  for ( const QgsPoint &pixel : std::as_const( mResults->rawPoints ) )
  {
    const double distance = originalCurveGeos.lineLocatePoint( pixel, &lastError );

    QgsRasterLayerProfileResults::Result res;
    res.distance = distance;
    res.height = pixel.z();
    mResults->results.push_back( res );
  }

  return true;
}

QgsAbstractProfileResults *QgsRasterLayerProfileGenerator::takeResults()
{
  return mResults.release();
}
