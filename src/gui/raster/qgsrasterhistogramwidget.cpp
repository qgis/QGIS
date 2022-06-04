/***************************************************************************
                         qgsrasterhistogramwidget.cpp
                         ---------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterrendererwidget.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasterminmaxwidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsdoublevalidator.h"
#include "qgssettings.h"

#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QActionGroup>
#include <QRandomGenerator>

// QWT Charting widget
#include <qwt_global.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_histogram.h>
#include <qwt_scale_div.h>

#ifdef Q_OS_WIN
#include <time.h>
#endif

constexpr int SAMPLE_SIZE = 250000; // number of sample cells

QgsRasterHistogramWidget::QgsRasterHistogramWidget( QgsRasterLayer *lyr, QWidget *parent )
  : QgsMapLayerConfigWidget( lyr, nullptr, parent )
  , mRasterLayer( lyr )

{
  setupUi( this );
  connect( mSaveAsImageButton, &QToolButton::clicked, this, &QgsRasterHistogramWidget::mSaveAsImageButton_clicked );
  connect( cboHistoBand, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterHistogramWidget::cboHistoBand_currentIndexChanged );
  connect( btnHistoMin, &QToolButton::toggled, this, &QgsRasterHistogramWidget::btnHistoMin_toggled );
  connect( btnHistoMax, &QToolButton::toggled, this, &QgsRasterHistogramWidget::btnHistoMax_toggled );
  connect( btnHistoCompute, &QPushButton::clicked, this, &QgsRasterHistogramWidget::btnHistoCompute_clicked );

  mSaveAsImageButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileSave.svg" ) ) );

  mRendererWidget = nullptr;
  mRendererName = QStringLiteral( "singlebandgray" );

  mHistoMin = 0;
  mHistoMax = 0;

  mHistoPicker = nullptr;
  mHistoZoomer = nullptr;
  mHistoMarkerMin = nullptr;
  mHistoMarkerMax = nullptr;

  const QgsSettings settings;
  mHistoShowMarkers = settings.value( QStringLiteral( "Raster/histogram/showMarkers" ), false ).toBool();
  // mHistoLoadApplyAll = settings.value( "/Raster/histogram/loadApplyAll", false ).toBool();
  mHistoZoomToMinMax = settings.value( QStringLiteral( "Raster/histogram/zoomToMinMax" ), false ).toBool();
  mHistoUpdateStyleToMinMax = settings.value( QStringLiteral( "Raster/histogram/updateStyleToMinMax" ), true ).toBool();
  mHistoDrawLines = settings.value( QStringLiteral( "Raster/histogram/drawLines" ), true ).toBool();
  // mHistoShowBands = (HistoShowBands) settings.value( "/Raster/histogram/showBands", (int) ShowAll ).toInt();
  mHistoShowBands = ShowAll;

  bool isInt = true;
  if ( true )
  {
    //band selector
    const int myBandCountInt = mRasterLayer->bandCount();
    for ( int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt )
    {
      cboHistoBand->addItem( mRasterLayer->bandName( myIteratorInt ) );
      const Qgis::DataType mySrcDataType = mRasterLayer->dataProvider()->sourceDataType( myIteratorInt );
      if ( !( mySrcDataType == Qgis::DataType::Byte ||
              mySrcDataType == Qgis::DataType::Int16 || mySrcDataType == Qgis::DataType::Int32 ||
              mySrcDataType == Qgis::DataType::UInt16 || mySrcDataType == Qgis::DataType::UInt32 ) )
        isInt = false;
    }

    // histo min/max selectors
    leHistoMin->setValidator( new QgsDoubleValidator( this ) );
    leHistoMax->setValidator( new QgsDoubleValidator( this ) );
    // this might generate many refresh events! test..
    // connect( leHistoMin, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateHistoMarkers() ) );
    // connect( leHistoMax, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateHistoMarkers() ) );
    // connect( leHistoMin, SIGNAL( textChanged( const QString & ) ), this, SLOT( applyHistoMin() ) );
    // connect( leHistoMax, SIGNAL( textChanged( const QString & ) ), this, SLOT( applyHistoMax() ) );
    connect( leHistoMin, &QLineEdit::editingFinished, this, &QgsRasterHistogramWidget::applyHistoMin );
    connect( leHistoMax, &QLineEdit::editingFinished, this, &QgsRasterHistogramWidget::applyHistoMax );

    // histo actions
    // TODO move/add options to qgis options dialog
    QMenu *menu = new QMenu( this );
    menu->setSeparatorsCollapsible( false );
    btnHistoActions->setMenu( menu );
    QActionGroup *group = nullptr;
    QAction *action = nullptr;

    // min/max options
    group = new QActionGroup( this );
    group->setExclusive( false );
    connect( group, &QActionGroup::triggered, this, &QgsRasterHistogramWidget::histoActionTriggered );
    action = new QAction( tr( "Min/Max options" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    action = new QAction( tr( "Always show min/max markers" ), group );
    action->setData( QVariant( "Show markers" ) );
    action->setCheckable( true );
    action->setChecked( mHistoShowMarkers );
    menu->addAction( action );
    action = new QAction( tr( "Zoom to min/max" ), group );
    action->setData( QVariant( "Zoom min_max" ) );
    action->setCheckable( true );
    action->setChecked( mHistoZoomToMinMax );
    menu->addAction( action );
    action = new QAction( tr( "Update style to min/max" ), group );
    action->setData( QVariant( "Update min_max" ) );
    action->setCheckable( true );
    action->setChecked( mHistoUpdateStyleToMinMax );
    menu->addAction( action );

    // visibility options
    group = new QActionGroup( this );
    group->setExclusive( false );
    connect( group, &QActionGroup::triggered, this, &QgsRasterHistogramWidget::histoActionTriggered );
    action = new QAction( tr( "Visibility" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    group = new QActionGroup( this );
    group->setExclusive( true ); // these options are exclusive
    connect( group, &QActionGroup::triggered, this, &QgsRasterHistogramWidget::histoActionTriggered );
    action = new QAction( tr( "Show all bands" ), group );
    action->setData( QVariant( "Show all" ) );
    action->setCheckable( true );
    action->setChecked( mHistoShowBands == ShowAll );
    menu->addAction( action );
    action = new QAction( tr( "Show RGB/Gray band(s)" ), group );
    action->setData( QVariant( "Show RGB" ) );
    action->setCheckable( true );
    action->setChecked( mHistoShowBands == ShowRGB );
    menu->addAction( action );
    action = new QAction( tr( "Show selected band" ), group );
    action->setData( QVariant( "Show selected" ) );
    action->setCheckable( true );
    action->setChecked( mHistoShowBands == ShowSelected );
    menu->addAction( action );

    // display options
    group = new QActionGroup( this );
    group->setExclusive( false );
    connect( group, &QActionGroup::triggered, this, &QgsRasterHistogramWidget::histoActionTriggered );
    action = new QAction( tr( "Display" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    // should we plot as histogram instead of line plot? (int data only)
    action = new QAction( QString(), group );
    action->setData( QVariant( "Draw lines" ) );
    if ( isInt )
    {
      action->setText( tr( "Draw as lines" ) );
      action->setCheckable( true );
      action->setChecked( mHistoDrawLines );
    }
    else
    {
      action->setText( tr( "Draw as lines (only int layers)" ) );
      action->setEnabled( false );
    }
    menu->addAction( action );

    // actions
    action = new QAction( tr( "Actions" ), group );
    action->setSeparator( true );
    menu->addAction( action );

    // load actions
    group = new QActionGroup( this );
    group->setExclusive( false );
    connect( group, &QActionGroup::triggered, this, &QgsRasterHistogramWidget::histoActionTriggered );
    action = new QAction( tr( "Reset" ), group );
    action->setData( QVariant( "Load reset" ) );
    menu->addAction( action );

    // these actions have been disabled for api cleanup, restore them eventually
    // TODO restore these in qgis 2.4
#if 0
    // Load min/max needs 3 params (method, extent, accuracy), cannot put it in single item
    action = new QAction( tr( "Load min/max" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    action = new QAction( tr( "Estimate (faster)" ), group );
    action->setData( QVariant( "Load estimate" ) );
    menu->addAction( action );
    action = new QAction( tr( "Actual (slower)" ), group );
    action->setData( QVariant( "Load actual" ) );
    menu->addAction( action );
    action = new QAction( tr( "Current extent" ), group );
    action->setData( QVariant( "Load extent" ) );
    menu->addAction( action );
    action = new QAction( tr( "Use stddev (1.0)" ), group );
    action->setData( QVariant( "Load 1 stddev" ) );
    menu->addAction( action );
    action = new QAction( tr( "Use stddev (custom)" ), group );
    action->setData( QVariant( "Load stddev" ) );
    menu->addAction( action );
    action = new QAction( tr( "Load for each band" ), group );
    action->setData( QVariant( "Load apply all" ) );
    action->setCheckable( true );
    action->setChecked( mHistoLoadApplyAll );
    menu->addAction( action );
#endif

    //others
    action = new QAction( tr( "Recompute Histogram" ), group );
    action->setData( QVariant( "Compute histogram" ) );
    menu->addAction( action );

  }

} // QgsRasterHistogramWidget ctor

void QgsRasterHistogramWidget::setRendererWidget( const QString &name, QgsRasterRendererWidget *rendererWidget )
{
  mRendererName = name;
  mRendererWidget = rendererWidget;
  refreshHistogram();
  cboHistoBand_currentIndexChanged( -1 );
}

void QgsRasterHistogramWidget::setActive( bool activeFlag )
{
  if ( activeFlag )
  {
    refreshHistogram();
    cboHistoBand_currentIndexChanged( -1 );
  }
  else
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    btnHistoMin->setChecked( false );
    btnHistoMax->setChecked( false );
  }
}

void QgsRasterHistogramWidget::btnHistoCompute_clicked()
{
// Histogram computation can be called either by clicking the "Compute Histogram" button
// which is only visible if there is no cached histogram or by calling the
// "Compute Histogram" action. Due to limitations in the gdal api, it is not possible
// to re-calculate the histogram if it has already been calculated
  computeHistogram( true );
  refreshHistogram();
}

// Compute the number of bins
// Logic partially borrowed to QgsRasterInterface::initHistogram(),
// but with a limitation to 1000 bins. Otherwise the histogram will be
// unreadable (see https://github.com/qgis/QGIS/issues/38298)
// NOTE: the number of bins should probably be let to the user, and/or adaptative
// to the width in pixels of the chart.
static int getBinCount( QgsRasterInterface *rasterInterface,
                        int bandNo,
                        int sampleSize )
{
  const Qgis::DataType mySrcDataType = rasterInterface->sourceDataType( bandNo );
  const double statsMin = mySrcDataType == Qgis::DataType::Byte ? 0 :
                          rasterInterface->bandStatistics( bandNo, QgsRasterBandStats::Min, QgsRectangle(), sampleSize ).minimumValue;
  const double statsMax = mySrcDataType == Qgis::DataType::Byte ? 255 :
                          rasterInterface->bandStatistics( bandNo, QgsRasterBandStats::Max, QgsRectangle(), sampleSize ).maximumValue;
  const QgsRectangle extent( rasterInterface->extent() );

  // Calc resolution from sampleSize
  double xRes, yRes;
  xRes = yRes = std::sqrt( ( static_cast<double>( extent.width( ) ) * extent.height() ) / sampleSize );

  // But limit by physical resolution
  if ( rasterInterface->capabilities() & QgsRasterInterface::Size )
  {
    const double srcXRes = extent.width() / rasterInterface->xSize();
    const double srcYRes = extent.height() / rasterInterface->ySize();
    if ( xRes < srcXRes ) xRes = srcXRes;
    if ( yRes < srcYRes ) yRes = srcYRes;
  }

  const int histogramWidth = static_cast <int>( extent.width() / xRes );
  const int histogramHeight = static_cast <int>( extent.height() / yRes );

  int binCount = static_cast<int>( std::min( static_cast<qint64>( 1000 ),
                                   static_cast<qint64>( histogramWidth ) * histogramHeight ) );

  if ( mySrcDataType == Qgis::DataType::Int16 || mySrcDataType == Qgis::DataType::Int32 ||
       mySrcDataType == Qgis::DataType::UInt16 || mySrcDataType == Qgis::DataType::UInt32 )
  {
    binCount = static_cast<int>( std::min( static_cast<qint64>( binCount ),
                                           static_cast<qint64>( std::ceil( statsMax - statsMin + 1 ) ) ) );
  }

  return binCount;
}

bool QgsRasterHistogramWidget::computeHistogram( bool forceComputeFlag )
{

  //bool myIgnoreOutOfRangeFlag = true;
  //bool myThoroughBandScanFlag = false;
  const int myBandCountInt = mRasterLayer->bandCount();

  // if forceComputeFlag = false make sure raster has cached histogram, else return false
  if ( ! forceComputeFlag )
  {
    for ( int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt )
    {
      const int sampleSize = SAMPLE_SIZE; // number of sample cells
      const int binCount = getBinCount( mRasterLayer->dataProvider(), myIteratorInt, sampleSize );
      if ( !mRasterLayer->dataProvider()->hasHistogram( myIteratorInt, binCount, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize ) )
      {
        QgsDebugMsg( QStringLiteral( "band %1 does not have cached histo" ).arg( myIteratorInt ) );
        return false;
      }
    }
  }

  // compute histogram
  stackedWidget2->setCurrentIndex( 1 );

  const std::unique_ptr< QgsRasterBlockFeedback > feedback( new QgsRasterBlockFeedback() );
  connect( feedback.get(), &QgsRasterBlockFeedback::progressChanged, mHistogramProgress, &QProgressBar::setValue );
  QApplication::setOverrideCursor( Qt::WaitCursor );

  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    const int sampleSize = SAMPLE_SIZE; // number of sample cells
    const int binCount = getBinCount( mRasterLayer->dataProvider(), myIteratorInt, sampleSize );
    mRasterLayer->dataProvider()->histogram( myIteratorInt, binCount, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize, false, feedback.get() );
  }

  // mHistogramProgress->hide();
  stackedWidget2->setCurrentIndex( 0 );
  QApplication::restoreOverrideCursor();

  return true;
}


void QgsRasterHistogramWidget::refreshHistogram()
{
  // Explanation:
  // We use the gdal histogram creation routine is called for each selected
  // layer. Currently the hist is hardcoded to create 256 bins. Each bin stores
  // the total number of cells that fit into the range defined by that bin.
  //
  // The graph routine below determines the greatest number of pixels in any given
  // bin in all selected layers, and the min. It then draws a scaled line between min
  // and max - scaled to image height. 1 line drawn per selected band
  //
  const int myBandCountInt = mRasterLayer->bandCount();


  if ( ! computeHistogram( false ) )
  {
    QgsDebugMsg( QStringLiteral( "raster does not have cached histogram" ) );
    stackedWidget2->setCurrentIndex( 2 );
    return;
  }

  // clear plot
  mpPlot->detachItems();

  //ensure all children get removed
  mpPlot->setAutoDelete( true );
  mpPlot->setTitle( QObject::tr( "Raster Histogram" ) );
  mpPlot->insertLegend( new QwtLegend(), QwtPlot::BottomLegend );
  // Set axis titles
  mpPlot->setAxisTitle( QwtPlot::xBottom, QObject::tr( "Pixel Value" ) );
  mpPlot->setAxisTitle( QwtPlot::yLeft, QObject::tr( "Frequency" ) );
  mpPlot->setAxisAutoScale( QwtPlot::yLeft );

  // x axis scale only set after computing global min/max across bands (see below)
  // add a grid
  QwtPlotGrid *myGrid = new QwtPlotGrid();
  myGrid->attach( mpPlot );

  // make colors list
  mHistoColors.clear();
  mHistoColors << Qt::black; // first element, not used
  QVector<QColor> myColors;
  myColors << Qt::red << Qt::green << Qt::blue << Qt::magenta << Qt::darkYellow << Qt::cyan;

  // make sure colors are always the same for a given band count
  QRandomGenerator colorGenerator( myBandCountInt * 100 );
  while ( myColors.size() <= myBandCountInt )
  {
    myColors <<
             QColor( colorGenerator.bounded( 1, 256 ),
                     colorGenerator.bounded( 1, 256 ),
                     colorGenerator.bounded( 1, 256 ) );
  }

  // assign colors to each band, depending on the current RGB/gray band selection
  // grayscale
  QList< int > mySelectedBands = rendererSelectedBands();
  if ( mRendererName == QLatin1String( "singlebandgray" ) )
  {
    const int myGrayBand = mySelectedBands[0];
    for ( int i = 1; i <= myBandCountInt; i++ )
    {
      if ( i == myGrayBand )
      {
        mHistoColors << Qt::darkGray;
        cboHistoBand->setItemData( i - 1, QColor( Qt::darkGray ), Qt::ForegroundRole );
      }
      else
      {
        if ( ! myColors.isEmpty() )
        {
          mHistoColors << myColors.first();
          myColors.pop_front();
        }
        else
        {
          mHistoColors << Qt::black;
        }
        cboHistoBand->setItemData( i - 1, QColor( Qt::black ), Qt::ForegroundRole );
      }
    }
  }
  // RGB
  else if ( mRendererName == QLatin1String( "multibandcolor" ) )
  {
    const int myRedBand = mySelectedBands[0];
    const int myGreenBand = mySelectedBands[1];
    const int myBlueBand = mySelectedBands[2];
    // remove RGB, which are reserved for the actual RGB bands
    // show name of RGB bands in appropriate color in bold
    myColors.remove( 0, 3 );
    for ( int i = 1; i <= myBandCountInt; i++ )
    {
      QColor myColor;
      if ( i == myRedBand )
        myColor = Qt::red;
      else if ( i == myGreenBand )
        myColor = Qt::green;
      else if ( i == myBlueBand )
        myColor = Qt::blue;
      else
      {
        if ( ! myColors.isEmpty() )
        {
          myColor = myColors.first();
          myColors.pop_front();
        }
        else
        {
          myColor = Qt::black;
        }
        cboHistoBand->setItemData( i - 1, QColor( Qt::black ), Qt::ForegroundRole );
      }
      if ( i == myRedBand ||  i == myGreenBand || i == myBlueBand )
      {
        cboHistoBand->setItemData( i - 1, myColor, Qt::ForegroundRole );
      }
      mHistoColors << myColor;
    }
  }
  else
  {
    mHistoColors << myColors;
  }

  //
  //now draw actual graphs
  //

  //sometimes there are more bins than needed
  //we find out the last one that actually has data in it
  //so we can discard the rest and set the x-axis scales correctly
  //
  // scan through to get counts from layers' histograms
  //
  mHistoMin = 0;
  mHistoMax = 0;
  bool myFirstIteration = true;
  /* Gets selected band list, if mHistoShowBands != ShowAll */
  mySelectedBands = histoSelectedBands();
  double myBinXStep = 1;
  double myBinX = 0;

  for ( int bandNumber = 1;
        bandNumber <= myBandCountInt;
        ++bandNumber )
  {
    /* skip this band if mHistoShowBands != ShowAll and this band is not selected */
    if ( mHistoShowBands != ShowAll )
    {
      if ( ! mySelectedBands.contains( bandNumber ) )
        continue;
    }

    const int sampleSize = SAMPLE_SIZE; // number of sample cells

    const std::unique_ptr< QgsRasterBlockFeedback > feedback( new QgsRasterBlockFeedback() );
    connect( feedback.get(), &QgsRasterBlockFeedback::progressChanged, mHistogramProgress, &QProgressBar::setValue );

    const int binCount = getBinCount( mRasterLayer->dataProvider(), bandNumber, sampleSize );
    const QgsRasterHistogram myHistogram = mRasterLayer->dataProvider()->histogram( bandNumber, binCount, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize, false, feedback.get() );

    QgsDebugMsg( QStringLiteral( "got raster histo for band %1 : min=%2 max=%3 count=%4" ).arg( bandNumber ).arg( myHistogram.minimum ).arg( myHistogram.maximum ).arg( myHistogram.binCount ) );

    const Qgis::DataType mySrcDataType = mRasterLayer->dataProvider()->sourceDataType( bandNumber );
    bool myDrawLines = true;
    if ( ! mHistoDrawLines &&
         ( mySrcDataType == Qgis::DataType::Byte ||
           mySrcDataType == Qgis::DataType::Int16 || mySrcDataType == Qgis::DataType::Int32 ||
           mySrcDataType == Qgis::DataType::UInt16 || mySrcDataType == Qgis::DataType::UInt32 ) )
    {
      myDrawLines = false;
    }

    QwtPlotCurve *mypCurve = nullptr;
    if ( myDrawLines )
    {
      mypCurve = new QwtPlotCurve( tr( "Band %1" ).arg( bandNumber ) );
      //mypCurve->setCurveAttribute( QwtPlotCurve::Fitted );
      mypCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
      mypCurve->setPen( QPen( mHistoColors.at( bandNumber ) ) );
    }

    QwtPlotHistogram *mypHisto = nullptr;
    if ( ! myDrawLines )
    {
      mypHisto = new QwtPlotHistogram( tr( "Band %1" ).arg( bandNumber ) );
      mypHisto->setRenderHint( QwtPlotItem::RenderAntialiased );
      //mypHisto->setPen( QPen( mHistoColors.at( myIteratorInt ) ) );
      mypHisto->setPen( QPen( Qt::lightGray ) );
      // this is needed in order to see the colors in the legend
      mypHisto->setBrush( QBrush( mHistoColors.at( bandNumber ) ) );
    }

    QVector<QPointF> data;
    QVector<QwtIntervalSample> dataHisto;

    // calculate first bin x value and bin step size
    myBinXStep = ( myHistogram.maximum - myHistogram.minimum ) / myHistogram.binCount;
    myBinX = myHistogram.minimum + myBinXStep / 2.0;

    for ( int myBin = 0; myBin < myHistogram.binCount; myBin++ )
    {
      const int myBinValue = myHistogram.histogramVector.at( myBin );
      if ( myDrawLines )
      {
        data << QPointF( myBinX, myBinValue );
      }
      else
      {
        dataHisto << QwtIntervalSample( myBinValue, myBinX - myBinXStep / 2.0, myBinX + myBinXStep / 2.0 );
      }
      myBinX += myBinXStep;
    }

    if ( myDrawLines )
    {
      mypCurve->setSamples( data );
      mypCurve->attach( mpPlot );
    }
    else
    {
      mypHisto->setSamples( dataHisto );
      mypHisto->attach( mpPlot );
    }

    if ( myFirstIteration || mHistoMin > myHistogram.minimum )
    {
      mHistoMin = myHistogram.minimum;
    }
    if ( myFirstIteration || mHistoMax < myHistogram.maximum )
    {
      mHistoMax = myHistogram.maximum;
    }
    QgsDebugMsg( QStringLiteral( "computed histo min = %1 max = %2" ).arg( mHistoMin ).arg( mHistoMax ) );
    myFirstIteration = false;
  }

  if ( mHistoMin < mHistoMax )
  {
    // for x axis use band pixel values rather than gdal hist. bin values
    // subtract -0.5 to prevent rounding errors
    // see http://www.gdal.org/classGDALRasterBand.html#3f8889607d3b2294f7e0f11181c201c8
    // fix x range for non-Byte data
    mpPlot->setAxisScale( QwtPlot::xBottom,
                          mHistoMin - myBinXStep / 2,
                          mHistoMax + myBinXStep / 2 );
    mpPlot->setEnabled( true );
    mpPlot->replot();

    // histo plot markers
    // memory leak?
    mHistoMarkerMin = new QwtPlotMarker();
    mHistoMarkerMin->attach( mpPlot );
    mHistoMarkerMax = new QwtPlotMarker();
    mHistoMarkerMax->attach( mpPlot );
    updateHistoMarkers();

    // histo picker
    if ( !mHistoPicker )
    {
      mHistoPicker = new QwtPlotPicker( mpPlot->canvas() );
      // mHistoPicker->setTrackerMode( QwtPicker::ActiveOnly );
      mHistoPicker->setTrackerMode( QwtPicker::AlwaysOff );
      mHistoPicker->setRubberBand( QwtPicker::VLineRubberBand );
      mHistoPicker->setStateMachine( new QwtPickerDragPointMachine );
      connect( mHistoPicker, static_cast < void ( QwtPlotPicker::* )( const QPointF & ) > ( &QwtPlotPicker::selected ), this, &QgsRasterHistogramWidget::histoPickerSelected );
    }
    mHistoPicker->setEnabled( false );

    // plot zoomer
    if ( !mHistoZoomer )
    {
      mHistoZoomer = new QwtPlotZoomer( mpPlot->canvas() );
      mHistoZoomer->setStateMachine( new QwtPickerDragRectMachine );
      mHistoZoomer->setTrackerMode( QwtPicker::AlwaysOff );
    }
    mHistoZoomer->setEnabled( true );
  }
  else
  {
    mpPlot->setDisabled( true );
    if ( mHistoPicker )
      mHistoPicker->setEnabled( false );
    if ( mHistoZoomer )
      mHistoZoomer->setEnabled( false );
  }

  stackedWidget2->setCurrentIndex( 0 );
  // icon from http://findicons.com/icon/169577/14_zoom?id=171427
  mpPlot->canvas()->setCursor( QCursor( QgsApplication::getThemePixmap( QStringLiteral( "/mIconZoom.svg" ) ) ) );
  //  cboHistoBand_currentIndexChanged( -1 );
  QApplication::restoreOverrideCursor();
}

