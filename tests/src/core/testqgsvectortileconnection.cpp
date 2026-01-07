/***************************************************************************
  testqgsvectortileconnection.cpp
  --------------------------------------
  Date                 : January 2022
  Copyright            : (C) 2022 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
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
#include "qgsvectortileconnection.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the vector tile provider connection class
 */
class TestQgsVectorTileConnection : public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorTileConnection() = default;

  private:
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void test_encodedUri();
};


void TestQgsVectorTileConnection::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsVectorTileConnection::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorTileConnection::test_encodedUri()
{
  QgsVectorTileProviderConnection::Data conn;
  conn.url = u"https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=abcdef12345"_s;
  conn.zMin = 0;
  conn.zMax = 18;
  QString uri = QgsVectorTileProviderConnection::encodedUri( conn );
  QCOMPARE( uri, u"type=xyz&url=https%3A%2F%2Fapi.maptiler.com%2Ftiles%2Fv3%2F%7Bz%7D%2F%7Bx%7D%2F%7By%7D.pbf%3Fkey%3Dabcdef12345&zmax=18&zmin=0"_s );

  conn.url = u"file:///home/user/tiles.mbtiles"_s;
  conn.zMin = 0;
  conn.zMax = 18;
  uri = QgsVectorTileProviderConnection::encodedUri( conn );
  QCOMPARE( uri, u"type=mbtiles&url=file%3A%2F%2F%2Fhome%2Fuser%2Ftiles.mbtiles&zmax=18&zmin=0"_s );
}


QGSTEST_MAIN( TestQgsVectorTileConnection )
#include "testqgsvectortileconnection.moc"
