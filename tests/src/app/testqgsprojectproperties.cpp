/***************************************************************************
     testqgsprojectproperties.cpp
     -------------------------
    Date                 : 2018-11-21
    Copyright            : (C) 2018 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
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
#include "qgsprojectproperties.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsprojectdisplaysettings.h"
#include "qgsbearingnumericformat.h"
#include "qgsrasterlayer.h"
#include "qgsprojecttimesettings.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrasterlayertemporalproperties.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the project properties dialog
 */
class TestQgsProjectProperties : public QObject
{
    Q_OBJECT
  public:
    TestQgsProjectProperties();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testProjectPropertiesDirty();
    void testEllipsoidChange();
    void testEllipsoidCrsSync();
    void testBearingFormat();
    void testTimeSettings();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsProjectProperties::TestQgsProjectProperties() = default;

//runs before all tests
void TestQgsProjectProperties::initTestCase()
{
  qDebug() << "TestQgsProjectProperties::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsProjectProperties::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProjectProperties::testProjectPropertiesDirty()
{
  // create a temporary layer
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "none?field=code:int&field=regular:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  // add layer to project, to insure presence of layer-related project settings
  QgsProject::instance()->addMapLayer( tempLayer.get() );

  // opening the project properties for the first time in a new project does write new entries
  // call apply twice here to test that subsequent opening will not dirty project
  QgsProjectProperties *pp = new QgsProjectProperties( mQgisApp->mapCanvas() );
  pp->apply();
  delete pp;
  QgsProject::instance()->setDirty( false );
  pp = new QgsProjectProperties( mQgisApp->mapCanvas() );
  pp->apply();
  delete pp;
  QCOMPARE( QgsProject::instance()->isDirty(), false );
}

void TestQgsProjectProperties::testEllipsoidChange()
{
  QgsProject::instance()->clear();
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );

  std::unique_ptr< QgsProjectProperties > pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );

  QgsProject::instance()->setEllipsoid( QStringLiteral( "ESRI:107900" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "ESRI:107900" ) );

  QgsProject::instance()->setEllipsoid( QStringLiteral( "EPSG:7002" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "EPSG:7002" ) );

  QgsProject::instance()->setEllipsoid( QStringLiteral( "EPSG:7005" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "EPSG:7005" ) );

  QgsProject::instance()->setEllipsoid( QStringLiteral( "NONE" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );

  QgsProject::instance()->setEllipsoid( QStringLiteral( "PARAMETER:55:66" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "PARAMETER:55:66" ) );

}

void TestQgsProjectProperties::testEllipsoidCrsSync()
{
  // test logic around syncing ellipsoid choice to project CRS

  QgsProject::instance()->clear();

  // if project has a crs and ellipsoid is none, then ellipsoid should not be changed when project crs is changed
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );

