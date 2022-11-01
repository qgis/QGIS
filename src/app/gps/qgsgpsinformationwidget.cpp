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
#include "qgsappgpsconnection.h"
#include "qgscoordinatetransform.h"
#include "qgsfeatureaction.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmaptooladdfeature.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsstatusbar.h"
#include "gmath.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgspolygon.h"
#include "qgsgpsconnection.h"
#include "qgscoordinateutils.h"
#include "qgsgui.h"
#include "qgslinesymbol.h"
#include "qgssymbollayerutils.h"
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
  connect( mBtnAddVertex, &QPushButton::clicked, this, &QgsGpsInformationWidget::mBtnAddVertex_clicked );
  connect( mBtnCloseFeature, &QPushButton::clicked, this, &QgsGpsInformationWidget::mBtnCloseFeature_clicked );
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

  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

  mCanvasToWgs84Transform = QgsCoordinateTransform( mMapCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ]
  {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mMapCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );
  connect( QgsProject::instance(), &QgsProject::transformContextChanged, this, [ = ]
  {
    mCanvasToWgs84Transform = QgsCoordinateTransform( mMapCanvas->mapSettings().destinationCrs(), mWgs84CRS, QgsProject::instance() );
  } );
  mDistanceCalculator.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mDistanceCalculator.setSourceCrs( mWgs84CRS, QgsProject::instance()->transformContext() );
  connect( QgsProject::instance(), &QgsProject::ellipsoidChanged, this, [ = ]
  {
    mDistanceCalculator.setEllipsoid( QgsProject::instance()->ellipsoid() );
  } );

  mLastNmeaPosition.lat = nmea_degree2radian( 0.0 );
  mLastNmeaPosition.lon = nmea_degree2radian( 0.0 );

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

  //auto digitizing behavior
  mCbxAutoAddVertices->setChecked( mySettings.value( QStringLiteral( "autoAddVertices" ), "false", QgsSettings::Gps ).toBool() );

  mBtnAddVertex->setEnabled( !mCbxAutoAddVertices->isChecked() );

  mCbxAutoCommit->setChecked( mySettings.value( QStringLiteral( "autoCommit" ), "false", QgsSettings::Gps ).toBool() );

  mBtnDebug->setVisible( mySettings.value( QStringLiteral( "showDebug" ), "false", QgsSettings::Gps ).toBool() );  // use a registry setting to control - power users/devs could set it

  // status = unknown
  setStatusIndicator( Qgis::GpsFixStatus::NoData );

  //SLM - added functionality
  mLogFile = nullptr;

  connect( QgisApp::instance(), &QgisApp::activeLayerChanged,
           this, &QgsGpsInformationWidget::updateCloseFeatureButton );

  mStackedWidget->setCurrentIndex( 3 ); // force to Options
  mBtnPosition->setFocus( Qt::TabFocusReason );


  mAcquisitionTimer = std::unique_ptr<QTimer>( new QTimer( this ) );
  mAcquisitionTimer->setSingleShot( true );

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

  connect( mAcquisitionTimer.get(), &QTimer::timeout,
           this, &QgsGpsInformationWidget::switchAcquisition );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsGpsInformationWidget::gpsSettingsChanged );
  gpsSettingsChanged();

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

  delete mRubberBand;

#ifdef WITH_QWTPOLAR
  delete mpSatellitesGrid;
#endif

  QgsSettings mySettings;
  mySettings.setValue( QStringLiteral( "autoAddVertices" ), mCbxAutoAddVertices->isChecked(), QgsSettings::Gps );
  mySettings.setValue( QStringLiteral( "autoCommit" ), mCbxAutoCommit->isChecked(), QgsSettings::Gps );
  mySettings.setValue( QStringLiteral( "timestampTimeZone" ), mCboTimeZones->currentText(), QgsSettings::Gps );
  mySettings.setValue( QStringLiteral( "applyLeapSeconds" ), mCbxLeapSeconds->isChecked(), QgsSettings::Gps );
  mySettings.setValue( QStringLiteral( "leapSecondsCorrection" ), mLeapSeconds->value(), QgsSettings::Gps );
}

void QgsGpsInformationWidget::gpsSettingsChanged()
{
  updateTrackAppearance();

  QgsSettings settings;
  int acquisitionInterval = 0;
  if ( QgsGpsConnection::settingsGpsConnectionType.exists() )
  {
    acquisitionInterval = static_cast< int >( QgsGpsConnection::settingGpsAcquisitionInterval.value() );
    mDistanceThreshold = QgsGpsConnection::settingGpsDistanceThreshold.value();
  }
  else
  {
    // legacy settings
    acquisitionInterval = settings.value( QStringLiteral( "acquisitionInterval" ), 0, QgsSettings::Gps ).toInt();
    mDistanceThreshold = settings.value( QStringLiteral( "distanceThreshold" ), 0, QgsSettings::Gps ).toDouble();
  }

  mAcquisitionInterval = acquisitionInterval * 1000;
  if ( mAcquisitionTimer->isActive() )
    mAcquisitionTimer->stop();
  mAcquisitionEnabled = true;

  switchAcquisition();
}

