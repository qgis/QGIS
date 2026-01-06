/***************************************************************************
    testqgsgpsintegration.cpp
     --------------------------
    Date                 : 2019-06-19
    Copyright            : (C) 2019 by Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "gps/qgsappgpsdigitizing.h"
#include "gps/qgsappgpssettingsmenu.h"
#include "options/qgsgpsoptions.h"
#include "qgisapp.h"
#include "qgsappgpsconnection.h"
#include "qgsapplication.h"
#include "qgsgpsconnection.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsprojectgpssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QSignalSpy>
#include <QTimeZone>

/**
 * \ingroup UnitTests
 * This is a unit test for the GPS integration
 */
class TestQgsGpsIntegration : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsGpsIntegration()
      : QgsTest( u"GPS Integration Tests"_s ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testTimeStampFields();
    void testGpsOptionsTimeZoneWidgets();
    void testStorePreferredFields();
    void testTimestamp();
    void testTimestampWrite();
    void testMultiPartLayers();
    void testFollowActiveLayer();
    void testTrackDistance();

  private:
    QDateTime _testWrite( QgsVectorLayer *vlayer, QgsAppGpsDigitizing &gpsDigitizing, const QString &fieldName, Qt::TimeSpec timeSpec, bool commit = false );
    QgsVectorLayer *tempLayer = nullptr;
    QgsVectorLayer *tempLayerString = nullptr;
    QgsVectorLayer *tempLayerDateTime = nullptr;
    QgsVectorLayer *tempLayerLineString = nullptr;
    QgsVectorLayer *tempGpkgLayerPointString = nullptr;
    QgisApp *mQgisApp = nullptr;
};

void TestQgsGpsIntegration::initTestCase()
{
  // setup the test QSettings environment

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QCoreApplication::setOrganizationName( u"QGISGpsTests"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
  QgsSettings().clear();

  mQgisApp = new QgisApp();


  tempLayer = new QgsVectorLayer( u"Point?crs=epsg:4326&field=intf:int"_s, u"vl1"_s, u"memory"_s );
  tempLayerString = new QgsVectorLayer( u"Point?crs=epsg:4326&field=stringf:string&field=intf:int"_s, u"vl2"_s, u"memory"_s );
  tempLayerDateTime = new QgsVectorLayer( u"Point?crs=epsg:4326&field=datetimef:datetime&field=intf:int"_s, u"vl3"_s, u"memory"_s );
  tempLayerLineString = new QgsVectorLayer( u"Linestring?crs=epsg:4326&field=intf:int&field=stringf:string"_s, u"vl4"_s, u"memory"_s );

  QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog->setValue( true );

  const QString tempPath = QDir::tempPath() + u"/gps_timestamp.gpkg"_s;
  QFile::copy( TEST_DATA_DIR + u"/gps_timestamp.gpkg"_s, tempPath );
  tempGpkgLayerPointString = new QgsVectorLayer( u"%1|layername=points"_s.arg( tempPath ), u"vl4"_s );
  Q_ASSERT( tempGpkgLayerPointString->isValid() );
  Q_ASSERT( tempLayer->isValid() );
  Q_ASSERT( tempLayerString->isValid() );
  Q_ASSERT( tempLayerDateTime->isValid() );
  Q_ASSERT( tempLayerLineString->isValid() );
  QgsProject::instance()->addMapLayers( { tempLayer, tempLayerString, tempLayerDateTime, tempGpkgLayerPointString, tempLayerLineString } );
}

void TestQgsGpsIntegration::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QDateTime TestQgsGpsIntegration::_testWrite( QgsVectorLayer *vlayer, QgsAppGpsDigitizing &gpsDigitizing, const QString &fieldName, Qt::TimeSpec timeSpec, bool commit )
{
  mQgisApp->setActiveLayer( vlayer );
  vlayer->startEditing();
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( vlayer, fieldName );
  gpsDigitizing.mTimeStampSpec = timeSpec;
  gpsDigitizing.createFeature();
  const auto fids { vlayer->allFeatureIds() };
  const auto fid { std::min_element( fids.begin(), fids.end() ) };
  const QgsFeature f { vlayer->getFeature( *fid ) };
  if ( commit )
    vlayer->commitChanges();
  else
    vlayer->rollBack();
  return f.attribute( fieldName ).toDateTime();
}

void TestQgsGpsIntegration::testTimeStampFields()
{
  QgsAppGpsSettingsMenu settingsMenu( nullptr );
  QgsAppGpsConnection connection( nullptr );
  QgsAppGpsDigitizing gpsDigitizing( &connection, mQgisApp->mapCanvas() );

  mQgisApp->setActiveLayer( tempLayer );
  // allow menu to populate
  settingsMenu.timeStampMenuAboutToShow();
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().count(), 1 );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->text(), u"Do Not Store"_s );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->data().toString(), QString() );
  QVERIFY( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->isChecked() );

  mQgisApp->setActiveLayer( tempLayerString );
  settingsMenu.timeStampMenuAboutToShow();
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().count(), 2 );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->text(), u"Do Not Store"_s );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->data().toString(), QString() );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->text(), u"stringf"_s );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->data().toString(), u"stringf"_s );
  QVERIFY( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->isChecked() );
  settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->trigger();

  mQgisApp->setActiveLayer( tempLayerDateTime );
  settingsMenu.timeStampMenuAboutToShow();
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().count(), 2 );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->text(), u"Do Not Store"_s );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->data().toString(), QString() );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->text(), u"datetimef"_s );
  QCOMPARE( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->data().toString(), u"datetimef"_s );
  QVERIFY( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->isChecked() );
  settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->trigger();

  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( QgsProject::instance()->gpsSettings()->destinationLayer(), QString() );
  settingsMenu.timeStampMenuAboutToShow();
  QVERIFY( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 0 )->isChecked() );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( QgsProject::instance()->gpsSettings()->destinationLayer(), u"datetimef"_s );
  settingsMenu.timeStampMenuAboutToShow();
  QVERIFY( settingsMenu.mTimeStampDestinationFieldMenu->actions().at( 1 )->isChecked() );
}

