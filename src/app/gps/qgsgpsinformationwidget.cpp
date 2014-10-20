/***************************************************************************
               qgsgpsinformationwidget.cpp  -  description
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Tim Sutton and Marco Hugentobler
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgpsinformationwidget.h"

#include "info.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsfeatureaction.h"
#include "qgsgeometry.h"
#include "qgsgpsconnectionregistry.h"
#include "qgsgpsdetector.h"
#include "qgslayertreeview.h"
#include "qgslogger.h"
#include "qgsmaprenderer.h"
#include "qgsmaptooladdfeature.h"
#include "qgsnmeaconnection.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"


// QWT Charting widget
#include <qwt_global.h>
#if (QWT_VERSION<0x060000)
#include <qwt_array.h>
#include <qwt_data.h>
#endif
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>

// QWT Polar plot add on
#include <qwt_symbol.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_scale_engine.h>

#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QColorDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QPen>

QgsGPSInformationWidget::QgsGPSInformationWidget( QgsMapCanvas * thepCanvas, QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , mNmea( 0 )
    , mpCanvas( thepCanvas )
{
  setupUi( this );

  mpLastLayer = 0;

  mLastGpsPosition = QgsPoint( 0.0, 0.0 );

  mpMapMarker = 0;
  mpRubberBand = 0;
  populateDevices();
  QWidget * mpHistogramWidget = mStackedWidget->widget( 1 );
  //
  // Set up the graph for signal strength
  //
  mpPlot = new QwtPlot( mpHistogramWidget );
  mpPlot->setAutoReplot( false );   // plot on demand
  //mpPlot->setTitle(QObject::tr("Signal Status"));
  //mpPlot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
  // Set axis titles
  //mpPlot->setAxisTitle(QwtPlot::xBottom, QObject::tr("Satellite"));
  //mpPlot->setAxisTitle(QwtPlot::yLeft, QObject::tr("Value"));
  mpPlot->setAxisScale( QwtPlot::xBottom, 0, 20 );
  mpPlot->setAxisScale( QwtPlot::yLeft, 0, 100 );  // max is 50dB SNR, I believe - SLM
  // add a grid
  //QwtPlotGrid * mypGrid = new QwtPlotGrid();
  //mypGrid->attach( mpPlot );
  //display satellites first
  mpCurve = new QwtPlotCurve();
  mpCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
  mpCurve->setPen( QPen( Qt::blue ) );
  mpCurve->setBrush( QBrush( Qt::blue ) );
  mpPlot->enableAxis( QwtPlot::yLeft, false );
  mpPlot->enableAxis( QwtPlot::xBottom, false );
  mpCurve->attach( mpPlot );
  //ensure all children get removed
  mpPlot->setAutoDelete( true );
  QVBoxLayout *mpHistogramLayout = new QVBoxLayout( mpHistogramWidget );
  mpHistogramLayout->setContentsMargins( 0, 0, 0, 0 );
  mpHistogramLayout->addWidget( mpPlot );
  mpHistogramWidget->setLayout( mpHistogramLayout );

  //
  // Set up the polar graph for satellite pos
  //
  QWidget * mpPolarWidget = mStackedWidget->widget( 2 );
  mpSatellitesWidget = new QwtPolarPlot( /*QwtText( tr( "Satellite View" ), QwtText::PlainText ),*/ mpPolarWidget );  // possible title for graph removed for now as it is too large in small windows
  mpSatellitesWidget->setAutoReplot( false );   // plot on demand (after all data has been handled)
  mpSatellitesWidget->setPlotBackground( Qt::white );
  // scales
  mpSatellitesWidget->setScale( QwtPolar::ScaleAzimuth,
                                360, //min - reverse the min/max values to get compass orientation - increasing clockwise
                                0, //max
                                90 //interval - just show cardinal and intermediate (NE, N, NW, etc.) compass points (in degrees)
                              );
  mpSatellitesWidget->setAzimuthOrigin( M_PI_2 );    // to get compass orientation - need to rotate 90 deg. ccw; this is in Radians (not indicated in QwtPolarPlot docs)

//  mpSatellitesWidget->setScaleMaxMinor( QwtPolar::ScaleRadius, 2 );  // seems unnecessary
  mpSatellitesWidget->setScale( QwtPolar::ScaleRadius,
                                90, //min - reverse the min/max to get 0 at edge, 90 at center
                                0, //max
                                45 //interval
                              );

  // grids, axes

  QwtPolarGrid * mypSatellitesGrid = new QwtPolarGrid();
  mypSatellitesGrid->setGridAttribute( QwtPolarGrid::AutoScaling, false );   // This fixes the issue of autoscaling on the Radius grid. It is ON by default AND is separate from the scaleData.doAutoScale in QwtPolarPlot::setScale(), etc. THIS IS VERY TRICKY!
  mypSatellitesGrid->setPen( QPen( Qt::black ) );
  QPen minorPen( Qt::gray );  // moved outside of for loop; NOTE setting the minor pen isn't necessary if the minor grids aren't shown
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
  {
    //mypSatellitesGrid->showGrid( scaleId );
    //mypSatellitesGrid->showMinorGrid(scaleId);
    mypSatellitesGrid->setMinorGridPen( scaleId, minorPen );
  }