void QgsGpsInformationWidget::updateTrackAppearance()
{
  if ( !mRubberBand )
    return;

  QDomDocument doc;
  QDomElement elem;
  const QString trackLineSymbolXml = QgsGpsInformationWidget::settingTrackLineSymbol.value();
  if ( !trackLineSymbolXml.isEmpty() )
  {
    doc.setContent( trackLineSymbolXml );
    elem = doc.documentElement();
    std::unique_ptr< QgsLineSymbol > trackLineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, QgsReadWriteContext() ) );
    if ( trackLineSymbol )
    {
      mRubberBand->setSymbol( trackLineSymbol.release() );
    }
    mRubberBand->update();
  }
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

  mSecondLastGpsPosition = QgsPointXY();

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
  if ( mBlockGpsStateChanged )
    return;

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
  nmeaPOS newNmeaPosition;
  nmeaTIME newNmeaTime;
  double newAlt = 0.0;
  if ( validFlag )
  {
    myNewCenter = QgsPointXY( info.longitude, info.latitude );
    newNmeaPosition.lat = nmea_degree2radian( info.latitude );
    newNmeaPosition.lon = nmea_degree2radian( info.longitude );
    newAlt = info.elevation;
    nmea_time_now( &newNmeaTime );
  }
  else
  {
    myNewCenter = mLastGpsPosition;
    newNmeaPosition = mLastNmeaPosition;
    newAlt = mLastElevation;
  }
  if ( !mAcquisitionEnabled || ( nmea_distance( &newNmeaPosition, &mLastNmeaPosition ) < mDistanceThreshold ) )
  {
    // do not update position if update is disabled by timer or distance is under threshold
    myNewCenter = mLastGpsPosition;

  }
  if ( validFlag && mAcquisitionEnabled )
  {
    // position updated by valid data, reset timer
    switchAcquisition();
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
  } //position

  // Avoid refreshing / panning if we haven't moved
  if ( mLastGpsPosition != myNewCenter )
  {
    mSecondLastGpsPosition = mLastGpsPosition;
    mLastGpsPosition = myNewCenter;
    mLastNmeaPosition = newNmeaPosition;
    mLastNmeaTime = newNmeaTime;
    mLastElevation = newAlt;

    if ( mCbxAutoAddVertices->isChecked() )
    {
      addVertex();
    }
  }
}

void QgsGpsInformationWidget::mBtnAddVertex_clicked()
{
  addVertex();
}

void QgsGpsInformationWidget::addVertex()
{
  QgsDebugMsg( QStringLiteral( "Adding Vertex" ) );

  if ( !mRubberBand )
  {
    createRubberBand();
  }

  // we store the capture list in wgs84 and then transform to layer crs when
  // calling close feature
  const QgsPoint point = QgsPoint( mLastGpsPosition.x(), mLastGpsPosition.y(), mLastElevation );
  mCaptureList.push_back( point );


  // we store the rubber band points in map canvas CRS so transform to map crs
  // potential problem with transform errors and wrong coordinates if map CRS is changed after points are stored - SLM
  // should catch map CRS change and transform the points
  QgsPointXY myPoint;
  if ( mMapCanvas )
  {
    myPoint = mCanvasToWgs84Transform.transform( mLastGpsPosition, Qgis::TransformDirection::Reverse );
  }
  else
  {
    myPoint = mLastGpsPosition;
  }

  mRubberBand->addPoint( myPoint );
}

void QgsGpsInformationWidget::mBtnResetFeature_clicked()
{
  mBlockGpsStateChanged++;
  createRubberBand(); //deletes existing rubberband
  mCaptureList.clear();
  mBlockGpsStateChanged--;
}