void QgsRasterHistogramWidget::mSaveAsImageButton_clicked()
{
  if ( !mpPlot )
    return;

  const QPair< QString, QString> myFileNameAndFilter = QgsGuiUtils::getSaveAsImageName( this, tr( "Choose a file name to save the map image as" ) );
  const QFileInfo myInfo( myFileNameAndFilter.first );
  if ( !myInfo.baseName().isEmpty() )
  {
    histoSaveAsImage( myFileNameAndFilter.first );
  }
}

bool QgsRasterHistogramWidget::histoSaveAsImage( const QString &filename,
    int width, int height, int quality )
{
  // make sure dir. exists
  const QFileInfo myInfo( filename );
  const QDir myDir( myInfo.dir() );
  if ( ! myDir.exists() )
  {
    QgsDebugMsg( QStringLiteral( "Error, directory %1 non-existent (theFilename = %2)" ).arg( myDir.absolutePath(), filename ) );
    return false;
  }

  // prepare the pixmap
  QPixmap myPixmap( width, height );
  const QRect myQRect( 5, 5, width - 10, height - 10 ); // leave a 5px border on all sides
  myPixmap.fill( Qt::white ); // Qt::transparent ?

  QwtPlotRenderer myRenderer;
  myRenderer.setDiscardFlags( QwtPlotRenderer::DiscardBackground |
                              QwtPlotRenderer::DiscardCanvasBackground );
  myRenderer.setLayoutFlags( QwtPlotRenderer::FrameWithScales );

  QPainter myPainter;
  myPainter.begin( &myPixmap );
  myRenderer.render( mpPlot, &myPainter, myQRect );
  myPainter.end();

  // save pixmap to file
  myPixmap.save( filename, nullptr, quality );

  // should do more error checking
  return true;
}

