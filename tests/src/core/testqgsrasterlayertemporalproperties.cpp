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
    void testChangedSignal();

  private:
    QgsRasterLayerTemporalProperties *temporalProperties = nullptr;
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
  // create a temporal property that will be used in all tests...

  temporalProperties = new QgsRasterLayerTemporalProperties();
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
  QgsDateTimeRange dateTimeRange = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ) ),
                                   QDateTime( QDate( 2020, 12, 31 ) ) );

  temporalProperties->setFixedTemporalRange( dateTimeRange );

  QCOMPARE( temporalProperties->fixedTemporalRange(), dateTimeRange );
}

void TestQgsRasterLayerTemporalProperties::testChangedSignal()
{
  QCOMPARE( temporalProperties->temporalSource(), QgsMapLayerTemporalProperties::TemporalSource::Layer );
  QSignalSpy spy( temporalProperties, SIGNAL( changed() ) );

  temporalProperties->setTemporalSource( QgsMapLayerTemporalProperties::TemporalSource::Layer );
  QCOMPARE( spy.count(), 0 );
  temporalProperties->setTemporalSource( QgsMapLayerTemporalProperties::TemporalSource::Project );
  QCOMPARE( spy.count(), 1 );

  temporalProperties->setIsActive( true );
  QCOMPARE( spy.count(), 2 );
}

QGSTEST_MAIN( TestQgsRasterLayerTemporalProperties )
#include "testqgsrasterlayertemporalproperties.moc"