//  mypSatellitesGrid->setAxisPen( QwtPolar::AxisAzimuth, QPen( Qt::black ) );

  mypSatellitesGrid->showAxis( QwtPolar::AxisAzimuth, true );
  mypSatellitesGrid->showAxis( QwtPolar::AxisLeft, false ); //alt axis
  mypSatellitesGrid->showAxis( QwtPolar::AxisRight, false );//alt axis
  mypSatellitesGrid->showAxis( QwtPolar::AxisTop, false );//alt axis
  mypSatellitesGrid->showAxis( QwtPolar::AxisBottom, false );//alt axis
  mypSatellitesGrid->showGrid( QwtPolar::ScaleAzimuth, false ); // hide the grid; just show ticks at edge
  mypSatellitesGrid->showGrid( QwtPolar::ScaleRadius, true );
//  mypSatellitesGrid->showMinorGrid( QwtPolar::ScaleAzimuth, true );
  mypSatellitesGrid->showMinorGrid( QwtPolar::ScaleRadius, true );   // for 22.5, 67.5 degree circles
  mypSatellitesGrid->attach( mpSatellitesWidget );

  //QwtLegend *legend = new QwtLegend;
  //mpSatellitesWidget->insertLegend(legend, QwtPolarPlot::BottomLegend);
  QVBoxLayout *mpPolarLayout = new QVBoxLayout( mpPolarWidget );
  mpPolarLayout->setContentsMargins( 0, 0, 0, 0 );
  mpPolarLayout->addWidget( mpSatellitesWidget );
  mpPolarWidget->setLayout( mpPolarLayout );

  // replot on command
  mpSatellitesWidget->replot();
  mpPlot->replot();

  // Restore state
  QSettings mySettings;
  mGroupShowMarker->setChecked( mySettings.value( "/gps/showMarker", "true" ).toBool() );
  mSliderMarkerSize->setValue( mySettings.value( "/gps/markerSize", "12" ).toInt() );
  mSpinTrackWidth->setValue( mySettings.value( "/gps/trackWidth", "2" ).toInt() );
  mTrackColor = mySettings.value( "/gps/trackColor", QColor( Qt::red ) ).value<QColor>();
  QString myPortMode = mySettings.value( "/gps/portMode", "scanPorts" ).toString();

  mSpinMapExtentMultiplier->setValue( mySettings.value( "/gps/mapExtentMultiplier", "50" ).toInt() );
  mDateTimeFormat = mySettings.value( "/gps/dateTimeFormat", "" ).toString(); // zero-length string signifies default format

  mGpsdHost->setText( mySettings.value( "/gps/gpsdHost", "localhost" ).toString() );
  mGpsdPort->setText( mySettings.value( "/gps/gpsdPort", 2947 ).toString() );
  mGpsdDevice->setText( mySettings.value( "/gps/gpsdDevice" ).toString() );

  //port mode
  if ( myPortMode == "scanPorts" )
  {
    mRadAutodetect->setChecked( true );
  }
  else if ( myPortMode == "internalGPS" )
  {
    mRadInternal->setChecked( true );
  }
  else if ( myPortMode == "explicitPort" )
  {
    mRadUserPath->setChecked( true );
  }
  else if ( myPortMode == "gpsd" )
  {
    mRadGpsd->setChecked( true );
  }
  //disable the internal port method if build is without QtLocation
#ifndef HAVE_QT_MOBILITY_LOCATION
  mRadInternal->setDisabled( true );
  mRadAutodetect->setChecked( true );
#endif

  //auto digitising behaviour
  mCbxAutoAddVertices->setChecked( mySettings.value( "/gps/autoAddVertices", "false" ).toBool() );

  mCbxAutoCommit->setChecked( mySettings.value( "/gps/autoCommit", "false" ).toBool() );

  //pan mode
  QString myPanMode = mySettings.value( "/gps/panMode", "recenterWhenNeeded" ).toString();
  if ( myPanMode == "none" )
  {
    radNeverRecenter->setChecked( true );
  }
  else if ( myPanMode == "recenterAlways" )
  {
    radRecenterMap->setChecked( true );
  }
  else
  {
    radRecenterWhenNeeded->setChecked( true );
  }

  mWgs84CRS.createFromOgcWmsCrs( "EPSG:4326" );

  mBtnDebug->setVisible( mySettings.value( "/gps/showDebug", "false" ).toBool() );  // use a registry setting to control - power users/devs could set it

  // status = unknown
  setStatusIndicator( NoData );

  //SLM - added functionality
  mLogFile = 0;

  connect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
           this, SLOT( updateCloseFeatureButton( QgsMapLayer* ) ) );

  mStackedWidget->setCurrentIndex( 3 ); // force to Options
  mBtnPosition->setFocus( Qt::TabFocusReason );
}

QgsGPSInformationWidget::~QgsGPSInformationWidget()
{
  if ( mNmea )
  {
    disconnectGps();
  }

  if ( mpMapMarker )
    delete mpMapMarker;

  QSettings mySettings;
  mySettings.setValue( "/gps/lastPort", mCboDevices->itemData( mCboDevices->currentIndex() ).toString() );
  mySettings.setValue( "/gps/trackWidth", mSpinTrackWidth->value() );
  mySettings.setValue( "/gps/trackColor", mTrackColor );
  mySettings.setValue( "/gps/markerSize", mSliderMarkerSize->value() );
  mySettings.setValue( "/gps/showMarker", mGroupShowMarker->isChecked() );
  mySettings.setValue( "/gps/autoAddVertices", mCbxAutoAddVertices->isChecked() );
  mySettings.setValue( "/gps/autoCommit", mCbxAutoCommit->isChecked() );

  mySettings.setValue( "/gps/mapExtentMultiplier", mSpinMapExtentMultiplier->value() );

  // scan, explicit port or gpsd
  if ( mRadAutodetect->isChecked() )
  {
    mySettings.setValue( "/gps/portMode", "scanPorts" );
  }
  else if ( mRadInternal->isChecked() )
  {
    mySettings.setValue( "/gps/portMode", "internalGPS" );
  }
  else if ( mRadUserPath->isChecked() )
  {
    mySettings.setValue( "/gps/portMode", "explicitPort" );
  }
  else
  {
    mySettings.setValue( "/gps/portMode", "gpsd" );
  }

  mySettings.setValue( "/gps/gpsdHost", mGpsdHost->text() );
  mySettings.setValue( "/gps/gpsdPort", mGpsdPort->text().toInt() );
  mySettings.setValue( "/gps/gpsdDevice", mGpsdDevice->text() );

  // pan mode
  if ( radRecenterMap->isChecked() )
  {
    mySettings.setValue( "/gps/panMode", "recenterAlways" );
  }
  else if ( radRecenterWhenNeeded->isChecked() )
  {
    mySettings.setValue( "/gps/panMode", "recenterWhenNeeded" );
  }
  else
  {
    mySettings.setValue( "/gps/panMode", "none" );
  }

  if ( mpRubberBand )
  {
    delete mpRubberBand;
  }
}