void QgsRasterHistogramWidget::setSelectedBand( int bandNo )
{
  cboHistoBand->setCurrentIndex( bandNo - 1 );
}

void QgsRasterHistogramWidget::cboHistoBand_currentIndexChanged( int index )
{
  if ( mHistoShowBands == ShowSelected )
    refreshHistogram();

  // get the current index value, index can be -1
  index = cboHistoBand->currentIndex();
  if ( mHistoPicker )
  {
    mHistoPicker->setEnabled( false );
    mHistoPicker->setRubberBandPen( QPen( mHistoColors.at( index + 1 ) ) );
  }
  if ( mHistoZoomer )
    mHistoZoomer->setEnabled( true );
  btnHistoMin->setEnabled( true );
  btnHistoMax->setEnabled( true );

  const QPair< QString, QString > myMinMax = rendererMinMax( index + 1 );
  leHistoMin->setText( myMinMax.first );
  leHistoMax->setText( myMinMax.second );

  applyHistoMin();
  applyHistoMax();
}

void QgsRasterHistogramWidget::histoActionTriggered( QAction *action )
{
  if ( ! action )
    return;
  histoAction( action->data().toString(), action->isChecked() );
}

void QgsRasterHistogramWidget::histoAction( const QString &actionName, bool actionFlag )
{
  if ( actionName.isEmpty() )
    return;

  // this approach is a bit of a hack, but this way we don't have to define slots for each action
  QgsDebugMsg( QStringLiteral( "band = %1 action = %2" ).arg( cboHistoBand->currentIndex() + 1 ).arg( actionName ) );

  // checkeable actions
  if ( actionName == QLatin1String( "Show markers" ) )
  {
    mHistoShowMarkers = actionFlag;
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Raster/histogram/showMarkers" ), mHistoShowMarkers );
    updateHistoMarkers();
    return;
  }
  else if ( actionName == QLatin1String( "Zoom min_max" ) )
  {
    mHistoZoomToMinMax = actionFlag;
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Raster/histogram/zoomToMinMax" ), mHistoZoomToMinMax );
    return;
  }
  else if ( actionName == QLatin1String( "Update min_max" ) )
  {
    mHistoUpdateStyleToMinMax = actionFlag;
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Raster/histogram/updateStyleToMinMax" ), mHistoUpdateStyleToMinMax );
    return;
  }
  else if ( actionName == QLatin1String( "Show all" ) )
  {
    mHistoShowBands = ShowAll;
    // settings.setValue( "/Raster/histogram/showBands", static_cast<int>(mHistoShowBands) );
    refreshHistogram();
    return;
  }
  else if ( actionName == QLatin1String( "Show selected" ) )
  {
    mHistoShowBands = ShowSelected;
    // settings.setValue( "/Raster/histogram/showBands", static_cast<int>(mHistoShowBands) );
    refreshHistogram();
    return;
  }
  else if ( actionName == QLatin1String( "Show RGB" ) )
  {
    mHistoShowBands = ShowRGB;
    // settings.setValue( "/Raster/histogram/showBands", static_cast<int>(mHistoShowBands) );
    refreshHistogram();
    return;
  }
  else if ( actionName == QLatin1String( "Draw lines" ) )
  {
    mHistoDrawLines = actionFlag;
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Raster/histogram/drawLines" ), mHistoDrawLines );
    btnHistoCompute_clicked(); // refresh
    return;
  }
#if 0
  else if ( actionName == "Load apply all" )
  {
    mHistoLoadApplyAll = actionFlag;
    settings.setValue( "/Raster/histogram/loadApplyAll", mHistoLoadApplyAll );
    return;
  }
#endif
  // Load actions
  // TODO - separate calculations from rendererwidget so we can do them without
  else if ( actionName.left( 5 ) == QLatin1String( "Load " ) && mRendererWidget )
  {
    QVector<int> myBands;
    bool ok = false;

#if 0
    double minMaxValues[2];

    // find which band(s) need updating (all or current)
    if ( mHistoLoadApplyAll )
    {
      int myBandCountInt = mRasterLayer->bandCount();
      for ( int i = 1; i <= myBandCountInt; i++ )
      {
        if ( i != cboHistoBand->currentIndex() + 1 )
          myBands << i;
      }
    }
#endif

    // add current band to the end
    myBands << cboHistoBand->currentIndex() + 1;

    // get stddev value once if needed
#if 0
    double myStdDev = 1.0;
    if ( actionName == "Load stddev" )
    {
      myStdDev = mRendererWidget->stdDev().toDouble();
    }
#endif

    // don't update markers every time
    leHistoMin->blockSignals( true );
    leHistoMax->blockSignals( true );

    // process each band
    const auto constMyBands = myBands;
    for ( const int bandNo : constMyBands )
    {
      ok = false;
#if 0
      if ( actionName == "Load actual" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::Actual,
                                          bandNo, minMaxValues );
      }
      else if ( actionName == "Load estimate" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::Estimate,
                                          bandNo, minMaxValues );
      }
      else if ( actionName == "Load extent" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::CurrentExtent,
                                          bandNo, minMaxValues );
      }
      else if ( actionName == "Load 1 stddev" ||
                actionName == "Load stddev" )
      {
        ok = mRendererWidget->bandMinMaxFromStdDev( myStdDev, bandNo, minMaxValues );
      }
