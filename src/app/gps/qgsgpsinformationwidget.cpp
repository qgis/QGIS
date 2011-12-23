/***************************************************************************
               qgsgpsinformationwidget.h  -  description
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
/*  $Id: qgisapp.h 12390 2009-12-09 21:35:43Z jef $ */
#include "qgsgpsinformationwidget.h"
#include "qgsnmeaconnection.h"
#include "qgsgpsconnectionregistry.h"
#include "qgsgpsdetector.h"
#include "qgscoordinatetransform.h"
#include <qgspoint.h>
#include <qgsrubberband.h>
#include "qgsmaprenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsfeatureaction.h"
#include "qgsgeometry.h"

//for avoid intersections static method
#include "qgsmaptooladdfeature.h"

// QWT Charting widget
#include <qwt_array.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
// QWT Polar plot add on
#include <qpen.h>
#include <qwt_data.h>
#include <qwt_symbol.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_scale_engine.h>

#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QPointF>
#include <QColorDialog>


QgsGPSInformationWidget::QgsGPSInformationWidget( QgsMapCanvas * thepCanvas, QWidget * parent, Qt::WindowFlags f ):
    QWidget( parent, f ),
    mNmea( 0 ),
    mpCanvas( thepCanvas )
{
  setupUi( this );
  mpMapMarker = 0;
  mpRubberBand = 0;
  populateDevices();
  QWidget * mpHistogramWidget = mStackedWidget->widget( 1 );
  //
  // Set up the graph for signal strength
  //
  mpPlot = new QwtPlot( mpHistogramWidget );
  //mpPlot->setTitle(QObject::tr("Signal Status"));
  //mpPlot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
  // Set axis titles
  //mpPlot->setAxisTitle(QwtPlot::xBottom, QObject::tr("Satellite"));
  //mpPlot->setAxisTitle(QwtPlot::yLeft, QObject::tr("Value"));
  mpPlot->setAxisScale( QwtPlot::xBottom, 0, 20 );
  mpPlot->setAxisScale( QwtPlot::yLeft, 0, 100 );
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
  //mpSatellitesWidget = new QwtPolarPlot(QwtText(tr("Satellite Positions")), myTab );
  mpSatellitesWidget = new QwtPolarPlot( mpPolarWidget );
  mpSatellitesWidget->setAutoReplot( true );
  mpSatellitesWidget->setPlotBackground( Qt::white );
  // scales
  mpSatellitesWidget->setScale( QwtPolar::Azimuth,
                                0, //min
                                360, //max
                                30 //interval
                              );

  mpSatellitesWidget->setScaleMaxMinor( QwtPolar::Azimuth, 10 );
  mpSatellitesWidget->setScale( QwtPolar::Radius,
                                0, //min
                                100 //max
                              );

  // grids, axes

  QwtPolarGrid * mypSatellitesGrid = new QwtPolarGrid();
  mypSatellitesGrid->setPen( QPen( Qt::black ) );
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
  {
    mypSatellitesGrid->showGrid( scaleId );
    //mypSatellitesGrid->showMinorGrid(scaleId);
    QPen minorPen( Qt::gray );
    mypSatellitesGrid->setMinorGridPen( scaleId, minorPen );
  }
  mypSatellitesGrid->setAxisPen( QwtPolar::AxisAzimuth, QPen( Qt::black ) );

  mypSatellitesGrid->showAxis( QwtPolar::AxisAzimuth, true );
  mypSatellitesGrid->showAxis( QwtPolar::AxisLeft, false ); //alt axis
  mypSatellitesGrid->showAxis( QwtPolar::AxisRight, false );//alt axis
  mypSatellitesGrid->showAxis( QwtPolar::AxisTop, false );//alt axis
  mypSatellitesGrid->showAxis( QwtPolar::AxisBottom, false );//alt axis
  mypSatellitesGrid->showGrid( QwtPolar::Azimuth, true );
  mypSatellitesGrid->showGrid( QwtPolar::Radius, true );
  mypSatellitesGrid->attach( mpSatellitesWidget );

  //QwtLegend *legend = new QwtLegend;
  //mpSatellitesWidget->insertLegend(legend,  QwtPolarPlot::BottomLegend);
  QVBoxLayout *mpPolarLayout = new QVBoxLayout( mpPolarWidget );
  mpPolarLayout->setContentsMargins( 0, 0, 0, 0 );
  mpPolarLayout->addWidget( mpSatellitesWidget );
  mpPolarWidget->setLayout( mpPolarLayout );


  // Restore state
  QSettings mySettings;
  mSliderMarkerSize->setValue( mySettings.value( "/gps/markerSize", "12" ).toInt() );
  mSpinTrackWidth->setValue( mySettings.value( "/gps/trackWidth", "2" ).toInt() );
  QString myPortMode = mySettings.value( "/gps/portMode", "scanPorts" ).toString();

  mGpsdHost->setText( mySettings.value( "/gps/gpsdHost", "localhost" ).toString() );
  mGpsdPort->setText( mySettings.value( "/gps/gpsdPort", 2947 ).toString() );
  mGpsdDevice->setText( mySettings.value( "/gps/gpsdDevice" ).toString() );

  //port mode
  if ( myPortMode == "scanPorts" )
  {
    mRadAutodetect->setChecked( true );
  }
  else if ( myPortMode == "explicitPort" )
  {
    mRadUserPath->setChecked( true );
  }
  else if ( myPortMode == "gpsd" )
  {
    mRadGpsd->setChecked( true );
  }
  //auto digitising behaviour
  bool myAutoAddVertexFlag = mySettings.value( "/gps/autoAddVertices", "false" ).toBool();
  mCbxAutoAddVertices->setChecked( myAutoAddVertexFlag );
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
  // Set up the rubber band to show digitising
  createRubberBand();
  mWgs84CRS.createFromOgcWmsCrs( "EPSG:4326" );
  //for now I am hiding accuracy and date widgets
  mDateTime->hide();
  mVerticalAccuracy->hide();
  mHorizontalAccuracy->hide();
  mBtnDebug->hide();
}

