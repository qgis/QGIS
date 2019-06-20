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
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
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
    void testTimestamp();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsGpsInformationWidget::TestQgsGpsInformationWidget() = default;

//runs before all tests
void TestQgsGpsInformationWidget::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsGpsInformationWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsGpsInformationWidget::testTimestamp()
{

  //create a temporary layer
  QgsVectorLayer *tempLayer( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=intf:int" ),
                             QStringLiteral( "vl" ),
                             QStringLiteral( "memory" ) ) );
  QgsVectorLayer *tempLayerString( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=stringf:string&field=intf:int" ),
                                   QStringLiteral( "vl" ),
                                   QStringLiteral( "memory" ) ) );
  QgsVectorLayer *tempLayerDateTime( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=datetimef:datetime&field=intf:int" ),
                                     QStringLiteral( "vl" ),
                                     QStringLiteral( "memory" ) ) );

  QgsProject::instance()->addMapLayers( { tempLayer, tempLayerString, tempLayerDateTime } );

  QVERIFY( tempLayer->isValid() );
  QgsMapCanvas *canvas = mQgisApp->mapCanvas();
  QgsGpsInformationWidget widget( canvas );
  canvas->setCurrentLayer( tempLayer );
  QVERIFY( ! widget.mGboxTimestamp->isEnabled() );

  canvas->setCurrentLayer( tempLayerString );
  QVERIFY( widget.mGboxTimestamp->isEnabled() );
  QVERIFY( widget.mCboTimestampField->findText( QStringLiteral( "stringf" ) ) != -1 );
  QVERIFY( widget.mCboTimestampField->findText( QStringLiteral( "intf" ) ) == -1 );

  canvas->setCurrentLayer( tempLayerDateTime );
  QVERIFY( widget.mGboxTimestamp->isEnabled() );
  QVERIFY( widget.mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) != -1 );
  QVERIFY( widget.mCboTimestampField->findText( QStringLiteral( "intf" ) ) == -1 );

  // Check tz combo
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  QVERIFY( ! widget.mCboTimeZones->isEnabled() );
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::LocalTime ) );
  QVERIFY( ! widget.mCboTimeZones->isEnabled() );
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::TimeZone ) );
  QVERIFY( widget.mCboTimeZones->isEnabled() );

  canvas->setCurrentLayer( tempLayer );
  QVERIFY( ! widget.mGboxTimestamp->isEnabled() );

  // Test timestamp conversions
  canvas->setCurrentLayer( tempLayerDateTime );
  // 2019/06/19 12:27:34.543[UTC]
  widget.mLastNmeaTime = { 119, 5, 19, 12, 27, 34, 543 };
  QDateTime dateTime( QDate( 2019, 6, 19 ), QTime( 12, 27, 34, 543 ) );
  dateTime.setTimeSpec( Qt::TimeSpec::UTC );

  widget.mCbxLeapSeconds->setChecked( false );
  widget.mLeapSeconds->setValue( 7 );

  int fieldIdx { tempLayerDateTime->fields().indexOf( QStringLiteral( "datetimef" ) ) };
  widget.mCboTimestampField->setCurrentIndex( widget.mCboTimestampField->findText( QStringLiteral( "datetimef" ) ) );
  QVERIFY( fieldIdx != -1 );
  // UTC
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  QVariant dt { widget.timestamp( tempLayerDateTime, fieldIdx ) };
  QCOMPARE( dt.toDateTime(), dateTime );
  QVERIFY( widget.mPreferredTimestampFields.contains( tempLayerDateTime->id() ) );
  QCOMPARE( widget.mPreferredTimestampFields[ tempLayerDateTime->id() ], QStringLiteral( "datetimef" ) );

  // Test store preferred fields
  canvas->setCurrentLayer( tempLayerString );
  fieldIdx = tempLayerString->fields().indexOf( QStringLiteral( "stringf" ) );
  QVERIFY( fieldIdx != -1 );
  widget.mCboTimestampField->setCurrentIndex( widget.mCboTimestampField->findText( QStringLiteral( "stringf" ) ) );
  canvas->setCurrentLayer( tempLayerDateTime );
  QVERIFY( widget.mPreferredTimestampFields.contains( tempLayerDateTime->id() ) );
  QCOMPARE( widget.mPreferredTimestampFields[ tempLayerString->id() ], QStringLiteral( "stringf" ) );
  QVERIFY( widget.mPreferredTimestampFields.contains( tempLayerDateTime->id() ) );
  QCOMPARE( widget.mPreferredTimestampFields[ tempLayerDateTime->id() ], QStringLiteral( "datetimef" ) );

  // Test string
  canvas->setCurrentLayer( tempLayerString );
  // UTC
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::UTC ) );
  dt = widget.timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), dateTime.toString( Qt::DateFormat::ISODate ) );
  // Local Time (not very robust because we cannot change the system timezone and it may be GMT)
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::LocalTime ) );
  dt = widget.timestamp( tempLayerString, fieldIdx );
  QDateTime localTime( dateTime.toLocalTime() );
  QCOMPARE( dt.toString(), localTime.toString( Qt::DateFormat::ISODate ) );
  // Timezone
  widget.mCboTimestampFormat->setCurrentIndex( widget.mCboTimestampFormat->findData( Qt::TimeSpec::TimeZone ) );
  widget.mCboTimeZones->setCurrentIndex( widget.mCboTimeZones->findText( QStringLiteral( "Asia/Colombo" ) ) ) ;
  QDateTime tzTime( dateTime.toTimeZone( QTimeZone( QStringLiteral( "Asia/Colombo" ).toUtf8() ) ) );
  dt = widget.timestamp( tempLayerString, fieldIdx );
  QCOMPARE( dt.toString(), tzTime.toString( Qt::DateFormat::ISODate ) );

  // Test that preferred field is stored
  canvas->setCurrentLayer( tempLayerDateTime );
  QCOMPARE( widget.mCboTimestampField->currentText(), QStringLiteral( "datetimef" ) );
}

QGSTEST_MAIN( TestQgsGpsInformationWidget )
#include "testqgsgpsinformationwidget.moc"