#endif

      // apply current item
      cboHistoBand->setCurrentIndex( bandNo - 1 );
      if ( !ok || actionName == QLatin1String( "Load reset" ) )
      {
        leHistoMin->clear();
        leHistoMax->clear();
#if 0
        // TODO - fix gdal provider: changes data type when nodata value is not found
        // this prevents us from getting proper min and max values here
        minMaxValues[0] = QgsContrastEnhancement::minimumValuePossible(
                            ( Qgis::DataType ) mRasterLayer->dataProvider()->dataType( bandNo ) );
        minMaxValues[1] = QgsContrastEnhancement::maximumValuePossible(
                            ( Qgis::DataType ) mRasterLayer->dataProvider()->dataType( bandNo ) );
      }
      else
      {
        leHistoMin->setText( QString::number( minMaxValues[0] ) );
        leHistoMax->setText( QString::number( minMaxValues[1] ) );
#endif
      }
      applyHistoMin();
      applyHistoMax();
    }
    // update markers
    leHistoMin->blockSignals( false );
    leHistoMax->blockSignals( false );
    updateHistoMarkers();
  }
  else if ( actionName == QLatin1String( "Compute histogram" ) )
  {
    btnHistoCompute_clicked();
  }
  else
  {
    QgsDebugMsg( "Invalid action " + actionName );
    return;
  }
}