QgsGPSInformationWidget::~QgsGPSInformationWidget()
{
  if ( mpMapMarker )
    delete mpMapMarker;

  QSettings mySettings;
  mySettings.setValue( "/gps/lastPort", mCboDevices->itemData( mCboDevices->currentIndex() ).toString() );
  mySettings.setValue( "/gps/trackWidth", mSpinTrackWidth->value() );
  mySettings.setValue( "/gps/markerSize", mSliderMarkerSize->value() );
  mySettings.setValue( "/gps/autoAddVertices", mCbxAutoAddVertices->isChecked() );
  // scan, explicit port or gpsd
  if ( mRadAutodetect->isChecked() )
  {
    mySettings.setValue( "/gps/portMode", "scanPorts" );
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
  QSettings mySettings;
  mySettings.setValue( "/gps/trackWidth", theValue );
  if ( mpRubberBand )
  {
    mpRubberBand->setWidth( theValue );
  }
}

void QgsGPSInformationWidget::on_mBtnTrackColour_clicked( )
{
  QSettings mySettings;
  QColor myColor( mySettings.value( "/qgis/gps/line_color_red", 255 ).toInt(),
                  mySettings.value( "/qgis/gps/line_color_green", 0 ).toInt(),
                  mySettings.value( "/qgis/gps/line_color_blue", 0 ).toInt() );
  myColor = QColorDialog::getColor( myColor, this );
  mySettings.setValue( "/qgis/gps/line_color_red", myColor.red() );
  mySettings.setValue( "/qgis/gps/line_color_green", myColor.green() );
  mySettings.setValue( "/qgis/gps/line_color_blue", myColor.blue() );
  setTrackColour();
}


void QgsGPSInformationWidget::setTrackColour( )
{
  QSettings mySettings;
  QColor myColor( mySettings.value( "/qgis/gps/line_color_red", 255 ).toInt(),
                  mySettings.value( "/qgis/gps/line_color_green", 0 ).toInt(),
                  mySettings.value( "/qgis/gps/line_color_blue", 0 ).toInt() );
  if ( mpRubberBand )
  {
    mpRubberBand->setColor( myColor );
  }
}
void QgsGPSInformationWidget::on_mBtnPosition_clicked( )
{
  mStackedWidget->setCurrentIndex( 0 );
}

void QgsGPSInformationWidget::on_mBtnSignal_clicked( )
{
  mStackedWidget->setCurrentIndex( 1 );
}

void QgsGPSInformationWidget::on_mBtnSatellites_clicked( )
{
  mStackedWidget->setCurrentIndex( 2 );
}

void QgsGPSInformationWidget::on_mBtnOptions_clicked( )
{
  mStackedWidget->setCurrentIndex( 3 );
}

void QgsGPSInformationWidget::on_mBtnDebug_clicked( )
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

  mGPSTextEdit->append( tr( "Connecting..." ) );

  QgsGPSDetector *detector = new QgsGPSDetector( port );
  connect( detector, SIGNAL( detected( QgsGPSConnection * ) ), this, SLOT( connected( QgsGPSConnection * ) ) );
  connect( detector, SIGNAL( detectionFailed() ), this, SLOT( timedout() ) );
}

