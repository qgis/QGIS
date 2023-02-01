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
#include "qgssettings.h"
#include "qgsstatusbar.h"
#include "qgsmapcanvas.h"
#include "qgsgpsconnection.h"
#include "qgscoordinateutils.h"
#include "qgsappgpssettingsmenu.h"
#include "qgsappgpsdigitizing.h"

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
    QgsMapCanvas *mapCanvas, QgsAppGpsDigitizing *digitizing, QWidget *parent )
  : QgsPanelWidget( parent )
  , mConnection( connection )
  , mMapCanvas( mapCanvas )
  , mDigitizing( digitizing )
{
  Q_ASSERT( mMapCanvas ); // precondition
  setupUi( this );
  connect( mConnectButton, &QPushButton::toggled, this, &QgsGpsInformationWidget::mConnectButton_toggled );
  connect( mBtnPosition, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnPosition_clicked );
  connect( mBtnSignal, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnSignal_clicked );
  connect( mBtnSatellites, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnSatellites_clicked );
  connect( mBtnDebug, &QToolButton::clicked, this, &QgsGpsInformationWidget::mBtnDebug_clicked );

  mBtnPopupOptions->setAutoRaise( true );
  mBtnPopupOptions->setToolTip( tr( "Settings" ) );
  mBtnPopupOptions->setMenu( QgisApp::instance()->gpsSettingsMenu() );
  mBtnPopupOptions->setPopupMode( QToolButton::InstantPopup );

  QWidget *mpHistogramWidget = mStackedWidget->widget( 1 );
#ifndef WITH_QWTPOLAR
  mBtnSatellites->setVisible( false );
#endif
  //
  // Set up the graph for signal strength
  //
  mPlot = new QwtPlot( mpHistogramWidget );
  mPlot->setAutoReplot( false );   // plot on demand
  //mPlot->setTitle(QObject::tr("Signal Status"));
  //mPlot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
  // Set axis titles
  //mPlot->setAxisTitle(QwtPlot::xBottom, QObject::tr("Satellite"));
  //mPlot->setAxisTitle(QwtPlot::yLeft, QObject::tr("Value"));
  mPlot->setAxisScale( QwtPlot::xBottom, 0, 20 );
  mPlot->setAxisScale( QwtPlot::yLeft, 0, 60 );  // max is 50dB SNR, I believe - SLM
  // add a grid
  QwtPlotGrid *mGrid = new QwtPlotGrid();
  mGrid->enableX( false );
  mGrid->attach( mPlot );
  //display satellites first
  mCurve = new QwtPlotCurve();
  mCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
  mCurve->setPen( QPen( Qt::blue ) );
  mCurve->setBrush( QBrush( Qt::blue ) );
  mPlot->enableAxis( QwtPlot::yLeft, true );
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

  mStackedWidget->setCurrentIndex( 0 );
  mBtnPosition->setFocus( Qt::TabFocusReason );

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

  connect( mDigitizing, &QgsAppGpsDigitizing::trackVertexAdded, this, &QgsGpsInformationWidget::updateTrackInformation );
  connect( mDigitizing, &QgsAppGpsDigitizing::trackReset, this, &QgsGpsInformationWidget::updateTrackInformation );
  connect( mDigitizing, &QgsAppGpsDigitizing::distanceAreaChanged, this, &QgsGpsInformationWidget::updateTrackInformation );
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

void QgsGpsInformationWidget::mBtnDebug_clicked()
{
  mStackedWidget->setCurrentIndex( 3 );
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
  mTxtAltitudeDiff->clear();
  mTxtAltitudeEllipsoid->clear();
  mTxtDateTime->clear();
  mTxtSpeed->clear();
  mTxtDirection->clear();
  mTxtHdop->clear();
  mTxtVdop->clear();
  mTxtPdop->clear();
  mTxtHacc->clear();
  mTxtVacc->clear();
  mTxt3Dacc->clear();
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
}

void QgsGpsInformationWidget::updateTrackInformation()
{
  const double totalTrackLength = mDigitizing->totalTrackLength();
  const double directTrackLength = mDigitizing->trackDistanceFromStart();

  const QgsSettings settings;
  const bool keepBaseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();
  const int decimalPlaces = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), 3 ).toInt();

  if ( totalTrackLength > 0 )
  {
    mTxtTotalTrackLength->setEnabled( true );
    mTxtTotalTrackLength->setText( mDigitizing->distanceArea().formatDistance( totalTrackLength, decimalPlaces, mDigitizing->distanceArea().lengthUnits(), keepBaseUnit ) );
  }
  else
  {
    mTxtTotalTrackLength->setEnabled( false );
    mTxtTotalTrackLength->setText( tr( "Not available" ) );
  }

  if ( directTrackLength > 0 )
  {
    mTxtDirectTrackLength->setEnabled( true );
    mTxtDirectTrackLength->setText( mDigitizing->distanceArea().formatDistance( directTrackLength, decimalPlaces, mDigitizing->distanceArea().lengthUnits(), keepBaseUnit ) );
  }
  else
  {
    mTxtDirectTrackLength->setEnabled( false );
    mTxtDirectTrackLength->setText( tr( "Not available" ) );
  }
}

void QgsGpsInformationWidget::gpsDisconnected()
{
  // clear position page fields to give better indication that something happened (or didn't happen)
  mTxtLatitude->clear();
  mTxtLongitude->clear();
  mTxtAltitude->clear();
  mTxtAltitudeDiff->clear();
  mTxtAltitudeEllipsoid->clear();
  mTxtDateTime->clear();
  mTxtSpeed->clear();
  mTxtDirection->clear();
  mTxtHdop->clear();
  mTxtVdop->clear();
  mTxtPdop->clear();
  mTxtHacc->clear();
  mTxtVacc->clear();
  mTxt3Dacc->clear();
  mTxtFixMode->clear();
  mTxtFixType->clear();
  mTxtQuality->clear();
  mTxtSatellitesUsed->clear();
  mTxtStatus->clear();

  // Clear Plot Signal data
  QVector<QPointF> data;
  mCurve->setSamples( data );
  mPlot->replot();

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
  if ( mStackedWidget->currentIndex() == 3 ) //debug
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
    mTxtAltitudeEllipsoid->setText( tr( "%1 m" ).arg( info.elevation + info.elevation_diff, 0, 'f', 3 ) );

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
      mTxtSpeed->setText( tr( "%1 km/h" ).arg( info.speed, 0, 'f', 3 ) );
    }
    else
    {
      mTxtSpeed->setEnabled( false );
      mTxtSpeed->setText( tr( "Not available" ) );
    }
    if ( std::isfinite( info.direction ) )
    {
      mTxtDirection->setEnabled( true );
      mTxtDirection->setText( QString::number( info.direction, 'f', 3 ) + QStringLiteral( "°" ) );
    }
    else
    {
      mTxtDirection->setEnabled( false );
      mTxtDirection->setText( tr( "Not available" ) );
    }
    mTxtHdop->setText( QString::number( info.hdop, 'f', 3 ) );
    mTxtVdop->setText( QString::number( info.vdop, 'f', 3 ) );
    mTxtPdop->setText( QString::number( info.pdop, 'f', 3 ) );
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