void QgsRasterHistogramWidget::applyHistoMin()
{
  if ( ! mRendererWidget )
    return;

  const int bandNo = cboHistoBand->currentIndex() + 1;
  const QList< int > selectedBands = rendererSelectedBands();
  QString min;
  for ( int i = 0; i <= selectedBands.size(); i++ )
  {
    if ( bandNo == mRendererWidget->selectedBand( i ) )
    {
      min = leHistoMin->text();
      if ( mHistoUpdateStyleToMinMax && mRendererWidget->min( i ) != min )
      {
        mRendererWidget->setMin( min, i );
        if ( mRendererWidget->contrastEnhancementAlgorithm() == QgsContrastEnhancement::NoEnhancement )
        {
          mRendererWidget->setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum );
        }
        if ( mRendererWidget->minMaxWidget() )
        {
          mRendererWidget->minMaxWidget()->userHasSetManualMinMaxValues();
        }
      }
    }
  }

  updateHistoMarkers();

  if ( ! min.isEmpty() && mHistoZoomToMinMax && mHistoZoomer )
  {
    QRectF rect = mHistoZoomer->zoomRect();
    rect.setLeft( min.toDouble() );
    mHistoZoomer->zoom( rect );
  }
  emit widgetChanged();
}

void QgsRasterHistogramWidget::applyHistoMax()
{
  if ( ! mRendererWidget )
    return;

  const int bandNo = cboHistoBand->currentIndex() + 1;
  const QList< int > mySelectedBands = rendererSelectedBands();
  QString max;
  for ( int i = 0; i <= mySelectedBands.size(); i++ )
  {
    if ( bandNo == mRendererWidget->selectedBand( i ) )
    {
      max = leHistoMax->text();
      if ( mHistoUpdateStyleToMinMax && mRendererWidget->max( i ) != max )
      {
        mRendererWidget->setMax( max, i );
        if ( mRendererWidget->contrastEnhancementAlgorithm() == QgsContrastEnhancement::NoEnhancement )
        {
          mRendererWidget->setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum );
        }
        if ( mRendererWidget->minMaxWidget() )
        {
          mRendererWidget->minMaxWidget()->userHasSetManualMinMaxValues();
        }
      }
    }
  }

  updateHistoMarkers();

  if ( ! max.isEmpty() && mHistoZoomToMinMax && mHistoZoomer )
  {
    QRectF rect = mHistoZoomer->zoomRect();
    rect.setRight( max.toDouble() );
    mHistoZoomer->zoom( rect );
  }
  emit widgetChanged();
}