void QgsGPSInformationWidget::on_mSpinTrackWidth_valueChanged( int theValue )
{
  if ( mpRubberBand )
  {
    mpRubberBand->setWidth( theValue );
  }
}

void QgsGPSInformationWidget::on_mBtnTrackColor_clicked()
{
  QColor myColor = QColorDialog::getColor( mTrackColor, this );
  if ( myColor.isValid() )  // check that a color was picked
  {
    mTrackColor = myColor;
    if ( mpRubberBand )
    {
      mpRubberBand->setColor( myColor );
    }
  }
}

void QgsGPSInformationWidget::on_mBtnPosition_clicked()
{
  mStackedWidget->setCurrentIndex( 0 );
  if ( mNmea )
    displayGPSInformation( mNmea->currentGPSInformation() );
}

void QgsGPSInformationWidget::on_mBtnSignal_clicked()
{
  mStackedWidget->setCurrentIndex( 1 );
  if ( mNmea )
    displayGPSInformation( mNmea->currentGPSInformation() );
}

void QgsGPSInformationWidget::on_mBtnSatellites_clicked()
{
  mStackedWidget->setCurrentIndex( 2 );
  if ( mNmea )
    displayGPSInformation( mNmea->currentGPSInformation() );
}

void QgsGPSInformationWidget::on_mBtnOptions_clicked()
{
  mStackedWidget->setCurrentIndex( 3 );
}

void QgsGPSInformationWidget::on_mBtnDebug_clicked()
{
  mStackedWidget->setCurrentIndex( 4 );
}

void QgsGPSInformationWidget::on_mConnectButton_toggled( bool theFlag )
{
  if ( theFlag )
  {
    connectGps();
  }
  else
  {
    disconnectGps();
  }
}

void QgsGPSInformationWidget::connectGps()
{
  // clear position page fields to give better indication that something happened (or didn't happen)
  mTxtLatitude->clear();
  mTxtLongitude->clear();
  mTxtAltitude->clear();
  mTxtDateTime->clear();
  mTxtSpeed->clear();
  mTxtDirection->clear();
  mTxtHdop->clear();
  mTxtVdop->clear();
  mTxtPdop->clear();
  mTxtFixMode->clear();
  mTxtFixType->clear();
  mTxtQuality->clear();
  mTxtSatellitesUsed->clear();
  mTxtStatus->clear();

  mLastGpsPosition = QgsPoint( 0.0, 0.0 );

  QString port;

  if ( mRadUserPath->isChecked() )
  {
    port = mCboDevices->itemData( mCboDevices->currentIndex() ).toString();

    if ( port.isEmpty() )
    {
      QMessageBox::information( this, tr( "/gps" ), tr( "No path to the GPS port "
                                "is specified. Please enter a path then try again." ) );
      //toggle the button back off
      mConnectButton->setChecked( false );
      return;
    }
  }
  else if ( mRadGpsd->isChecked() )
  {
    port = QString( "%1:%2:%3" ).arg( mGpsdHost->text() ).arg( mGpsdPort->text() ).arg( mGpsdDevice->text() );
  }
  else if ( mRadInternal->isChecked() )
  {
    port = QString( "internalGPS" );
  }

  mGPSPlainTextEdit->appendPlainText( tr( "Connecting..." ) );
  showStatusBarMessage( tr( "Connecting to GPS device..." ) );

  QgsGPSDetector *detector = new QgsGPSDetector( port );
  connect( detector, SIGNAL( detected( QgsGPSConnection * ) ), this, SLOT( connected( QgsGPSConnection * ) ) );
  connect( detector, SIGNAL( detectionFailed() ), this, SLOT( timedout() ) );
  detector->advance();   // start the detection process
}

void QgsGPSInformationWidget::timedout()
{
  mConnectButton->setChecked( false );
  mNmea = NULL;
  mGPSPlainTextEdit->appendPlainText( tr( "Timed out!" ) );
  showStatusBarMessage( tr( "Failed to connect to GPS device." ) );
}