void QgsGpsInformationWidget::mBtnCloseFeature_clicked()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer )
    return;

  if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry && mCaptureList.size() < 2 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "Cannot close a line feature until it has at least two vertices." ) );
    return;
  }
  else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry && mCaptureList.size() < 3 )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
        tr( "Cannot close a polygon feature until it has at least three vertices." ) );
    return;
  }

  // Handle timestamp
  QgsAttributeMap attrMap;
  const int idx { vlayer->fields().indexOf( mCboTimestampField->currentText() ) };
  if ( idx != -1 )
  {
    const QVariant ts = timestamp( vlayer, idx );
    if ( ts.isValid() )
    {
      attrMap[ idx ] = ts;
    }
  }

  const QgsCoordinateTransform t( mWgs84CRS, vlayer->crs(), QgsProject::instance() );
  const bool is3D = QgsWkbTypes::hasZ( vlayer->wkbType() );
  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsFeature f;
      try
      {
        const QgsPointXY pointXY = t.transform( mLastGpsPosition );

        QgsGeometry g;
        if ( is3D )
          g = QgsGeometry( new QgsPoint( pointXY.x(), pointXY.y(), mLastElevation ) );
        else
          g = QgsGeometry::fromPointXY( pointXY );

        if ( QgsWkbTypes::isMultiType( vlayer->wkbType() ) )
          g.convertToMultiType();

        f.setGeometry( g );
      }
      catch ( QgsCsException & )
      {
        QgisApp::instance()->messageBar()->pushCritical( tr( "Add Feature" ),
            tr( "Error reprojecting feature to layer CRS." ) );
        return;
      }

      QgsFeatureAction action( tr( "Feature Added" ), f, vlayer, QUuid(), -1, this );
      if ( action.addFeature( attrMap ) )
      {
        if ( mCbxAutoCommit->isChecked() )
        {
          // should canvas->isDrawing() be checked?
          if ( !vlayer->commitChanges() ) //assumed to be vector layer and is editable and is in editing mode (preconditions have been tested)
          {
            QgisApp::instance()->messageBar()->pushCritical(
              tr( "Save Layer Edits" ),
              tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
              .arg( vlayer->name(),
                    vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );
          }

          vlayer->startEditing();
        }
      }

      break;
    }

    case QgsWkbTypes::LineGeometry:
    case QgsWkbTypes::PolygonGeometry:
    {
      mBlockGpsStateChanged++;

      QgsFeature f;
      QgsGeometry g;

      std::unique_ptr<QgsLineString> ring( new QgsLineString( mCaptureList ) );
      if ( ! is3D )
        ring->dropZValue();

      if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry )
      {

        g = QgsGeometry( ring.release() );
        try
        {
          g.transform( t );
        }
        catch ( QgsCsException & )
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
              tr( "Error reprojecting feature to layer CRS." ) );
          return;
        }
        if ( QgsWkbTypes::isMultiType( vlayer->wkbType() ) )
          g.convertToMultiType();
      }
      else if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry )
      {
        ring->close();
        std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
        polygon->setExteriorRing( ring.release() );

        g = QgsGeometry( polygon.release() );
        try
        {
          g.transform( t );
        }
        catch ( QgsCsException & )
        {
          mBlockGpsStateChanged--;
          QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ),
              tr( "Error reprojecting feature to layer CRS." ) );
          return;
        }

        if ( QgsWkbTypes::isMultiType( vlayer->wkbType() ) )
          g.convertToMultiType();

        const int avoidIntersectionsReturn = g.avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers() );
        if ( avoidIntersectionsReturn == 1 )
        {
          //not a polygon type. Impossible to get there
        }
        else if ( avoidIntersectionsReturn == 2 )
        {
          //bail out...
          QgisApp::instance()->messageBar()->pushWarning( tr( "Add Feature" ), tr( "The feature could not be added because removing the polygon intersections would change the geometry type." ) );
          mBlockGpsStateChanged--;
          return;
        }
        else if ( avoidIntersectionsReturn == 3 )
        {
          QgisApp::instance()->messageBar()->pushCritical( tr( "Add Feature" ), tr( "The feature has been added, but at least one geometry intersected is invalid. These geometries must be manually repaired." ) );
          mBlockGpsStateChanged--;
          return;
        }
      }

      f.setGeometry( g );
      QgsFeatureAction action( tr( "Feature added" ), f, vlayer, QUuid(), -1, this );
      if ( action.addFeature( attrMap ) )
      {
        if ( mCbxAutoCommit->isChecked() )
        {
          if ( !vlayer->commitChanges() )
          {
            QgisApp::instance()->messageBar()->pushCritical( tr( "Save Layer Edits" ),
                tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                .arg( vlayer->name(),
                      vlayer->commitErrors().join( QLatin1String( "\n  " ) ) ) );
          }

          vlayer->startEditing();
        }
        delete mRubberBand;
        mRubberBand = nullptr;

        // delete the elements of mCaptureList
        mCaptureList.clear();
      } // action.addFeature()

      mBlockGpsStateChanged--;
      break;
    }

    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return;
  }
  vlayer->triggerRepaint();

  // force focus back to GPS window/ Add Feature button for ease of use by keyboard
  activateWindow();
  mBtnCloseFeature->setFocus( Qt::OtherFocusReason );
}

void QgsGpsInformationWidget::createRubberBand()
{
  delete mRubberBand;

  mRubberBand = new QgsRubberBand( mMapCanvas, QgsWkbTypes::LineGeometry );
  updateTrackAppearance();
  mRubberBand->show();
}