void QgsRasterHistogramWidget::btnHistoMin_toggled()
{
  if ( mpPlot && mHistoPicker )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    if ( btnHistoMin->isChecked() )
    {
      btnHistoMax->setChecked( false );
      QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    if ( mHistoZoomer )
      mHistoZoomer->setEnabled( ! btnHistoMin->isChecked() );
    mHistoPicker->setEnabled( btnHistoMin->isChecked() );
  }
  updateHistoMarkers();
}

void QgsRasterHistogramWidget::btnHistoMax_toggled()
{
  if ( mpPlot && mHistoPicker )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    if ( btnHistoMax->isChecked() )
    {
      btnHistoMin->setChecked( false );
      QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    if ( mHistoZoomer )
      mHistoZoomer->setEnabled( ! btnHistoMax->isChecked() );
    mHistoPicker->setEnabled( btnHistoMax->isChecked() );
  }
  updateHistoMarkers();
}

// local function used by histoPickerSelected(), to get a rounded picked value
// this is sensitive and may not always be correct, needs more testing
QString findClosestTickVal( double target, const QwtScaleDiv *scale, int div = 100 )
{
  if ( !scale ) return QString();

  QList< double > minorTicks = scale->ticks( QwtScaleDiv::MinorTick );
  QList< double > majorTicks = scale->ticks( QwtScaleDiv::MajorTick );
  const double diff = ( minorTicks[1] - minorTicks[0] ) / div;
  double min = majorTicks[0] - diff;
  if ( min > target )
    min -= ( majorTicks[1] - majorTicks[0] );
  const double max = scale->upperBound();
  double closest = target;
  double current = min;

  while ( current < max )
  {
    current += diff;
    if ( current > target )
    {
      closest = ( std::fabs( target - current + diff ) < std::fabs( target - current ) ) ? current - diff : current;
      break;
    }
  }

  // QgsDebugMsg( QStringLiteral( "target=%1 div=%2 closest=%3" ).arg( target ).arg( div ).arg( closest ) );
  return QLocale().toString( closest );
}