void TestQgsGpsIntegration::testGpsOptionsTimeZoneWidgets()
{
  // Check tz combo
  QgsGpsOptionsWidget widget( nullptr );
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  QVERIFY( !widget.mCboTimeZones->isEnabled() );
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::LocalTime ) );
  QVERIFY( !widget.mCboTimeZones->isEnabled() );
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::TimeZone ) );
  QVERIFY( widget.mCboTimeZones->isEnabled() );
}

void TestQgsGpsIntegration::testStorePreferredFields()
{
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsAppGpsConnection connection( nullptr );

  QgsAppGpsSettingsMenu menu( nullptr );
  QgsAppGpsDigitizing gpsDigitizing( &connection, canvas );

  mQgisApp->setActiveLayer( tempLayerString );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( tempLayerString, u"stringf"_s );
  mQgisApp->setActiveLayer( tempLayerDateTime );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( tempLayerDateTime, u"datetimef"_s );
  mQgisApp->setActiveLayer( tempLayerLineString );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( tempLayerLineString, u"stringf"_s );
  mQgisApp->setActiveLayer( tempGpkgLayerPointString );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( tempGpkgLayerPointString, u"datetimef"_s );

  mQgisApp->setActiveLayer( tempLayer );
  QMap<QString, QString> preferredTimeStamps = QgsProject::instance()->gpsSettings()->destinationTimeStampFields();
  QVERIFY( preferredTimeStamps.contains( tempLayerString->id() ) );
  QCOMPARE( preferredTimeStamps[tempLayerString->id()], u"stringf"_s );
  QVERIFY( preferredTimeStamps.contains( tempLayerDateTime->id() ) );
  QCOMPARE( preferredTimeStamps[tempLayerDateTime->id()], u"datetimef"_s );
}