void QgsGPSInformationWidget::connected( QgsGPSConnection *conn )
{
  mNmea = conn;
  connect( mNmea, SIGNAL( stateChanged( const QgsGPSInformation& ) ),
           this, SLOT( displayGPSInformation( const QgsGPSInformation& ) ) );
  mGPSPlainTextEdit->appendPlainText( tr( "Connected!" ) );
  mConnectButton->setText( tr( "Dis&connect" ) );
  //insert connection into registry such that it can also be used by other dialogs or plugins
  QgsGPSConnectionRegistry::instance()->registerConnection( mNmea );
  showStatusBarMessage( tr( "Connected to GPS device." ) );

  if ( mLogFileGroupBox->isChecked() && ! mTxtLogFile->text().isEmpty() )
  {
    if ( ! mLogFile )
    {
      mLogFile = new QFile( mTxtLogFile->text() );
    }

    if ( mLogFile->open( QIODevice::Append ) )  // open in binary and explicitly output CR + LF per NMEA
    {
      mLogFileTextStream.setDevice( mLogFile );

      // crude way to separate chunks - use when manually editing file - NMEA parsers should discard
      mLogFileTextStream << "====" << "\r\n";

      connect( mNmea, SIGNAL( nmeaSentenceReceived( const QString& ) ), this, SLOT( logNmeaSentence( const QString& ) ) ); // added to handle raw data
    }
    else  // error opening file
    {
      delete mLogFile;
      mLogFile = 0;

      // need to indicate why - this just reports that an error occurred
      showStatusBarMessage( tr( "Error opening log file." ) );
    }
  }
}

void QgsGPSInformationWidget::disconnectGps()
{
  if ( mLogFile && mLogFile->isOpen() )
  {
    disconnect( mNmea, SIGNAL( nmeaSentenceReceived( const QString& ) ), this, SLOT( logNmeaSentence( const QString& ) ) );
    mLogFile->close();
    delete mLogFile;
    mLogFile = 0;
  }

  QgsGPSConnectionRegistry::instance()->unregisterConnection( mNmea );
  delete mNmea;
  mNmea = NULL;
  if ( mpMapMarker )  // marker should not be shown on GPS disconnected - not current position
  {
    delete mpMapMarker;
    mpMapMarker = NULL;
  }
  mGPSPlainTextEdit->appendPlainText( tr( "Disconnected..." ) );
  mConnectButton->setChecked( false );
  mConnectButton->setText( tr( "&Connect" ) );
  showStatusBarMessage( tr( "Disconnected from GPS device." ) );

  setStatusIndicator( NoData );
}

