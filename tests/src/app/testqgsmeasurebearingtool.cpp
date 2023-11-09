/***************************************************************************
     testqgsmeasurebearingtool.cpp
     ----------------------
    Date                 : 2021-06-15
    Copyright            : (C) 2021 by Nyall Dawson
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
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsmeasuretool.h"
#include "qgsmeasuredialog.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"
#include "qgsmaptoolmeasurebearing.h"
#include "testqgsmaptoolutils.h"
#include "qgsdisplayangle.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the measure bearing tool
 */
class TestQgsMeasureBearingTool : public QObject
{
    Q_OBJECT
  public:
    TestQgsMeasureBearingTool();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testBearingCartesian();
    void testBearingEllipsoid();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
};

TestQgsMeasureBearingTool::TestQgsMeasureBearingTool() = default;

//runs before all tests
void TestQgsMeasureBearingTool::initTestCase()
{
  qDebug() << "TestQgsMeasureBearingTool::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mQgisApp = new QgisApp();
  mCanvas = new QgsMapCanvas();

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 50, 50 );
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mCanvas->setExtent( QgsRectangle( -11554312, -5082786, 15123372, 11957046 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );
}

//runs after all tests
void TestQgsMeasureBearingTool::cleanupTestCase()
{
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMeasureBearingTool::testBearingCartesian()
{
  // set project CRS and set ellipsoid to none, so that Cartesian calculations are performed
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3857" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QString() );

  QgsMapToolMeasureBearing *mapTool = new QgsMapToolMeasureBearing( mCanvas ) ;
  mCanvas->setMapTool( mapTool );

  QVERIFY( !mapTool->mResultDisplay );

  TestQgsMapToolUtils tools( mapTool );

  tools.mouseMove( 8749058, 1916460 );
  tools.mouseClick( 8749058, 1916460, Qt::LeftButton );
  tools.mouseMove( 14498439, -2694154 );

  QVERIFY( mapTool->mResultDisplay );
  QGSCOMPARENEAR( mapTool->mResultDisplay->value(), 2.1995926132, 0.001 );
  QCOMPARE( mapTool->mResultDisplay->text(), QStringLiteral( "126.027373\u00B0E" ) );

  tools.mouseClick( 14498439, -2694154, Qt::LeftButton );
  QGSCOMPARENEAR( mapTool->mResultDisplay->value(), 2.1995926132, 0.001 );
  QCOMPARE( mapTool->mResultDisplay->text(), QStringLiteral( "126.027373\u00B0E" ) );

  tools.mouseMove( 555496, 3291312 );
  tools.mouseClick( 555496, 3291312, Qt::LeftButton );
  tools.mouseMove( -611045, 5082786 );
  tools.mouseClick( -611045, 5082786, Qt::LeftButton );
  QGSCOMPARENEAR( mapTool->mResultDisplay->value(), -0.5880026035, 0.001 );
  QCOMPARE( mapTool->mResultDisplay->text(), QStringLiteral( "33.690068\u00B0W" ) );
}

void TestQgsMeasureBearingTool::testBearingEllipsoid()
{
  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3857" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "EPSG:7030" ) );

  QgsMapToolMeasureBearing *mapTool = new QgsMapToolMeasureBearing( mCanvas ) ;
  mCanvas->setMapTool( mapTool );

  QVERIFY( !mapTool->mResultDisplay );

  TestQgsMapToolUtils tools( mapTool );

  tools.mouseMove( 8749058, 1916460 );
  tools.mouseClick( 8749058, 1916460, Qt::LeftButton );
  tools.mouseMove( 14498439, -2694154 );

  QVERIFY( mapTool->mResultDisplay );
  QGSCOMPARENEAR( mapTool->mResultDisplay->value(), 2.1679949043, 0.001 );
  QCOMPARE( mapTool->mResultDisplay->text(), QStringLiteral( "124.216958\u00B0E" ) );

  tools.mouseClick( 14498439, -2694154, Qt::LeftButton );
  QGSCOMPARENEAR( mapTool->mResultDisplay->value(), 2.1679949043, 0.001 );
  QCOMPARE( mapTool->mResultDisplay->text(), QStringLiteral( "124.216958\u00B0E" ) );

  tools.mouseMove( 555496, 3291312 );
  tools.mouseClick( 555496, 3291312, Qt::LeftButton );
  tools.mouseMove( -611045, 5082786 );
  tools.mouseClick( -611045, 5082786, Qt::LeftButton );
  QGSCOMPARENEAR( mapTool->mResultDisplay->value(), -0.5448498177, 0.001 );
  QCOMPARE( mapTool->mResultDisplay->text(), QStringLiteral( "31.217595\u00B0W" ) );
}

QGSTEST_MAIN( TestQgsMeasureBearingTool )
#include "testqgsmeasurebearingtool.moc"
