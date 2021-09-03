/***************************************************************************
    testqgsgpsinformationwidget.cpp
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
#include "qgstest.h"
#include <QTimeZone>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgssettingsregistrycore.h"
#include "gps/qgsgpsinformationwidget.h"
#include "nmeatime.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the GPS information widget
 */
class TestQgsGpsInformationWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsGpsInformationWidget();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testGuiSignals();
    void testStorePreferredFields();
    void testTimestamp();
    void testTimestampWrite();
    void testMultiPartLayers();

  private:
    std::unique_ptr<QgsGpsInformationWidget> prepareWidget();
    QDateTime _testWrite( QgsVectorLayer *vlayer, QgsGpsInformationWidget *widget, const QString &fieldName, Qt::TimeSpec timeSpec, bool commit = false );
    QgsVectorLayer *tempLayer = nullptr;
    QgsVectorLayer *tempLayerString = nullptr;
    QgsVectorLayer *tempLayerDateTime = nullptr;
    QgsVectorLayer *tempLayerLineString = nullptr;
    QgsVectorLayer *tempGpkgLayerPointString = nullptr;
    QgisApp *mQgisApp = nullptr;
};

TestQgsGpsInformationWidget::TestQgsGpsInformationWidget() = default;

//runs before all tests
void TestQgsGpsInformationWidget::initTestCase()
{
  // setup the test QSettings environment

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mQgisApp = new QgisApp();


  tempLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=intf:int" ),
                                  QStringLiteral( "vl1" ),
                                  QStringLiteral( "memory" ) );
  tempLayerString = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=stringf:string&field=intf:int" ),
                                        QStringLiteral( "vl2" ),
                                        QStringLiteral( "memory" ) );
  tempLayerDateTime = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=datetimef:datetime&field=intf:int" ),
                                          QStringLiteral( "vl3" ),
                                          QStringLiteral( "memory" ) );
  tempLayerLineString = new QgsVectorLayer( QStringLiteral( "Linestring?crs=epsg:4326&field=intf:int&field=stringf:string" ),
      QStringLiteral( "vl4" ),
      QStringLiteral( "memory" ) );

  QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog.setValue( true );

  const QString tempPath = QDir::tempPath() + QStringLiteral( "/gps_timestamp.gpkg" );
  QFile::copy( TEST_DATA_DIR + QStringLiteral( "/gps_timestamp.gpkg" ), tempPath );
  tempGpkgLayerPointString = new QgsVectorLayer( QStringLiteral( "%1|layername=points" ).arg( tempPath ),
      QStringLiteral( "vl4" ) );
  Q_ASSERT( tempGpkgLayerPointString->isValid() );
  Q_ASSERT( tempLayer->isValid() );
  Q_ASSERT( tempLayerString->isValid() );
  Q_ASSERT( tempLayerDateTime->isValid() );
  Q_ASSERT( tempLayerLineString->isValid() );
  QgsProject::instance()->addMapLayers( { tempLayer, tempLayerString, tempLayerDateTime, tempGpkgLayerPointString, tempLayerLineString } );
}

//runs after all tests
void TestQgsGpsInformationWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


std::unique_ptr<QgsGpsInformationWidget> TestQgsGpsInformationWidget::prepareWidget()
{
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  std::unique_ptr<QgsGpsInformationWidget> widget = std::make_unique<QgsGpsInformationWidget>( canvas );
  // Widget config and input values
  // 2019/06/19 12:27:34.543[UTC]
  widget->mLastNmeaTime = { 119, 5, 19, 12, 27, 34, 543 };
  canvas->setCurrentLayer( tempLayerString );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "stringf" ) ) );
  canvas->setCurrentLayer( tempLayerDateTime );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) );
  canvas->setCurrentLayer( tempLayerLineString );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "stringf" ) ) );
  canvas->setCurrentLayer( tempGpkgLayerPointString );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) );

  widget->mCboTimeZones->setCurrentIndex( widget->mCboTimeZones->findText( QStringLiteral( "Asia/Colombo" ) ) );
  widget->mCbxLeapSeconds->setChecked( false );
  widget->mLeapSeconds->setValue( 7 );
  return widget;
}

