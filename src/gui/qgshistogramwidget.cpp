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


QgsHistogramWidget::QgsHistogramWidget( QWidget *parent, QgsVectorLayer* layer, const QString& fieldOrExp )
    : QWidget( parent )
    , mVectorLayer( layer )
    , mSourceFieldExp( fieldOrExp )
    , mXAxisTitle( QObject::tr( "Value" ) )
    , mYAxisTitle( QObject::tr( "Count" ) )
{
  setupUi( this );

  mPlot = mpPlot;

  // hide the ugly canvas frame
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  QFrame* plotCanvasFrame = dynamic_cast<QFrame*>( mpPlot->canvas() );
  if ( plotCanvasFrame )
    plotCanvasFrame->setFrameStyle( QFrame::NoFrame );
#else
  mpPlot->canvas()->setFrameStyle( QFrame::NoFrame );
#endif

  QSettings settings;
  mMeanCheckBox->setChecked( settings.value( "/HistogramWidget/showMean", false ).toBool() );
  mStdevCheckBox->setChecked( settings.value( "/HistogramWidget/showStdev", false ).toBool() );

  connect( mBinsSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( refresh() ) );
  connect( mMeanCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( refresh() ) );
  connect( mStdevCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( refresh() ) );
  connect( mLoadValuesButton, SIGNAL( clicked() ), this, SLOT( refreshValues() ) );

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
  QSettings settings;
  settings.setValue( "/HistogramWidget/showMean", mMeanCheckBox->isChecked() );
  settings.setValue( "/HistogramWidget/showStdev", mStdevCheckBox->isChecked() );
}

static bool _rangesByLower( const QgsRendererRangeV2& a, const QgsRendererRangeV2& b )
{
  return a.lowerValue() < b.lowerValue() ? -1 : 0;
}

void QgsHistogramWidget::setGraduatedRanges( const QgsRangeList &ranges )
{
  mRanges = ranges;
  qSort( mRanges.begin(), mRanges.end(), _rangesByLower );
}

void QgsHistogramWidget::refreshValues()
{
  mValues.clear();

  if ( !mVectorLayer || mSourceFieldExp.isEmpty() )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  bool ok;
  mValues = mVectorLayer->getDoubleValues( mSourceFieldExp, ok );

  if ( ! ok )
  {
    QApplication::restoreOverrideCursor();
    return;
  }


  qSort( mValues.begin(), mValues.end() );
  mHistogram.setValues( mValues );
  mBinsSpinBox->blockSignals( true );
  mBinsSpinBox->setValue( qMax( mHistogram.optimalNumberBins(), 30 ) );
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
  QwtPlotGrid * grid = new QwtPlotGrid();
  grid->enableX( false );
  grid->setPen( mGridPen );
  grid->attach( mpPlot );

  // make colors list
  mHistoColors.clear();
  Q_FOREACH ( const QgsRendererRangeV2& range, mRanges )
  {
    mHistoColors << ( range.symbol() ? range.symbol()->color() : Qt::black );
  }

  //draw histogram
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  QwtPlotHistogram * plotHistogram = 0;
  plotHistogram = createPlotHistogram( mRanges.count() > 0 ? mRanges.at( 0 ).label() : QString(),
                                       mRanges.count() > 0 ? QBrush( mHistoColors.at( 0 ) ) : mBrush,
                                       mRanges.count() > 0 ? Qt::NoPen : mPen );
#else
  HistogramItem *plotHistogramItem = 0;
  plotHistogramItem = createHistoItem( mRanges.count() > 0 ? mRanges.at( 0 ).label() : QString(),
                                       mRanges.count() > 0 ? QBrush( mHistoColors.at( 0 ) ) : mBrush,
                                       mRanges.count() > 0 ? Qt::NoPen : mPen );
#endif

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  QVector<QwtIntervalSample> dataHisto;
#else

  // we safely assume that QT>=4.0 (min version is 4.7), therefore QwtArray is a QVector, so don't set size here
  QwtArray<QwtDoubleInterval> intervalsHisto;
  QwtArray<double> valuesHisto;

#endif

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
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
      plotHistogram->setSamples( dataHisto );
      plotHistogram->attach( mpPlot );
      plotHistogram = createPlotHistogram( mRanges.at( rangeIndex ).label(), mHistoColors.at( rangeIndex ) );
      dataHisto.clear();
      dataHisto << QwtIntervalSample( lastValue, mRanges.at( rangeIndex - 1 ).upperValue(), edges.at( bin ) );
#else
      plotHistogramItem->setData( QwtIntervalData( intervalsHisto, valuesHisto ) );
      plotHistogramItem->attach( mpPlot );
      plotHistogramItem = createHistoItem( mRanges.at( rangeIndex ).label(), mHistoColors.at( rangeIndex ) );
      intervalsHisto.clear();
      valuesHisto.clear();
      intervalsHisto.append( QwtDoubleInterval( mRanges.at( rangeIndex - 1 ).upperValue(), edges.at( bin ) ) );
      valuesHisto.append( lastValue );
#endif
    }

    double upperEdge = mRanges.count() > 0 ? qMin( edges.at( bin + 1 ), mRanges.at( rangeIndex ).upperValue() )
                       : edges.at( bin + 1 );

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    dataHisto << QwtIntervalSample( binValue, edges.at( bin ), upperEdge );
#else
    intervalsHisto.append( QwtDoubleInterval( edges.at( bin ), upperEdge ) );
    valuesHisto.append( double( binValue ) );
#endif

    lastValue = binValue;
  }

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  plotHistogram->setSamples( dataHisto );
  plotHistogram->attach( mpPlot );
#else
  plotHistogramItem->setData( QwtIntervalData( intervalsHisto, valuesHisto ) );
  plotHistogramItem->attach( mpPlot );
#endif

  mRangeMarkers.clear();
  Q_FOREACH ( const QgsRendererRangeV2& range, mRanges )
  {
    QwtPlotMarker* rangeMarker = new QwtPlotMarker();
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
    QwtPlotMarker* meanMarker = new QwtPlotMarker();
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
    QwtPlotMarker* stdev1Marker = new QwtPlotMarker();
    stdev1Marker->attach( mpPlot );
    stdev1Marker->setLineStyle( QwtPlotMarker::VLine );
    stdev1Marker->setLinePen( mStdevPen );
    stdev1Marker->setXValue( mStats.mean() - mStats.stDev() );
    stdev1Marker->setLabel( QString( QChar( 963 ) ) );
    stdev1Marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    stdev1Marker->show();

    QwtPlotMarker* stdev2Marker = new QwtPlotMarker();
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

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
QwtPlotHistogram* QgsHistogramWidget::createPlotHistogram( const QString& title, const QBrush& brush, const QPen& pen ) const
{
  QwtPlotHistogram* histogram = new QwtPlotHistogram( title );
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
#else
HistogramItem * QgsHistogramWidget::createHistoItem( const QString& title, const QBrush& brush, const QPen& pen ) const
{
  HistogramItem* item = new HistogramItem( title );
  item->setColor( brush.color() );
  item->setFlat( true );
  item->setSpacing( 0 );
  if ( pen != Qt::NoPen )
  {
    item->setPen( pen );
  }
  else if ( brush.color().lightness() > 200 )
  {
    QPen p;
    p.setColor( brush.color().darker( 150 ) );
    p.setWidth( 0 );
    p.setCosmetic( true );
    item->setPen( p );
  }
  return item;
}
#endif

