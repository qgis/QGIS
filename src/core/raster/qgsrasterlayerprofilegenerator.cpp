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

QgsRasterLayerProfileGenerator::QgsRasterLayerProfileGenerator( QgsRasterLayer *layer, const QgsProfileRequest &request )
  : mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mOffset( layer->elevationProperties()->zOffset() )
  , mScale( layer->elevationProperties()->zScale() )
  , mRasterUnitsPerPixelX( layer->rasterUnitsPerPixelX() )
  , mRasterUnitsPerPixelY( layer->rasterUnitsPerPixelY() )
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

  std::unique_ptr< QgsGeometryEngine > curveEngine( QgsGeometry::createGeometryEngine( transformedCurve.get() ) );
  curveEngine->prepareGeometry();

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

    double currentY = blockExtent.yMaximum() - 0.5 * mRasterUnitsPerPixelY;
    for ( int row = 0; row < blockRows; ++row )
    {
      double currentX = blockExtent.xMinimum() + 0.5 * mRasterUnitsPerPixelX;
      for ( int col = 0; col < blockColumns; ++col, currentX += mRasterUnitsPerPixelX )
      {
        const double val = block->valueAndNoData( row, col, isNoData );
        if ( isNoData )
          continue;

        // does pixel intersect curve?
        QgsGeometry pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - halfPixelSizeX,
                                        currentY - halfPixelSizeY,
                                        currentX + halfPixelSizeX,
                                        currentY + halfPixelSizeY ) );
        if ( !curveEngine->intersects( pixelRectGeometry.constGet() ) )
          continue;

        QgsPoint pixel( currentX, currentY, val * mScale + mOffset );
        try
        {
          pixel.transform( rasterToTargetTransform );
        }
        catch ( QgsCsException & )
        {
          continue;
        }
        mRawPoints.append( pixel );
      }
      currentY -= mRasterUnitsPerPixelY;
    }
  }

  // convert x/y values back to distance/height values
  QgsGeos originalCurveGeos( mProfileCurve.get() );
  originalCurveGeos.prepareGeometry();
  mResults.reserve( mRawPoints.size() );
  QString lastError;
  for ( const QgsPoint &pixel : std::as_const( mRawPoints ) )
  {
    const double distance = originalCurveGeos.lineLocatePoint( pixel, &lastError );

    Result res;
    res.distance = distance;
    res.height = pixel.z();
    mResults.push_back( res );
  }

  return true;
}