void TestQgsGpsIntegration::testTimestamp()
{
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsAppGpsConnection connection( nullptr );
  QgsAppGpsDigitizing gpsDigitizing( &connection, canvas );

  // Widget config and input values
  // 2019/06/19 12:27:34.543[UTC]
  gpsDigitizing.mLastTime = QDateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ), Qt::UTC );
  gpsDigitizing.mApplyLeapSettings = false;
  gpsDigitizing.mLeapSeconds = 7;
  gpsDigitizing.mOffsetFromUtc = -36000;

  QDateTime dateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ) );
  dateTime.setTimeSpec( Qt::TimeSpec::UTC );
  const QDateTime tzTime( dateTime.toTimeZone( QTimeZone( u"Asia/Colombo"_s.toUtf8() ) ) );
  const QDateTime localTime( dateTime.toLocalTime() );

  ///////////////////////////////////////////
  // Test datetime layer
  mQgisApp->setActiveLayer( tempLayerDateTime );
  int fieldIdx = tempLayerDateTime->fields().indexOf( "datetimef"_L1 );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( tempLayerDateTime, u"datetimef"_s );

  // UTC
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::UTC;
  QVariant dt = gpsDigitizing.timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime(), dateTime );

  // offset from UTC
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::OffsetFromUTC;
  dt = gpsDigitizing.timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime().offsetFromUtc(), -36000 );
  QDateTime expected = dateTime.addSecs( -36000 );
  expected.setOffsetFromUtc( -36000 );
  QCOMPARE( dt.toDateTime(), expected );

  // Local time
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::LocalTime;
  dt = gpsDigitizing.timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime(), dateTime.toLocalTime() );

  // Leap seconds
  gpsDigitizing.mApplyLeapSettings = true;
  dt = gpsDigitizing.timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime(), dateTime.addSecs( 7 ) );
  gpsDigitizing.mApplyLeapSettings = false;

  ///////////////////////////////////////////
  // Test string
  mQgisApp->setActiveLayer( tempLayerString );
  fieldIdx = tempLayerString->fields().indexOf( "stringf"_L1 );
  QgsProject::instance()->gpsSettings()->setDestinationTimeStampField( tempLayerString, u"stringf"_s );

  // UTC
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::UTC;
  dt = gpsDigitizing.timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), dateTime.toString( Qt::DateFormat::ISODate ) );

  // offset from UTC
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::OffsetFromUTC;
  dt = gpsDigitizing.timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toString(), u"2019-06-19T02:27:34.543-10:00"_s );

  // Local Time (not very robust because we cannot change the system timezone and it may be GMT)
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::LocalTime;
  dt = gpsDigitizing.timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), localTime.toString( Qt::DateFormat::ISODate ) );

  // Timezone
  gpsDigitizing.mTimeStampSpec = Qt::TimeSpec::TimeZone;
  gpsDigitizing.mTimeZone = u"Asia/Colombo"_s;
  dt = gpsDigitizing.timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), tzTime.toString( Qt::DateFormat::ISODate ) );
}

