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
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2widget.h"
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
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
#include <qwt_plot_renderer.h>
#include <qwt_plot_histogram.h>
#else
#include "../raster/qwt5_histogram_item.h"
#endif


QgsGraduatedHistogramWidget::QgsGraduatedHistogramWidget( QWidget *parent )
    : QgsHistogramWidget( parent )
    , mRenderer( 0 )
    , mHistoPicker( 0 )
    , mPressedValue( 0 )
{
  //clear x axis title to make more room for graph
  setXAxisTitle( QString() );

  mFilter = new QgsGraduatedHistogramEventFilter( mPlot );

  connect( mFilter, SIGNAL( mousePress( double ) ), this, SLOT( mousePress( double ) ) );
  connect( mFilter, SIGNAL( mouseRelease( double ) ), this, SLOT( mouseRelease( double ) ) );

  mHistoPicker = new QwtPlotPicker( mPlot->canvas() );
  mHistoPicker->setTrackerMode( QwtPicker::ActiveOnly );
  mHistoPicker->setRubberBand( QwtPicker::VLineRubberBand );
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  mHistoPicker->setStateMachine( new QwtPickerDragPointMachine );
#else
  mHistoPicker->setSelectionFlags( QwtPicker::PointSelection | QwtPicker::DragSelection );
#endif
}


QgsGraduatedHistogramWidget::~QgsGraduatedHistogramWidget()
{
}

void QgsGraduatedHistogramWidget::setRenderer( QgsGraduatedSymbolRendererV2 *renderer )
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
    emit rangesModified( true );
  }

  refresh();
}

void QgsGraduatedHistogramWidget::findClosestRange( double value, int &closestRangeIndex, int& pixelDistance ) const
{
  const QgsRangeList& ranges = mRenderer->ranges();

  double minDistance = DBL_MAX;
  int pressedPixel = mPlot->canvasMap( QwtPlot::xBottom ).transform( value );
  for ( int i = 0; i < ranges.count() - 1; ++i )
  {
    if ( qAbs( mPressedValue - ranges.at( i ).upperValue() ) < minDistance )
    {
      closestRangeIndex = i;
      minDistance = qAbs( mPressedValue - ranges.at( i ).upperValue() );
      pixelDistance = qAbs( pressedPixel - mPlot->canvasMap( QwtPlot::xBottom ).transform( ranges.at( i ).upperValue() ) );
    }
  }
}

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
      const QMouseEvent* mouseEvent = static_cast<QMouseEvent* >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mousePress( posToValue( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      const QMouseEvent* mouseEvent = static_cast<QMouseEvent* >( event );
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

double QgsGraduatedHistogramEventFilter::posToValue( const QPointF &point ) const
{
  if ( !mPlot )
    return -99999999;

  return mPlot->canvasMap( QwtPlot::xBottom ).invTransform( point.x() );
}
