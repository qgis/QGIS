/***************************************************************************
                         qgsrasterminmaxwidget.h
                         ---------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSettings>

#include "qgsrasterlayer.h"
#include "qgsrasterminmaxwidget.h"

QgsRasterMinMaxWidget::QgsRasterMinMaxWidget( QgsRasterLayer* theLayer, QWidget *parent ):
    QWidget( parent )
    , mLayer( theLayer )
{
  QgsDebugMsg( "Entered." );
  setupUi( this );

  QSettings mySettings;
  double myLower = 100.0 * mySettings.value( "/Raster/cumulativeCutLower", QString::number( QgsRasterLayer::CUMULATIVE_CUT_LOWER ) ).toDouble();
  double myUpper = 100.0 * mySettings.value( "/Raster/cumulativeCutUpper", QString::number( QgsRasterLayer::CUMULATIVE_CUT_UPPER ) ).toDouble();
  mCumulativeCutLowerDoubleSpinBox->setValue( myLower );
  mCumulativeCutUpperDoubleSpinBox->setValue( myUpper );
}

QgsRasterMinMaxWidget::~QgsRasterMinMaxWidget()
{
}

void QgsRasterMinMaxWidget::on_mLoadPushButton_clicked()
{
  QgsDebugMsg( "Entered." );

  foreach( int myBand, mBands )
  {
    QgsDebugMsg( QString( "myBand = %1" ).arg( myBand ) );
    if ( myBand < 1 || myBand > mLayer->dataProvider()->bandCount() )
    {
      continue;
    }
    double myMin = std::numeric_limits<double>::quiet_NaN();
    double myMax = std::numeric_limits<double>::quiet_NaN();

    QgsRectangle myExtent; // empty == full
    if ( mCurrentExtentRadioButton->isChecked() )
    {
      myExtent = mExtent; // current
    }
    QgsDebugMsg( QString( "myExtent.isEmpty() = %1" ).arg( myExtent.isEmpty() ) );

    int mySampleSize = 0; // 0 == exact
    if ( mEstimateRadioButton->isChecked() )
    {
      mySampleSize = 250000;
    }

    if ( mCumulativeCutRadioButton->isChecked() )
    {
      double myLower = mCumulativeCutLowerDoubleSpinBox->value() / 100.0;
      double myUpper = mCumulativeCutUpperDoubleSpinBox->value() / 100.0;
      mLayer->dataProvider()->cumulativeCut( myBand, myLower, myUpper, myMin, myMax, myExtent, mySampleSize );
    }
    else if ( mMinMaxRadioButton->isChecked() )
    {
      // TODO: consider provider minimum/maximumValue() (has to be defined well in povider)
      QgsRasterBandStats myRasterBandStats = mLayer->dataProvider()->bandStatistics( myBand, QgsRasterBandStats::Min | QgsRasterBandStats::Max, myExtent, mySampleSize );
      myMin = myRasterBandStats.minimumValue;
      myMax = myRasterBandStats.maximumValue;
    }
    else if ( mStdDevRadioButton->isChecked() )
    {
      QgsRasterBandStats myRasterBandStats = mLayer->dataProvider()->bandStatistics( myBand, QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, myExtent, mySampleSize );
      double myStdDev = mStdDevSpinBox->value();
      myMin = myRasterBandStats.mean - ( myStdDev * myRasterBandStats.stdDev );
      myMax = myRasterBandStats.mean + ( myStdDev * myRasterBandStats.stdDev );

    }

    emit load( myBand, myMin, myMax );
  }
}