void TestQgsGpsIntegration::testTimestampWrite()
{
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsAppGpsConnection connection( nullptr );
  QgsAppGpsDigitizing gpsDigitizing( &connection, canvas );

  // Widget config and input values
  // 2019/06/19 12:27:34.543[UTC]
  gpsDigitizing.mLastTime = QDateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ), Qt::UTC );
  gpsDigitizing.mApplyLeapSettings = false;
  gpsDigitizing.mLeapSeconds = 7;
  gpsDigitizing.mTimeZone = u"Asia/Colombo"_s;
  gpsDigitizing.mOffsetFromUtc = -36000;

  QDateTime dateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ) );
  dateTime.setTimeSpec( Qt::TimeSpec::UTC );
  const QDateTime tzTime( dateTime.toTimeZone( QTimeZone( u"Asia/Colombo"_s.toUtf8() ) ) );
  const QDateTime localTime( dateTime.toLocalTime() );
  const QDateTime offsetFromUtcTime = dateTime.toOffsetFromUtc( -36000 );

  // Test write on datetime field
  QCOMPARE( _testWrite( tempLayerDateTime, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::UTC ), dateTime );
  QCOMPARE( _testWrite( tempLayerDateTime, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::OffsetFromUTC ), offsetFromUtcTime );
  QCOMPARE( _testWrite( tempLayerDateTime, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::LocalTime ), localTime );
  QCOMPARE( _testWrite( tempLayerDateTime, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::TimeZone ), tzTime );

  // Test write on string field
  QCOMPARE( _testWrite( tempLayerString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::UTC ).toString( Qt::DateFormat::ISODate ), dateTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempLayerString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::OffsetFromUTC ).toString( Qt::DateFormat::ISODate ), offsetFromUtcTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempLayerString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::LocalTime ).toString( Qt::DateFormat::ISODate ), localTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempLayerString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::TimeZone ).toString( Qt::DateFormat::ISODate ), tzTime.toString( Qt::DateFormat::ISODate ) );

  // Test write on line string field
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 1, 2 ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::UTC ).toString( Qt::DateFormat::ISODate ), dateTime.toString( Qt::DateFormat::ISODate ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 1, 2 ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::OffsetFromUTC ).toString( Qt::DateFormat::ISODate ), offsetFromUtcTime.toString( Qt::DateFormat::ISODate ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 1, 2 ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::LocalTime ).toString( Qt::DateFormat::ISODate ), localTime.toString( Qt::DateFormat::ISODate ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 1, 2 ) );
  gpsDigitizing.mCaptureListWgs84.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::TimeZone ).toString( Qt::DateFormat::ISODate ), tzTime.toString( Qt::DateFormat::ISODate ) );

  // Write on GPKG
  // Test write on datetime field
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::UTC, true ), dateTime );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::OffsetFromUTC, true ), offsetFromUtcTime );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::LocalTime, true ), localTime );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"datetimef"_s, Qt::TimeSpec::TimeZone, true ), tzTime );

  // Test write on string field
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::UTC, true ).toString( Qt::DateFormat::ISODate ), dateTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::OffsetFromUTC, true ).toString( Qt::DateFormat::ISODate ), offsetFromUtcTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::LocalTime, true ).toString( Qt::DateFormat::ISODate ), localTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, gpsDigitizing, u"stringf"_s, Qt::TimeSpec::TimeZone, true ).toString( Qt::DateFormat::ISODate ), tzTime.toString( Qt::DateFormat::ISODate ) );
}

void TestQgsGpsIntegration::testMultiPartLayers()
{
  QgsVectorLayer *multiLineString = new QgsVectorLayer( u"MultiLinestring?crs=epsg:4326&field=intf:int&field=stringf:string"_s, u"vl4"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( multiLineString );

  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsAppGpsConnection connection( nullptr );

  QgsAppGpsDigitizing gpsDigitizing( &connection, canvas );
  mQgisApp->setActiveLayer( multiLineString );
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), multiLineString );
  multiLineString->startEditing();

  // not possible, no points
  gpsDigitizing.createFeature();
  QCOMPARE( multiLineString->featureCount(), 0L );
  // need at least 2 points
  gpsDigitizing.createVertexAtCurrentLocation();
  gpsDigitizing.createFeature();
  QCOMPARE( multiLineString->featureCount(), 0L );

  gpsDigitizing.createVertexAtCurrentLocation();
  gpsDigitizing.createFeature();
  QCOMPARE( multiLineString->featureCount(), 1L );
  QgsFeature f;
  QVERIFY( multiLineString->getFeatures().nextFeature( f ) );
  QCOMPARE( f.geometry().wkbType(), Qgis::WkbType::MultiLineString );
  multiLineString->rollBack();

  // multipolygon
  QgsVectorLayer *multiPolygon = new QgsVectorLayer( u"MultiPolygon?crs=epsg:4326&field=intf:int&field=stringf:string"_s, u"vl4"_s, u"memory"_s );

  QgsProject::instance()->addMapLayer( multiPolygon );
  mQgisApp->setActiveLayer( multiPolygon );
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), multiPolygon );

  multiPolygon->startEditing();

  // not possible, no points
  gpsDigitizing.createFeature();
  QCOMPARE( multiPolygon->featureCount(), 0L );

  // need at least 3 points
  gpsDigitizing.createVertexAtCurrentLocation();
  gpsDigitizing.createFeature();
  QCOMPARE( multiPolygon->featureCount(), 0L );
  gpsDigitizing.createVertexAtCurrentLocation();
  gpsDigitizing.createFeature();
  QCOMPARE( multiPolygon->featureCount(), 0L );

  gpsDigitizing.createVertexAtCurrentLocation();
  gpsDigitizing.createFeature();
  QCOMPARE( multiPolygon->featureCount(), 1L );
  QVERIFY( multiPolygon->getFeatures().nextFeature( f ) );
  QCOMPARE( f.geometry().wkbType(), Qgis::WkbType::MultiPolygon );
  multiPolygon->rollBack();
}

