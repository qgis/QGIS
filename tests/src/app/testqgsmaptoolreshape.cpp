/***************************************************************************
     testqgsmaptoolreshape.cpp
     --------------------------------
    Date                 : 2017-12-1
    Copyright            : (C) 2017 by Paul Blottiere
    Email                : paul.blottiere@oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgslinestring.h"
#include "qgsmaptoolreshape.h"
#include "qgisapp.h"

class TestQgsMapToolReshape : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolReshape() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void reshapeWithBindingLine();

  private:
    QgisApp *mQgisApp = nullptr;
};

void TestQgsMapToolReshape::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::showSettings();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );

  mQgisApp = new QgisApp();
}

void TestQgsMapToolReshape::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolReshape::init()
{
}

void TestQgsMapToolReshape::cleanup()
{
}

void TestQgsMapToolReshape::reshapeWithBindingLine()
{
  // prepare vector layer
  std::unique_ptr<QgsVectorLayer> vl;
  vl.reset( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=name:string(20)" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );

  QgsGeometry g0 = QgsGeometry::fromWkt( "LineString (0 0, 1 1, 1 2)" );
  QgsFeature f0;
  f0.setGeometry( g0 );
  f0.setAttribute( 0, "polyline0" );

  QgsGeometry g1 = QgsGeometry::fromWkt( "LineString (2 1, 3 2, 3 3, 2 2)" );
  QgsFeature f1;
  f1.setGeometry( g1 );
  f1.setAttribute( 0, "polyline1" );

  vl->dataProvider()->addFeatures( QgsFeatureList() << f0 << f1 );

  // prepare canvas
  QList<QgsMapLayer *> layers;
  layers.append( vl.get() );

  QgsCoordinateReferenceSystem srs( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  mQgisApp->mapCanvas()->setDestinationCrs( srs );
  mQgisApp->mapCanvas()->setLayers( layers );
  mQgisApp->mapCanvas()->setCurrentLayer( vl.get() );

  // reshape to add line to polyline0
  QgsLineString cl0;
  cl0.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 1 ) );

  QgsCompoundCurve curve0( *cl0.toCurveType() );

  QgsMapToolReshape tool0( mQgisApp->mapCanvas() );
  tool0.mCaptureCurve = curve0;

  vl->startEditing();
  tool0.reshape( vl.get() );

  f0 = vl->getFeature( 1 );
  QCOMPARE( f0.geometry().asWkt(), QStringLiteral( "LineString (0 0, 1 1, 1 2, 2 1)" ) );

  f1 = vl->getFeature( 2 );
  QCOMPARE( f1.geometry().asWkt(), QStringLiteral( "LineString (2 1, 3 2, 3 3, 2 2)" ) );

  vl->rollBack();

  // reshape to add line to polyline1
  QgsLineString cl1;
  cl1.setPoints( QgsPointSequence() << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) );

  QgsCompoundCurve curve1( *cl1.toCurveType() );

  QgsMapToolReshape tool1( mQgisApp->mapCanvas() );
  tool1.mCaptureCurve = curve1;

  vl->startEditing();
  tool1.reshape( vl.get() );

  f0 = vl->getFeature( 1 );
  QCOMPARE( f0.geometry().asWkt(), QStringLiteral( "LineString (0 0, 1 1, 1 2)" ) );

  f1 = vl->getFeature( 2 );
  QCOMPARE( f1.geometry().asWkt(), QStringLiteral( "LineString (1 2, 2 1, 3 2, 3 3, 2 2)" ) );

  vl->rollBack();
}

QGSTEST_MAIN( TestQgsMapToolReshape )
#include "testqgsmaptoolreshape.moc"
