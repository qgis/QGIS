/***************************************************************************
     testqgsmapcanvasdockwidget.cpp
     --------------------------------------
    Date                 : March 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QApplication>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDockWidget>
#include <QSignalSpy>

#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasdockwidget.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the app map canvas dock widgets.
 */
class TestQgsMapCanvasDockWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapCanvasDockWidget();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {};         // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testNoSync();
    void testScaleSync();
    void testCenterSync();
    void testScaleAndCenterSync();

  private:
};

TestQgsMapCanvasDockWidget::TestQgsMapCanvasDockWidget() = default;

//runs before all tests
void TestQgsMapCanvasDockWidget::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();
}

//runs after all tests
void TestQgsMapCanvasDockWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapCanvasDockWidget::testNoSync()
{
  // canvases should be completely independent
  QgsMapCanvas mainCanvas;
  mainCanvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mainCanvas.setFrameStyle( QFrame::NoFrame );
  mainCanvas.resize( 600, 600 );
  mainCanvas.setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  mainCanvas.show();

  double testScalingFactor = 44823779 / mainCanvas.scale();
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 44823779, 10000 );

  QgsMapCanvasDockWidget dock( QStringLiteral( "dock" ) );
  dock.setMainCanvas( &mainCanvas );
  dock.mapCanvas()->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  dock.mapCanvas()->setFrameStyle( QFrame::NoFrame );
  dock.setFixedSize( 600, 600 );
  dock.mapCanvas()->setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  dock.show();

  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 44823779, 1000000 );

  dock.setViewCenterSynchronized( false );
  dock.setViewScaleSynchronized( false );

  QSignalSpy resizeTimerSpy( &dock.mResizeTimer, &QTimer::timeout );
  resizeTimerSpy.wait();

  mainCanvas.zoomScale( 89647558 );

  // dock should not inherit scale
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 44823779, 1000000 );

  // change scale in dock and check it is not synced to main canvas
  dock.mapCanvas()->zoomScale( 1500000 );
  testScalingFactor = 1500000 / dock.mapCanvas()->scale();
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1500000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 89647558, 1000 );

  // extent should NOT be synced
  dock.mapCanvas()->setCenter( QgsPointXY( -22329833, 3515327 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -11281815, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), 4287781, 1000 );

  mainCanvas.setCenter( QgsPointXY( -4467497, -227904 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );
}

void TestQgsMapCanvasDockWidget::testScaleSync()
{
  QgsMapCanvas mainCanvas;
  mainCanvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mainCanvas.setFrameStyle( QFrame::NoFrame );
  mainCanvas.setFixedSize( 600, 600 );
  mainCanvas.setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  mainCanvas.show();

  double testScalingFactor = 44823779 / mainCanvas.scale();
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 44823779, 10000 );

  QgsMapCanvasDockWidget dock( QStringLiteral( "dock" ) );
  dock.setMainCanvas( &mainCanvas );
  dock.mapCanvas()->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  dock.mapCanvas()->setFrameStyle( QFrame::NoFrame );
  dock.setFixedSize( 600, 600 );
  dock.mapCanvas()->setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  dock.show();

  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 44823779, 1000000 );

  dock.setViewCenterSynchronized( false );
  dock.setViewScaleSynchronized( true );
  dock.setScaleFactor( 1 );

  QSignalSpy resizeTimerSpy( &dock.mResizeTimer, &QTimer::timeout );
  resizeTimerSpy.wait();

  mainCanvas.zoomScale( 89647558 );
  testScalingFactor = 89647558 / mainCanvas.scale();

  // dock should inherit scale
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 89647558, 1000000 );

  // ensure scale is multiplied by factor
  dock.setScaleFactor( 2.5 );
  mainCanvas.zoomScale( 44823779 );

  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 17929511, 1000000 );

  // change scale in dock and check it is respected by main canvas
  dock.setScaleFactor( 1.0 );
  dock.mapCanvas()->zoomScale( 1500000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1500000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 1500000, 1000 );

  dock.setScaleFactor( 2.0 );
  dock.mapCanvas()->zoomScale( 1000000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1000000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 2000000, 1000 );

  // extent should NOT be synced, and scale should not change when extent changes
  dock.mapCanvas()->setCenter( QgsPointXY( -22329833, 3515327 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -11281815, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), 4287781, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1000000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale(), 2000000, 1000 );

  mainCanvas.setCenter( QgsPointXY( -4467497, -227904 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1000000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 2000000, 1000 );

  dock.setFixedSize( 1200, 1200 );
  resizeTimerSpy.wait();

  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 500000, 10000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 1000000, 100000 );
}