void QgsGPSInformationWidget::displayGPSInformation( const QgsGPSInformation& info )
{
#if QWT_VERSION<0x060000
  QwtArray<double> myXData;//qwtarray is just a wrapped qvector
  QwtArray<double> mySignalData;//qwtarray is just a wrapped qvector
#else
  QVector<QPointF> data;
#endif

  // set validity flag and status from GPS data
  // based on GGA, GSA and RMC sentences - the logic does not require all
  bool validFlag = false; // true if GPS indicates position fix
  FixStatus fixStatus = NoData;

  // no fix if any of the three report bad; default values are invalid values and won't be changed if the corresponding NMEA msg is not received
  if ( info.status == 'V' || info.fixType == NMEA_FIX_BAD || info.quality == 0 ) // some sources say that 'V' indicates position fix, but is below acceptable quality
  {
    fixStatus = NoFix;
  }
  else if ( info.fixType == NMEA_FIX_2D ) // 2D indication (from GGA)
  {
    fixStatus = Fix2D;
    validFlag = true;
  }
  else if ( info.status == 'A' || info.fixType == NMEA_FIX_3D || info.quality > 0 ) // good
  {
    fixStatus = Fix3D;
    validFlag = true;
  }
  else  // unknown status (not likely)
  {
  }

  // set visual status indicator -- do only on change of state
  if ( fixStatus != mLastFixStatus )
  {
    setStatusIndicator( fixStatus );
  }

  if ( mStackedWidget->currentIndex() == 1 && info.satInfoComplete ) //signal
  {
    mpPlot->setAxisScale( QwtPlot::xBottom, 0, info.satellitesInView.size() );
  } //signal

  if ( mStackedWidget->currentIndex() == 2 && info.satInfoComplete ) //satellites
  {
    while ( !mMarkerList.isEmpty() )
    {
      delete mMarkerList.takeFirst();
    }
  } //satellites

  if ( mStackedWidget->currentIndex() == 4 ) //debug
  {
    mGPSPlainTextEdit->clear();
  } //debug

  for ( int i = 0; i < info.satellitesInView.size(); ++i ) //satellite processing loop
  {
    QgsSatelliteInfo currentInfo = info.satellitesInView.at( i );

    if ( mStackedWidget->currentIndex() == 1 && info.satInfoComplete ) //signal
    {
#if QWT_VERSION<0x060000
      myXData.append( i );
      mySignalData.append( 0 );
      myXData.append( i );
      mySignalData.append( currentInfo.signal );
      myXData.append( i + 1 );
      mySignalData.append( currentInfo.signal );
      myXData.append( i + 1 );
      mySignalData.append( 0 );
#else
      data << QPointF( i, 0 );
      data << QPointF( i, currentInfo.signal );
      data << QPointF( i + 1, currentInfo.signal );
      data << QPointF( i + 1, 0 );
#endif
    } //signal

    if ( mStackedWidget->currentIndex() == 2 && info.satInfoComplete ) //satellites
    {
      QColor bg( Qt::white ); // moved several items outside of the following if block to minimize loop time
      bg.setAlpha( 200 );
      QColor myColor;
      QBrush symbolBrush( Qt::black );
      QBrush textBgBrush( bg );
      QSize markerSize( 9, 9 );
      // Add a marker to the polar plot
      if ( currentInfo.id > 0 )       // don't show satellite if id=0 (no satellite indication)
      {
        QwtPolarMarker *mypMarker = new QwtPolarMarker();
#if (QWT_POLAR_VERSION<0x010000)
        mypMarker->setPosition( QwtPolarPoint( currentInfo.azimuth, currentInfo.elevation ) );
#else
        mypMarker->setPosition( QwtPointPolar( currentInfo.azimuth, currentInfo.elevation ) );
#endif
        if ( currentInfo.signal < 30 ) //weak signal
        {
          myColor = Qt::red;
        }
        else
        {
          myColor = Qt::black; //strong signal
        }
#if (QWT_POLAR_VERSION<0x010000)
        mypMarker->setSymbol( QwtSymbol( QwtSymbol::Ellipse,
                                         symbolBrush, QPen( myColor ), markerSize ) );
#else
        mypMarker->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
                                             symbolBrush, QPen( myColor ), markerSize ) );
#endif
        mypMarker->setLabelAlignment( Qt::AlignHCenter | Qt::AlignTop );
        QwtText text( QString::number( currentInfo.id ) );
        text.setColor( myColor );
        text.setBackgroundBrush( textBgBrush );
        mypMarker->setLabel( text );
        mypMarker->attach( mpSatellitesWidget );
        mMarkerList << mypMarker;
      } // currentInfo.id > 0
    } //satellites
  } //satellite processing loop

  if ( mStackedWidget->currentIndex() == 1 && info.satInfoComplete ) //signal
  {
#if (QWT_VERSION<0x060000)
    mpCurve->setData( myXData, mySignalData );
#else
    mpCurve->setSamples( data );
#endif
    mpPlot->replot();
  } //signal

  if ( mStackedWidget->currentIndex() == 2 && info.satInfoComplete ) //satellites
  {
    mpSatellitesWidget->replot();
  } //satellites

  if ( validFlag )
  {
    validFlag = info.longitude >= -180.0 && info.longitude <= 180.0 && info.latitude >= -90.0 && info.latitude <= 90.0;
  }

  QgsPoint myNewCenter;
  if ( validFlag )
  {
    myNewCenter = QgsPoint( info.longitude, info.latitude );
  }
  else
  {
    myNewCenter = mLastGpsPosition;
  }

  if ( mStackedWidget->currentIndex() == 0 ) //position
  {
    mTxtLatitude->setText( QString::number( info.latitude, 'f', 8 ) );
    mTxtLongitude->setText( QString::number( info.longitude, 'f', 8 ) );
    mTxtAltitude->setText( tr( "%1 m" ).arg( info.elevation, 0, 'f', 1 ) ); // don't know of any GPS receivers that output better than 0.1 m precision
    if ( mDateTimeFormat.isEmpty() )
    {
      mTxtDateTime->setText( info.utcDateTime.toString( Qt::TextDate ) );  // default format
    }
    else
    {
      mTxtDateTime->setText( info.utcDateTime.toString( mDateTimeFormat ) );  //user specified format string for testing the millisecond part of time
    }
    mTxtSpeed->setText( tr( "%1 km/h" ).arg( info.speed, 0, 'f', 1 ) );
    mTxtDirection->setText( QString::number( info.direction, 'f', 1 ) + QString::fromUtf8( "°" ) );
    mTxtHdop->setText( QString::number( info.hdop, 'f', 1 ) );
    mTxtVdop->setText( QString::number( info.vdop, 'f', 1 ) );
    mTxtPdop->setText( QString::number( info.pdop, 'f', 1 ) );
    mTxtHacc->setText( QString::number( info.hacc, 'f', 1 ) + "m" );
    mTxtVacc->setText( QString::number( info.vacc, 'f', 1 ) + "m" );
    mTxtFixMode->setText( info.fixMode == 'A' ? tr( "Automatic" ) : info.fixMode == 'M' ? tr( "Manual" ) : "" ); // A=automatic 2d/3d, M=manual; allowing for anything else
    mTxtFixType->setText( info.fixType == 3 ? tr( "3D" ) : info.fixType == 2 ? tr( "2D" ) : info.fixType == 1 ? tr( "No fix" ) : QString::number( info.fixType ) ); // 1=no fix, 2=2D, 3=3D; allowing for anything else
    mTxtQuality->setText( info.quality == 2 ? tr( "Differential" ) : info.quality == 1 ? tr( "Non-differential" ) : info.quality == 0 ? tr( "No position" ) : info.quality > 2 ? QString::number( info.quality ) : "" ); // allowing for anything else
    mTxtSatellitesUsed->setText( QString::number( info.satellitesUsed ) );
    mTxtStatus->setText( info.status == 'A' ? tr( "Valid" ) : info.status == 'V' ? tr( "Invalid" ) : "" );
  } //position

  // Avoid refreshing / panning if we havent moved
  if ( mLastGpsPosition != myNewCenter )
  {
    mLastGpsPosition = myNewCenter;

    // Pan based on user specified behaviour
    if ( radRecenterMap->isChecked() || radRecenterWhenNeeded->isChecked() )
    {
      QgsCoordinateReferenceSystem mypSRS = mpCanvas->mapSettings().destinationCrs();
      QgsCoordinateTransform myTransform( mWgs84CRS, mypSRS ); // use existing WGS84 CRS

      QgsPoint myPoint = myTransform.transform( myNewCenter );
      //keep the extent the same just center the map canvas in the display so our feature is in the middle
      QgsRectangle myRect( myPoint, myPoint );  // empty rect can be used to set new extent that is centered on the point used to construct the rect

      // testing if position is outside some proportion of the map extent
      // this is a user setting - useful range: 5% to 100% (0.05 to 1.0)
      QgsRectangle myExtentLimit( mpCanvas->extent() );
      myExtentLimit.scale( mSpinMapExtentMultiplier->value() * 0.01 );

      // only change the extents if the point is beyond the current extents to minimise repaints
      if ( radRecenterMap->isChecked() ||
           ( radRecenterWhenNeeded->isChecked() && !myExtentLimit.contains( myPoint ) ) )
      {
        mpCanvas->setExtent( myRect );
        mpCanvas->refresh();
      }
    } //otherwise never recenter automatically

    if ( mCbxAutoAddVertices->isChecked() )
    {
      addVertex();
    }
  } // mLastGpsPosition != myNewCenter

  // new marker position after recentering
  if ( mGroupShowMarker->isChecked() ) // show marker
  {
    if ( validFlag ) // update cursor position if valid position
    {                // initially, cursor isn't drawn until first valid fix; remains visible until GPS disconnect
      if ( ! mpMapMarker )
      {
        mpMapMarker = new QgsGpsMarker( mpCanvas );
      }
      mpMapMarker->setSize( mSliderMarkerSize->value() );
      mpMapMarker->setCenter( myNewCenter );
    }
  }
  else
  {
    if ( mpMapMarker )
    {
      delete mpMapMarker;
      mpMapMarker = 0;
    }
  } // show marker
}

