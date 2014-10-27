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
#include "qgisgui.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterrendererwidget.h"
#include "qgsrasterhistogramwidget.h"

#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QSettings>

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
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
#include <qwt_plot_renderer.h>
#include <qwt_plot_histogram.h>
#else
#include "qwt5_histogram_item.h"
#endif

// this has been removed, now we let the provider/raster interface decide
// how many bins are suitable depending on data type and range
//#define RASTER_HISTOGRAM_BINS 256

QgsRasterHistogramWidget::QgsRasterHistogramWidget( QgsRasterLayer* lyr, QWidget *parent )
    : QWidget( parent ),
    mRasterLayer( lyr ), mRendererWidget( 0 )
{
  setupUi( this );

  mSaveAsImageButton->setIcon( QgsApplication::getThemeIcon( "/mActionFileSave.svg" ) );

  mRendererWidget = 0;
  mRendererName = "singlebandgray";

  mHistoMin = 0;
  mHistoMax = 0;

  mHistoPicker = NULL;
  mHistoZoomer = NULL;
  mHistoMarkerMin = NULL;
  mHistoMarkerMax = NULL;

  QSettings settings;
  mHistoShowMarkers = settings.value( "/Raster/histogram/showMarkers", false ).toBool();
  // mHistoLoadApplyAll = settings.value( "/Raster/histogram/loadApplyAll", false ).toBool();
  mHistoZoomToMinMax = settings.value( "/Raster/histogram/zoomToMinMax", false ).toBool();
  mHistoUpdateStyleToMinMax = settings.value( "/Raster/histogram/updateStyleToMinMax", true ).toBool();
  mHistoDrawLines = settings.value( "/Raster/histogram/drawLines", true ).toBool();
  // mHistoShowBands = (HistoShowBands) settings.value( "/Raster/histogram/showBands", (int) ShowAll ).toInt();
  mHistoShowBands = ShowAll;

  bool isInt = true;
  if ( true )
  {
    //band selector
    int myBandCountInt = mRasterLayer->bandCount();
    for ( int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt )
    {
      cboHistoBand->addItem( mRasterLayer->bandName( myIteratorInt ) );
      QGis::DataType mySrcDataType = mRasterLayer->dataProvider()->srcDataType( myIteratorInt );
      if ( !( mySrcDataType == QGis::Byte ||
              mySrcDataType == QGis::Int16 || mySrcDataType == QGis::Int32 ||
              mySrcDataType == QGis::UInt16 || mySrcDataType == QGis::UInt32 ) )
        isInt = false;
    }

    // histo min/max selectors
    leHistoMin->setValidator( new QDoubleValidator( this ) );
    leHistoMax->setValidator( new QDoubleValidator( this ) );
    // this might generate many refresh events! test..
    // connect( leHistoMin, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateHistoMarkers() ) );
    // connect( leHistoMax, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateHistoMarkers() ) );
    // connect( leHistoMin, SIGNAL( textChanged( const QString & ) ), this, SLOT( applyHistoMin() ) );
    // connect( leHistoMax, SIGNAL( textChanged( const QString & ) ), this, SLOT( applyHistoMax() ) );
    connect( leHistoMin, SIGNAL( editingFinished() ), this, SLOT( applyHistoMin() ) );
    connect( leHistoMax, SIGNAL( editingFinished() ), this, SLOT( applyHistoMax() ) );

    // histo actions
    // TODO move/add options to qgis options dialog
    QMenu* menu = new QMenu( this );
    menu->setSeparatorsCollapsible( false );
    btnHistoActions->setMenu( menu );
    QActionGroup* group;
    QAction* action;

    // min/max options
    group = new QActionGroup( this );
    group->setExclusive( false );
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
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
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
    action = new QAction( tr( "Visibility" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    group = new QActionGroup( this );
    group->setExclusive( true ); // these options are exclusive
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
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
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
    action = new QAction( tr( "Display" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    // should we plot as histogram instead of line plot? (int data only)
    action = new QAction( "", group );
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
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
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


QgsRasterHistogramWidget::~QgsRasterHistogramWidget()
{
}

void QgsRasterHistogramWidget::setRendererWidget( const QString& name, QgsRasterRendererWidget* rendererWidget )
{
  mRendererName = name;
  mRendererWidget = rendererWidget;
  refreshHistogram();
  on_cboHistoBand_currentIndexChanged( -1 );
}

void QgsRasterHistogramWidget::setActive( bool theActiveFlag )
{
  if ( theActiveFlag )
  {
    refreshHistogram();
    on_cboHistoBand_currentIndexChanged( -1 );
  }
  else
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    btnHistoMin->setChecked( false );
    btnHistoMax->setChecked( false );
  }
}

void QgsRasterHistogramWidget::on_btnHistoCompute_clicked()
{
// Histogram computation can be called either by clicking the "Compute Histogram" button
// which is only visible if there is no cached histogram or by calling the
// "Compute Histogram" action. Due to limitations in the gdal api, it is not possible
// to re-calculate the histogram if it has already been calculated
  computeHistogram( true );
  refreshHistogram();
}

bool QgsRasterHistogramWidget::computeHistogram( bool forceComputeFlag )
{
  QgsDebugMsg( "entered." );

  //bool myIgnoreOutOfRangeFlag = true;
  //bool myThoroughBandScanFlag = false;
  int myBandCountInt = mRasterLayer->bandCount();

  // if forceComputeFlag = false make sure raster has cached histogram, else return false
  if ( ! forceComputeFlag )
  {
    for ( int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt )
    {
      int sampleSize = 250000; // number of sample cells
      if ( !mRasterLayer->dataProvider()->hasHistogram( myIteratorInt, 0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize ) )
      {
        QgsDebugMsg( QString( "band %1 does not have cached histo" ).arg( myIteratorInt ) );
        return false;
      }
    }
  }

  // compute histogram
  stackedWidget2->setCurrentIndex( 1 );
  connect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  QApplication::setOverrideCursor( Qt::WaitCursor );

  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    int sampleSize = 250000; // number of sample cells
    mRasterLayer->dataProvider()->histogram( myIteratorInt, 0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize );
  }

  disconnect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
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
  int myBandCountInt = mRasterLayer->bandCount();

  QgsDebugMsg( "entered." );

  if ( ! computeHistogram( false ) )
  {
    QgsDebugMsg( QString( "raster does not have cached histogram" ) );
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
  QwtPlotGrid * myGrid = new QwtPlotGrid();
  myGrid->attach( mpPlot );

  // make colors list
  mHistoColors.clear();
  mHistoColors << Qt::black; // first element, not used
  QVector<QColor> myColors;
  myColors << Qt::red << Qt::green << Qt::blue << Qt::magenta << Qt::darkYellow << Qt::cyan;
  srand( myBandCountInt * 100 ); // make sure colors are always the same for a given band count
  while ( myColors.size() <= myBandCountInt )
  {
    myColors <<
    QColor( 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ),
            1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ),
            1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ) );
  }

  // assign colors to each band, depending on the current RGB/gray band selection
  // grayscale
  QList< int > mySelectedBands = rendererSelectedBands();
  if ( mRendererName == "singlebandgray" )
  {
    int myGrayBand = mySelectedBands[0];
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
  else if ( mRendererName == "multibandcolor" )
  {
    int myRedBand = mySelectedBands[0];
    int myGreenBand = mySelectedBands[1];
    int myBlueBand = mySelectedBands[2];
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

  //somtimes there are more bins than needed
  //we find out the last one that actually has data in it
  //so we can discard the rest and set the x-axis scales correctly
  //
  // scan through to get counts from layers' histograms
  //
  mHistoMin = 0;
  mHistoMax = 0;
  bool myFirstIteration = true;
  /* get selected band list, if mHistoShowBands != ShowAll */
  mySelectedBands = histoSelectedBands();
  double myBinXStep = 1;
  double myBinX = 0;

  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    /* skip this band if mHistoShowBands != ShowAll and this band is not selected */
    if ( mHistoShowBands != ShowAll )
    {
      if ( ! mySelectedBands.contains( myIteratorInt ) )
        continue;
    }

    int sampleSize = 250000; // number of sample cells
    QgsRasterHistogram myHistogram = mRasterLayer->dataProvider()->histogram( myIteratorInt, 0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), QgsRectangle(), sampleSize );

    QgsDebugMsg( QString( "got raster histo for band %1 : min=%2 max=%3 count=%4" ).arg( myIteratorInt ).arg( myHistogram.minimum ).arg( myHistogram.maximum ).arg( myHistogram.binCount ) );

    QGis::DataType mySrcDataType = mRasterLayer->dataProvider()->srcDataType( myIteratorInt );
    bool myDrawLines = true;
    if ( ! mHistoDrawLines &&
         ( mySrcDataType == QGis::Byte ||
           mySrcDataType == QGis::Int16 || mySrcDataType == QGis::Int32 ||
           mySrcDataType == QGis::UInt16 || mySrcDataType == QGis::UInt32 ) )
    {
      myDrawLines = false;
    }

    QwtPlotCurve * mypCurve = 0;
    if ( myDrawLines )
    {
      mypCurve = new QwtPlotCurve( tr( "Band %1" ).arg( myIteratorInt ) );
      //mypCurve->setCurveAttribute( QwtPlotCurve::Fitted );
      mypCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
      mypCurve->setPen( QPen( mHistoColors.at( myIteratorInt ) ) );
    }

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    QwtPlotHistogram * mypHisto = 0;
    if ( ! myDrawLines )
    {
      mypHisto = new QwtPlotHistogram( tr( "Band %1" ).arg( myIteratorInt ) );
      mypHisto->setRenderHint( QwtPlotItem::RenderAntialiased );
      //mypHisto->setPen( QPen( mHistoColors.at( myIteratorInt ) ) );
      mypHisto->setPen( QPen( Qt::lightGray ) );
      // this is needed in order to see the colors in the legend
      mypHisto->setBrush( QBrush( mHistoColors.at( myIteratorInt ) ) );
    }
#else
    HistogramItem *mypHistoItem = 0;
    if ( ! myDrawLines )
    {
      mypHistoItem = new HistogramItem( tr( "Band %1" ).arg( myIteratorInt ) );
      mypHistoItem->setRenderHint( QwtPlotItem::RenderAntialiased );
      mypHistoItem->setColor( mHistoColors.at( myIteratorInt ) );
    }
#endif

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    QVector<QPointF> data;
    QVector<QwtIntervalSample> dataHisto;
#else
    QVector<double> myX2Data;
    QVector<double> myY2Data;
    // we safely assume that QT>=4.0 (min version is 4.7), therefore QwtArray is a QVector, so don't set size here
    QwtArray<QwtDoubleInterval> intervalsHisto;
    QwtArray<double> valuesHisto;

#endif

    // calculate first bin x value and bin step size if not Byte data
    if ( mySrcDataType != QGis::Byte )
    {
      myBinXStep = ( myHistogram.maximum - myHistogram.minimum ) / myHistogram.binCount;
      myBinX = myHistogram.minimum + myBinXStep / 2.0;
    }
    else
    {
      myBinXStep = 1;
      myBinX = 0;
    }

    for ( int myBin = 0; myBin < myHistogram.binCount; myBin++ )
    {
      int myBinValue = myHistogram.histogramVector.at( myBin );
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
      if ( myDrawLines )
      {
        data << QPointF( myBinX, myBinValue );
      }
      else
      {
        dataHisto << QwtIntervalSample( myBinValue, myBinX - myBinXStep / 2.0, myBinX + myBinXStep / 2.0 );
      }
#else
      if ( myDrawLines )
      {
        myX2Data.append( double( myBinX ) );
        myY2Data.append( double( myBinValue ) );
      }
      else
      {
        intervalsHisto.append( QwtDoubleInterval( myBinX - myBinXStep / 2.0, myBinX + myBinXStep / 2.0 ) );
        valuesHisto.append( double( myBinValue ) );
      }
#endif
      myBinX += myBinXStep;
    }

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
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
#else
    if ( myDrawLines )
    {
      mypCurve->setData( myX2Data, myY2Data );
      mypCurve->attach( mpPlot );
    }
    else
    {
      mypHistoItem->setData( QwtIntervalData( intervalsHisto, valuesHisto ) );
      mypHistoItem->attach( mpPlot );
    }
#endif

    if ( myFirstIteration || mHistoMin > myHistogram.minimum )
    {
      mHistoMin = myHistogram.minimum;
    }
    if ( myFirstIteration || mHistoMax < myHistogram.maximum )
    {
      mHistoMax = myHistogram.maximum;
    }
    QgsDebugMsg( QString( "computed histo min = %1 max = %2" ).arg( mHistoMin ).arg( mHistoMax ) );
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
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
      mHistoPicker->setStateMachine( new QwtPickerDragPointMachine );
      connect( mHistoPicker, SIGNAL( selected( const QPointF & ) ), this, SLOT( histoPickerSelected( const QPointF & ) ) );
#else
      mHistoPicker->setSelectionFlags( QwtPicker::PointSelection | QwtPicker::DragSelection );
      connect( mHistoPicker, SIGNAL( selected( const QwtDoublePoint & ) ), this, SLOT( histoPickerSelectedQwt5( const QwtDoublePoint & ) ) );
#endif
    }
    mHistoPicker->setEnabled( false );

    // plot zoomer
    if ( !mHistoZoomer )
    {
      mHistoZoomer = new QwtPlotZoomer( mpPlot->canvas() );
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
      mHistoZoomer->setStateMachine( new QwtPickerDragRectMachine );
#else
      mHistoZoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
#endif
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

  disconnect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  stackedWidget2->setCurrentIndex( 0 );
  // icon from http://findicons.com/icon/169577/14_zoom?id=171427
  mpPlot->canvas()->setCursor( QCursor( QgsApplication::getThemePixmap( "/mIconZoom.svg" ) ) );
  //  on_cboHistoBand_currentIndexChanged( -1 );
  QApplication::restoreOverrideCursor();
}

void QgsRasterHistogramWidget::on_mSaveAsImageButton_clicked()
{
  if ( mpPlot == 0 )
  {
    return;
  }

  QPair< QString, QString> myFileNameAndFilter = QgisGui::getSaveAsImageName( this, tr( "Choose a file name to save the map image as" ) );
  QFileInfo myInfo( myFileNameAndFilter.first );
  if ( QFileInfo( myFileNameAndFilter.first ).baseName() != "" )
  {
    histoSaveAsImage( myFileNameAndFilter.first );
  }
}

bool QgsRasterHistogramWidget::histoSaveAsImage( const QString& theFilename,
    int width, int height, int quality )
{
  // make sure dir. exists
  QFileInfo myInfo( theFilename );
  QDir myDir( myInfo.dir() );
  if ( ! myDir.exists() )
  {
    QgsDebugMsg( QString( "Error, directory %1 non-existent (theFilename = %2)" ).arg( myDir.absolutePath() ).arg( theFilename ) );
    return false;
  }

  // prepare the pixmap
  QPixmap myPixmap( width, height );
  QRect myQRect( 5, 5, width - 10, height - 10 ); // leave a 5px border on all sides
  myPixmap.fill( Qt::white ); // Qt::transparent ?

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  QwtPlotRenderer myRenderer;
  myRenderer.setDiscardFlags( QwtPlotRenderer::DiscardBackground |
                              QwtPlotRenderer::DiscardCanvasBackground );
  myRenderer.setLayoutFlags( QwtPlotRenderer::FrameWithScales );

  QPainter myPainter;
  myPainter.begin( &myPixmap );
  myRenderer.render( mpPlot, &myPainter, myQRect );
  myPainter.end();
#else
  QwtPlotPrintFilter myFilter;
  int myOptions = QwtPlotPrintFilter::PrintAll;
  myOptions &= ~QwtPlotPrintFilter::PrintBackground;
  myOptions |= QwtPlotPrintFilter::PrintFrameWithScales;
  myFilter.setOptions( myOptions );

  QPainter myPainter;
  myPainter.begin( &myPixmap );
  mpPlot->print( &myPainter, myQRect, myFilter );
  myPainter.end();

  // "fix" for bug in qwt5 - legend and plot shifts a bit
  // can't see how to avoid this without picking qwt5 apart...
  refreshHistogram();
  refreshHistogram();
#endif

  // save pixmap to file
  myPixmap.save( theFilename, 0, quality );

  // should do more error checking
  return true;
}

void QgsRasterHistogramWidget::setSelectedBand( int theBandNo )
{
  cboHistoBand->setCurrentIndex( theBandNo - 1 );
}

void QgsRasterHistogramWidget::on_cboHistoBand_currentIndexChanged( int index )
{
  if ( mHistoShowBands == ShowSelected )
    refreshHistogram();

  // get the current index value, index can be -1
  index = cboHistoBand->currentIndex();
  if ( mHistoPicker != NULL )
  {
    mHistoPicker->setEnabled( false );
    mHistoPicker->setRubberBandPen( QPen( mHistoColors.at( index + 1 ) ) );
  }
  if ( mHistoZoomer != NULL )
    mHistoZoomer->setEnabled( true );
  btnHistoMin->setEnabled( true );
  btnHistoMax->setEnabled( true );

  QPair< QString, QString > myMinMax = rendererMinMax( index + 1 );
  leHistoMin->setText( myMinMax.first );
  leHistoMax->setText( myMinMax.second );

  applyHistoMin();
  applyHistoMax();
}

void QgsRasterHistogramWidget::histoActionTriggered( QAction* action )
{
  if ( ! action )
    return;
  histoAction( action->data().toString(), action->isChecked() );
}

void QgsRasterHistogramWidget::histoAction( const QString actionName, bool actionFlag )
{
  if ( actionName == "" )
    return;

  // this approach is a bit of a hack, but this way we don't have to define slots for each action
  QgsDebugMsg( QString( "band = %1 action = %2" ).arg( cboHistoBand->currentIndex() + 1 ).arg( actionName ) );

  // checkeable actions
  if ( actionName == "Show markers" )
  {
    mHistoShowMarkers = actionFlag;
    QSettings settings;
    settings.setValue( "/Raster/histogram/showMarkers", mHistoShowMarkers );
    updateHistoMarkers();
    return;
  }
  else if ( actionName == "Zoom min_max" )
  {
    mHistoZoomToMinMax = actionFlag;
    QSettings settings;
    settings.setValue( "/Raster/histogram/zoomToMinMax", mHistoZoomToMinMax );
    return;
  }
  else if ( actionName == "Update min_max" )
  {
    mHistoUpdateStyleToMinMax = actionFlag;
    QSettings settings;
    settings.setValue( "/Raster/histogram/updateStyleToMinMax", mHistoUpdateStyleToMinMax );
    return;
  }
  else if ( actionName == "Show all" )
  {
    mHistoShowBands = ShowAll;
    // settings.setValue( "/Raster/histogram/showBands", (int)mHistoShowBands );
    refreshHistogram();
    return;
  }
  else if ( actionName == "Show selected" )
  {
    mHistoShowBands = ShowSelected;
    // settings.setValue( "/Raster/histogram/showBands", (int)mHistoShowBands );
    refreshHistogram();
    return;
  }
  else if ( actionName == "Show RGB" )
  {
    mHistoShowBands = ShowRGB;
    // settings.setValue( "/Raster/histogram/showBands", (int)mHistoShowBands );
    refreshHistogram();
    return;
  }
  else if ( actionName == "Draw lines" )
  {
    mHistoDrawLines = actionFlag;
    QSettings settings;
    settings.setValue( "/Raster/histogram/drawLines", mHistoDrawLines );
    on_btnHistoCompute_clicked(); // refresh
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
  else if ( actionName.left( 5 ) == "Load " && mRendererWidget )
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
    /*
    double myStdDev = 1.0;
    if ( actionName == "Load stddev" )
    {
      myStdDev = mRendererWidget->stdDev().toDouble();
    }
    */

    // don't update markers every time
    leHistoMin->blockSignals( true );
    leHistoMax->blockSignals( true );

    // process each band
    foreach ( int theBandNo, myBands )
    {
      ok = false;
#if 0
      if ( actionName == "Load actual" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::Actual,
                                          theBandNo, minMaxValues );
      }
      else if ( actionName == "Load estimate" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::Estimate,
                                          theBandNo, minMaxValues );
      }
      else if ( actionName == "Load extent" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::CurrentExtent,
                                          theBandNo, minMaxValues );
      }
      else if ( actionName == "Load 1 stddev" ||
                actionName == "Load stddev" )
      {
        ok = mRendererWidget->bandMinMaxFromStdDev( myStdDev, theBandNo, minMaxValues );
      }
#endif

      // apply current item
      cboHistoBand->setCurrentIndex( theBandNo - 1 );
      if ( !ok || actionName == "Load reset" )
      {
        leHistoMin->clear();
        leHistoMax->clear();
#if 0
        // TODO - fix gdal provider: changes data type when nodata value is not found
        // this prevents us from getting proper min and max values here
        minMaxValues[0] = QgsContrastEnhancement::minimumValuePossible(
                            ( QGis::DataType ) mRasterLayer->dataProvider()->dataType( theBandNo ) );
        minMaxValues[1] = QgsContrastEnhancement::maximumValuePossible(
                            ( QGis::DataType ) mRasterLayer->dataProvider()->dataType( theBandNo ) );
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
  else if ( actionName == "Compute histogram" )
  {
    on_btnHistoCompute_clicked();
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

  int theBandNo = cboHistoBand->currentIndex() + 1;
  QList< int > mySelectedBands = rendererSelectedBands();
  QString min;
  for ( int i = 0; i <= mySelectedBands.size(); i++ )
  {
    if ( theBandNo == mRendererWidget->selectedBand( i ) )
    {
      min = leHistoMin->text();
      if ( mHistoUpdateStyleToMinMax )
        mRendererWidget->setMin( min, i );
    }
  }

  updateHistoMarkers();

  if ( ! min.isEmpty() && mHistoZoomToMinMax && mHistoZoomer )
  {
    QRectF rect = mHistoZoomer->zoomRect();
    rect.setLeft( min.toDouble() );
    mHistoZoomer->zoom( rect );
  }

}

void QgsRasterHistogramWidget::applyHistoMax()
{
  if ( ! mRendererWidget )
    return;

  int theBandNo = cboHistoBand->currentIndex() + 1;
  QList< int > mySelectedBands = rendererSelectedBands();
  QString max;
  for ( int i = 0; i <= mySelectedBands.size(); i++ )
  {
    if ( theBandNo == mRendererWidget->selectedBand( i ) )
    {
      max = leHistoMax->text();
      if ( mHistoUpdateStyleToMinMax )
        mRendererWidget->setMax( max, i );
    }
  }

  updateHistoMarkers();

  if ( ! max.isEmpty() && mHistoZoomToMinMax && mHistoZoomer )
  {
    QRectF rect = mHistoZoomer->zoomRect();
    rect.setRight( max.toDouble() );
    mHistoZoomer->zoom( rect );
  }
}

void QgsRasterHistogramWidget::on_btnHistoMin_toggled()
{
  if ( mpPlot != NULL && mHistoPicker != NULL )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    if ( btnHistoMin->isChecked() )
    {
      btnHistoMax->setChecked( false );
      QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    if ( mHistoZoomer != NULL )
      mHistoZoomer->setEnabled( ! btnHistoMax->isChecked() );
    mHistoPicker->setEnabled( btnHistoMin->isChecked() );
  }
  updateHistoMarkers();
}

void QgsRasterHistogramWidget::on_btnHistoMax_toggled()
{
  if ( mpPlot != NULL && mHistoPicker != NULL )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    if ( btnHistoMax->isChecked() )
    {
      btnHistoMin->setChecked( false );
      QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    if ( mHistoZoomer != NULL )
      mHistoZoomer->setEnabled( ! btnHistoMax->isChecked() );
    mHistoPicker->setEnabled( btnHistoMax->isChecked() );
  }
  updateHistoMarkers();
}

// local function used by histoPickerSelected(), to get a rounded picked value
// this is sensitive and may not always be correct, needs more testing
QString findClosestTickVal( double target, const QwtScaleDiv * scale, int div = 100 )
{
  if ( !scale ) return "";

  QList< double > minorTicks = scale->ticks( QwtScaleDiv::MinorTick );
  QList< double > majorTicks = scale->ticks( QwtScaleDiv::MajorTick );
  double diff = ( minorTicks[1] - minorTicks[0] ) / div;
  double min = majorTicks[0] - diff;
  if ( min > target )
    min -= ( majorTicks[1] - majorTicks[0] );
#if defined(QWT_VERSION) && QWT_VERSION<0x050200
  double max = scale->hBound();
#else
  double max = scale->upperBound();
#endif
  double closest = target;
  double current = min;

  while ( current < max )
  {
    current += diff;
    if ( current > target )
    {
      closest = ( abs( target - current + diff ) < abs( target - current ) ) ? current - diff : current;
      break;
    }
  }

  // QgsDebugMsg( QString( "target=%1 div=%2 closest=%3" ).arg( target ).arg( div ).arg( closest ) );
  return QString::number( closest );
}

void QgsRasterHistogramWidget::histoPickerSelected( const QPointF & pos )
{
  if ( btnHistoMin->isChecked() || btnHistoMax->isChecked() )
  {
#if defined(QWT_VERSION) && QWT_VERSION>=0x060100
    const QwtScaleDiv * scale = &mpPlot->axisScaleDiv( QwtPlot::xBottom );
#else
    const QwtScaleDiv * scale = mpPlot->axisScaleDiv( QwtPlot::xBottom );
#endif

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

void QgsRasterHistogramWidget::histoPickerSelectedQwt5( const QwtDoublePoint & pos )
{
  histoPickerSelected( QPointF( pos.x(), pos.y() ) );
}

void QgsRasterHistogramWidget::updateHistoMarkers()
{
  // hack to not update markers
  if ( leHistoMin->signalsBlocked() )
    return;
  // todo error checking
  if ( mpPlot == NULL || mHistoMarkerMin == NULL || mHistoMarkerMax == NULL )
    return;

  int theBandNo = cboHistoBand->currentIndex() + 1;
  QList< int > mySelectedBands = histoSelectedBands();

  if (( ! mHistoShowMarkers && ! btnHistoMin->isChecked() && ! btnHistoMax->isChecked() ) ||
      ( ! mySelectedBands.isEmpty() && ! mySelectedBands.contains( theBandNo ) ) )
  {
    mHistoMarkerMin->hide();
    mHistoMarkerMax->hide();
    mpPlot->replot();
    return;
  }

  double minVal = mHistoMin;
  double maxVal = mHistoMax;
  QString minStr = leHistoMin->text();
  QString maxStr = leHistoMax->text();
  if ( minStr != "" )
    minVal = minStr.toDouble();
  if ( maxStr != "" )
    maxVal = maxStr.toDouble();

  QPen linePen = QPen( mHistoColors.at( theBandNo ) );
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

  if ( mRendererName == "singlebandgray" )
  {
    mySelectedBands << mRendererWidget->selectedBand();
  }
  else if ( mRendererName == "multibandcolor" )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      mySelectedBands << mRendererWidget->selectedBand( i );
    }
  }

  return mySelectedBands;
}

QPair< QString, QString > QgsRasterHistogramWidget::rendererMinMax( int theBandNo )
{
  QPair< QString, QString > myMinMax;

  if ( ! mRendererWidget )
    return myMinMax;

  if ( mRendererName == "singlebandgray" )
  {
    if ( theBandNo == mRendererWidget->selectedBand() )
    {
      myMinMax.first = mRendererWidget->min();
      myMinMax.second = mRendererWidget->max();
    }
  }
  else if ( mRendererName == "multibandcolor" )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      if ( theBandNo == mRendererWidget->selectedBand( i ) )
      {
        myMinMax.first = mRendererWidget->min( i );
        myMinMax.second = mRendererWidget->max( i );
        break;
      }
    }
  }

  // TODO - there are 2 definitions of raster data type that should be unified
  // QgsRasterDataProvider::DataType and QGis::DataType
  // TODO - fix gdal provider: changes data type when nodata value is not found
  // this prevents us from getting proper min and max values here
  // minStr = QString::number( QgsContrastEnhancement::minimumValuePossible( ( QGis::DataType )
  //                                                                         mRasterLayer->dataProvider()->dataType( theBandNo ) ) );
  // maxStr = QString::number( QgsContrastEnhancement::maximumValuePossible( ( QGis::DataType )
  //                                                                         mRasterLayer->dataProvider()->dataType( theBandNo ) ) );

  // if we get an empty result, fill with default value (histo min/max)
  if ( myMinMax.first.isEmpty() )
    myMinMax.first = QString::number( mHistoMin );
  if ( myMinMax.second.isEmpty() )
    myMinMax.second = QString::number( mHistoMax );

  QgsDebugMsg( QString( "bandNo %1 got min/max [%2] [%3]" ).arg( theBandNo ).arg( myMinMax.first ).arg( myMinMax.second ) );

  return myMinMax;
}
