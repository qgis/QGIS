/***************************************************************************
                         qgshistogramwidget.cpp
                         ----------------------
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

#include "qgshistogramwidget.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsstatisticalsummary.h"
#include "qgssettings.h"

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


QgsHistogramWidget::QgsHistogramWidget( QWidget *parent, QgsVectorLayer *layer, const QString &fieldOrExp )
  : QWidget( parent )
  , mVectorLayer( layer )
  , mSourceFieldExp( fieldOrExp )
  , mXAxisTitle( QObject::tr( "Value" ) )
  , mYAxisTitle( QObject::tr( "Count" ) )
{
  setupUi( this );

  mPlot = mpPlot;

  // hide the ugly canvas frame
  QFrame *plotCanvasFrame = dynamic_cast<QFrame *>( mpPlot->canvas() );
  if ( plotCanvasFrame )
    plotCanvasFrame->setFrameStyle( QFrame::NoFrame );

  QgsSettings settings;
  mMeanCheckBox->setChecked( settings.value( QStringLiteral( "HistogramWidget/showMean" ), false ).toBool() );
  mStdevCheckBox->setChecked( settings.value( QStringLiteral( "HistogramWidget/showStdev" ), false ).toBool() );

  connect( mBinsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsHistogramWidget::refresh );
  connect( mMeanCheckBox, &QAbstractButton::toggled, this, &QgsHistogramWidget::refresh );
  connect( mStdevCheckBox, &QAbstractButton::toggled, this, &QgsHistogramWidget::refresh );
  connect( mLoadValuesButton, &QAbstractButton::clicked, this, &QgsHistogramWidget::refreshValues );

  mGridPen = QPen( QColor( 0, 0, 0, 40 ) );
  mMeanPen = QPen( QColor( 10, 10, 10, 220 ) );
  mMeanPen.setStyle( Qt::DashLine );
  mStdevPen = QPen( QColor( 30, 30, 30, 200 ) );
  mStdevPen.setStyle( Qt::DashLine );

  if ( layer && !mSourceFieldExp.isEmpty() )
  {
    refresh();
  }
}

QgsHistogramWidget::~QgsHistogramWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "HistogramWidget/showMean" ), mMeanCheckBox->isChecked() );
  settings.setValue( QStringLiteral( "HistogramWidget/showStdev" ), mStdevCheckBox->isChecked() );
}

static bool _rangesByLower( const QgsRendererRange &a, const QgsRendererRange &b )
{
  return a.lowerValue() < b.lowerValue();
}

void QgsHistogramWidget::setGraduatedRanges( const QgsRangeList &ranges )
{
  mRanges = ranges;
  std::sort( mRanges.begin(), mRanges.end(), _rangesByLower );
}

void QgsHistogramWidget::refreshValues()
{
  mValues.clear();

  if ( !mVectorLayer || mSourceFieldExp.isEmpty() )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  bool ok;
  mValues = QgsVectorLayerUtils::getDoubleValues( mVectorLayer, mSourceFieldExp, ok );

  if ( ! ok )
  {
    QApplication::restoreOverrideCursor();
    return;
  }


  std::sort( mValues.begin(), mValues.end() );
  mHistogram.setValues( mValues );
  mBinsSpinBox->blockSignals( true );
  mBinsSpinBox->setValue( std::max( mHistogram.optimalNumberBins(), 30 ) );
  mBinsSpinBox->blockSignals( false );

  mStats.setStatistics( QgsStatisticalSummary::StDev );
  mStats.calculate( mValues );

  mpPlot->setEnabled( true );
  mMeanCheckBox->setEnabled( true );
  mStdevCheckBox->setEnabled( true );
  mBinsSpinBox->setEnabled( true );

  QApplication::restoreOverrideCursor();

  //also force a redraw
  refresh();
}

void QgsHistogramWidget::refresh()
{
  drawHistogram();
}

void QgsHistogramWidget::setLayer( QgsVectorLayer *layer )
{
  if ( layer == mVectorLayer )
    return;

  mVectorLayer = layer;
  clearHistogram();
}

void QgsHistogramWidget::clearHistogram()
{
  mValues.clear();
  mHistogram.setValues( mValues );
  refresh();

  mpPlot->setEnabled( false );
  mMeanCheckBox->setEnabled( false );
  mStdevCheckBox->setEnabled( false );
  mBinsSpinBox->setEnabled( false );
}

void QgsHistogramWidget::setSourceFieldExp( const QString &fieldOrExp )
{
  if ( fieldOrExp == mSourceFieldExp )
    return;

  mSourceFieldExp = fieldOrExp;
  clearHistogram();
}

void QgsHistogramWidget::drawHistogram()
{
  // clear plot
  mpPlot->detachItems();

  //ensure all children get removed
  mpPlot->setAutoDelete( true );
  // Set axis titles
  if ( !mXAxisTitle.isEmpty() )
    mpPlot->setAxisTitle( QwtPlot::xBottom, mXAxisTitle );
  if ( !mYAxisTitle.isEmpty() )
    mpPlot->setAxisTitle( QwtPlot::yLeft, mYAxisTitle );
  mpPlot->setAxisFont( QwtPlot::xBottom, this->font() );
  mpPlot->setAxisFont( QwtPlot::yLeft, this->font() );
  QFont titleFont = this->font();
  titleFont.setBold( true );
  QwtText xAxisText = mpPlot->axisTitle( QwtPlot::xBottom );
  xAxisText.setFont( titleFont );
  mpPlot->setAxisTitle( QwtPlot::xBottom, xAxisText );
  QwtText yAxisText = mpPlot->axisTitle( QwtPlot::yLeft );
  yAxisText.setFont( titleFont );
  mpPlot->setAxisTitle( QwtPlot::yLeft, yAxisText );
  mpPlot->setAxisAutoScale( QwtPlot::yLeft );
  mpPlot->setAxisAutoScale( QwtPlot::xBottom );

  // add a grid
  QwtPlotGrid *grid = new QwtPlotGrid();
  grid->enableX( false );
  grid->setPen( mGridPen );
  grid->attach( mpPlot );

  // make colors list
  mHistoColors.clear();
  for ( const QgsRendererRange &range : qgis::as_const( mRanges ) )
  {
    mHistoColors << ( range.symbol() ? range.symbol()->color() : Qt::black );
  }

  //draw histogram
  QwtPlotHistogram *plotHistogram = nullptr;
  plotHistogram = createPlotHistogram( !mRanges.isEmpty() ? mRanges.at( 0 ).label() : QString(),
                                       !mRanges.isEmpty() ? QBrush( mHistoColors.at( 0 ) ) : mBrush,
                                       !mRanges.isEmpty() ? Qt::NoPen : mPen );
  QVector<QwtIntervalSample> dataHisto;

  int bins = mBinsSpinBox->value();
  QList<double> edges = mHistogram.binEdges( bins );
  QList<int> counts = mHistogram.counts( bins );

  int rangeIndex = 0;
  int lastValue = 0;

  for ( int bin = 0; bin < bins; ++bin )
  {
    int binValue = counts.at( bin );

    //current bin crosses two graduated ranges, so we split between
    //two histogram items
    if ( rangeIndex < mRanges.count() - 1 && edges.at( bin ) > mRanges.at( rangeIndex ).upperValue() )
    {
      rangeIndex++;
      plotHistogram->setSamples( dataHisto );
      plotHistogram->attach( mpPlot );
      plotHistogram = createPlotHistogram( mRanges.at( rangeIndex ).label(), mHistoColors.at( rangeIndex ) );
      dataHisto.clear();
      dataHisto << QwtIntervalSample( lastValue, mRanges.at( rangeIndex - 1 ).upperValue(), edges.at( bin ) );
    }

    double upperEdge = !mRanges.isEmpty() ? std::min( edges.at( bin + 1 ), mRanges.at( rangeIndex ).upperValue() )
                       : edges.at( bin + 1 );

    dataHisto << QwtIntervalSample( binValue, edges.at( bin ), upperEdge );

    lastValue = binValue;
  }

  plotHistogram->setSamples( dataHisto );
  plotHistogram->attach( mpPlot );

  mRangeMarkers.clear();
  for ( const QgsRendererRange &range : qgis::as_const( mRanges ) )
  {
    QwtPlotMarker *rangeMarker = new QwtPlotMarker();
    rangeMarker->attach( mpPlot );
    rangeMarker->setLineStyle( QwtPlotMarker::VLine );
    rangeMarker->setXValue( range.upperValue() );
    rangeMarker->setLabel( QString::number( range.upperValue() ) );
    rangeMarker->setLabelOrientation( Qt::Vertical );
    rangeMarker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    rangeMarker->show();
    mRangeMarkers << rangeMarker;
  }

  if ( mMeanCheckBox->isChecked() )
  {
    QwtPlotMarker *meanMarker = new QwtPlotMarker();
    meanMarker->attach( mpPlot );
    meanMarker->setLineStyle( QwtPlotMarker::VLine );
    meanMarker->setLinePen( mMeanPen );
    meanMarker->setXValue( mStats.mean() );
    meanMarker->setLabel( QString( QChar( 956 ) ) );
    meanMarker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    meanMarker->show();
  }

  if ( mStdevCheckBox->isChecked() )
  {
    QwtPlotMarker *stdev1Marker = new QwtPlotMarker();
    stdev1Marker->attach( mpPlot );
    stdev1Marker->setLineStyle( QwtPlotMarker::VLine );
    stdev1Marker->setLinePen( mStdevPen );
    stdev1Marker->setXValue( mStats.mean() - mStats.stDev() );
    stdev1Marker->setLabel( QString( QChar( 963 ) ) );
    stdev1Marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    stdev1Marker->show();

    QwtPlotMarker *stdev2Marker = new QwtPlotMarker();
    stdev2Marker->attach( mpPlot );
    stdev2Marker->setLineStyle( QwtPlotMarker::VLine );
    stdev2Marker->setLinePen( mStdevPen );
    stdev2Marker->setXValue( mStats.mean() + mStats.stDev() );
    stdev2Marker->setLabel( QString( QChar( 963 ) ) );
    stdev2Marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    stdev2Marker->show();
  }

  mpPlot->setEnabled( true );
  mpPlot->replot();
}

QwtPlotHistogram *QgsHistogramWidget::createPlotHistogram( const QString &title, const QBrush &brush, const QPen &pen ) const
{
  QwtPlotHistogram *histogram = new QwtPlotHistogram( title );
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

