/***************************************************************************
     testqgsmaptoolselect.cpp
     --------------------------------
    Date                 : 2016-06-23
    Copyright            : (C) 2016 by Sandro Santilli
    Email                : strk@kbt.io
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"
#include "qgsmaptoolselect.h"
#include "qgsmaptoolselectutils.h"

#include "cpl_conv.h"

class TestQgsMapToolSelect : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolSelect()
        : canvas( 0 )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void selectInvalidPolygons(); // test selecting invalid polygons

  private:
    QgsMapCanvas* canvas;

    QgsFeatureList testSelectVector( QgsVectorLayer* layer, double xGeoref, double yGeoref );

    // Release return with delete []
    unsigned char *
    hex2bytes( const char *hex, int *size )
    {
      QByteArray ba = QByteArray::fromHex( hex );
      unsigned char *out = new unsigned char[ba.size()];
      memcpy( out, ba.data(), ba.size() );
      *size = ba.size();
      return out;
    }

    // TODO: make this a QgsGeometry member...
    QgsGeometry geomFromHexWKB( const char *hexwkb )
    {
      int wkbsize;
      unsigned char *wkb = hex2bytes( hexwkb, &wkbsize );
      QgsGeometry geom;
      // NOTE: QgsGeometry takes ownership of wkb
      geom.fromWkb( wkb, wkbsize );
      return geom;
    }

};

void TestQgsMapToolSelect::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  QgsApplication::showSettings();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );
}

void TestQgsMapToolSelect::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolSelect::init()
{
  canvas = new QgsMapCanvas();
}

void TestQgsMapToolSelect::cleanup()
{
  delete canvas;
}

// private
QgsFeatureList
TestQgsMapToolSelect::testSelectVector( QgsVectorLayer* layer, double xGeoref, double yGeoref )
{
  QScopedPointer< QgsMapToolSelect > tool( new QgsMapToolSelect( canvas ) );
  QgsPoint mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );

  // make given vector layer current
  canvas->setCurrentLayer( layer );

  QScopedPointer< QgsMapMouseEvent > event( new QgsMapMouseEvent(
        canvas,
        QEvent::MouseButtonRelease,
        QPoint( mapPoint.x(), mapPoint.y() )
      ) );

  // trigger mouseRelease handler
  tool->canvasReleaseEvent( event.data() );

  // return selected features
  return layer->selectedFeatures();
}

void TestQgsMapToolSelect::selectInvalidPolygons()
{
  //create a temporary layer
  QScopedPointer< QgsVectorLayer > memoryLayer( new QgsVectorLayer( "Polygon?field=pk:int", "vl", "memory" ) );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature f1( memoryLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( "pk", 1 );
  // This geometry is an invalid polygon (3 distinct vertices).
  // GEOS reported invalidity: Points of LinearRing do not form a closed linestring
  f1.setGeometry( geomFromHexWKB(
                    "010300000001000000030000000000000000000000000000000000000000000000000024400000000000000000000000000000244000000000000024400000000000000000"
                  ) );
  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  canvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  QgsFeatureList selected;
  selected = testSelectVector( memoryLayer.data(), 4, 6 );
  QCOMPARE( selected.length(), 0 );
  selected = testSelectVector( memoryLayer.data(), 6, 4 );
  QCOMPARE( selected.length(), 1 );
  QCOMPARE( selected[0].attribute( "pk" ), QVariant( 1 ) );

}


QTEST_MAIN( TestQgsMapToolSelect )
#include "testqgsmaptoolselect.moc"
