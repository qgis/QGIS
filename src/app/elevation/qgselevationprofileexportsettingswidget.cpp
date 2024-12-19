/***************************************************************************
                          qgselevationprofileexportsettingswidget.h
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

#include "qgselevationprofileexportsettingswidget.h"
#include "qgsplot.h"

QgsElevationProfileExportSettingsWidget::QgsElevationProfileExportSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
}

void QgsElevationProfileExportSettingsWidget::setPlotSettings( const Qgs2DPlot &plot )
{
  mSpinMinDistance->setValue( plot.xMinimum() );
  mSpinMinDistance->setClearValue( plot.xMinimum() );
  mSpinMinDistance->setShowClearButton( true );

  mSpinMaxDistance->setValue( plot.xMaximum() );
  mSpinMaxDistance->setClearValue( plot.xMaximum() );
  mSpinMaxDistance->setShowClearButton( true );

  mSpinMinElevation->setValue( plot.yMinimum() );
  mSpinMinElevation->setClearValue( plot.yMinimum() );
  mSpinMinElevation->setShowClearButton( true );

  mSpinMaxElevation->setValue( plot.yMaximum() );
  mSpinMaxElevation->setClearValue( plot.yMaximum() );
  mSpinMaxElevation->setShowClearButton( true );

  mSpinLabelIntervalX->setValue( plot.xAxis().labelInterval() );
  mSpinLabelIntervalX->setClearValue( plot.xAxis().labelInterval() );
  mSpinLabelIntervalX->setShowClearButton( true );

  mSpinMinorIntervalX->setValue( plot.xAxis().gridIntervalMinor() );
  mSpinMinorIntervalX->setClearValue( plot.xAxis().gridIntervalMinor() );
  mSpinMinorIntervalX->setShowClearButton( true );

  mSpinMajorIntervalX->setValue( plot.xAxis().gridIntervalMajor() );
  mSpinMajorIntervalX->setClearValue( plot.xAxis().gridIntervalMajor() );
  mSpinMajorIntervalX->setShowClearButton( true );


  mSpinLabelIntervalY->setValue( plot.yAxis().labelInterval() );
  mSpinLabelIntervalY->setClearValue( plot.yAxis().labelInterval() );
  mSpinLabelIntervalY->setShowClearButton( true );

  mSpinMinorIntervalY->setValue( plot.yAxis().gridIntervalMinor() );
  mSpinMinorIntervalY->setClearValue( plot.yAxis().gridIntervalMinor() );
  mSpinMinorIntervalY->setShowClearButton( true );

  mSpinMajorIntervalY->setValue( plot.yAxis().gridIntervalMajor() );
  mSpinMajorIntervalY->setClearValue( plot.yAxis().gridIntervalMajor() );
  mSpinMajorIntervalY->setShowClearButton( true );
}

void QgsElevationProfileExportSettingsWidget::updatePlotSettings( Qgs2DPlot &plot )
{
  plot.setXMinimum( mSpinMinDistance->value() );
  plot.setXMaximum( mSpinMaxDistance->value() );
  plot.setYMinimum( mSpinMinElevation->value() );
  plot.setYMaximum( mSpinMaxElevation->value() );

  plot.xAxis().setLabelInterval( mSpinLabelIntervalX->value() );
  plot.xAxis().setGridIntervalMinor( mSpinMinorIntervalX->value() );
  plot.xAxis().setGridIntervalMajor( mSpinMajorIntervalX->value() );

  plot.yAxis().setLabelInterval( mSpinLabelIntervalY->value() );
  plot.yAxis().setGridIntervalMinor( mSpinMinorIntervalY->value() );
  plot.yAxis().setGridIntervalMajor( mSpinMajorIntervalY->value() );
}