void QgsGPSInformationWidget::timedout()
{
  mConnectButton->setChecked( false );
  mNmea = NULL;
  mGPSTextEdit->append( tr( "Timed out!" ) );
}

void QgsGPSInformationWidget::connected( QgsGPSConnection *conn )
{
  mNmea = conn;
  QObject::connect( mNmea, SIGNAL( stateChanged( const QgsGPSInformation& ) ),
                    this, SLOT( displayGPSInformation( const QgsGPSInformation& ) ) );
  mGPSTextEdit->append( tr( "Connected!" ) );
  mConnectButton->setText( tr( "Disconnect" ) );
  //insert connection into registry such that it can also be used by other dialogs or plugins
  QgsGPSConnectionRegistry::instance()->registerConnection( mNmea );
}

void QgsGPSInformationWidget::disconnectGps()
{
  QgsGPSConnectionRegistry::instance()->unregisterConnection( mNmea );
  delete mNmea;
  mGPSTextEdit->append( tr( "Disconnected..." ) );
  mConnectButton->setChecked( false );
  mConnectButton->setText( tr( "Connect" ) );
}


void QgsGPSInformationWidget::displayGPSInformation( const QgsGPSInformation& info )
{
  mGPSTextEdit->clear();

  QwtArray<double> myXData;//qwtarray is just a wrapped qvector
  QwtArray<double> mySignalData;//qwtarray is just a wrapped qvector
  mpPlot->setAxisScale( QwtPlot::xBottom, 0, info.satellitesInView.size() );
  while ( !mMarkerList.isEmpty() )
  {
    delete mMarkerList.takeFirst();
  }

  for ( int i = 0; i < info.satellitesInView.size(); ++i )
  {
    QgsSatelliteInfo currentInfo = info.satellitesInView.at( i );

    myXData.append( i );
    mySignalData.append( 0 );
    myXData.append( i );
    mySignalData.append( currentInfo.signal );
    myXData.append( i + 1 );
    mySignalData.append( currentInfo.signal );
    myXData.append( i + 1 );
    mySignalData.append( 0 );
    mGPSTextEdit->append( "Satellite" );
    if ( currentInfo.inUse )
    {
      mGPSTextEdit->append( "In use" );
    }
    else
    {
      mGPSTextEdit->append( "Not in use" );
    }
    mGPSTextEdit->append( "id: " + QString::number( currentInfo.id ) );
    mGPSTextEdit->append( "elevation: " + QString::number( currentInfo.elevation ) );
    mGPSTextEdit->append( "azimuth: " + QString::number( currentInfo.azimuth ) );
    mGPSTextEdit->append( "signal: " + QString::number( currentInfo.signal ) );
    // Add a marker to the polar plot
    QwtPolarMarker *mypMarker = new QwtPolarMarker();
    mypMarker->setPosition( QwtPolarPoint( currentInfo.azimuth, currentInfo.elevation ) );
    QColor myColour;
    if ( currentInfo.signal < 30 ) //weak signal
    {
      myColour = Qt::red;
    }
    else
    {
      myColour = Qt::black; //strong signal
    }
    mypMarker->setSymbol( QwtSymbol( QwtSymbol::Ellipse,
                                     QBrush( Qt::black ), QPen( myColour ), QSize( 9, 9 ) ) );
    mypMarker->setLabelAlignment( Qt::AlignHCenter | Qt::AlignTop );
    QwtText text( QString::number( currentInfo.id ) );
    text.setColor( myColour );
    QColor bg( Qt::white );
    bg.setAlpha( 200 );
    text.setBackgroundBrush( QBrush( bg ) );
    mypMarker->setLabel( text );
    mypMarker->attach( mpSatellitesWidget );
    mMarkerList << mypMarker;
  }
  mpCurve->setData( myXData, mySignalData );
  mpPlot->replot();
  if ( mpMapMarker )
    delete mpMapMarker;

  //after loosing connection, the first gps info sometimes has uninitialized coords
  QgsPoint myNewCenter;
  if ( doubleNear( info.longitude, 0.0 ) && doubleNear( info.latitude, 0.0 ) )
  {
    myNewCenter = mLastGpsPosition;
  }
  else
  {
    myNewCenter = QgsPoint( info.longitude, info.latitude );
  }

  if ( mGroupShowMarker->isChecked() )
  {
    mpMapMarker = new QgsGpsMarker( mpCanvas );
    mpMapMarker->setSize( mSliderMarkerSize->value() );
    mpMapMarker->setCenter( myNewCenter );
    mpMapMarker->update();
  }
  else
  {
    mpMapMarker = 0;
  }
  mSpinLatitude->setValue( info.latitude );
  mSpinLongitude->setValue( info.longitude );
  mSpinElevation->setValue( info.elevation );
  mHorizontalAccuracy->setValue( 10 - info.hdop );
  mVerticalAccuracy->setValue( 10 - info.vdop );
  mGPSTextEdit->append( "longitude: " + QString::number( info.longitude ) );
  mGPSTextEdit->append( "latitude: " + QString::number( info.latitude ) );
  mGPSTextEdit->append( "elevation: " + QString::number( info.elevation ) );
  mGPSTextEdit->append( "pdop: " + QString::number( info.pdop ) );
  mGPSTextEdit->append( "hdop: " + QString::number( info.hdop ) );
  mGPSTextEdit->append( "vdop: " + QString::number( info.vdop ) );

  // Avoid refreshing / panning if we havent moved
  if ( mLastGpsPosition != myNewCenter )
  {
    mLastGpsPosition = myNewCenter;
    if ( mCbxAutoAddVertices->isChecked() )
    {
      addVertex();
    }
    // Pan based on user specified behaviour

    if ( radRecenterMap->isChecked() || radRecenterWhenNeeded->isChecked() )
    {
      QgsCoordinateReferenceSystem mypSRS = mpCanvas->mapRenderer()->destinationCrs();
      QgsCoordinateReferenceSystem myLatLongRefSys = QgsCoordinateReferenceSystem( 4326 );
      QgsCoordinateTransform myTransform( myLatLongRefSys, mypSRS );

      QgsPoint myPoint = myTransform.transform( myNewCenter );
      //keep the extent the same just center the map canvas in the display so our feature is in the middle
      QgsRectangle myRect(
        myPoint.x( ) - ( mpCanvas->extent( ).width( ) / 2 ),
        myPoint.y( ) - ( mpCanvas->extent( ).height( ) / 2 ),
        myPoint.x( ) + ( mpCanvas->extent( ).width( ) / 2 ),
        myPoint.y( ) + ( mpCanvas->extent( ).height( ) / 2 ) );

      // only change the extents if the point is beyond the current extents to minimise repaints
      if (( !mpCanvas->extent().contains( myPoint ) &&
            radRecenterWhenNeeded->isChecked() ) ||
          radRecenterMap->isChecked() )
      {
        mpCanvas->setExtent( myRect );
        mpCanvas->refresh( );
      }
    } //otherwise never recenter automatically
  }
}

