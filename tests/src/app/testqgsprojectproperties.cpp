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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsbearingnumericformat.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsproject.h"
#include "qgsprojectdisplaysettings.h"
#include "qgsprojectproperties.h"
#include "qgsprojectstylesettings.h"
#include "qgsprojecttimesettings.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

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
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testProjectPropertiesDirty();
    void testEllipsoidChange();
    void testEllipsoidCrsSync();
    void testBearingFormat();
    void testTimeSettings();
    void testColorSettings();

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
  auto tempLayer = std::make_unique<QgsVectorLayer>( u"none?field=code:int&field=regular:string"_s, u"vl"_s, u"memory"_s );
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
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );

  auto pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );

  QgsProject::instance()->setEllipsoid( u"ESRI:107900"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"ESRI:107900"_s );

  QgsProject::instance()->setEllipsoid( u"EPSG:7002"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"EPSG:7002"_s );

  QgsProject::instance()->setEllipsoid( u"EPSG:7005"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"EPSG:7005"_s );

  QgsProject::instance()->setEllipsoid( u"NONE"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );

  QgsProject::instance()->setEllipsoid( u"PARAMETER:55:66"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"PARAMETER:55:66"_s );
}

void TestQgsProjectProperties::testEllipsoidCrsSync()
{
  // test logic around syncing ellipsoid choice to project CRS

  QgsProject::instance()->clear();

  // if project has a crs and ellipsoid is none, then ellipsoid should not be changed when project crs is changed
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );

  auto pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->crs().authid(), u"EPSG:3111"_s );
  // ellipsoid must remain not set
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );

  // if ellipsoid is not set to none, then it should always be synced with the project crs choice
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  pp->apply();
  pp.reset();
  // ellipsoid must remain not set
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );

  // but if ellipsoid is initially set, then changing the project CRS should update the ellipsoid to match
  QgsProject::instance()->setEllipsoid( u"EPSG:7021"_s );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  pp->apply();
  pp.reset();
  // ellipsoid should be updated to match CRS ellipsoid
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"EPSG:7019"_s );

  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( u"EPSG:4240"_s ) );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"EPSG:7015"_s );

  // try creating a crs from a non-standard WKT string (in this case, the invalid WKT definition of EPSG:31370 used by
  // some ArcGIS versions: see https://github.com/OSGeo/PROJ/issues/1781
  const QString wkt = QStringLiteral( R"""(PROJCS["Belge 1972 / Belgian Lambert 72",GEOGCS["Belge 1972",DATUM["Reseau_National_Belge_1972",SPHEROID["International 1924",6378388,297],AUTHORITY["EPSG","6313"]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",90],PARAMETER["central_meridian",4.36748666666667],PARAMETER["standard_parallel_1",49.8333339],PARAMETER["standard_parallel_2",51.1666672333333],PARAMETER["false_easting",150000.01256],PARAMETER["false_northing",5400088.4378],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" );
  const QgsCoordinateReferenceSystem customCrs = QgsCoordinateReferenceSystem::fromWkt( wkt );
  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( customCrs );
  pp->apply();
  pp.reset();
  QCOMPARE( QgsProject::instance()->ellipsoid().left( 30 ), u"PARAMETER:6378388:6356911.9461"_s );

  // ok. Next bit of logic -- if the project is initially set to NO projection and NO ellipsoid, then first setting the project CRS should set an ellipsoid to match
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem() );
  QgsProject::instance()->setEllipsoid( u"NONE"_s );

  pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->setSelectedCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) );
  pp->apply();
  pp.reset();
  // ellipsoid should be updated to match CRS ellipsoid
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"EPSG:7019"_s );
}

void TestQgsProjectProperties::testBearingFormat()
{
  QgsProject::instance()->clear();
  auto format = std::make_unique<QgsBearingNumericFormat>();
  format->setNumberDecimalPlaces( 9 );
  QgsProject::instance()->displaySettings()->setBearingFormat( format.release() );

  auto pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  pp->apply();
  QCOMPARE( QgsProject::instance()->displaySettings()->bearingFormat()->numberDecimalPlaces(), 9 );
}

void TestQgsProjectProperties::testTimeSettings()
{
  QgsProject::instance()->clear();
  const QgsDateTimeRange range = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime(), Qt::UTC ), QDateTime( QDate( 2020, 12, 31 ), QTime(), Qt::UTC ) );

  QgsProject::instance()->timeSettings()->setTemporalRange( range );
  const QgsDateTimeRange projectRange = QgsProject::instance()->timeSettings()->temporalRange();

  auto projectProperties = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );

  QCOMPARE( projectRange, range );

  // Test setting Project temporal range using temporal layers

  QgsRasterLayer *firstLayer = new QgsRasterLayer( QString(), u"firstLayer"_s, u"wms"_s );
  QgsRasterLayer *secondLayer = new QgsRasterLayer( QString(), u"secondLayer"_s, u"wms"_s );
  QgsRasterLayer *thirdLayer = new QgsRasterLayer( QString(), u"thirdLayer"_s, u"wms"_s );

  const QgsDateTimeRange firstRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime(), Qt::UTC ), QDateTime( QDate( 2020, 3, 31 ), QTime(), Qt::UTC ) );
  const QgsDateTimeRange secondRange = QgsDateTimeRange( QDateTime( QDate( 2020, 4, 1 ), QTime(), Qt::UTC ), QDateTime( QDate( 2020, 7, 31 ), QTime(), Qt::UTC ) );
  const QgsDateTimeRange thirdRange = QgsDateTimeRange( QDateTime( QDate( 2019, 1, 1 ), QTime(), Qt::UTC ), QDateTime( QDate( 2020, 2, 28 ), QTime(), Qt::UTC ) );

  firstLayer->temporalProperties()->setIsActive( true );
  qobject_cast<QgsRasterLayerTemporalProperties *>( firstLayer->temporalProperties() )->setFixedTemporalRange( firstRange );
  secondLayer->temporalProperties()->setIsActive( true );
  qobject_cast<QgsRasterLayerTemporalProperties *>( secondLayer->temporalProperties() )->setFixedTemporalRange( secondRange );
  thirdLayer->temporalProperties()->setIsActive( true );
  qobject_cast<QgsRasterLayerTemporalProperties *>( thirdLayer->temporalProperties() )->setFixedTemporalRange( thirdRange );

  QgsProject::instance()->addMapLayers( { firstLayer, secondLayer, thirdLayer } );

  projectProperties->calculateFromLayersButton_clicked();
  projectProperties->apply();

  const QgsDateTimeRange expectedRange = QgsDateTimeRange( thirdRange.begin(), secondRange.end() );
  const QgsDateTimeRange secondProjectRange = QgsProject::instance()->timeSettings()->temporalRange();

  QCOMPARE( secondProjectRange, expectedRange );
}

void TestQgsProjectProperties::testColorSettings()
{
  QgsProject::instance()->clear();
  QCOMPARE( QgsProject::instance()->styleSettings()->colorModel(), Qgis::ColorModel::Rgb );
  QVERIFY( !QgsProject::instance()->styleSettings()->colorSpace().isValid() );

  auto pp = std::make_unique<QgsProjectProperties>( mQgisApp->mapCanvas() );
  QCOMPARE( static_cast<Qgis::ColorModel>( pp->mColorModel->currentData().toInt() ), Qgis::ColorModel::Rgb );
  QVERIFY( !pp->mColorSpace.isValid() );
#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )
  QCOMPARE( pp->mColorSpaceName->text(), u"<i>None</i>"_s );
  QVERIFY( !pp->mRemoveIccProfile->isEnabled() );
#else
  QVERIFY( !pp->mRemoveIccProfile->isVisible() );
  QVERIFY( !pp->mAddIccProfile->isVisible() );
  QVERIFY( !pp->mSaveIccProfile->isVisible() );
  QVERIFY( !pp->mColorSpaceName->isVisible() );
  QVERIFY( !pp->mIccProfileLabel->isVisible() );
#endif

  pp->mColorModel->setCurrentIndex( pp->mColorModel->findData( QVariant::fromValue( Qgis::ColorModel::Cmyk ) ) );
  QCOMPARE( static_cast<Qgis::ColorModel>( pp->mColorModel->currentData().toInt() ), Qgis::ColorModel::Cmyk );

#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )

  const QString iccProfileFilePath = QStringLiteral( TEST_DATA_DIR ) + "/sRGB2014.icc";
  pp->addIccProfile( iccProfileFilePath );
  QCOMPARE( pp->mColorSpaceName->text(), u"sRGB2014"_s );
  QVERIFY( pp->mRemoveIccProfile->isEnabled() );
  QVERIFY( !pp->mColorModel->isEnabled() );
  QCOMPARE( static_cast<Qgis::ColorModel>( pp->mColorModel->currentData().toInt() ), Qgis::ColorModel::Rgb );

  pp->apply();
  QCOMPARE( QgsProject::instance()->styleSettings()->colorModel(), Qgis::ColorModel::Rgb );
  QVERIFY( QgsProject::instance()->styleSettings()->colorSpace().isValid() );
  QCOMPARE( QgsProject::instance()->styleSettings()->colorSpace().description(), u"sRGB2014"_s );

  pp->removeIccProfile();
  QVERIFY( !pp->mColorSpace.isValid() );
  QCOMPARE( pp->mColorSpaceName->text(), u"<i>None</i>"_s );
  QVERIFY( !pp->mRemoveIccProfile->isEnabled() );

#endif
}


QGSTEST_MAIN( TestQgsProjectProperties )

#include "testqgsprojectproperties.moc"
