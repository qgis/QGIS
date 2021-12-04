/***************************************************************************
  testqgsvectortileutils.cpp
  --------------------------------------
  Date                 : November 2021
  Copyright            : (C) 2021 by Mathieu Pellerin
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
#include <QObject>
#include <QString>

//qgis includes...
#include "qgsapplication.h"
#include "qgsvectortileutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the vector tile utils class
 */
class TestQgsVectorTileUtils : public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorTileUtils() = default;

  private:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_scaleToZoomLevel();
};


void TestQgsVectorTileUtils::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsVectorTileUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorTileUtils::test_scaleToZoomLevel()
{
  // test zoom level logic
  int zoomLevel = QgsVectorTileUtils::scaleToZoomLevel( 288896, 0, 20 );
  QCOMPARE( zoomLevel, 10 );
}


QGSTEST_MAIN( TestQgsVectorTileUtils )
#include "testqgsvectortileutils.moc"
