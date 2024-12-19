/***************************************************************************
                         testqgsrasterlayertemporalproperties.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>

//qgis includes...
#include <qgsrasterlayertemporalproperties.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterLayerTemporalProperties class.
 */
class TestQgsRasterLayerTemporalProperties : public QObject
{
    Q_OBJECT

  public:
    TestQgsRasterLayerTemporalProperties() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void checkSettingTemporalRange();
    void testReadWrite();
    void testVisibleInTimeRange();

};

void TestQgsRasterLayerTemporalProperties::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsRasterLayerTemporalProperties::init()
{
}

void TestQgsRasterLayerTemporalProperties::cleanup()
{
}

void TestQgsRasterLayerTemporalProperties::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterLayerTemporalProperties::checkSettingTemporalRange()
{
  QgsRasterLayerTemporalProperties temporalProperties;
  const QgsDateTimeRange dateTimeRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
                                         QDateTime( QDate( 2020, 12, 31 ), QTime( 0, 0, 0 ) ) );

  temporalProperties.setFixedTemporalRange( dateTimeRange );

  QCOMPARE( temporalProperties.fixedTemporalRange(), dateTimeRange );
}

void TestQgsRasterLayerTemporalProperties::testReadWrite()
{
  QgsRasterLayerTemporalProperties temporalProperties;

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  QDomElement node = doc.createElement( QStringLiteral( "temp" ) );
  // read none existent node
  temporalProperties.readXml( node.toElement(), QgsReadWriteContext() );

  // must not be active!
  QVERIFY( !temporalProperties.isActive() );

  temporalProperties.setIsActive( true );
  temporalProperties.setMode( Qgis::RasterTemporalMode::TemporalRangeFromDataProvider );
  temporalProperties.setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod::MatchExactUsingEndOfRange );

  temporalProperties.writeXml( node, doc, QgsReadWriteContext() );

  QgsRasterLayerTemporalProperties temporalProperties2;
  temporalProperties2.readXml( node, QgsReadWriteContext() );
  QVERIFY( temporalProperties2.isActive() );
  QCOMPARE( temporalProperties2.mode(), Qgis::RasterTemporalMode::TemporalRangeFromDataProvider );
  QCOMPARE( temporalProperties2.intervalHandlingMethod(), Qgis::TemporalIntervalMatchMethod::MatchExactUsingEndOfRange );

  temporalProperties.setIsActive( false );
  QDomElement node2 = doc.createElement( QStringLiteral( "temp" ) );
  temporalProperties.writeXml( node2, doc, QgsReadWriteContext() );
  QgsRasterLayerTemporalProperties temporalProperties3;
  temporalProperties3.readXml( node2, QgsReadWriteContext() );
  QVERIFY( !temporalProperties3.isActive() );
  QCOMPARE( temporalProperties3.mode(), Qgis::RasterTemporalMode::TemporalRangeFromDataProvider );
  QCOMPARE( temporalProperties3.intervalHandlingMethod(), Qgis::TemporalIntervalMatchMethod::MatchExactUsingEndOfRange );

  temporalProperties.setMode( Qgis::RasterTemporalMode::FixedTemporalRange );
  temporalProperties.setFixedTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
      QDateTime( QDate( 2020, 12, 31 ), QTime( 0, 0, 0 ) ) ) );
  QDomElement node3 = doc.createElement( QStringLiteral( "temp" ) );
  temporalProperties.writeXml( node3, doc, QgsReadWriteContext() );
  QgsRasterLayerTemporalProperties temporalProperties4;
  temporalProperties4.readXml( node3, QgsReadWriteContext() );
  QVERIFY( !temporalProperties4.isActive() );
  QCOMPARE( temporalProperties4.mode(), Qgis::RasterTemporalMode::FixedTemporalRange );
  QCOMPARE( temporalProperties4.fixedTemporalRange(), QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
            QDateTime( QDate( 2020, 12, 31 ), QTime( 0, 0, 0 ) ) ) );

}

void TestQgsRasterLayerTemporalProperties::testVisibleInTimeRange()
{
  QgsRasterLayerTemporalProperties props;
  // by default, should be visible regardless of time range
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange() ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ) ) ) );

  // when in data provider time handling mode, we also should always render regardless of time range
  props.setIsActive( true );
  props.setMode( Qgis::RasterTemporalMode::TemporalRangeFromDataProvider );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange() ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ) ) ) );
  // fix temporal range should be ignored while in ModeTemporalRangeFromDataProvider
  props.setFixedTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
                               QDateTime( QDate( 2020, 1, 5 ), QTime( 0, 0, 0 ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange() ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2019, 1, 1 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2019, 1, 2 ), QTime( 0, 0, 0 ) ) ) ) );

  // switch to fixed time mode
  props.setMode( Qgis::RasterTemporalMode::FixedTemporalRange );
  // should be visible in infinite time ranges
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange() ) );
  // should not be visible -- outside of fixed time range
  QVERIFY( !props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2019, 1, 1 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2019, 1, 2 ), QTime( 0, 0, 0 ) ) ) ) );
  // should be visible -- intersects fixed time range
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 2 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ) ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 2 ), QTime( 0, 0, 0 ) ),
           QDateTime( ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime(),
           QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ) ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2019, 1, 2 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ) ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2019, 1, 2 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2021, 1, 3 ), QTime( 0, 0, 0 ) ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ) ) ) );
  QVERIFY( props.isVisibleInTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 5 ), QTime( 0, 0, 0 ) ),
           QDateTime( QDate( 2020, 1, 5 ), QTime( 0, 0, 0 ) ) ) ) );
}

QGSTEST_MAIN( TestQgsRasterLayerTemporalProperties )
#include "testqgsrasterlayertemporalproperties.moc"
