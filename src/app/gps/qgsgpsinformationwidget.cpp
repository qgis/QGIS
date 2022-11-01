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

#include "qgisapp.h"
#include "qgsappgpsconnection.h"
#include "qgsmaptooladdfeature.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsstatusbar.h"
#include "qgsmapcanvas.h"
#include "qgsgpsconnection.h"
#include "qgscoordinateutils.h"
#include "qgsappgpssettingsmenu.h"

// QWT Charting widget

#include <qwt_global.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>

#ifdef WITH_QWTPOLAR
// QWT Polar plot add on
#include <qwt_symbol.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_scale_engine.h>
#endif

#include <QMessageBox>
#include <QFileInfo>
#include <QColorDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QPen>
#include <QTimeZone>


QgsGpsInformationWidget::QgsGpsInformationWidget( QgsAppGpsConnection *connection,
    QgsMapCanvas *mapCanvas, QWidget *parent )
  : QgsPanelWidget( parent )
  , mConnection( connection )
  , mMapCanvas( mapCanvas )
{
  Q_ASSERT( mMapCanvas ); // precondition
  setupUi( this );
  connect( mConnectButton, &QPushButton::toggled, this, &QgsGpsInformationWidget::mConnectButton_toggled );
  connect( mBtnPosition, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnPosition_clicked );
  connect( mBtnSignal, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnSignal_clicked );
  connect( mBtnSatellites, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnSatellites_clicked );
  connect( mBtnOptions, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnOptions_clicked );
  connect( mBtnDebug, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnDebug_clicked );
  connect( mBtnResetFeature, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnResetFeature_clicked );

  mBtnPopupOptions->setAutoRaise( true );
  mBtnPopupOptions->setToolTip( tr( "Settings" ) );
  mBtnPopupOptions->setMenu( QgisApp::instance()->gpsSettingsMenu() );
  mBtnPopupOptions->setPopupMode( QToolButton::InstantPopup );

  mLogFilename->setDialogTitle( tr( "GPS Log File" ) );
  mLogFilename->setStorageMode( QgsFileWidget::SaveFile );
  mLogFilename->setFilter( tr( "NMEA files" ) + " (*.nmea)" );
  mLogFilename->lineEdit()->setShowClearButton( false );
  const QString lastLogFolder = settingLastLogFolder.value();
  mLogFilename->setDefaultRoot( lastLogFolder.isEmpty() ? QDir::homePath() : lastLogFolder );
  connect( mLogFilename, &QgsFileWidget::fileChanged, this, [ = ]
  {
    settingLastLogFolder.setValue( QFileInfo( mLogFilename->filePath() ).absolutePath() );
  } );


  QWidget *mpHistogramWidget = mStackedWidget->widget( 1 );
#ifndef WITH_QWTPOLAR
  mBtnSatellites->setVisible( false );
#endif
  //
  // Set up the graph for signal strength
  //
  mPlot = new QwtPlot( mpHistogramWidget );
  mPlot->setAutoReplot( false );   // plot on demand
  //mpPlot->setTitle(QObject::tr("Signal Status"));
  //mpPlot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
  // Set axis titles
  //mpPlot->setAxisTitle(QwtPlot::xBottom, QObject::tr("Satellite"));
  //mpPlot->setAxisTitle(QwtPlot::yLeft, QObject::tr("Value"));
  mPlot->setAxisScale( QwtPlot::xBottom, 0, 20 );
  mPlot->setAxisScale( QwtPlot::yLeft, 0, 100 );  // max is 50dB SNR, I believe - SLM
  // add a grid
  //QwtPlotGrid * mypGrid = new QwtPlotGrid();
  //mypGrid->attach( mpPlot );
  //display satellites first
  mCurve = new QwtPlotCurve();
  mCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
  mCurve->setPen( QPen( Qt::blue ) );
  mCurve->setBrush( QBrush( Qt::blue ) );
  mPlot->enableAxis( QwtPlot::yLeft, false );
  mPlot->enableAxis( QwtPlot::xBottom, false );
  mCurve->attach( mPlot );
  //ensure all children get removed
  mPlot->setAutoDelete( true );
  QVBoxLayout *mpHistogramLayout = new QVBoxLayout( mpHistogramWidget );
  mpHistogramLayout->setContentsMargins( 0, 0, 0, 0 );
  mpHistogramLayout->addWidget( mPlot );
  mpHistogramWidget->setLayout( mpHistogramLayout );

  //
  // Set up the polar graph for satellite pos
  //
#ifdef WITH_QWTPOLAR
  QWidget *mpPolarWidget = mStackedWidget->widget( 2 );
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
  mpSatellitesGrid = new QwtPolarGrid();
  mpSatellitesGrid->setGridAttribute( QwtPolarGrid::AutoScaling, false );   // This fixes the issue of autoscaling on the Radius grid. It is ON by default AND is separate from the scaleData.doAutoScale in QwtPolarPlot::setScale(), etc. THIS IS VERY TRICKY!
  mpSatellitesGrid->setPen( QPen( Qt::black ) );
  QPen minorPen( Qt::gray );  // moved outside of for loop; NOTE setting the minor pen isn't necessary if the minor grids aren't shown
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
  {
    //mpSatellitesGrid->showGrid( scaleId );
    //mpSatellitesGrid->showMinorGrid(scaleId);
    mpSatellitesGrid->setMinorGridPen( scaleId, minorPen );
  }
//  mpSatellitesGrid->setAxisPen( QwtPolar::AxisAzimuth, QPen( Qt::black ) );

  mpSatellitesGrid->showAxis( QwtPolar::AxisAzimuth, true );
  mpSatellitesGrid->showAxis( QwtPolar::AxisLeft, false ); //alt axis
  mpSatellitesGrid->showAxis( QwtPolar::AxisRight, false );//alt axis
  mpSatellitesGrid->showAxis( QwtPolar::AxisTop, false );//alt axis
  mpSatellitesGrid->showAxis( QwtPolar::AxisBottom, false );//alt axis
  mpSatellitesGrid->showGrid( QwtPolar::ScaleAzimuth, false ); // hide the grid; just show ticks at edge
  mpSatellitesGrid->showGrid( QwtPolar::ScaleRadius, true );
//  mpSatellitesGrid->showMinorGrid( QwtPolar::ScaleAzimuth, true );
  mpSatellitesGrid->showMinorGrid( QwtPolar::ScaleRadius, true );   // for 22.5, 67.5 degree circles
  mpSatellitesGrid->attach( mpSatellitesWidget );

  //QwtLegend *legend = new QwtLegend;
  //mpSatellitesWidget->insertLegend(legend, QwtPolarPlot::BottomLegend);
  QVBoxLayout *mpPolarLayout = new QVBoxLayout( mpPolarWidget );
  mpPolarLayout->setContentsMargins( 0, 0, 0, 0 );
  mpPolarLayout->addWidget( mpSatellitesWidget );
  mpPolarWidget->setLayout( mpPolarLayout );

  // replot on command
  mpSatellitesWidget->replot();
#endif
  mPlot->replot();

  const QgsSettings mySettings;

  // Restore state
  mDateTimeFormat = mySettings.value( QStringLiteral( "dateTimeFormat" ), "", QgsSettings::Gps ).toString(); // zero-length string signifies default format

  mBtnDebug->setVisible( mySettings.value( QStringLiteral( "showDebug" ), "false", QgsSettings::Gps ).toBool() );  // use a registry setting to control - power users/devs could set it

  // status = unknown
  setStatusIndicator( Qgis::GpsFixStatus::NoData );

  //SLM - added functionality
  mLogFile = nullptr;

  mStackedWidget->setCurrentIndex( 3 ); // force to Options
  mBtnPosition->setFocus( Qt::TabFocusReason );

  // Timestamp
  mCboTimestampField->setAllowEmptyFieldName( true );
  mCboTimestampField->setFilters( QgsFieldProxyModel::Filter::String | QgsFieldProxyModel::Filter::DateTime );
  // Qt::LocalTime  0 Locale dependent time (Timezones and Daylight Savings Time).
  // Qt::UTC  1 Coordinated Universal Time, replaces Greenwich Mean Time.
  // SKIP this one: Qt::OffsetFromUTC  2 An offset in seconds from Coordinated Universal Time.
  // Qt::TimeZone 3 A named time zone using a specific set of Daylight Savings rules.
  mCboTimestampFormat->addItem( tr( "Local Time" ), Qt::TimeSpec::LocalTime );
  mCboTimestampFormat->addItem( tr( "UTC" ), Qt::TimeSpec::UTC );
  mCboTimestampFormat->addItem( tr( "Time Zone" ), Qt::TimeSpec::TimeZone );
  mCboTimestampFormat->setCurrentIndex( mySettings.value( QStringLiteral( "timeStampFormat" ), Qt::LocalTime, QgsSettings::Gps ).toInt() );
  connect( mCboTimestampFormat, qOverload< int >( &QComboBox::currentIndexChanged ),
           this, &QgsGpsInformationWidget::timestampFormatChanged );
  connect( mCboTimestampField, qOverload< int >( &QComboBox::currentIndexChanged ),
           this, [ = ]( int index )
  {
    const bool enabled { index > 0 };
    mCboTimestampFormat->setEnabled( enabled );
    mLblTimestampFormat->setEnabled( enabled );
    mCbxLeapSeconds->setEnabled( enabled );
    mLeapSeconds->setEnabled( enabled );
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
    if ( vlayer && ! mPopulatingFields )
    {
      mPreferredTimestampFields[ vlayer->id() ] = mCboTimestampField->currentText();
    }
    updateTimeZones();
  } );
  updateTimeZones();

  connect( mMapCanvas, &QgsMapCanvas::currentLayerChanged,
           this, &QgsGpsInformationWidget::updateTimestampDestinationFields );

  updateTimestampDestinationFields( mMapCanvas->currentLayer() );

  const auto constTzs { QTimeZone::availableTimeZoneIds() };
  for ( const auto &tzId : constTzs )
  {
    mCboTimeZones->addItem( tzId );
  }

  const QString lastTz { mySettings.value( QStringLiteral( "timestampTimeZone" ), QVariant(), QgsSettings::Gps ).toString() };
  int tzIdx { mCboTimeZones->findText( lastTz ) };
  if ( tzIdx == -1 )
  {
    const QString currentTz { QTimeZone::systemTimeZoneId() };
    tzIdx = mCboTimeZones->findText( currentTz );
  }
  mCboTimeZones->setCurrentIndex( tzIdx );

  mCbxLeapSeconds->setChecked( mySettings.value( QStringLiteral( "applyLeapSeconds" ), true, QgsSettings::Gps ).toBool() );
  // Leap seconds as of 2019-06-20, if the default changes, it can be updated in qgis_global_settings.ini
  mLeapSeconds->setValue( mySettings.value( QStringLiteral( "leapSecondsCorrection" ), 18, QgsSettings::Gps ).toInt() );
  mLeapSeconds->setClearValue( 18 );

  connect( mConnection, &QgsAppGpsConnection::connecting, this, &QgsGpsInformationWidget::gpsConnecting );
  connect( mConnection, &QgsAppGpsConnection::connectionTimedOut, this, &QgsGpsInformationWidget::timedout );
  connect( mConnection, &QgsAppGpsConnection::connected, this, &QgsGpsInformationWidget::gpsConnected );
  connect( mConnection, &QgsAppGpsConnection::disconnected, this, &QgsGpsInformationWidget::gpsDisconnected );
  connect( mConnection, &QgsAppGpsConnection::stateChanged, this, &QgsGpsInformationWidget::displayGPSInformation );
  connect( mConnection, &QgsAppGpsConnection::fixStatusChanged, this, &QgsGpsInformationWidget::setStatusIndicator );

  connect( mConnection, &QgsAppGpsConnection::statusChanged, this, [ = ]( Qgis::GpsConnectionStatus status )
  {
    switch ( status )
    {
      case Qgis::GpsConnectionStatus::Disconnected:
        whileBlocking( mConnectButton )->setChecked( false );
        mConnectButton->setText( tr( "Connect" ) );
        mConnectButton->setEnabled( true );
        break;
      case Qgis::GpsConnectionStatus::Connecting:
        whileBlocking( mConnectButton )->setChecked( true );
        mConnectButton->setText( tr( "Connecting" ) );
        mConnectButton->setEnabled( false );
        break;
      case Qgis::GpsConnectionStatus::Connected:
        whileBlocking( mConnectButton )->setChecked( true );
        mConnectButton->setText( tr( "Disconnect" ) );
        mConnectButton->setEnabled( true );
        break;
    }
  } );
}