void QgsGpsInformationWidget::logNmeaSentence( const QString &nmeaString )
{
  if ( mLogFileGroupBox->isChecked() && mLogFile && mLogFile->isOpen() )
  {
    mLogFileTextStream << nmeaString << "\r\n"; // specifically output CR + LF (NMEA requirement)
  }
}

void QgsGpsInformationWidget::updateCloseFeatureButton( QgsMapLayer *lyr )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( lyr );

  if ( !( vlayer && vlayer->isValid() ) )
    return;

  // Add feature button tracks edit state of layer
  if ( vlayer != mLastLayer )
  {
    if ( mLastLayer )  // disconnect previous layer
    {
      disconnect( mLastLayer, &QgsVectorLayer::editingStarted,
                  this, &QgsGpsInformationWidget::layerEditStateChanged );
      disconnect( mLastLayer, &QgsVectorLayer::editingStopped,
                  this, &QgsGpsInformationWidget::layerEditStateChanged );
    }
    if ( vlayer ) // connect new layer
    {
      connect( vlayer, &QgsVectorLayer::editingStarted,
               this, &QgsGpsInformationWidget::layerEditStateChanged );
      connect( vlayer, &QgsVectorLayer::editingStopped,
               this, &QgsGpsInformationWidget::layerEditStateChanged );
    }
    mLastLayer = vlayer;
  }

  QString buttonLabel = tr( "&Add Feature" );
  if ( vlayer )
  {
    QgsVectorDataProvider *provider = vlayer->dataProvider();
    const QgsWkbTypes::GeometryType layerGeometryType = vlayer->geometryType();

    bool enable = provider->capabilities() & QgsVectorDataProvider::AddFeatures &&  // layer can add features
                  vlayer->isEditable() && vlayer->isSpatial();

    switch ( layerGeometryType )
    {
      case QgsWkbTypes::PointGeometry:
        buttonLabel = tr( "&Add Point" );
        break;

      case QgsWkbTypes::LineGeometry:
        buttonLabel = tr( "&Add Line" );
        break;

      case QgsWkbTypes::PolygonGeometry:
        buttonLabel = tr( "&Add Polygon" );
        break;

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
        enable = false;
        break;
    }

    mBtnCloseFeature->setEnabled( enable );
  }
  else
  {
    mBtnCloseFeature->setEnabled( false );
  }
  mBtnCloseFeature->setText( buttonLabel );
}

void QgsGpsInformationWidget::layerEditStateChanged()
{
  updateCloseFeatureButton( mLastLayer );
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

QVariant QgsGpsInformationWidget::timestamp( QgsVectorLayer *vlayer, int idx )
{
  QVariant value;
  if ( idx != -1 )
  {
    QDateTime time( QDate( 1900 + mLastNmeaTime.year, mLastNmeaTime.mon + 1, mLastNmeaTime.day ),
                    QTime( mLastNmeaTime.hour, mLastNmeaTime.min, mLastNmeaTime.sec, mLastNmeaTime.msec ) );
    // Time from GPS is UTC time
    time.setTimeSpec( Qt::UTC );
    // Apply leap seconds correction
    if ( mCbxLeapSeconds->isChecked() && mLeapSeconds->value() != 0 )
    {
      time = time.addSecs( mLeapSeconds->value() );
    }
    // Desired format
    const Qt::TimeSpec timeSpec { static_cast<Qt::TimeSpec>( mCboTimestampFormat->currentData( ).toInt() ) };
    time = time.toTimeSpec( timeSpec );
    if ( timeSpec == Qt::TimeSpec::TimeZone )
    {
      // Get timezone from the combo
      const QTimeZone destTz( mCboTimeZones->currentText().toUtf8() );
      if ( destTz.isValid() )
      {
        time = time.toTimeZone( destTz );
      }
    }
    else if ( timeSpec == Qt::TimeSpec::LocalTime )
    {
      time = time.toLocalTime();
    }
    else if ( timeSpec == Qt::TimeSpec::UTC )
    {
      // Do nothing: we are already in UTC
    }

    // Only string and datetime fields are supported
    switch ( vlayer->fields().at( idx ).type() )
    {
      case QVariant::String:
        value = time.toString( Qt::DateFormat::ISODate );
        break;
      case QVariant::DateTime:
        value = time;
        break;
      default:
        break;
    }
  }
  return value;
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

void QgsGpsInformationWidget::switchAcquisition()
{
  if ( mAcquisitionInterval > 0 )
  {
    if ( mAcquisitionEnabled )
      mAcquisitionTimer->start( mAcquisitionInterval );
    else
      //wait only acquisitionInterval/10 for new valid data
      mAcquisitionTimer->start( mAcquisitionInterval / 10 );
    // anyway switch to enabled / disabled acquisition
    mAcquisitionEnabled = !mAcquisitionEnabled;
  }
}
