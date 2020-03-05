/***************************************************************************
                         testqgsrasterdataprovidertemporalcapabilities.cpp
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
#include <qgsrasterdataprovidertemporalcapabilities.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterDataProviderTemporalCapabilities class.
 */
class TestQgsRasterDataProviderTemporalCapabilities : public QObject
{
    Q_OBJECT

  public:
    TestQgsRasterDataProviderTemporalCapabilities() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void checkActiveStatus();
    void checkTemporalRange();
    void checkReferenceTemporalRange();

  private:
    QgsRasterDataProviderTemporalCapabilities *temporalCapabilities = nullptr;
};

void TestQgsRasterDataProviderTemporalCapabilities::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsRasterDataProviderTemporalCapabilities::init()
{
  //create some objects that will be used in all tests...
  //create a temporal object that will be used in all tests...

  temporalCapabilities = new QgsRasterDataProviderTemporalCapabilities( true );
}

void TestQgsRasterDataProviderTemporalCapabilities::cleanup()
{
}

void TestQgsRasterDataProviderTemporalCapabilities::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterDataProviderTemporalCapabilities::checkActiveStatus()
{
  temporalCapabilities->setIsActive( false );
  QCOMPARE( temporalCapabilities->isActive(), false );
}

void TestQgsRasterDataProviderTemporalCapabilities::checkTemporalRange()
{
  QgsDateTimeRange fixedDateTimeRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ) ),
                                        QDateTime( QDate( 2020, 12, 31 ) ) );
  QgsDateTimeRange dateTimeRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ) ),
                                   QDateTime( QDate( 2020, 3, 1 ) ) );
  QgsDateTimeRange outOfLimitsRange = QgsDateTimeRange( QDateTime( QDate( 2019, 1, 1 ) ),
                                      QDateTime( QDate( 2021, 3, 1 ) ) );

  temporalCapabilities->setFixedTemporalRange( fixedDateTimeRange );
  temporalCapabilities->setRequestedTemporalRange( dateTimeRange );

  QCOMPARE( temporalCapabilities->fixedTemporalRange(), fixedDateTimeRange );
  QCOMPARE( temporalCapabilities->requestedTemporalRange(), dateTimeRange );

  // Test setting out of fixed temporal range limits, should not update the temporal range.
  temporalCapabilities->setRequestedTemporalRange( outOfLimitsRange );
  QCOMPARE( temporalCapabilities->requestedTemporalRange(), dateTimeRange );

  // Test if setting the requested temporal range with the fixed temporal range object,
  // will result in to setting the requested temporal range with the fixed temporal range.
  temporalCapabilities->setRequestedTemporalRange( fixedDateTimeRange );
  QCOMPARE( temporalCapabilities->requestedTemporalRange(), fixedDateTimeRange );
}

void TestQgsRasterDataProviderTemporalCapabilities::checkReferenceTemporalRange()
{
  QgsDateTimeRange fixedDateTimeRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ) ),
                                        QDateTime( QDate( 2020, 12, 31 ) ) );
  QgsDateTimeRange dateTimeRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ) ),
                                   QDateTime( QDate( 2020, 3, 1 ) ) );
  QgsDateTimeRange outOfLimitsRange = QgsDateTimeRange( QDateTime( QDate( 2019, 1, 1 ) ),
                                      QDateTime( QDate( 2021, 3, 1 ) ) );

  temporalCapabilities->setFixedReferenceTemporalRange( fixedDateTimeRange );
  temporalCapabilities->setRequestedReferenceTemporalRange( dateTimeRange );

  QCOMPARE( temporalCapabilities->fixedReferenceTemporalRange(), fixedDateTimeRange );
  QCOMPARE( temporalCapabilities->requestedReferenceTemporalRange(), dateTimeRange );

  temporalCapabilities->setRequestedReferenceTemporalRange( outOfLimitsRange );
  QCOMPARE( temporalCapabilities->requestedReferenceTemporalRange(), dateTimeRange );

  temporalCapabilities->setRequestedReferenceTemporalRange( fixedDateTimeRange );
  QCOMPARE( temporalCapabilities->requestedReferenceTemporalRange(), fixedDateTimeRange );
}

QGSTEST_MAIN( TestQgsRasterDataProviderTemporalCapabilities )
#include "testqgsrasterdataprovidertemporalcapabilities.moc"
