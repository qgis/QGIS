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
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

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
  conn.url = QStringLiteral( "https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=abcdef12345" );
  conn.zMin = 0;
  conn.zMax = 18;
  QString uri = QgsVectorTileProviderConnection::encodedUri( conn );
  QCOMPARE( uri, QStringLiteral( "type=xyz&url=https://api.maptiler.com/tiles/v3/%7Bz%7D/%7Bx%7D/%7By%7D.pbf?key%3Dabcdef12345&zmax=18&zmin=0" ) );

  conn.url = QStringLiteral( "file:///home/user/tiles.mbtiles" );
  conn.zMin = 0;
  conn.zMax = 18;
  uri = QgsVectorTileProviderConnection::encodedUri( conn );
  QCOMPARE( uri, QStringLiteral( "type=mbtiles&url=file:///home/user/tiles.mbtiles&zmax=18&zmin=0" ) );
}


QGSTEST_MAIN( TestQgsVectorTileConnection )
#include "testqgsvectortileconnection.moc"