  std::unique_ptr< QgsProjectProperties > pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  // ellipsoid must remain not set
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );

  // if ellipsoid is not set to none, then it should always be synced with the project crs choice
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  pp->apply();
  pp.reset();
  // ellipsoid must remain not set
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "NONE" ) );

  // but if ellipsoid is initially set, then changing the project CRS should update the ellipsoid to match
  QgsProject::instance()->setEllipsoid( QStringLiteral( "EPSG:7021" ) );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  pp->apply();
  pp.reset();
  // ellipsoid should be updated to match CRS ellipsoid
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "EPSG:7019" ) );

  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4240" ) ) );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "EPSG:7015" ) );

  // try creating a crs from a non-standard WKT string (in this case, the invalid WKT definition of EPSG:31370 used by
  // some ArcGIS versions: see https://github.com/OSGeo/PROJ/issues/1781
  const QString wkt = QStringLiteral( R"""(PROJCS["Belge 1972 / Belgian Lambert 72",GEOGCS["Belge 1972",DATUM["Reseau_National_Belge_1972",SPHEROID["International 1924",6378388,297],AUTHORITY["EPSG","6313"]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",90],PARAMETER["central_meridian",4.36748666666667],PARAMETER["standard_parallel_1",49.8333339],PARAMETER["standard_parallel_2",51.1666672333333],PARAMETER["false_easting",150000.01256],PARAMETER["false_northing",5400088.4378],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" );
  const QgsCoordinateReferenceSystem customCrs = QgsCoordinateReferenceSystem::fromWkt( wkt );
  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( customCrs );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid().left( 30 ), QStringLiteral( "PARAMETER:6378388:6356911.9461" ) );

  // ok. Next bit of logic -- if the project is initially set to NO projection and NO ellipsoid, then first setting the project CRS should set an ellipsoid to match
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem() );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "NONE" ) );

  pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  pp->apply();
  pp.reset();
  // ellipsoid should be updated to match CRS ellipsoid
  QCOMPARE( QgsProject::instance()->ellipsoid(), QStringLiteral( "EPSG:7019" ) );
}

void TestQgsProjectProperties::testBearingFormat()
{
  QgsProject::instance()->clear();
  std::unique_ptr< QgsBearingNumericFormat > format = std::make_unique< QgsBearingNumericFormat >();
  format->setNumberDecimalPlaces( 9 );
  QgsProject::instance()->displaySettings()->setBearingFormat( format.release() );

  std::unique_ptr< QgsProjectProperties > pp = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );
  pp->apply();
  QCOMPARE( QgsProject::instance()->displaySettings()->bearingFormat()->numberDecimalPlaces(), 9 );
}

void TestQgsProjectProperties::testTimeSettings()
{
  QgsProject::instance()->clear();
  const QgsDateTimeRange range = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime(), Qt::UTC ),
                                 QDateTime( QDate( 2020, 12, 31 ), QTime(), Qt::UTC ) );

  QgsProject::instance()->timeSettings()->setTemporalRange( range );
  const QgsDateTimeRange projectRange = QgsProject::instance()->timeSettings()->temporalRange();

  std::unique_ptr< QgsProjectProperties > projectProperties = std::make_unique< QgsProjectProperties >( mQgisApp->mapCanvas() );

  QCOMPARE( projectRange, range );

  // Test setting Project temporal range using temporal layers

  QgsRasterLayer *firstLayer = new QgsRasterLayer( QString(), QStringLiteral( "firstLayer" ), QStringLiteral( "wms" ) );
  QgsRasterLayer *secondLayer = new QgsRasterLayer( QString(), QStringLiteral( "secondLayer" ), QStringLiteral( "wms" ) );
  QgsRasterLayer *thirdLayer = new QgsRasterLayer( QString(), QStringLiteral( "thirdLayer" ), QStringLiteral( "wms" ) );

  const QgsDateTimeRange firstRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime(), Qt::UTC ),
                                      QDateTime( QDate( 2020, 3, 31 ), QTime(), Qt::UTC ) );
  const QgsDateTimeRange secondRange = QgsDateTimeRange( QDateTime( QDate( 2020, 4, 1 ), QTime(), Qt::UTC ),
                                       QDateTime( QDate( 2020, 7, 31 ), QTime(), Qt::UTC ) );
  const QgsDateTimeRange thirdRange = QgsDateTimeRange( QDateTime( QDate( 2019, 1, 1 ), QTime(), Qt::UTC ),
                                      QDateTime( QDate( 2020, 2, 28 ), QTime(), Qt::UTC ) );

  firstLayer->temporalProperties()->setIsActive( true );
  qobject_cast< QgsRasterLayerTemporalProperties * >( firstLayer->temporalProperties() )->setFixedTemporalRange( firstRange );
  secondLayer->temporalProperties()->setIsActive( true );
  qobject_cast< QgsRasterLayerTemporalProperties * >( secondLayer->temporalProperties() )->setFixedTemporalRange( secondRange );
  thirdLayer->temporalProperties()->setIsActive( true );
  qobject_cast< QgsRasterLayerTemporalProperties * >( thirdLayer->temporalProperties() )->setFixedTemporalRange( thirdRange );

  QgsProject::instance()->addMapLayers( { firstLayer, secondLayer, thirdLayer } );

  projectProperties->calculateFromLayersButton_clicked();
  projectProperties->apply();

  const QgsDateTimeRange expectedRange = QgsDateTimeRange( thirdRange.begin(), secondRange.end() );
  const QgsDateTimeRange secondProjectRange = QgsProject::instance()->timeSettings()->temporalRange();

  QCOMPARE( secondProjectRange, expectedRange );
}

QGSTEST_MAIN( TestQgsProjectProperties )

#include "testqgsprojectproperties.moc"