void QgsGPSInformationWidget::on_mCbxAutoAddVertices_toggled( bool theFlag )
{
  if ( theFlag )
  {
    mBtnAddVertex->hide();
  }
  else
  {
    mBtnAddVertex->show();
  }
}

void QgsGPSInformationWidget::on_mBtnAddVertex_clicked( )
{
  addVertex();
}

void QgsGPSInformationWidget::addVertex( )
{

  if ( !mpRubberBand )
  {
    createRubberBand( );
  }

  // we store the capture list in wgs84 and then transform to layer crs when
  // calling close feature
  mCaptureList.push_back( mLastGpsPosition );
  // we store the rubber band points in map canvas CRS so transform to map crs
  QgsPoint myPoint;
  if ( mpCanvas && mpCanvas->mapRenderer() )
  {
    QgsCoordinateTransform t( mWgs84CRS, mpCanvas->mapRenderer()->destinationCrs() );
    myPoint = t.transform( mLastGpsPosition );
  }
  else
  {
    myPoint = mLastGpsPosition;
  }
  mpRubberBand->addPoint( myPoint );
}

void QgsGPSInformationWidget::on_mBtnResetFeature_clicked( )
{
  mNmea->disconnect();
  if ( mpRubberBand )
  {
    delete mpRubberBand;
    mpRubberBand = 0;
  }
  createRubberBand( );
  mCaptureList.clear();
  connectGpsSlot();
}

