/***************************************************************************
                         qgsrasterrendererwidget.cpp
                         ---------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererwidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"


QString QgsRasterRendererWidget::displayBandName( int band ) const
{
  QString name;
  if ( !mRasterLayer )
  {
    return name;
  }

  const QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return name;
  }

  name = provider->generateBandName( band );

  QString colorInterp = provider->colorInterpretationName( band );
  if ( colorInterp != "Undefined" )
  {
    name.append( QString( " (%1)" ).arg( colorInterp ) );
  }
  return name;
}

bool QgsRasterRendererWidget::bandMinMax( LoadMinMaxAlgo loadAlgo, int band, double* minMaxValues )
{
  if ( !mRasterLayer )
  {
    return false;
  }
  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return false;
  }
  if ( band <= 0 )
  {
    return false;
  }

  if ( loadAlgo == Estimate )
  {
    minMaxValues[0] = provider->minimumValue( band );
    minMaxValues[1] = provider->maximumValue( band );
  }
  else if ( loadAlgo == Actual )
  {
    QgsRasterBandStats rasterBandStats = mRasterLayer->bandStatistics( band );
    minMaxValues[0] = rasterBandStats.minimumValue;
    minMaxValues[1] = rasterBandStats.maximumValue;
  }
  else if ( loadAlgo == CurrentExtent )
  {
    mRasterLayer->computeMinimumMaximumFromLastExtent( band, minMaxValues );
  }
  else if ( loadAlgo == CumulativeCut )
  {
    // Currently 2 - 98% cumulative pixel count cut
    bool myIgnoreOutOfRangeFlag = true;
    bool myThoroughBandScanFlag = false;
    mRasterLayer->populateHistogram( band, RASTER_HISTOGRAM_BINS, myIgnoreOutOfRangeFlag, myThoroughBandScanFlag );

    QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( band );
    double myBinXStep = myRasterBandStats.range / RASTER_HISTOGRAM_BINS;
    int myCount = 0;
    int myMinCount = ( int ) qRound( 0.02 * myRasterBandStats.elementCount );
    int myMaxCount = ( int ) qRound( 0.98 * myRasterBandStats.elementCount );
    bool myMinFound = false;
    QgsDebugMsg( QString( "RASTER_HISTOGRAM_BINS = %1 range = %2 minimumValue = %3 myBinXStep = %4" ).arg( RASTER_HISTOGRAM_BINS ).arg( myRasterBandStats.range ).arg( myRasterBandStats.minimumValue ).arg( myBinXStep ) );
    for ( int myBin = 0; myBin < RASTER_HISTOGRAM_BINS; myBin++ )
    {
      int myBinValue = myRasterBandStats.histogramVector->value( myBin );
      myCount += myBinValue;
      if ( !myMinFound && myCount > myMinCount )
      {
        minMaxValues[0] = myRasterBandStats.minimumValue + myBin * myBinXStep;
        myMinFound = true;
      }
      if ( myCount > myMaxCount )
      {
        minMaxValues[1] = myRasterBandStats.minimumValue + myBin * myBinXStep;
        break;
      }
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool QgsRasterRendererWidget::bandMinMaxFromStdDev( double stdDev, int band, double* minMaxValues )
{
  if ( !mRasterLayer )
  {
    return false;
  }
  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return false;
  }
  if ( band < 0 )
  {
    return false;
  }

  QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( band );
  minMaxValues[0] = myRasterBandStats.mean - ( stdDev * myRasterBandStats.stdDev );
  minMaxValues[1] = myRasterBandStats.mean + ( stdDev * myRasterBandStats.stdDev );

  return true;
}