QDateTime TestQgsGpsInformationWidget::_testWrite( QgsVectorLayer *vlayer, QgsGpsInformationWidget *widget, const QString &fieldName, Qt::TimeSpec timeSpec, bool commit )
{
  widget->mMapCanvas->setCurrentLayer( vlayer );
  vlayer->startEditing();
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( fieldName ) );
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( timeSpec ) );
  widget->mBtnCloseFeature_clicked();
  const auto fids { vlayer->allFeatureIds() };
  const auto fid { std::min_element( fids.begin(), fids.end() ) };
  const QgsFeature f { vlayer->getFeature( *fid ) };
  if ( commit )
    vlayer->commitChanges();
  else
    vlayer->rollBack();
  return f.attribute( fieldName ).toDateTime();
}

void TestQgsGpsInformationWidget::testGuiSignals()
{
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  std::unique_ptr<QgsGpsInformationWidget> widget = prepareWidget();
  canvas->setCurrentLayer( tempLayer );
  QVERIFY( ! widget->mGboxTimestamp->isEnabled() );

  canvas->setCurrentLayer( tempLayerString );
  QVERIFY( widget->mGboxTimestamp->isEnabled() );
  QVERIFY( widget->mCboTimestampField->findText( QStringLiteral( "stringf" ) ) != -1 );
  QVERIFY( widget->mCboTimestampField->findText( QStringLiteral( "intf" ) ) == -1 );

  canvas->setCurrentLayer( tempLayerDateTime );
  QVERIFY( widget->mGboxTimestamp->isEnabled() );
  QVERIFY( widget->mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) != -1 );
  QVERIFY( widget->mCboTimestampField->findText( QStringLiteral( "intf" ) ) == -1 );

  // Check tz combo
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  QVERIFY( ! widget->mCboTimeZones->isEnabled() );
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::LocalTime ) );
  QVERIFY( ! widget->mCboTimeZones->isEnabled() );
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::TimeZone ) );
  QVERIFY( widget->mCboTimeZones->isEnabled() );

  canvas->setCurrentLayer( tempLayer );
  QVERIFY( ! widget->mGboxTimestamp->isEnabled() );
}

void TestQgsGpsInformationWidget::testStorePreferredFields()
{
  std::unique_ptr<QgsGpsInformationWidget> widget = prepareWidget();
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  canvas->setCurrentLayer( tempLayerDateTime );
  int fieldIdx = tempLayerDateTime->fields().indexOf( QLatin1String( "datetimef" ) );
  QVERIFY( fieldIdx != -1 );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) );

  canvas->setCurrentLayer( tempLayerString );
  fieldIdx = tempLayerString->fields().indexOf( QLatin1String( "stringf" ) );
  QVERIFY( fieldIdx != -1 );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "stringf" ) ) );

  canvas->setCurrentLayer( tempLayer );
  QVERIFY( widget->mPreferredTimestampFields.contains( tempLayerDateTime->id() ) );
  QCOMPARE( widget->mPreferredTimestampFields[ tempLayerString->id() ], QStringLiteral( "stringf" ) );
  QVERIFY( widget->mPreferredTimestampFields.contains( tempLayerDateTime->id() ) );
  QCOMPARE( widget->mPreferredTimestampFields[ tempLayerDateTime->id() ], QStringLiteral( "datetimef" ) );
}