QgsGpsInformationWidget::~QgsGpsInformationWidget()
{
  if ( mConnection->isConnected() )
  {
    gpsDisconnected();
  }


#ifdef WITH_QWTPOLAR
  delete mpSatellitesGrid;
#endif

  QgsSettings mySettings;
  mySettings.setValue( QStringLiteral( "timestampTimeZone" ), mCboTimeZones->currentText(), QgsSettings::Gps );
  mySettings.setValue( QStringLiteral( "applyLeapSeconds" ), mCbxLeapSeconds->isChecked(), QgsSettings::Gps );
  mySettings.setValue( QStringLiteral( "leapSecondsCorrection" ), mLeapSeconds->value(), QgsSettings::Gps );
}

void QgsGpsInformationWidget::mBtnPosition_clicked()
{
  mStackedWidget->setCurrentIndex( 0 );
  if ( QgsGpsConnection *connection = mConnection->connection() )
    displayGPSInformation( connection->currentGPSInformation() );
}

void QgsGpsInformationWidget::mBtnSignal_clicked()
{
  mStackedWidget->setCurrentIndex( 1 );
  if ( QgsGpsConnection *connection = mConnection->connection() )
    displayGPSInformation( connection->currentGPSInformation() );
}

void QgsGpsInformationWidget::mBtnSatellites_clicked()
{
  mStackedWidget->setCurrentIndex( 2 );
  if ( QgsGpsConnection *connection = mConnection->connection() )
    displayGPSInformation( connection->currentGPSInformation() );
}