void TestQgsGpsIntegration::testFollowActiveLayer()
{
  QgsProject::instance()->gpsSettings()->setDestinationFollowsActiveLayer( true );

  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsAppGpsConnection connection( nullptr );

  QgsAppGpsSettingsMenu menu( nullptr );
  QgsAppGpsDigitizing gpsDigitizing( &connection, canvas );

  mQgisApp->setActiveLayer( tempLayerString );
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), tempLayerString );

  mQgisApp->setActiveLayer( tempLayerDateTime );
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), tempLayerDateTime );

  // disable auto follow active layer
  QgsProject::instance()->gpsSettings()->setDestinationFollowsActiveLayer( false );

  QgsProject::instance()->gpsSettings()->setDestinationLayer( tempLayerLineString );
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), tempLayerLineString );

  mQgisApp->setActiveLayer( tempLayerString );
  //should be unchanged
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), tempLayerLineString );
}

void TestQgsGpsIntegration::testTrackDistance()
{
  QgsVectorLayer *lineString = new QgsVectorLayer( u"Linestring?crs=epsg:4326&field=intf:int&field=stringf:string"_s, u"vl"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( lineString );

  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsAppGpsConnection connection( nullptr );

  QgsAppGpsDigitizing gpsDigitizing( &connection, canvas );
  QgsProject::instance()->gpsSettings()->setDestinationFollowsActiveLayer( true );
  mQgisApp->setActiveLayer( lineString );
  QCOMPARE( QgsProject::instance()->gpsSettings()->destinationLayer(), lineString );
  lineString->startEditing();

  QSignalSpy spy( &gpsDigitizing, &QgsAppGpsDigitizing::trackVertexAdded );

  QgsGpsInformation info;
  info.latitude = 45;
  info.longitude = 100;

  gpsDigitizing.gpsStateChanged( info );
  gpsDigitizing.createVertexAtCurrentLocation();
  QCOMPARE( spy.count(), 1 );

  QCOMPARE( gpsDigitizing.totalTrackLength(), 0 );
  QCOMPARE( gpsDigitizing.trackDistanceFromStart(), 0 );

  info.latitude = 46;
  info.longitude = 100;

  gpsDigitizing.gpsStateChanged( info );
  gpsDigitizing.createVertexAtCurrentLocation();
  QCOMPARE( spy.count(), 2 );

  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"NONE"_s );
  QGSCOMPARENEAR( gpsDigitizing.totalTrackLength(), 1, 0.01 );
  QGSCOMPARENEAR( gpsDigitizing.trackDistanceFromStart(), 1, 0.01 );
  QgsProject::instance()->setEllipsoid( u"EPSG:7030"_s );
  QCOMPARE( QgsProject::instance()->ellipsoid(), u"EPSG:7030"_s );
  QGSCOMPARENEAR( gpsDigitizing.totalTrackLength(), 111141.548, 1 );
  QGSCOMPARENEAR( gpsDigitizing.trackDistanceFromStart(), 111141.548, 1 );

  info.latitude = 46;
  info.longitude = 101;

  gpsDigitizing.gpsStateChanged( info );
  gpsDigitizing.createVertexAtCurrentLocation();
  QCOMPARE( spy.count(), 3 );
  QGSCOMPARENEAR( gpsDigitizing.totalTrackLength(), 188604.338, 1 );
  QGSCOMPARENEAR( gpsDigitizing.trackDistanceFromStart(), 135869.0912, 1 );

  QSignalSpy spyReset( &gpsDigitizing, &QgsGpsLogger::trackReset );
  gpsDigitizing.resetTrack();
  QCOMPARE( spyReset.count(), 1 );
}


QGSTEST_MAIN( TestQgsGpsIntegration )
#include "testqgsgpsintegration.moc"