void QgsRasterHistogramWidget::histoPickerSelected( QPointF pos )
{
  if ( btnHistoMin->isChecked() || btnHistoMax->isChecked() )
  {
    const QwtScaleDiv *scale = &mpPlot->axisScaleDiv( QwtPlot::xBottom );

    if ( btnHistoMin->isChecked() )
    {
      leHistoMin->setText( findClosestTickVal( pos.x(), scale ) );
      applyHistoMin();
      btnHistoMin->setChecked( false );
    }
    else // if ( btnHistoMax->isChecked() )
    {
      leHistoMax->setText( findClosestTickVal( pos.x(), scale ) );
      applyHistoMax();
      btnHistoMax->setChecked( false );
    }
  }
  if ( QApplication::overrideCursor() )
    QApplication::restoreOverrideCursor();
}

void QgsRasterHistogramWidget::histoPickerSelectedQwt5( QwtDoublePoint pos )
{
  histoPickerSelected( QPointF( pos.x(), pos.y() ) );
}

void QgsRasterHistogramWidget::updateHistoMarkers()
{
  // hack to not update markers
  if ( leHistoMin->signalsBlocked() )
    return;
  // todo error checking
  if ( !mpPlot || !mHistoMarkerMin || !mHistoMarkerMax )
    return;

  const int bandNo = cboHistoBand->currentIndex() + 1;
  const QList< int > mySelectedBands = histoSelectedBands();

  if ( ( ! mHistoShowMarkers && ! btnHistoMin->isChecked() && ! btnHistoMax->isChecked() ) ||
       ( ! mySelectedBands.isEmpty() && ! mySelectedBands.contains( bandNo ) ) )
  {
    mHistoMarkerMin->hide();
    mHistoMarkerMax->hide();
    mpPlot->replot();
    return;
  }

  double minVal = mHistoMin;
  double maxVal = mHistoMax;
  const QString minStr = leHistoMin->text();
  const QString maxStr = leHistoMax->text();
  if ( !minStr.isEmpty() )
    minVal = minStr.toDouble();
  if ( !maxStr.isEmpty() )
    maxVal = maxStr.toDouble();

  QPen linePen = QPen( mHistoColors.at( bandNo ) );
  linePen.setStyle( Qt::DashLine );
  mHistoMarkerMin->setLineStyle( QwtPlotMarker::VLine );
  mHistoMarkerMin->setLinePen( linePen );
  mHistoMarkerMin->setXValue( minVal );
  mHistoMarkerMin->show();
  mHistoMarkerMax->setLineStyle( QwtPlotMarker::VLine );
  mHistoMarkerMax->setLinePen( linePen );
  mHistoMarkerMax->setXValue( maxVal );
  mHistoMarkerMax->show();

  mpPlot->replot();
}