void TestQgsGpsInformationWidget::testTimestamp()
{
  std::unique_ptr<QgsGpsInformationWidget> widget = prepareWidget();
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();

  QDateTime dateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ) );
  dateTime.setTimeSpec( Qt::TimeSpec::UTC );
  const QDateTime tzTime( dateTime.toTimeZone( QTimeZone( QStringLiteral( "Asia/Colombo" ).toUtf8() ) ) );
  const QDateTime localTime( dateTime.toLocalTime() );

  ///////////////////////////////////////////
  // Test datetime layer
  canvas->setCurrentLayer( tempLayerDateTime );

  int fieldIdx { tempLayerDateTime->fields().indexOf( QLatin1String( "datetimef" ) ) };
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) );
  QVERIFY( fieldIdx != -1 );
  // UTC
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  QVariant dt = widget->timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime(), dateTime );

  // Local time
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::LocalTime ) );
  dt = widget->timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime(), dateTime.toLocalTime() );

  // Leap seconds
  widget->mCbxLeapSeconds->setChecked( true );
  dt = widget->timestamp( tempLayerDateTime, fieldIdx );
  QCOMPARE( dt.toDateTime(), dateTime.addSecs( 7 ) );
  widget->mCbxLeapSeconds->setChecked( false );

  ///////////////////////////////////////////
  // Test string
  canvas->setCurrentLayer( tempLayerString );
  fieldIdx = tempLayerString->fields().indexOf( QLatin1String( "stringf" ) );
  widget->mCboTimestampField->setCurrentIndex( widget->mCboTimestampField->findText( QStringLiteral( "stringf" ) ) );

  // UTC
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  dt = widget->timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), dateTime.toString( Qt::DateFormat::ISODate ) );

  // Local Time (not very robust because we cannot change the system timezone and it may be GMT)
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::LocalTime ) );
  dt = widget->timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), localTime.toString( Qt::DateFormat::ISODate ) );

  // Timezone
  widget->mCboTimestampFormat->setCurrentIndex( widget->mCboTimestampFormat->findData( Qt::TimeSpec::TimeZone ) );
  widget->mCboTimeZones->setCurrentIndex( widget->mCboTimeZones->findText( QStringLiteral( "Asia/Colombo" ) ) ) ;
  dt = widget->timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), tzTime.toString( Qt::DateFormat::ISODate ) );

}

void TestQgsGpsInformationWidget::testTimestampWrite()
{
  std::unique_ptr<QgsGpsInformationWidget> widget = prepareWidget();
  QDateTime dateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ) );
  dateTime.setTimeSpec( Qt::TimeSpec::UTC );
  const QDateTime tzTime( dateTime.toTimeZone( QTimeZone( QStringLiteral( "Asia/Colombo" ).toUtf8() ) ) );
  const QDateTime localTime( dateTime.toLocalTime() );

  // Test write on datetime field
  QCOMPARE( _testWrite( tempLayerDateTime, widget.get(), QStringLiteral( "datetimef" ),  Qt::TimeSpec::UTC ), dateTime );
  QCOMPARE( _testWrite( tempLayerDateTime, widget.get(), QStringLiteral( "datetimef" ),  Qt::TimeSpec::LocalTime ), localTime );
  QCOMPARE( _testWrite( tempLayerDateTime, widget.get(), QStringLiteral( "datetimef" ),  Qt::TimeSpec::TimeZone ),  tzTime );

  // Test write on string field
  QCOMPARE( _testWrite( tempLayerString, widget.get(), QStringLiteral( "stringf" ),  Qt::TimeSpec::UTC ).toString( Qt::DateFormat::ISODate ), dateTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempLayerString, widget.get(), QStringLiteral( "stringf" ),  Qt::TimeSpec::LocalTime ).toString( Qt::DateFormat::ISODate ), localTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempLayerString, widget.get(), QStringLiteral( "stringf" ),  Qt::TimeSpec::TimeZone ).toString( Qt::DateFormat::ISODate ),  tzTime.toString( Qt::DateFormat::ISODate ) );

  // Test write on line string field
  widget->mCaptureList.push_back( QgsPoint( 1, 2 ) );
  widget->mCaptureList.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, widget.get(), QStringLiteral( "stringf" ),  Qt::TimeSpec::UTC ).toString( Qt::DateFormat::ISODate ), dateTime.toString( Qt::DateFormat::ISODate ) );
  widget->mCaptureList.push_back( QgsPoint( 1, 2 ) );
  widget->mCaptureList.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, widget.get(), QStringLiteral( "stringf" ),  Qt::TimeSpec::LocalTime ).toString( Qt::DateFormat::ISODate ), localTime.toString( Qt::DateFormat::ISODate ) );
  widget->mCaptureList.push_back( QgsPoint( 1, 2 ) );
  widget->mCaptureList.push_back( QgsPoint( 3, 4 ) );
  QCOMPARE( _testWrite( tempLayerLineString, widget.get(), QStringLiteral( "stringf" ),  Qt::TimeSpec::TimeZone ).toString( Qt::DateFormat::ISODate ),  tzTime.toString( Qt::DateFormat::ISODate ) );

  // Write on GPKG
  // Test write on datetime field
  QCOMPARE( _testWrite( tempGpkgLayerPointString, widget.get(), QStringLiteral( "datetimef" ), Qt::TimeSpec::UTC, true ), dateTime );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, widget.get(), QStringLiteral( "datetimef" ), Qt::TimeSpec::LocalTime, true ), localTime );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, widget.get(), QStringLiteral( "datetimef" ), Qt::TimeSpec::TimeZone, true ),  tzTime );

  // Test write on string field
  QCOMPARE( _testWrite( tempGpkgLayerPointString, widget.get(), QStringLiteral( "stringf" ),
                        Qt::TimeSpec::UTC, true ).toString( Qt::DateFormat::ISODate ), dateTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, widget.get(), QStringLiteral( "stringf" ),
                        Qt::TimeSpec::LocalTime, true ).toString( Qt::DateFormat::ISODate ), localTime.toString( Qt::DateFormat::ISODate ) );
  QCOMPARE( _testWrite( tempGpkgLayerPointString, widget.get(), QStringLiteral( "stringf" ),
                        Qt::TimeSpec::TimeZone, true ).toString( Qt::DateFormat::ISODate ),  tzTime.toString( Qt::DateFormat::ISODate ) );


}

