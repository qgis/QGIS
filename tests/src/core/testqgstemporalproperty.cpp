/***************************************************************************
                         testqgstemporalproperty.cpp
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
#include "qgsrange.h"

//qgis includes...
#include <qgstemporalproperty.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsTemporalProperty class.
 */
class TestQgsTemporalProperty : public QObject
{
    Q_OBJECT

  public:
    TestQgsTemporalProperty() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void checkSettingTemporalStatus();

  private:
    QgsTemporalProperty *temporalProperty = nullptr;
};

void TestQgsTemporalProperty::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsTemporalProperty::init()
{
  // create a temporal property that will be used in all tests...

  temporalProperty = new QgsTemporalProperty();
}

void TestQgsTemporalProperty::cleanup()
{
}

void TestQgsTemporalProperty::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTemporalProperty::checkSettingTemporalStatus()
{
  temporalProperty->setIsActive( true );

  QCOMPARE( temporalProperty->isActive(), true );
}

QGSTEST_MAIN( TestQgsTemporalProperty )
#include "testqgstemporalproperty.moc"