QList< int > QgsRasterHistogramWidget::histoSelectedBands()
{
  QList< int > mySelectedBands;

  if ( mHistoShowBands != ShowAll )
  {
    if ( mHistoShowBands == ShowSelected )
    {
      mySelectedBands << cboHistoBand->currentIndex() + 1;
    }
    else if ( mHistoShowBands == ShowRGB )
    {
      mySelectedBands = rendererSelectedBands();
    }
  }

  return mySelectedBands;
}

QList< int > QgsRasterHistogramWidget::rendererSelectedBands()
{
  QList< int > mySelectedBands;

  if ( ! mRendererWidget )
  {
    mySelectedBands << -1 << -1 << -1; // make sure we return 3 elements
    return mySelectedBands;
  }

  if ( mRendererName == QLatin1String( "singlebandgray" ) ||
       mRendererName == QLatin1String( "singlebandpseudocolor" ) )
  {
    mySelectedBands << mRendererWidget->selectedBand();
  }
  else if ( mRendererName == QLatin1String( "multibandcolor" ) )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      mySelectedBands << mRendererWidget->selectedBand( i );
    }
  }

  return mySelectedBands;
}

QPair< QString, QString > QgsRasterHistogramWidget::rendererMinMax( int bandNo )
{
  QPair< QString, QString > myMinMax;

  if ( ! mRendererWidget )
    return myMinMax;

  if ( mRendererName == QLatin1String( "singlebandgray" ) ||
       mRendererName == QLatin1String( "singlebandpseudocolor" ) )
  {
    if ( bandNo == mRendererWidget->selectedBand() )
    {
      myMinMax.first = mRendererWidget->min();
      myMinMax.second = mRendererWidget->max();
    }
  }
  else if ( mRendererName == QLatin1String( "multibandcolor" ) )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      if ( bandNo == mRendererWidget->selectedBand( i ) )
      {
        myMinMax.first = mRendererWidget->min( i );
        myMinMax.second = mRendererWidget->max( i );
        break;
      }
    }
  }

  // TODO - there are 2 definitions of raster data type that should be unified
  // QgsRasterDataProvider::DataType and Qgis::DataType
  // TODO - fix gdal provider: changes data type when nodata value is not found
  // this prevents us from getting proper min and max values here
  // minStr = QString::number( QgsContrastEnhancement::minimumValuePossible( ( Qgis::DataType )
  //                                                                         mRasterLayer->dataProvider()->dataType( bandNo ) ) );
  // maxStr = QString::number( QgsContrastEnhancement::maximumValuePossible( ( Qgis::DataType )
  //                                                                         mRasterLayer->dataProvider()->dataType( bandNo ) ) );

  // if we get an empty result, fill with default value (histo min/max)
  if ( myMinMax.first.isEmpty() )
    myMinMax.first = QLocale().toString( mHistoMin );
  if ( myMinMax.second.isEmpty() )
    myMinMax.second = QLocale().toString( mHistoMax );

  QgsDebugMsg( QStringLiteral( "bandNo %1 got min/max [%2] [%3]" ).arg( bandNo ).arg( myMinMax.first, myMinMax.second ) );

  return myMinMax;
}

void QgsRasterHistogramWidget::apply()
{

}
