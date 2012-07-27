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
    QgsRasterBandStats rasterBandStats = mRasterLayer->dataProvider()->bandStatistics( band );
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
    //QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( band );

    int sampleSize = 250000; // number of sample cells
    QgsRasterHistogram myHistogram =  mRasterLayer->dataProvider()->histogram( band, 0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize );

    double myBinXStep = ( myHistogram.maximum - myHistogram.minimum ) / myHistogram.binCount;
    int myCount = 0;
    int myMinCount = ( int ) qRound( 0.02 * myHistogram.nonNullCount );
    int myMaxCount = ( int ) qRound( 0.98 * myHistogram.nonNullCount );
    bool myMinFound = false;
    QgsDebugMsg( QString( "binCount = %1 minimum = %2 maximum = %3 myBinXStep = %4" ).arg( myHistogram.binCount ).arg( myHistogram.minimum ).arg( myHistogram.maximum ).arg( myBinXStep ) );

    for ( int myBin = 0; myBin < myHistogram.histogramVector.size(); myBin++ )
    {
      int myBinValue = myHistogram.histogramVector.value( myBin );
      myCount += myBinValue;
      if ( !myMinFound && myCount > myMinCount )
      {
        minMaxValues[0] = myHistogram.minimum + myBin * myBinXStep;
        myMinFound = true;
      }
      if ( myCount > myMaxCount )
      {
        minMaxValues[1] = myHistogram.minimum + myBin * myBinXStep;
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

  QgsRasterBandStats myRasterBandStats = mRasterLayer->dataProvider()->bandStatistics( band );
  minMaxValues[0] = myRasterBandStats.mean - ( stdDev * myRasterBandStats.stdDev );
  minMaxValues[1] = myRasterBandStats.mean + ( stdDev * myRasterBandStats.stdDev );

  return true;
}