void QgsGpsInformationWidget::mBtnOptions_clicked()
{
  mStackedWidget->setCurrentIndex( 3 );
}

void QgsGpsInformationWidget::mBtnDebug_clicked()
{
  mStackedWidget->setCurrentIndex( 4 );
}

void QgsGpsInformationWidget::mConnectButton_toggled( bool flag )
{
  if ( flag )
  {
    mConnection->connectGps();
  }
  else
  {
    mConnection->disconnectGps();
  }
}

void QgsGpsInformationWidget::gpsConnecting()
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

  mGPSPlainTextEdit->appendPlainText( tr( "Connecting…" ) );
}

void QgsGpsInformationWidget::timedout()
{
  mGPSPlainTextEdit->appendPlainText( tr( "Timed out!" ) );
}

void QgsGpsInformationWidget::gpsConnected()
{
  mGPSPlainTextEdit->appendPlainText( tr( "Connected!" ) );

  if ( mLogFileGroupBox->isChecked() && !mLogFilename->filePath().isEmpty() )
  {
    if ( !mLogFile )
    {
      mLogFile = new QFile( mLogFilename->filePath() );
    }

    if ( mLogFile->open( QIODevice::Append ) )  // open in binary and explicitly output CR + LF per NMEA
    {
      mLogFileTextStream.setDevice( mLogFile );

      // crude way to separate chunks - use when manually editing file - NMEA parsers should discard
      mLogFileTextStream << "====" << "\r\n";

      connect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsGpsInformationWidget::logNmeaSentence ); // added to handle raw data
    }
    else  // error opening file
    {
      delete mLogFile;
      mLogFile = nullptr;

      // need to indicate why - this just reports that an error occurred
      showStatusBarMessage( tr( "Error opening log file." ) );
    }
  }
}

