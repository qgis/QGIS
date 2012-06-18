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

  name = provider->colorInterpretationName( band );
  if ( name == "Undefined" )
  {
    name = provider->generateBandName( band );
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
  if ( band < 0 )
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
