/***************************************************************************
     testqgstessellator.cpp
     ----------------------
    Date                 : October 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgstessellator.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the node tool
 */
class TestQgsTessellator : public QObject
{
    Q_OBJECT
  public:
    TestQgsTessellator();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testBasic();

  private:
};

TestQgsTessellator::TestQgsTessellator() = default;


//runs before all tests
void TestQgsTessellator::initTestCase()
{
}

//runs after all tests
void TestQgsTessellator::cleanupTestCase()
{
}

void TestQgsTessellator::testBasic()
{
  QgsPolygonV2 polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsTessellator t( 0, 0, false );
  t.addPolygon( polygon, 0 );

  QVector<float> polygonData = t.data();
  QCOMPARE( polygonData.count(), 2 * 3 * 3 ); // two triangles (3 points with x/y/z coords)
}


QGSTEST_MAIN( TestQgsTessellator )
#include "testqgstessellator.moc"