void QgsGPSInformationWidget::on_mBtnAddVertex_clicked()
{
  addVertex();
}

void QgsGPSInformationWidget::addVertex()
{
  QgsDebugMsg( "Adding Vertex" );

  if ( !mpRubberBand )
  {
    createRubberBand();
  }

  // we store the capture list in wgs84 and then transform to layer crs when
  // calling close feature
  mCaptureList.push_back( mLastGpsPosition );

  // we store the rubber band points in map canvas CRS so transform to map crs
  // potential problem with transform errors and wrong coordinates if map CRS is changed after points are stored - SLM
  // should catch map CRS change and transform the points
  QgsPoint myPoint;
  if ( mpCanvas )
  {
    QgsCoordinateTransform t( mWgs84CRS, mpCanvas->mapSettings().destinationCrs() );
    myPoint = t.transform( mLastGpsPosition );
  }
  else
  {
    myPoint = mLastGpsPosition;
  }

  mpRubberBand->addPoint( myPoint );
}

void QgsGPSInformationWidget::on_mBtnResetFeature_clicked()
{
  mNmea->disconnect( this, SLOT( displayGPSInformation( const QgsGPSInformation& ) ) );
  createRubberBand(); //deletes existing rubberband
  mCaptureList.clear();
  connectGpsSlot();
}