void QgsGPSInformationWidget::on_mBtnCloseFeature_clicked( )
{
  mNmea->disconnect();
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mpCanvas->currentLayer() );

  // -------------- preconditions ------------------------
  if ( !vlayer )
  {
    QMessageBox::information( 0, tr( "Not a vector layer" ),
                              tr( "The current layer is not a vector layer" ) );
    connectGpsSlot();
    return;
  }
  QGis::WkbType layerWKBType = vlayer->wkbType();

  //no support for adding features to 2.5D types yet
  if ( layerWKBType == QGis::WKBLineString25D || layerWKBType == QGis::WKBPolygon25D ||
       layerWKBType == QGis::WKBMultiLineString25D || layerWKBType == QGis::WKBPoint25D || layerWKBType == QGis::WKBMultiPoint25D )
  {
    QMessageBox::critical( 0, tr( "2.5D shape type not supported" ), tr(
                             "Adding features to 2.5D shapetypes is not supported yet. Please "
                             "select a different editable, non 2.5D layer and try again." ) );
    connectGpsSlot();
    return;
  }

  // Multipart features not supported
  if ( layerWKBType == QGis::WKBMultiLineString ||
       layerWKBType == QGis::WKBMultiPoint ||
       layerWKBType == QGis::WKBMultiLineString ||
       layerWKBType == QGis::WKBMultiPolygon )
  {
    QMessageBox::critical( 0, tr( "Multipart shape type not supported" ), tr(
                             "Adding features to multipart shapetypes is not supported yet. Please "
                             "select a different editable, non 2.5D layer and try again." ) );
    connectGpsSlot();
    return;

  }

  QgsVectorDataProvider* provider = vlayer->dataProvider();

  if ( !( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    QMessageBox::information( 0, tr( "Layer cannot be added to" ),
                              tr( "The data provider for this layer does not support the addition of features." ) );
    connectGpsSlot();
    return;
  }

  if ( !vlayer->isEditable() )
  {
    QMessageBox::information( 0, tr( "Layer not editable" ),
                              tr( "Cannot edit the vector layer. Use 'Toggle Editing' to make it editable." )
                            );
    connectGpsSlot();
    return;
  }

  //lines: bail out if there are not at least two vertices
  if ( layerWKBType == QGis::WKBLineString  && mCaptureList.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough vertices" ),
                              tr( "Cannot close a line feature until it has at least two vertices." ) );
    connectGpsSlot();
    return;
  }

  //polygons: bail out if there are not at least three vertices
  if ( layerWKBType == QGis::WKBPolygon && mCaptureList.size() < 3 )
  {
    QMessageBox::information( 0, tr( "Not enough vertices" ),
                              tr( "Cannot close a polygon feature until it has at least three vertices." ) );
    connectGpsSlot();
    return;
  }
  // -------------- end of preconditions ------------------------

  //
  // POINT CAPTURING
  //
  if ( layerWKBType == QGis::WKBPoint )
  {
    //only do the rest for provider with feature addition support
    //note that for the grass provider, this will return false since
    //grass provider has its own mechanism of feature addition
    if ( provider->capabilities() & QgsVectorDataProvider::AddFeatures )
    {
      QgsFeature* f = new QgsFeature( 0, "WKBPoint" );

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
        mpCanvas->refresh();

      delete f;
    }
  }
  else // Line or poly
  {
    //create QgsFeature with wkb representation
    QgsFeature* f = new QgsFeature( 0, "WKBLineString" );
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
    else //unknown type
    {
      QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot add feature. "
                             "Unknown WKB type. Choose a different layer and try again." ) );
      connectGpsSlot();
      return; //unknown wkbtype
    }

    QgsFeatureAction action( tr( "Feature added" ), *f, vlayer, -1, -1, this );
    if ( action.addFeature() )
      mpCanvas->refresh();

    delete f;

    delete mpRubberBand;
    mpRubberBand = NULL;

    // delete the elements of mCaptureList
    mCaptureList.clear();
    mpCanvas->refresh();
  }
  connectGpsSlot();
}

void QgsGPSInformationWidget::connectGpsSlot( )
{
  QObject::connect( mNmea, SIGNAL( stateChanged( const QgsGPSInformation& ) ), this, SLOT( displayGPSInformation( const QgsGPSInformation& ) ) );
}

void QgsGPSInformationWidget::on_mBtnRefreshDevices_clicked( )
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

void QgsGPSInformationWidget::createRubberBand( )
{
  if ( mpRubberBand )
  {
    delete mpRubberBand;
  }
  QSettings settings;
  mpRubberBand = new QgsRubberBand( mpCanvas, false );
  setTrackColour();
  mpRubberBand->setWidth( settings.value( "/gps/trackWidth", 2 ).toInt() );
  mpRubberBand->show();
}

QPointF QgsGPSInformationWidget::gpsToPixelPosition( const QgsPoint& point )
{
  //transform to map crs
  QgsPoint myCenter;
  if ( mpCanvas && mpCanvas->mapRenderer() )
  {
    QgsCoordinateTransform t( mWgs84CRS, mpCanvas->mapRenderer()->destinationCrs() );
    myCenter = t.transform( point );
  }
  else
  {
    myCenter = point;
  }

  double x = myCenter.x(), y = myCenter.y();
  mpCanvas->getCoordinateTransform()->transformInPlace( x, y );
  return QPointF( x, y ) ;
}
