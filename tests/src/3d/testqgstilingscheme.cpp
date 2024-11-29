/***************************************************************************
     testqgstilingscheme.cpp
     ----------------------
    Date                 : March 2023
    Copyright            : (C) 2023 by Jean Felder
    Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <cmath>

#include "qgscoordinatereferencesystem.h"
#include "qgstilingscheme.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsTilingScheme : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsTilingScheme()
      : QgsTest( QStringLiteral( "Test QgsTilingScheme" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testTileTransforms();

  private:
};

//runs before all tests
void TestQgsTilingScheme::initTestCase()
{
}

//runs after all tests
void TestQgsTilingScheme::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTilingScheme::testTileTransforms()
{
  const double xMin = 369381.0;
  const double yMin = 6461912.0;
  const double xMax = 388156.0;
  const double yMax = 6518400.0;
  const QgsCoordinateReferenceSystem crs( "EPSG:2154" );
  const QgsRectangle rect( xMin, yMin, xMax, yMax );
  QgsTilingScheme tilingScheme( rect, crs );

  const double xMinFour = 376442.0;
  const double yMinTwo = 6465442.5;
  const double xMaxFour = 378207.25;
  const double yMaxTwo = 6467207.75;
  const int z = 5;

  const double tileSize = std::max( xMax - xMin, yMax - yMin ) / std::pow( 2, z );
  QCOMPARE( tileSize, 1765.25 );

  /*
   * Test tileToMap and mapToTile
   */

  QgsPointXY point425 = tilingScheme.tileToMap( 4, 2, z );
  const QgsPointXY expectedPoint425( xMinFour, yMinTwo );
  QCOMPARE( point425, expectedPoint425 );

  float x = 0.0f;
  float y = 0.0f;
  tilingScheme.mapToTile( expectedPoint425, z, x, y );
  QCOMPARE( x, 4.0f );
  QCOMPARE( y, 2.0f );

  // test (x+1, y+1) tile

  QgsPointXY point535 = tilingScheme.tileToMap( 5, 3, z );
  const QgsPointXY expectedPoint535( xMaxFour, yMaxTwo );
  QCOMPARE( point535, expectedPoint535 );

  tilingScheme.mapToTile( expectedPoint535, z, x, y );
  QCOMPARE( x, 5.0f );
  QCOMPARE( y, 3.0f );

  /*
   * Test tileToExtent and extentToTile
   */

  x = 4.0f;
  y = 2.0f;


  QgsRectangle rec425 = tilingScheme.tileToExtent( ( int ) x, ( int ) y, z );
  QgsRectangle expectedRec425 = QgsRectangle( xMinFour, yMinTwo, xMaxFour, yMaxTwo );
  QCOMPARE( rec425, expectedRec425 );

  int newX = 0;
  int newY = 0;
  int newZ = 0;
  tilingScheme.extentToTile( expectedRec425, newX, newY, newZ );
  QCOMPARE( newX, 4 );
  QCOMPARE( newY, 2 );
  QCOMPARE( newZ, 5 );


  // test (x, y+1) tile
  QgsRectangle rec435 = tilingScheme.tileToExtent( ( int ) x, ( int ) ( y + 1 ), z );
  QgsRectangle expectedRec435 = QgsRectangle( xMinFour, yMinTwo + tileSize, xMaxFour, yMaxTwo + tileSize );
  QCOMPARE( rec435, expectedRec435 );
  tilingScheme.extentToTile( expectedRec435, newX, newY, newZ );
  QCOMPARE( newX, 4 );
  QCOMPARE( newY, 3 );
  QCOMPARE( newZ, 5 );


  // test (x+1, y) tile
  QgsRectangle rec525 = tilingScheme.tileToExtent( ( int ) ( x + 1 ), ( int ) y, z );
  QgsRectangle expectedRec525 = QgsRectangle( xMinFour + tileSize, yMinTwo, xMaxFour + tileSize, yMaxTwo );
  QCOMPARE( rec525, expectedRec525 );

  tilingScheme.extentToTile( expectedRec525, newX, newY, newZ );
  QCOMPARE( newX, 5 );
  QCOMPARE( newY, 2 );
  QCOMPARE( newZ, 5 );


  // test (x, y-1) tile
  QgsRectangle rec415 = tilingScheme.tileToExtent( ( int ) x, ( int ) ( y - 1 ), z );
  QgsRectangle expectedRec415 = QgsRectangle( xMinFour, yMinTwo - tileSize, xMaxFour, yMaxTwo - tileSize );
  QCOMPARE( rec415, expectedRec415 );

  tilingScheme.extentToTile( expectedRec415, newX, newY, newZ );
  QCOMPARE( newX, 4 );
  QCOMPARE( newY, 1 );
  QCOMPARE( newZ, 5 );


  // test (x-1, y) tile
  QgsRectangle rec325 = tilingScheme.tileToExtent( ( int ) ( x - 1 ), ( int ) y, z );
  QgsRectangle expectedRec325 = QgsRectangle( xMinFour - tileSize, yMinTwo, xMaxFour - tileSize, yMaxTwo );
  QCOMPARE( rec325, expectedRec325 );
  tilingScheme.extentToTile( expectedRec325, newX, newY, newZ );
  QCOMPARE( newX, 3 );
  QCOMPARE( newY, 2 );
  QCOMPARE( newZ, 5 );
}

QGSTEST_MAIN( TestQgsTilingScheme )
#include "testqgstilingscheme.moc"