void QgsGPSInformationWidget::on_mBtnCloseFeature_clicked()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mpCanvas->currentLayer() );
  QGis::WkbType layerWKBType = vlayer->wkbType();

  // -------------- preconditions ------------------------
  // most of these preconditions are already handled due to the button being enabled/disabled based on layer geom type and editing capabilities, but not on valid GPS data

  //lines: bail out if there are not at least two vertices
  if ( layerWKBType == QGis::WKBLineString  && mCaptureList.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough vertices" ),
                              tr( "Cannot close a line feature until it has at least two vertices." ) );
    return;
  }

  //polygons: bail out if there are not at least three vertices
  if ( layerWKBType == QGis::WKBPolygon && mCaptureList.size() < 3 )
  {
    QMessageBox::information( 0, tr( "Not enough vertices" ),
                              tr( "Cannot close a polygon feature until it has at least three vertices." ) );
    return;
  }
  // -------------- end of preconditions ------------------------

  //
  // POINT CAPTURING
  //
  if ( layerWKBType == QGis::WKBPoint )
  {
    QgsFeature* f = new QgsFeature( 0 );

    int size = 0;
    char end = QgsApplication::endian();
    unsigned char *wkb = NULL;
    int wkbtype = 0;

    QgsCoordinateTransform t( mWgs84CRS, vlayer->crs() );
    QgsPoint myPoint = t.transform( mLastGpsPosition );
    double x = myPoint.x();
    double y = myPoint.y();

    size = 1 + sizeof( int ) + 2 * sizeof( double );
    wkb = new unsigned char[size];
    wkbtype = QGis::WKBPoint;
    memcpy( &wkb[0], &end, 1 );
    memcpy( &wkb[1], &wkbtype, sizeof( int ) );
    memcpy( &wkb[5], &x, sizeof( double ) );
    memcpy( &wkb[5] + sizeof( double ), &y, sizeof( double ) );

    f->setGeometryAndOwnership( &wkb[0], size );

    QgsFeatureAction action( tr( "Feature added" ), *f, vlayer, -1, -1, this );
    if ( action.addFeature() )
    {
      if ( mCbxAutoCommit->isChecked() )
      {
        // should canvas->isDrawing() be checked?
        if ( !vlayer->commitChanges() ) //assumed to be vector layer and is editable and is in editing mode (preconditions have been tested)
        {
          QMessageBox::information( this,
                                    tr( "Error" ),
                                    tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                                    .arg( vlayer->name() )
                                    .arg( vlayer->commitErrors().join( "\n  " ) ) );
        }

        vlayer->startEditing();
      }
    }

    delete f;
  } // layerWKBType == QGis::WKBPoint
  else // Line or poly
  {
    mNmea->disconnect( this, SLOT( displayGPSInformation( const QgsGPSInformation& ) ) );

    //create QgsFeature with wkb representation
    QgsFeature* f = new QgsFeature( 0 );
    unsigned char* wkb;
    int size;
    char end = QgsApplication::endian();

    if ( layerWKBType == QGis::WKBLineString )
    {
      size = 1 + 2 * sizeof( int ) + 2 * mCaptureList.size() * sizeof( double );
      wkb = new unsigned char[size];
      int wkbtype = QGis::WKBLineString;
      int length = mCaptureList.size();
      memcpy( &wkb[0], &end, 1 );
      memcpy( &wkb[1], &wkbtype, sizeof( int ) );
      memcpy( &wkb[1+sizeof( int )], &length, sizeof( int ) );
      int position = 1 + 2 * sizeof( int );
      double x, y;
      for ( QList<QgsPoint>::iterator it = mCaptureList.begin(); it != mCaptureList.end(); ++it )
      {
        QgsPoint savePoint = *it;
        // transform the gps point into the layer crs
        QgsCoordinateTransform t( mWgs84CRS, vlayer->crs() );
        QgsPoint myPoint = t.transform( savePoint );
        x = myPoint.x();
        y = myPoint.y();

        memcpy( &wkb[position], &x, sizeof( double ) );
        position += sizeof( double );

        memcpy( &wkb[position], &y, sizeof( double ) );
        position += sizeof( double );
      }
      f->setGeometryAndOwnership( &wkb[0], size );
    }
    else if ( layerWKBType == QGis::WKBPolygon )
    {
      size = 1 + 3 * sizeof( int ) + 2 * ( mCaptureList.size() + 1 ) * sizeof( double );
      wkb = new unsigned char[size];
      int wkbtype = QGis::WKBPolygon;
      int length = mCaptureList.size() + 1;//+1 because the first point is needed twice
      int numrings = 1;
      memcpy( &wkb[0], &end, 1 );
      memcpy( &wkb[1], &wkbtype, sizeof( int ) );
      memcpy( &wkb[1+sizeof( int )], &numrings, sizeof( int ) );
      memcpy( &wkb[1+2*sizeof( int )], &length, sizeof( int ) );
      int position = 1 + 3 * sizeof( int );
      double x, y;
      QList<QgsPoint>::iterator it;
      for ( it = mCaptureList.begin(); it != mCaptureList.end(); ++it )
      {
        QgsPoint savePoint = *it;
        // transform the gps point into the layer crs
        QgsCoordinateTransform t( mWgs84CRS, vlayer->crs() );
        QgsPoint myPoint = t.transform( savePoint );
        x = myPoint.x();
        y = myPoint.y();

        memcpy( &wkb[position], &x, sizeof( double ) );
        position += sizeof( double );

        memcpy( &wkb[position], &y, sizeof( double ) );
        position += sizeof( double );
      }
      // close the polygon
      it = mCaptureList.begin();
      QgsPoint savePoint = *it;
      x = savePoint.x();
      y = savePoint.y();

      memcpy( &wkb[position], &x, sizeof( double ) );
      position += sizeof( double );

      memcpy( &wkb[position], &y, sizeof( double ) );
      f->setGeometryAndOwnership( &wkb[0], size );

      int avoidIntersectionsReturn = f->geometry()->avoidIntersections();
      if ( avoidIntersectionsReturn == 1 )
      {
        //not a polygon type. Impossible to get there
      }
      else if ( avoidIntersectionsReturn == 2 )
      {
        //bail out...
        QMessageBox::critical( 0, tr( "Error" ), tr( "The feature could not be added because removing the polygon intersections would change the geometry type" ) );
        delete f;
        connectGpsSlot();
        return;
      }
      else if ( avoidIntersectionsReturn == 3 )
      {
        QMessageBox::critical( 0, tr( "Error" ), tr( "An error was reported during intersection removal" ) );
        connectGpsSlot();
        return;
      }
    }
    // Should never get here, as preconditions should have removed any that aren't handled
    else // layerWKBType == QGis::WKBPolygon  -  unknown type
    {
      QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot add feature. "
                             "Unknown WKB type. Choose a different layer and try again." ) );
      connectGpsSlot();
      return; //unknown wkbtype
    } // layerWKBType == QGis::WKBPolygon

    QgsFeatureAction action( tr( "Feature added" ), *f, vlayer, -1, -1, this );
    if ( action.addFeature() )
    {
      if ( mCbxAutoCommit->isChecked() )
      {
        if ( !vlayer->commitChanges() ) //swiped... er... appropriated from QgisApp saveEdits()
        {
          QMessageBox::information( this,
                                    tr( "Error" ),
                                    tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                                    .arg( vlayer->name() )
                                    .arg( vlayer->commitErrors().join( "\n  " ) ) );
        }

        vlayer->startEditing();
      }
      delete mpRubberBand;
      mpRubberBand = NULL;

      // delete the elements of mCaptureList
      mCaptureList.clear();
    } // action.addFeature()

    delete f;
    connectGpsSlot();
  } // layerWKBType == QGis::WKBPoint
  mpCanvas->refresh();  // NOTE: cancelling feature add refreshes canvas, OK does not; this may change, however, so do it anyway

  // force focus back to GPS window/ Add Feature button for ease of use by keyboard
  activateWindow();
  mBtnCloseFeature->setFocus( Qt::OtherFocusReason );
}

void QgsGPSInformationWidget::connectGpsSlot()
{
  connect( mNmea, SIGNAL( stateChanged( const QgsGPSInformation& ) ),
           this, SLOT( displayGPSInformation( const QgsGPSInformation& ) ) );
}

void QgsGPSInformationWidget::on_mBtnRefreshDevices_clicked()
{
  populateDevices();
}