void QgsGpsInformationWidget::gpsDisconnected()
{
  if ( mLogFile && mLogFile->isOpen() )
  {
    disconnect( mConnection, &QgsAppGpsConnection::nmeaSentenceReceived, this, &QgsGpsInformationWidget::logNmeaSentence );
    mLogFile->close();
    delete mLogFile;
    mLogFile = nullptr;
  }

  mGPSPlainTextEdit->appendPlainText( tr( "Disconnected…" ) );
}

void QgsGpsInformationWidget::displayGPSInformation( const QgsGpsInformation &info )
{
  QVector<QPointF> data;

  if ( mStackedWidget->currentIndex() == 1 && info.satInfoComplete ) //signal
  {
    mPlot->setAxisScale( QwtPlot::xBottom, 0, info.satellitesInView.size() );
  } //signal
#ifdef WITH_QWTPOLAR
  if ( mStackedWidget->currentIndex() == 2 && info.satInfoComplete ) //satellites
  {
    while ( !mMarkerList.isEmpty() )
    {
      delete mMarkerList.takeFirst();
    }
  } //satellites
#endif
  if ( mStackedWidget->currentIndex() == 4 ) //debug
  {
    mGPSPlainTextEdit->clear();
  } //debug

  for ( int i = 0; i < info.satellitesInView.size(); ++i ) //satellite processing loop
  {
    const QgsSatelliteInfo currentInfo = info.satellitesInView.at( i );

    if ( mStackedWidget->currentIndex() == 1 && info.satInfoComplete ) //signal
    {
      data << QPointF( i, 0 );
      data << QPointF( i, currentInfo.signal );
      data << QPointF( i + 1, currentInfo.signal );
      data << QPointF( i + 1, 0 );
    } //signal

    if ( mStackedWidget->currentIndex() == 2 && info.satInfoComplete ) //satellites
    {
      QColor bg( Qt::white ); // moved several items outside of the following if block to minimize loop time
      bg.setAlpha( 200 );
      QColor myColor;

      // Add a marker to the polar plot
      if ( currentInfo.id > 0 )       // don't show satellite if id=0 (no satellite indication)
      {
#ifdef WITH_QWTPOLAR
        QwtPolarMarker *mypMarker = new QwtPolarMarker();
#if (QWT_POLAR_VERSION<0x010000)
        mypMarker->setPosition( QwtPolarPoint( currentInfo.azimuth, currentInfo.elevation ) );
#else
        mypMarker->setPosition( QwtPointPolar( currentInfo.azimuth, currentInfo.elevation ) );
#endif
#endif
        if ( currentInfo.signal < 30 ) //weak signal
        {
          myColor = Qt::red;
        }
        else
        {
          myColor = Qt::black; //strong signal
        }
#ifdef WITH_QWTPOLAR
        QBrush symbolBrush( Qt::black );
        QSize markerSize( 9, 9 );
        QBrush textBgBrush( bg );
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
#endif
      } // currentInfo.id > 0
    } //satellites
  } //satellite processing loop

  if ( mStackedWidget->currentIndex() == 1 && info.satInfoComplete ) //signal
  {
    mCurve->setSamples( data );
    mPlot->replot();
  } //signal
#ifdef WITH_QWTPOLAR
  if ( mStackedWidget->currentIndex() == 2 && info.satInfoComplete ) //satellites
  {
    mpSatellitesWidget->replot();
  } //satellites
#endif

  const bool validFlag = info.isValid();
  QgsPointXY myNewCenter;
  if ( validFlag )
  {
    myNewCenter = QgsPointXY( info.longitude, info.latitude );
  }
  else
  {
    myNewCenter = mLastGpsPosition;
  }

  if ( mStackedWidget->currentIndex() == 0 ) //position
  {
    QString formattedX;
    QString formattedY;
    QgsCoordinateUtils::formatCoordinatePartsForProject( QgsProject::instance(), QgsPointXY( info.longitude, info.latitude ),
        QgsCoordinateReferenceSystem(), 8, formattedX, formattedY );

    mTxtLatitude->setText( formattedX );
    mTxtLongitude->setText( formattedY );

    mTxtAltitude->setText( tr( "%1 m" ).arg( info.elevation, 0, 'f', 3 ) );
    mTxtAltitudeDiff->setText( tr( "%1 m" ).arg( info.elevation_diff, 0, 'f', 3 ) );

    if ( mDateTimeFormat.isEmpty() )
    {
      mTxtDateTime->setText( info.utcDateTime.toString( Qt::TextDate ) );  // default format
    }
    else
    {
      mTxtDateTime->setText( info.utcDateTime.toString( mDateTimeFormat ) );  //user specified format string for testing the millisecond part of time
    }
    if ( std::isfinite( info.speed ) )
    {
      mTxtSpeed->setEnabled( true );
      mTxtSpeed->setText( tr( "%1 km/h" ).arg( info.speed, 0, 'f', 1 ) );
    }
    else
    {
      mTxtSpeed->setEnabled( false );
      mTxtSpeed->setText( tr( "Not available" ) );
    }
    if ( std::isfinite( info.direction ) )
    {
      mTxtDirection->setEnabled( true );
      mTxtDirection->setText( QString::number( info.direction, 'f', 1 ) + QStringLiteral( "°" ) );
    }
    else
    {
      mTxtDirection->setEnabled( false );
      mTxtDirection->setText( tr( "Not available" ) );
    }
    mTxtHdop->setText( QString::number( info.hdop, 'f', 1 ) );
    mTxtVdop->setText( QString::number( info.vdop, 'f', 1 ) );
    mTxtPdop->setText( QString::number( info.pdop, 'f', 1 ) );
    if ( std::isfinite( info.hacc ) )
    {
      mTxtHacc->setEnabled( true );
      mTxtHacc->setText( tr( "%1 m" ).arg( QLocale().toString( info.hacc, 'f', 3 ) ) );
    }
    else
    {
      mTxtHacc->setEnabled( false );
      mTxtHacc->setText( tr( "Not available" ) );
    }
    if ( std::isfinite( info.vacc ) )
    {
      mTxtVacc->setEnabled( true );
      mTxtVacc->setText( tr( "%1 m" ).arg( QLocale().toString( info.vacc, 'f', 3 ) ) );
    }
    else
    {
      mTxtVacc->setEnabled( false );
      mTxtVacc->setText( tr( "Not available" ) );
    }
    if ( std::isfinite( info.hvacc ) )
    {
      mTxt3Dacc->setEnabled( true );
      mTxt3Dacc->setText( tr( "%1 m" ).arg( QLocale().toString( info.hvacc, 'f', 3 ) ) );
    }
    else
    {
      mTxt3Dacc->setEnabled( false );
      mTxt3Dacc->setText( tr( "Not available" ) );
    }
    mTxtFixMode->setText( info.fixMode == 'A' ? tr( "Automatic" ) : info.fixMode == 'M' ? tr( "Manual" ) : QString() ); // A=automatic 2d/3d, M=manual; allowing for anything else
    mTxtFixType->setText( info.fixType == 3 ? tr( "3D" ) : info.fixType == 2 ? tr( "2D" ) : info.fixType == 1 ? tr( "No fix" ) : QString::number( info.fixType ) ); // 1=no fix, 2=2D, 3=3D; allowing for anything else
    mTxtQuality->setText( info.qualityDescription() );
    mTxtSatellitesUsed->setText( tr( "%1 used (%2 in view)" ).arg( info.satellitesUsed ).arg( info.satellitesInView.size() ) );
    mTxtStatus->setText( info.status == 'A' ? tr( "Valid" ) : info.status == 'V' ? tr( "Invalid" ) : QString() );
  }

  if ( mLastGpsPosition != myNewCenter )
  {
    mLastGpsPosition = myNewCenter;
  }
}

