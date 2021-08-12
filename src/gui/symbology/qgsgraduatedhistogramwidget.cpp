/***************************************************************************
                         qgsgraduatedhistogramwidget.cpp
                         -------------------------------
    begin                : May 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#include "qgsgraduatedhistogramwidget.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsgraduatedsymbolrendererwidget.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsstatisticalsummary.h"

#include <QSettings>
#include <QObject>
#include <QMouseEvent>

// QWT Charting widget
#include <qwt_global.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_histogram.h>
#include <qwt_scale_map.h>


QgsGraduatedHistogramWidget::QgsGraduatedHistogramWidget( QWidget *parent )
  : QgsHistogramWidget( parent )
{
  //clear x axis title to make more room for graph
  setXAxisTitle( QString() );

  mFilter = new QgsGraduatedHistogramEventFilter( mPlot );

  connect( mFilter, &QgsGraduatedHistogramEventFilter::mousePress, this, &QgsGraduatedHistogramWidget::mousePress );
  connect( mFilter, &QgsGraduatedHistogramEventFilter::mouseRelease, this, &QgsGraduatedHistogramWidget::mouseRelease );

  mHistoPicker = new QwtPlotPicker( mPlot->canvas() );
  mHistoPicker->setTrackerMode( QwtPicker::ActiveOnly );
  mHistoPicker->setRubberBand( QwtPicker::VLineRubberBand );
  mHistoPicker->setStateMachine( new QwtPickerDragPointMachine );
}

void QgsGraduatedHistogramWidget::setRenderer( QgsGraduatedSymbolRenderer *renderer )
{
  mRenderer = renderer;
}

void QgsGraduatedHistogramWidget::drawHistogram()
{
  if ( !mRenderer )
    return;

  bool pickerEnabled = false;
  if ( mRenderer->rangesOverlap() )
  {
    setToolTip( tr( "Ranges are overlapping and can't be edited by the histogram" ) );
    setGraduatedRanges( QgsRangeList() );
  }
  else if ( mRenderer->rangesHaveGaps() )
  {
    setToolTip( tr( "Ranges have gaps and can't be edited by the histogram" ) );
    setGraduatedRanges( QgsRangeList() );
  }
  else if ( mRenderer->ranges().isEmpty() )
  {
    setToolTip( QString() );
    setGraduatedRanges( QgsRangeList() );
  }
  else
  {
    setToolTip( QString() );
    setGraduatedRanges( mRenderer->ranges() );
    pickerEnabled = true;
  }
  QgsHistogramWidget::drawHistogram();

  // histo picker
  mHistoPicker->setEnabled( pickerEnabled );
  mFilter->blockSignals( !pickerEnabled );
}

void QgsGraduatedHistogramWidget::mousePress( double value )
{
  mPressedValue = value;

  int closestRangeIndex = 0;
  int minPixelDistance = 9999;
  findClosestRange( mPressedValue, closestRangeIndex, minPixelDistance );

  if ( minPixelDistance <= 6 )
  {
    //moving a break, so hide the break line
    mRangeMarkers.at( closestRangeIndex )->hide();
    mPlot->replot();
  }
}

void QgsGraduatedHistogramWidget::mouseRelease( double value )
{
  int closestRangeIndex = 0;
  int minPixelDistance = 9999;
  findClosestRange( mPressedValue, closestRangeIndex, minPixelDistance );

  if ( minPixelDistance <= 6 )
  {
    //do a sanity check - don't allow users to drag a break line
    //into the middle of a different range. Doing so causes overlap
    //of the ranges

    //new value needs to be within range covered by closestRangeIndex or
    //closestRangeIndex + 1
    if ( value <= mRenderer->ranges().at( closestRangeIndex ).lowerValue() ||
         value >= mRenderer->ranges().at( closestRangeIndex + 1 ).upperValue() )
    {
      refresh();
      return;
    }

    mRenderer->updateRangeUpperValue( closestRangeIndex, value );
    mRenderer->updateRangeLowerValue( closestRangeIndex + 1, value );
    emit rangesModified( false );
  }
  else
  {
    //if distance from markers is too big, add a break
    mRenderer->addBreak( value );
    // to fix the deprecated call to reset() in QgsGraduatedSymbolRendererWidget::refreshRanges,
    // this class should work on the model in the widget rather than adding break via the renderer.
    emit rangesModified( true );
  }

  refresh();
}

void QgsGraduatedHistogramWidget::findClosestRange( double value, int &closestRangeIndex, int &pixelDistance ) const
{
  const QgsRangeList &ranges = mRenderer->ranges();

  double minDistance = std::numeric_limits<double>::max();
  const int pressedPixel = mPlot->canvasMap( QwtPlot::xBottom ).transform( value );
  for ( int i = 0; i < ranges.count() - 1; ++i )
  {
    if ( std::fabs( mPressedValue - ranges.at( i ).upperValue() ) < minDistance )
    {
      closestRangeIndex = i;
      minDistance = std::fabs( mPressedValue - ranges.at( i ).upperValue() );
      pixelDistance = std::fabs( pressedPixel - mPlot->canvasMap( QwtPlot::xBottom ).transform( ranges.at( i ).upperValue() ) );
    }
  }
}

/// @cond PRIVATE

QgsGraduatedHistogramEventFilter::QgsGraduatedHistogramEventFilter( QwtPlot *plot )
  : QObject( plot )
  , mPlot( plot )
{
  mPlot->canvas()->installEventFilter( this );
}

bool QgsGraduatedHistogramEventFilter::eventFilter( QObject *object, QEvent *event )
{
  if ( !mPlot->isEnabled() )
    return QObject::eventFilter( object, event );

  switch ( event->type() )
  {
    case QEvent::MouseButtonPress:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mousePress( posToValue( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mouseRelease( posToValue( mouseEvent->pos() ) );
      }
      break;
    }
    default:
      break;
  }

  return QObject::eventFilter( object, event );
}

double QgsGraduatedHistogramEventFilter::posToValue( QPointF point ) const
{
  if ( !mPlot )
    return -99999999;

  return mPlot->canvasMap( QwtPlot::xBottom ).invTransform( point.x() );
}
///@endcond