void TestQgsMapCanvasDockWidget::testCenterSync()
{
  // canvases should be completely independent
  QgsMapCanvas mainCanvas;
  mainCanvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mainCanvas.setFrameStyle( QFrame::NoFrame );
  mainCanvas.setFixedSize( 600, 600 );
  mainCanvas.setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  mainCanvas.show();

  double testScalingFactor = 44823779 / mainCanvas.scale();
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 44823779, 10000 );

  QgsMapCanvasDockWidget dock( QStringLiteral( "dock" ) );
  dock.setMainCanvas( &mainCanvas );
  dock.mapCanvas()->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  dock.mapCanvas()->setFrameStyle( QFrame::NoFrame );
  dock.setFixedSize( 600, 600 );
  dock.mapCanvas()->setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  dock.show();

  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 44823779, 1000000 );

  dock.setViewCenterSynchronized( true );
  dock.setViewScaleSynchronized( false );

  QSignalSpy resizeTimerSpy( &dock.mResizeTimer, &QTimer::timeout );
  resizeTimerSpy.wait();

  mainCanvas.zoomScale( 89647558 );

  // dock should not inherit scale
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 44823779, 1000000 );

  // change scale in dock and check it is not synced to main canvas
  dock.mapCanvas()->zoomScale( 1500000 );
  testScalingFactor = 1500000 / dock.mapCanvas()->scale();
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1500000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 89647558, 1000 );

  // center SHOULD be synced
  dock.mapCanvas()->setCenter( QgsPointXY( -22329833, 3515327 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), 3515327, 1000 );

  mainCanvas.setCenter( QgsPointXY( -4467497, -227904 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), -227904, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );

  dock.setFixedSize( 1200, 1200 );
  resizeTimerSpy.wait();

  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), -227904, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 744966, 10000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 89647558, 1000 );
}

void TestQgsMapCanvasDockWidget::testScaleAndCenterSync()
{
  // canvases should be completely independent
  QgsMapCanvas mainCanvas;
  mainCanvas.setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mainCanvas.setFrameStyle( QFrame::NoFrame );
  mainCanvas.setFixedSize( 600, 600 );
  mainCanvas.setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  mainCanvas.show();

  double testScalingFactor = 44823779 / mainCanvas.scale();
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 44823779, 10000 );

  QgsMapCanvasDockWidget dock( QStringLiteral( "dock" ) );
  dock.setMainCanvas( &mainCanvas );
  dock.mapCanvas()->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  dock.mapCanvas()->setFrameStyle( QFrame::NoFrame );
  dock.setFixedSize( 600, 600 );
  dock.mapCanvas()->setExtent( QgsRectangle( -14839703, 2282029, -7723928, 6293534 ) );
  dock.show();

  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 44823779, 1000000 );

  dock.setViewCenterSynchronized( true );
  dock.setViewScaleSynchronized( true );

  QSignalSpy resizeTimerSpy( &dock.mResizeTimer, &QTimer::timeout );
  resizeTimerSpy.wait();

  mainCanvas.zoomScale( 89647558 );
  testScalingFactor = 89647558 / mainCanvas.scale();

  // dock should inherit scale
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 89647558, 1000000 );

  // ensure scale is multiplied by factor
  dock.setScaleFactor( 2.5 );
  mainCanvas.zoomScale( 44823779 );

  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 17929511, 1000000 );

  // change scale in dock and check it is respected by main canvas
  dock.setScaleFactor( 1.0 );
  dock.mapCanvas()->zoomScale( 1500000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1500000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 1500000, 1000 );

  dock.setScaleFactor( 2.0 );
  dock.mapCanvas()->zoomScale( 1000000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 1000000, 1000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 2000000, 1000 );

  // center SHOULD be synced
  dock.mapCanvas()->setCenter( QgsPointXY( -22329833, 3515327 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), 3515327, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -22329833, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), 3515327, 1000 );

  mainCanvas.setCenter( QgsPointXY( -4467497, -227904 ) );
  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), -227904, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );

  dock.setFixedSize( 1200, 1200 );
  resizeTimerSpy.wait();

  QGSCOMPARENEAR( dock.mapCanvas()->center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->center().y(), -227904, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().x(), -4467497, 1000 );
  QGSCOMPARENEAR( mainCanvas.center().y(), -227904, 1000 );
  QGSCOMPARENEAR( dock.mapCanvas()->scale() * testScalingFactor, 500000, 10000 );
  QGSCOMPARENEAR( mainCanvas.scale() * testScalingFactor, 1000000, 100000 );
}

QGSTEST_MAIN( TestQgsMapCanvasDockWidget )
#include "testqgsmapcanvasdockwidget.moc"