/* Copied from gps plugin */
void QgsGPSInformationWidget::populateDevices()
{
  QList< QPair<QString, QString> > ports = QgsGPSDetector::availablePorts();

  mCboDevices->clear();

  // add devices to combobox, but skip gpsd which is first.
  for ( int i = 1; i < ports.size(); i++ )
  {
    mCboDevices->addItem( ports[i].second, ports[i].first );
  }

  // remember the last ports used
  QSettings settings;
  QString lastPort = settings.value( "/gps/lastPort", "" ).toString();

  int idx = mCboDevices->findData( lastPort );
  mCboDevices->setCurrentIndex( idx < 0 ? 0 : idx );
}

void QgsGPSInformationWidget::createRubberBand()
{
  if ( mpRubberBand )
  {
    delete mpRubberBand;
  }
  mpRubberBand = new QgsRubberBand( mpCanvas, false );
  mpRubberBand->setColor( mTrackColor );
  mpRubberBand->setWidth( mSpinTrackWidth->value() );
  mpRubberBand->show();
}

void QgsGPSInformationWidget::on_mBtnLogFile_clicked()
{
//=========================
  // This does not allow for an extension other than ".nmea"
  // Retrieve last used log file dir from persistent settings
  QSettings settings;
  QString settingPath( "/gps/lastLogFileDir" );
  QString lastUsedDir = settings.value( settingPath, "." ).toString();
  QString saveFilePath = QFileDialog::getSaveFileName( this, tr( "Save GPS log file as" ), lastUsedDir, tr( "NMEA files" ) + " (*.nmea)" );
  if ( saveFilePath.isNull() ) //canceled
  {
    return;
  }
  QFileInfo myFI( saveFilePath );
  QString myPath = myFI.path();
  settings.setValue( settingPath, myPath );

  // make sure the .nmea extension is included in the path name. if not, add it...
  if ( "nmea" != myFI.suffix() )
  {
    saveFilePath = myFI.filePath() + ".nmea";
  }
  mTxtLogFile->setText( saveFilePath );
  mTxtLogFile->setToolTip( saveFilePath );
}

void QgsGPSInformationWidget::logNmeaSentence( const QString& nmeaString )
{
  if ( mLogFileGroupBox->isChecked() && mLogFile && mLogFile->isOpen() )
  {
    mLogFileTextStream << nmeaString << "\r\n"; // specifically output CR + LF (NMEA requirement)
  }
}

void QgsGPSInformationWidget::updateCloseFeatureButton( QgsMapLayer * lyr )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( lyr );

  // Add feature button tracks edit state of layer
  if ( vlayer != mpLastLayer )
  {
    if ( mpLastLayer )  // disconnect previous layer
    {
      disconnect( mpLastLayer, SIGNAL( editingStarted() ),
                  this, SLOT( layerEditStateChanged() ) );
      disconnect( mpLastLayer, SIGNAL( editingStopped() ),
                  this, SLOT( layerEditStateChanged() ) );
    }
    if ( vlayer ) // connect new layer
    {
      connect( vlayer, SIGNAL( editingStarted() ),
               this, SLOT( layerEditStateChanged() ) );
      connect( vlayer, SIGNAL( editingStopped() ),
               this, SLOT( layerEditStateChanged() ) );
    }
    mpLastLayer = vlayer;
  }

  QString buttonLabel = tr( "&Add feature" );
  if ( vlayer ) // must be vector layer
  {
    QgsVectorDataProvider* provider = vlayer->dataProvider();
    QGis::WkbType layerWKBType = vlayer->wkbType();

    bool enable =
      ( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) &&  // layer can add features
      vlayer->isEditable() && // layer is editing
      ( // layer has geometry type that can be handled
        layerWKBType == QGis::WKBPoint ||
        layerWKBType == QGis::WKBLineString ||
        layerWKBType == QGis::WKBPolygon
        // add more types here as they are handled
      )
      ;
    switch ( layerWKBType )
    {
      case QGis::WKBPoint:
        buttonLabel = tr( "&Add Point" );
        break;
      case QGis::WKBLineString:
        buttonLabel = tr( "&Add Line" );
        break;
      case QGis::WKBPolygon:
        buttonLabel = tr( "&Add Polygon" );
        break;
        // for the future (also prevent compiler warnings)
      case QGis::WKBMultiPoint:
      case QGis::WKBMultiLineString:
      case QGis::WKBMultiPolygon:
      case QGis::WKBPoint25D:
      case QGis::WKBLineString25D:
      case QGis::WKBPolygon25D:
      case QGis::WKBMultiPoint25D:
      case QGis::WKBMultiLineString25D:
      case QGis::WKBMultiPolygon25D:
      case QGis::WKBUnknown:
      case QGis::WKBNoGeometry:
        ;
    }
    mBtnCloseFeature->setEnabled( enable );
  }
  else
  {
    mBtnCloseFeature->setEnabled( false );
  }
  mBtnCloseFeature->setText( buttonLabel );
}

void QgsGPSInformationWidget::layerEditStateChanged()
{
  updateCloseFeatureButton( mpLastLayer );
}

void QgsGPSInformationWidget::setStatusIndicator( const FixStatus statusValue )
{
  mLastFixStatus = statusValue;
  // the pixmap will be expanded to the size of the label
  QPixmap status( 4, 4 );
  switch ( statusValue )
  {
    case NoFix:
      status.fill( Qt::red );
      break;
    case Fix2D:
      status.fill( Qt::yellow );
      break;
    case Fix3D:
      status.fill( Qt::green );
      break;
    case NoData:
    default: // anything else - shouldn't happen
      status.fill( Qt::darkGray );
  }
  mLblStatusIndicator->setPixmap( status );
}

void QgsGPSInformationWidget::showStatusBarMessage( const QString& msg )
{
  QgisApp::instance()->statusBar()->showMessage( msg );
}