void TestQgsGpsInformationWidget::testMultiPartLayers()
{
  std::unique_ptr< QgsVectorLayer >multiLineString = std::make_unique< QgsVectorLayer >( QStringLiteral( "MultiLinestring?crs=epsg:4326&field=intf:int&field=stringf:string" ),
      QStringLiteral( "vl4" ),
      QStringLiteral( "memory" ) );

  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  std::unique_ptr<QgsGpsInformationWidget> widget = std::make_unique<QgsGpsInformationWidget>( canvas );
  widget->mMapCanvas->setCurrentLayer( multiLineString.get() );
  multiLineString->startEditing();

  // not possible, no points
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiLineString->featureCount(), 0L );
  // need at least 2 points
  widget->mBtnAddVertex_clicked();
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiLineString->featureCount(), 0L );

  widget->mBtnAddVertex_clicked();
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiLineString->featureCount(), 1L );
  QgsFeature f;
  QVERIFY( multiLineString->getFeatures().nextFeature( f ) );
  QCOMPARE( f.geometry().wkbType(), QgsWkbTypes::MultiLineString );
  multiLineString->rollBack();

  // multipolygon
  std::unique_ptr< QgsVectorLayer >multiPolygon = std::make_unique< QgsVectorLayer >( QStringLiteral( "MultiPolygon?crs=epsg:4326&field=intf:int&field=stringf:string" ),
      QStringLiteral( "vl4" ),
      QStringLiteral( "memory" ) );

  widget = std::make_unique<QgsGpsInformationWidget>( canvas );
  widget->mMapCanvas->setCurrentLayer( multiPolygon.get() );
  multiPolygon->startEditing();

  // not possible, no points
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiPolygon->featureCount(), 0L );

  // need at least 3 points
  widget->mBtnAddVertex_clicked();
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiPolygon->featureCount(), 0L );
  widget->mBtnAddVertex_clicked();
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiPolygon->featureCount(), 0L );

  widget->mBtnAddVertex_clicked();
  widget->mBtnCloseFeature_clicked();
  QCOMPARE( multiPolygon->featureCount(), 1L );
  QVERIFY( multiPolygon->getFeatures().nextFeature( f ) );
  QCOMPARE( f.geometry().wkbType(), QgsWkbTypes::MultiPolygon );
  multiPolygon->rollBack();
}



QGSTEST_MAIN( TestQgsGpsInformationWidget )
#include "testqgsgpsinformationwidget.moc"
