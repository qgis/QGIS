/***************************************************************************
    qgscurveeditorwidget.cpp
    ------------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgscurveeditorwidget.h"
#include "qgsvectorlayer.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <algorithm>

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
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>

#include <qwt_plot_renderer.h>
#include <qwt_plot_histogram.h>

QgsCurveEditorWidget::QgsCurveEditorWidget( QWidget *parent, const QgsCurveTransform &transform )
  : QWidget( parent )
  , mCurve( transform )
{
  mPlot = new QwtPlot();
  mPlot->setMinimumSize( QSize( 0, 100 ) );
  mPlot->setAxisScale( QwtPlot::yLeft, 0, 1 );
  mPlot->setAxisScale( QwtPlot::yRight, 0, 1 );
  mPlot->setAxisScale( QwtPlot::xBottom, 0, 1 );
  mPlot->setAxisScale( QwtPlot::xTop, 0, 1 );

  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->addWidget( mPlot );
  setLayout( vlayout );

  // hide the ugly canvas frame
  mPlot->setFrameStyle( QFrame::NoFrame );
  QFrame *plotCanvasFrame = dynamic_cast<QFrame *>( mPlot->canvas() );
  if ( plotCanvasFrame )
    plotCanvasFrame->setFrameStyle( QFrame::NoFrame );

  mPlot->enableAxis( QwtPlot::yLeft, false );
  mPlot->enableAxis( QwtPlot::xBottom, false );

  // add a grid
  QwtPlotGrid *grid = new QwtPlotGrid();
  const QwtScaleDiv gridDiv( 0.0, 1.0, QList<double>(), QList<double>(), QList<double>() << 0.2 << 0.4 << 0.6 << 0.8 );
  grid->setXDiv( gridDiv );
  grid->setYDiv( gridDiv );
  grid->setPen( QPen( QColor( 0, 0, 0, 50 ) ) );
  grid->attach( mPlot );

  mPlotCurve = new QwtPlotCurve();
  mPlotCurve->setTitle( QStringLiteral( "Curve" ) );
  mPlotCurve->setPen( QPen( QColor( 30, 30, 30 ), 0.0 ) ),
             mPlotCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mPlotCurve->attach( mPlot );

  mPlotFilter = new QgsCurveEditorPlotEventFilter( mPlot );
  connect( mPlotFilter, &QgsCurveEditorPlotEventFilter::mousePress, this, &QgsCurveEditorWidget::plotMousePress );
  connect( mPlotFilter, &QgsCurveEditorPlotEventFilter::mouseRelease, this, &QgsCurveEditorWidget::plotMouseRelease );
  connect( mPlotFilter, &QgsCurveEditorPlotEventFilter::mouseMove, this, &QgsCurveEditorWidget::plotMouseMove );

  mPlotCurve->setVisible( true );
  updatePlot();
}

QgsCurveEditorWidget::~QgsCurveEditorWidget()
{
  if ( mGatherer && mGatherer->isRunning() )
  {
    connect( mGatherer.get(), &QgsHistogramValuesGatherer::finished, mGatherer.get(), &QgsHistogramValuesGatherer::deleteLater );
    mGatherer->stop();
    ( void )mGatherer.release();
  }
}

void QgsCurveEditorWidget::setCurve( const QgsCurveTransform &curve )
{
  mCurve = curve;
  updatePlot();
  emit changed();
}

void QgsCurveEditorWidget::setHistogramSource( const QgsVectorLayer *layer, const QString &expression )
{
  if ( !mGatherer )
  {
    mGatherer.reset( new QgsHistogramValuesGatherer() );
    connect( mGatherer.get(), &QgsHistogramValuesGatherer::calculatedHistogram, this, [ = ]
    {
      mHistogram.reset( new QgsHistogram( mGatherer->histogram() ) );
      updateHistogram();
    } );
  }

  const bool changed = mGatherer->layer() != layer || mGatherer->expression() != expression;
  if ( changed )
  {
    mGatherer->setExpression( expression );
    mGatherer->setLayer( layer );
    mGatherer->start();
    if ( mGatherer->isRunning() )
    {
      //stop any currently running task
      mGatherer->stop();
      while ( mGatherer->isRunning() )
      {
        QCoreApplication::processEvents();
      }
    }
    mGatherer->start();
  }
  else
  {
    updateHistogram();
  }
}

void QgsCurveEditorWidget::setMinHistogramValueRange( double minValueRange )
{
  mMinValueRange = minValueRange;
  updateHistogram();
}

void QgsCurveEditorWidget::setMaxHistogramValueRange( double maxValueRange )
{
  mMaxValueRange = maxValueRange;
  updateHistogram();
}

void QgsCurveEditorWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace )
  {
    QList< QgsPointXY > cp = mCurve.controlPoints();
    if ( mCurrentPlotMarkerIndex > 0 && mCurrentPlotMarkerIndex < cp.count() - 1 )
    {
      cp.removeAt( mCurrentPlotMarkerIndex );
      mCurve.setControlPoints( cp );
      updatePlot();
      emit changed();
    }
  }
}

void QgsCurveEditorWidget::plotMousePress( QPointF point )
{
  mCurrentPlotMarkerIndex = findNearestControlPoint( point );
  if ( mCurrentPlotMarkerIndex < 0 )
  {
    // add a new point
    mCurve.addControlPoint( point.x(), point.y() );
    mCurrentPlotMarkerIndex = findNearestControlPoint( point );
    emit changed();
  }
  updatePlot();
}


int QgsCurveEditorWidget::findNearestControlPoint( QPointF point ) const
{
  double minDist = 3.0 / mPlot->width();
  int currentPlotMarkerIndex = -1;

  const QList< QgsPointXY > controlPoints = mCurve.controlPoints();

  for ( int i = 0; i < controlPoints.count(); ++i )
  {
    const QgsPointXY currentPoint = controlPoints.at( i );
    double currentDist;
    currentDist = std::pow( point.x() - currentPoint.x(), 2.0 ) + std::pow( point.y() - currentPoint.y(), 2.0 );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      currentPlotMarkerIndex = i;
    }
  }
  return currentPlotMarkerIndex;
}


void QgsCurveEditorWidget::plotMouseRelease( QPointF )
{
}

void QgsCurveEditorWidget::plotMouseMove( QPointF point )
{
  if ( mCurrentPlotMarkerIndex < 0 )
    return;

  QList< QgsPointXY > cp = mCurve.controlPoints();
  bool removePoint = false;
  if ( mCurrentPlotMarkerIndex == 0 )
  {
    point.setX( std::min( point.x(), cp.at( 1 ).x() - 0.01 ) );
  }
  else
  {
    removePoint = point.x() <= cp.at( mCurrentPlotMarkerIndex - 1 ).x();
  }
  if ( mCurrentPlotMarkerIndex == cp.count() - 1 )
  {
    point.setX( std::max( point.x(), cp.at( mCurrentPlotMarkerIndex - 1 ).x() + 0.01 ) );
    removePoint = false;
  }
  else
  {
    removePoint = removePoint || point.x() >= cp.at( mCurrentPlotMarkerIndex + 1 ).x();
  }

  if ( removePoint )
  {
    cp.removeAt( mCurrentPlotMarkerIndex );
    mCurrentPlotMarkerIndex = -1;
  }
  else
  {
    cp[ mCurrentPlotMarkerIndex ] = QgsPointXY( point.x(), point.y() );
  }
  mCurve.setControlPoints( cp );
  updatePlot();
  emit changed();
}

void QgsCurveEditorWidget::addPlotMarker( double x, double y, bool isSelected )
{
  const QColor borderColor( 0, 0, 0 );

  const QColor brushColor = isSelected ? borderColor : QColor( 255, 255, 255, 0 );

  QwtPlotMarker *marker = new QwtPlotMarker();
  marker->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,  QBrush( brushColor ), QPen( borderColor, isSelected ? 2 : 1 ), QSize( 8, 8 ) ) );
  marker->setValue( x, y );
  marker->attach( mPlot );
  marker->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mMarkers << marker;
}

void QgsCurveEditorWidget::updateHistogram()
{
  if ( !mHistogram )
    return;

  //draw histogram
  const QBrush histoBrush( QColor( 0, 0, 0, 70 ) );

  delete mPlotHistogram;
  mPlotHistogram = createPlotHistogram( histoBrush );
  QVector<QwtIntervalSample> dataHisto;

  const int bins = 40;
  QList<double> edges = mHistogram->binEdges( bins );
  const QList<int> counts = mHistogram->counts( bins );

  // scale counts to 0->1
  const double max = *std::max_element( counts.constBegin(), counts.constEnd() );

  // scale bin edges to fit in 0->1 range
  if ( !qgsDoubleNear( mMinValueRange, mMaxValueRange ) )
  {
    std::transform( edges.begin(), edges.end(), edges.begin(),
                    [this]( double d ) -> double { return ( d - mMinValueRange ) / ( mMaxValueRange - mMinValueRange ); } );
  }

  for ( int bin = 0; bin < bins; ++bin )
  {
    const double binValue = counts.at( bin ) / max;

    const double upperEdge = edges.at( bin + 1 );

    dataHisto << QwtIntervalSample( binValue, edges.at( bin ), upperEdge );
  }

  mPlotHistogram->setSamples( dataHisto );
  mPlotHistogram->attach( mPlot );
  mPlot->replot();
}

void QgsCurveEditorWidget::updatePlot()
{
  // remove existing markers
  const auto constMMarkers = mMarkers;
  for ( QwtPlotMarker *marker : constMMarkers )
  {
    marker->detach();
    delete marker;
  }
  mMarkers.clear();

  QPolygonF curvePoints;
  QVector< double > x;

  int i = 0;
  const auto constControlPoints = mCurve.controlPoints();
  for ( const QgsPointXY &point : constControlPoints )
  {
    x << point.x();
    addPlotMarker( point.x(), point.y(), mCurrentPlotMarkerIndex == i );
    i++;
  }

  //add extra intermediate points

  for ( double p = 0; p <= 1.0; p += 0.01 )
  {
    x << p;
  }
  std::sort( x.begin(), x.end() );
  const QVector< double > y = mCurve.y( x );

  for ( int j = 0; j < x.count(); ++j )
  {
    curvePoints << QPointF( x.at( j ), y.at( j ) );
  }

  mPlotCurve->setSamples( curvePoints );
  mPlot->replot();
}

QwtPlotHistogram *QgsCurveEditorWidget::createPlotHistogram( const QBrush &brush, const QPen &pen ) const
{
  QwtPlotHistogram *histogram = new QwtPlotHistogram( QString() );
  histogram->setBrush( brush );
  if ( pen != Qt::NoPen )
  {
    histogram->setPen( pen );
  }
  else if ( brush.color().lightness() > 200 )
  {
    QPen p;
    p.setColor( brush.color().darker( 150 ) );
    p.setWidth( 0 );
    p.setCosmetic( true );
    histogram->setPen( p );
  }
  else
  {
    histogram->setPen( QPen( Qt::NoPen ) );
  }
  return histogram;
}

/// @cond PRIVATE

QgsCurveEditorPlotEventFilter::QgsCurveEditorPlotEventFilter( QwtPlot *plot )
  : QObject( plot )
  , mPlot( plot )
{
  mPlot->canvas()->installEventFilter( this );
}

bool QgsCurveEditorPlotEventFilter::eventFilter( QObject *object, QEvent *event )
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
        emit mousePress( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseMove:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->buttons() & Qt::LeftButton )
      {
        // only emit when button pressed
        emit mouseMove( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mouseRelease( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    default:
      break;
  }

  return QObject::eventFilter( object, event );
}

QPointF QgsCurveEditorPlotEventFilter::mapPoint( QPointF point ) const
{
  if ( !mPlot )
    return QPointF();

  return QPointF( mPlot->canvasMap( QwtPlot::xBottom ).invTransform( point.x() ),
                  mPlot->canvasMap( QwtPlot::yLeft ).invTransform( point.y() ) );
}


///@endcond
