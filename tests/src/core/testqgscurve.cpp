/***************************************************************************
     testqgscurve.cpp
     --------------------------------------
    Date                 : 21 July 2017
    Copyright            : (C) 2017-2019 by Sandro Santilli
    Email                : strk @ kbt.io
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QObject>
#include <QString>
#include <QApplication>
#include <memory> // for unique_ptr

//qgis includes...
#include "qgsabstractgeometry.h"
#include "qgscircularstring.h"
#include "qgsgeometry.h"
#include "qgsgeometryfactory.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgstest.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the operations on curve geometries
 */
class TestQgsCurve : public QObject
{
    Q_OBJECT

  public:
    TestQgsCurve() = default;;

  private slots:
    //void initTestCase();// will be called before the first testfunction is executed.
    //void cleanupTestCase();// will be called after the last testfunction was executed.
    //void init();// will be called before each testfunction is executed.
    //void cleanup();// will be called after every testfunction.

    void curveToLine();
};


#define TEST_C2L(circularString, tol, toltype, exp, prec) { \
    std::unique_ptr< QgsLineString > lineString( \
        circularString->curveToLine(tol, toltype) \
                                               ); \
    QVERIFY( lineString.get() ); \
    QString wkt_out = lineString->asWkt(prec); \
    QCOMPARE( wkt_out, QString( exp ) ); \
    /* Test reverse */ \
    std::unique_ptr< QgsCircularString > reversed( \
        circularString->reversed() \
                                                 ); \
    lineString.reset( \
                      reversed->curveToLine(tol, toltype) \
                    ); \
    wkt_out = lineString->asWkt(prec); \
    lineString.reset( \
                      reversed->curveToLine(tol, toltype) \
                    ); \
    std::unique_ptr< QgsLineString > expgeom( \
        dynamic_cast<QgsLineString *>( \
                                       QgsGeometryFactory::geomFromWkt( exp ).release() \
                                     ) \
                                            ); \
    expgeom.reset( expgeom->reversed() ); \
    QString exp_reversed = expgeom->asWkt(prec); \
    QCOMPARE( wkt_out, exp_reversed ); \
  }

void TestQgsCurve::curveToLine()
{
  std::unique_ptr< QgsCircularString > circularString;

  /* input: 2 quadrants arc (180 degrees, PI radians) */
  circularString.reset( dynamic_cast< QgsCircularString *>(
                          QgsGeometryFactory::geomFromWkt( QStringLiteral(
                                "CIRCULARSTRING(0 0,100 100,200 0)"
                              )
                                                         ).release()
                        ) );
  QVERIFY( circularString.get() );

  /* op: Maximum of 10 units of difference, symmetric */
  TEST_C2L( circularString, 10, QgsAbstractGeometry::MaximumDifference,
            "LineString (0 0, 29.29 70.71, 100 100, 170.71 70.71, 200 0)", 2 );

  /* op: Maximum of 300 units (higher than sagitta) of difference, symmetric */
  /* See https://github.com/qgis/QGIS/issues/31832 */
  TEST_C2L( circularString, 300, QgsAbstractGeometry::MaximumDifference,
            "LineString (0 0, 200 0)", 2 );

  /* op: Maximum of M_PI / 8 degrees of angle, (a)symmetric */
  /* See https://github.com/qgis/QGIS/issues/24616 */
  TEST_C2L( circularString, M_PI / 8, QgsAbstractGeometry::MaximumAngle,
            "LineString (0 0, 7.61 38.27, 29.29 70.71, 61.73 92.39, 100 100, 138.27 92.39, 170.71 70.71, 192.39 38.27, 200 0)", 2 );

  /* op: Maximum of 70 degrees of angle, symmetric */
  /* See https://github.com/qgis/QGIS/issues/24621 */
  TEST_C2L( circularString, 70 * M_PI / 180, QgsAbstractGeometry::MaximumAngle,
            "LineString (0 0, 50 86.6, 150 86.6, 200 0)", 2 );

  /* input: 2 arcs of 2 quadrants each (180 degrees + 180 degrees other direction) */
  circularString.reset( dynamic_cast<QgsCircularString *>(
                          QgsGeometryFactory::geomFromWkt( QStringLiteral(
                                "CIRCULARSTRING(0 0,100 100,200 0,300 -100,400 0)"
                              ) ).release()
                        ) );
  QVERIFY( circularString.get() );

  /* op: Maximum of M_PI / 3 degrees of angle */
  TEST_C2L( circularString, M_PI / 3, QgsAbstractGeometry::MaximumAngle,
            "LineString (0 0, 50 86.6, 150 86.6, 200 0, 250 -86.6, 350 -86.6, 400 0)", 2 );
}


QGSTEST_MAIN( TestQgsCurve )
#include "testqgscurve.moc"