void QgsGpsInformationWidget::logNmeaSentence( const QString &nmeaString )
{
  if ( mLogFileGroupBox->isChecked() && mLogFile && mLogFile->isOpen() )
  {
    mLogFileTextStream << nmeaString << "\r\n"; // specifically output CR + LF (NMEA requirement)
  }
}

void QgsGpsInformationWidget::setStatusIndicator( Qgis::GpsFixStatus statusValue )
{
  // the pixmap will be expanded to the size of the label
  QPixmap status( 4, 4 );
  switch ( statusValue )
  {
    case Qgis::GpsFixStatus::NoFix:
      status.fill( Qt::red );
      break;
    case Qgis::GpsFixStatus::Fix2D:
      status.fill( Qt::yellow );
      break;
    case Qgis::GpsFixStatus::Fix3D:
      status.fill( Qt::green );
      break;
    case Qgis::GpsFixStatus::NoData:
      status.fill( Qt::darkGray );
  }
  mLblStatusIndicator->setPixmap( status );
}

void QgsGpsInformationWidget::showStatusBarMessage( const QString &msg )
{
  QgisApp::instance()->statusBarIface()->showMessage( msg );
}

void QgsGpsInformationWidget::updateTimeZones()
{
  QgsSettings().setValue( QStringLiteral( "timestampFormat" ), mCboTimestampFormat->currentData( ), QgsSettings::Gps );
  const bool enabled { static_cast<Qt::TimeSpec>( mCboTimestampFormat->currentData( ).toInt() ) == Qt::TimeSpec::TimeZone };
  mCboTimeZones->setEnabled( enabled );
  mLblTimeZone->setEnabled( enabled );
}

