/***************************************************************************
                         testqgstemporalrangeobject.cpp
                         ---------------
    begin                : January 2020
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
#include <qgstemporalrangeobject.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsTemporalRangeObject class.
 */
class TestQgsTemporalRangeObject : public QObject
{
    Q_OBJECT

  public:
    TestQgsTemporalRangeObject() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void isTemporal();
    void checkSettingTemporal();
    void checkSettingTemporalRange();

  private:
    QgsTemporalRangeObject *temporalObject = nullptr;
};

void TestQgsTemporalRangeObject::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

}

void TestQgsTemporalRangeObject::init()
{
  //create some objects that will be used in all tests...
  //create a temporal object that will be used in all tests...

  temporalObject = new QgsTemporalRangeObject( true );
}

void TestQgsTemporalRangeObject::cleanup()
{
}

void TestQgsTemporalRangeObject::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTemporalRangeObject::isTemporal()
{
  QVERIFY( temporalObject->isTemporal() );
}

void TestQgsTemporalRangeObject::checkSettingTemporal()
{
  temporalObject->setIsTemporal( false );

  QCOMPARE( temporalObject->isTemporal(), false );
}

void TestQgsTemporalRangeObject::checkSettingTemporalRange()
{
  const QgsDateTimeRange dateTimeRange = QgsDateTimeRange();

  temporalObject->setTemporalRange( dateTimeRange );

  QCOMPARE( temporalObject->temporalRange(), dateTimeRange );
}

QGSTEST_MAIN( TestQgsTemporalRangeObject )
#include "testqgstemporalrangeobject.moc"