void QgsGpsInformationWidget::timestampFormatChanged( int )
{
  QgsSettings().setValue( QStringLiteral( "timestampFormat" ), mCboTimestampFormat->currentData( ).toInt(), QgsSettings::Gps );
  const bool enabled { static_cast<Qt::TimeSpec>( mCboTimestampFormat->currentData( ).toInt() ) == Qt::TimeSpec::TimeZone };
  mCboTimeZones->setEnabled( enabled );
  mLblTimeZone->setEnabled( enabled );
}

void QgsGpsInformationWidget::updateTimestampDestinationFields( QgsMapLayer *mapLayer )
{
  mPopulatingFields = true;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mapLayer );
  mGboxTimestamp->setEnabled( false );
  if ( vlayer )
  {
    mCboTimestampField->setLayer( mapLayer );
    if ( mCboTimestampField->count() > 1 )
    {
      mGboxTimestamp->setEnabled( true );
      // Set preferred if stored
      if ( mPreferredTimestampFields.contains( vlayer->id( ) ) )
      {
        const int idx { mCboTimestampField->findText( mPreferredTimestampFields[ vlayer->id( ) ] ) };
        if ( idx > 0 )
        {
          mCboTimestampField->setCurrentIndex( idx );
        }
      }
      // Cleanup preferred fields
      const auto constKeys { mPreferredTimestampFields.keys( ) };
      for ( const auto &layerId : constKeys )
      {
        if ( ! QgsProject::instance()->mapLayer( layerId ) )
        {
          mPreferredTimestampFields.remove( layerId );
        }
      }
    }
  }
  mPopulatingFields = false;
}
