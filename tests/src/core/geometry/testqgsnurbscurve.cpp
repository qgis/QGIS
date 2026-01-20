/***************************************************************************
    testqgsnurbscurve.cpp
    --------------------------------------
   Date                 : November 2025
   Copyright            : (C) 2025 by Loïc Bartoletti
   Email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <memory>

#include "qgscurve.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsnurbscurve.h"
#include "qgspoint.h"
#include "qgstest.h"
#include "qgsvertexid.h"
#include "testgeometryutils.h"

#include <QObject>
#include <QString>

class TestQgsNurbsCurve : public QObject
{
    Q_OBJECT
  private slots:
    void emptyConstructor();
    void constructorWithParams();
    void constructorDegree1();
    void constructorDegree2();
    void constructorDegree3();
    void constructorDegree4();
    void constructorDegree5();
    void clone();
    void geometryType();
    void dimension();
    void properties();
    void isBSpline();
    void isRational();
    void isBezier();
    void evaluation();
    void startEndPoints();
    void numPoints();
    void isEmpty();
    void length();
    void hasCurvedSegments();
    void curveToLine();
    void reversed();
    void boundingBox();
    void equals();
    void vertexAt();
    void addZValue();
    void addMValue();
    void dropZValue();
    void dropMValue();
    void toFromWkt();
    void toFromWktZ();
    void toFromWktM();
    void toFromWktZM();
    void toFromWktWithWeights();
    void toFromWktWithKnots();
    void toFromWktEmpty();
    void toFromWktInvalid();
    void toFromWkb();
    void wkbCompatibilityWithSFCGAL();
    void asGeometry();
    void cast();
    void isValidTests();
    void weightAccessTests();
    void evaluateInvalidNurbs();
};

void TestQgsNurbsCurve::emptyConstructor()
{
  QgsNurbsCurve curve;

  QVERIFY( curve.isEmpty() );
  QCOMPARE( curve.numPoints(), 0 );
  QCOMPARE( curve.vertexCount(), 0 );
  QCOMPARE( curve.degree(), 0 );
  QVERIFY( curve.controlPoints().isEmpty() );
  QVERIFY( curve.knots().isEmpty() );
  QVERIFY( curve.weights().isEmpty() );
  QVERIFY( !curve.is3D() );
  QVERIFY( !curve.isMeasure() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurve );
  QCOMPARE( curve.wktTypeStr(), QString( "NurbsCurve" ) );
  QCOMPARE( curve.geometryType(), QString( "NurbsCurve" ) );
  QCOMPARE( curve.dimension(), 1 );
}

void TestQgsNurbsCurve::constructorWithParams()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ), QgsPoint( 3, 1 ) };
  int degree = 2;
  QVector<double> knots { 0, 0, 0, 1, 2, 2, 2 };
  QVector<double> weights { 1, 1, 1, 1 };

  QgsNurbsCurve curve( controlPoints, degree, knots, weights );

  QVERIFY( !curve.isEmpty() );
  QCOMPARE( curve.degree(), 2 );
  QCOMPARE( curve.controlPoints().size(), 4 );
  QCOMPARE( curve.knots().size(), 7 );
  QCOMPARE( curve.weights().size(), 4 );

  QString error;
  QVERIFY( curve.isValid( error, Qgis::GeometryValidityFlags() ) );
}

void TestQgsNurbsCurve::constructorDegree1()
{
  // Linear NURBS curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) };
  int degree = 1;
  QVector<double> knots { 0, 0, 1, 1 };
  QVector<double> weights { 1, 1 };

  QgsNurbsCurve curve( controlPoints, degree, knots, weights );

  QCOMPARE( curve.degree(), 1 );
  QCOMPARE( curve.controlPoints().size(), 2 );
  QCOMPARE( curve.knots().size(), 4 );
}

void TestQgsNurbsCurve::constructorDegree2()
{
  // Quadratic NURBS curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 5, 10 ), QgsPoint( 10, 0 ) };
  int degree = 2;
  QVector<double> knots { 0, 0, 0, 1, 1, 1 };
  QVector<double> weights { 1, 1, 1 };

  QgsNurbsCurve curve( controlPoints, degree, knots, weights );

  QCOMPARE( curve.degree(), 2 );
  QCOMPARE( curve.controlPoints().size(), 3 );
  QCOMPARE( curve.knots().size(), 6 );
}

void TestQgsNurbsCurve::constructorDegree3()
{
  // Cubic NURBS curve (Bézier)
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 3, 10 ), QgsPoint( 7, 10 ), QgsPoint( 10, 0 ) };
  int degree = 3;
  QVector<double> knots { 0, 0, 0, 0, 1, 1, 1, 1 };
  QVector<double> weights { 1, 1, 1, 1 };

  QgsNurbsCurve curve( controlPoints, degree, knots, weights );

  QCOMPARE( curve.degree(), 3 );
  QCOMPARE( curve.controlPoints().size(), 4 );
  QCOMPARE( curve.knots().size(), 8 );
}

void TestQgsNurbsCurve::constructorDegree4()
{
  // Quartic NURBS curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 2, 8 ), QgsPoint( 5, 12 ), QgsPoint( 8, 8 ), QgsPoint( 10, 0 ) };
  int degree = 4;
  QVector<double> knots { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 };
  QVector<double> weights { 1, 1, 1, 1, 1 };

  QgsNurbsCurve curve( controlPoints, degree, knots, weights );

  QCOMPARE( curve.degree(), 4 );
  QCOMPARE( curve.controlPoints().size(), 5 );
  QCOMPARE( curve.knots().size(), 10 );
}

void TestQgsNurbsCurve::constructorDegree5()
{
  // Quintic NURBS curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 2 ), QgsPoint( 3, 4 ), QgsPoint( 5, 6 ), QgsPoint( 7, 4 ), QgsPoint( 8, 2 ), QgsPoint( 10, 0 ) };
  int degree = 5;
  QVector<double> knots { 0, 0, 0, 0, 0, 0, 0.5, 1, 1, 1, 1, 1, 1 };
  QVector<double> weights { 1, 1, 1, 1, 1, 1, 1 };

  QgsNurbsCurve curve( controlPoints, degree, knots, weights );

  QCOMPARE( curve.degree(), 5 );
  QCOMPARE( curve.controlPoints().size(), 7 );
  QCOMPARE( curve.knots().size(), 13 );
}

void TestQgsNurbsCurve::clone()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) };
  QgsNurbsCurve original( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  std::unique_ptr<QgsCurve> cloned( original.clone() );

  QVERIFY( cloned != nullptr );
  QCOMPARE( cloned->geometryType(), QString( "NurbsCurve" ) );
  QVERIFY( cloned.get() != &original );
  QGSCOMPARENEAR( original.length(), cloned->length(), 0.00001 );
}

void TestQgsNurbsCurve::geometryType()
{
  QgsNurbsCurve curve;
  QCOMPARE( curve.geometryType(), QString( "NurbsCurve" ) );
}

void TestQgsNurbsCurve::dimension()
{
  QgsNurbsCurve curve;
  QCOMPARE( curve.dimension(), 1 );
}

void TestQgsNurbsCurve::properties()
{
  // Test B-spline (all weights = 1)
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };
  QgsNurbsCurve bspline( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  QVERIFY( bspline.isBSpline() );
  QVERIFY( !bspline.isRational() );
}

void TestQgsNurbsCurve::isBSpline()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };

  // B-spline: all weights = 1
  QgsNurbsCurve bspline( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );
  QVERIFY( bspline.isBSpline() );

  // Not a B-spline: non-uniform weights
  QgsNurbsCurve rational( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 2, 1 } );
  QVERIFY( !rational.isBSpline() );
}

void TestQgsNurbsCurve::isRational()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };

  // Non-rational: all weights = 1
  QgsNurbsCurve nonRational( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );
  QVERIFY( !nonRational.isRational() );

  // Rational: non-uniform weights
  QgsNurbsCurve rational( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 2, 1 } );
  QVERIFY( rational.isRational() );
}

void TestQgsNurbsCurve::isBezier()
{
  // Cubic Bézier curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 2 ), QgsPoint( 2, 2 ), QgsPoint( 3, 0 ) };
  QgsNurbsCurve bezier( controlPoints, 3, QVector<double> { 0, 0, 0, 0, 1, 1, 1, 1 }, QVector<double> { 1, 1, 1, 1 } );

  QVERIFY( bezier.isBezier() );
}

void TestQgsNurbsCurve::evaluation()
{
  // Simple linear case (degree 1)
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) };
  QgsNurbsCurve linear( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QgsPoint start = linear.evaluate( 0.0 );
  QGSCOMPARENEAR( start.x(), 0.0, 0.00001 );
  QGSCOMPARENEAR( start.y(), 0.0, 0.00001 );

  QgsPoint mid = linear.evaluate( 0.5 );
  QGSCOMPARENEAR( mid.x(), 5.0, 0.00001 );
  QGSCOMPARENEAR( mid.y(), 5.0, 0.00001 );

  QgsPoint end = linear.evaluate( 1.0 );
  QGSCOMPARENEAR( end.x(), 10.0, 0.00001 );
  QGSCOMPARENEAR( end.y(), 10.0, 0.00001 );

  // Test t below 0 returns start point
  QgsPoint belowZero = linear.evaluate( -0.5 );
  QGSCOMPARENEAR( belowZero.x(), 0.0, 0.00001 );
  QGSCOMPARENEAR( belowZero.y(), 0.0, 0.00001 );

  // Test t above 1 returns end point
  QgsPoint aboveOne = linear.evaluate( 1.5 );
  QGSCOMPARENEAR( aboveOne.x(), 10.0, 0.00001 );
  QGSCOMPARENEAR( aboveOne.y(), 10.0, 0.00001 );

  // Test evaluate on empty NURBS returns empty point
  QgsNurbsCurve empty;
  QgsPoint emptyEval = empty.evaluate( 0.5 );
  QVERIFY( emptyEval.isEmpty() );
}

void TestQgsNurbsCurve::startEndPoints()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 5 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QgsPoint start = curve.startPoint();
  QgsPoint end = curve.endPoint();

  QGSCOMPARENEAR( start.x(), 0.0, 0.00001 );
  QGSCOMPARENEAR( start.y(), 0.0, 0.00001 );
  QGSCOMPARENEAR( end.x(), 10.0, 0.00001 );
  QGSCOMPARENEAR( end.y(), 5.0, 0.00001 );
}

void TestQgsNurbsCurve::numPoints()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };
  QgsNurbsCurve curve( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  QVERIFY( curve.numPoints() > 0 );
}

void TestQgsNurbsCurve::isEmpty()
{
  QgsNurbsCurve empty;
  QVERIFY( empty.isEmpty() );

  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );
  QVERIFY( !curve.isEmpty() );
}

void TestQgsNurbsCurve::length()
{
  // Simple linear case: 3-4-5 triangle
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 3, 4 ) };
  QgsNurbsCurve linear( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QGSCOMPARENEAR( linear.length(), 5.0, 0.01 );
}

void TestQgsNurbsCurve::hasCurvedSegments()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };
  QgsNurbsCurve curve( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  QVERIFY( curve.hasCurvedSegments() );
}

void TestQgsNurbsCurve::curveToLine()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };
  QgsNurbsCurve curve( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  std::unique_ptr<QgsLineString> line( curve.curveToLine() );

  QVERIFY( line != nullptr );
  QVERIFY( line->numPoints() > 2 );
  QCOMPARE( line->startPoint().x(), curve.startPoint().x() );
  QCOMPARE( line->startPoint().y(), curve.startPoint().y() );
  QCOMPARE( line->endPoint().x(), curve.endPoint().x() );
  QCOMPARE( line->endPoint().y(), curve.endPoint().y() );
}

void TestQgsNurbsCurve::reversed()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve original( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  std::unique_ptr<QgsCurve> reversed( original.reversed() );

  QCOMPARE( original.startPoint().x(), reversed->endPoint().x() );
  QCOMPARE( original.startPoint().y(), reversed->endPoint().y() );
  QCOMPARE( original.endPoint().x(), reversed->startPoint().x() );
  QCOMPARE( original.endPoint().y(), reversed->startPoint().y() );
}

void TestQgsNurbsCurve::boundingBox()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 5, 10 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve curve( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  QgsRectangle bbox = curve.boundingBox();

  QVERIFY( bbox.xMinimum() <= 0.0 );
  QVERIFY( bbox.xMaximum() >= 10.0 );
  QVERIFY( bbox.yMinimum() <= 0.0 );
  QVERIFY( bbox.yMaximum() >= 5.0 );
}

void TestQgsNurbsCurve::equals()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ), QgsPoint( 2, 0 ) };
  QgsNurbsCurve curve1( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );
  QgsNurbsCurve curve2( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );
  QgsNurbsCurve curve3( QVector<QgsPoint> { QgsPoint( 0, 0 ), QgsPoint( 2, 2 ) }, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QVERIFY( curve1.equals( curve2 ) );
  QVERIFY( !curve1.equals( curve3 ) );
}

void TestQgsNurbsCurve::vertexAt()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  if ( curve.numPoints() > 0 )
  {
    QgsPoint vertex = curve.vertexAt( QgsVertexId( 0, 0, 0 ) );
    QVERIFY( !vertex.isEmpty() );
  }
}

void TestQgsNurbsCurve::addZValue()
{
  // Start with a 2D curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QVERIFY( !curve.is3D() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurve );

  // Add Z value
  QVERIFY( curve.addZValue( 5.0 ) );
  QVERIFY( curve.is3D() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurveZ );

  // Check control points now have Z
  QGSCOMPARENEAR( curve.controlPoints()[0].z(), 5.0, 0.00001 );
  QGSCOMPARENEAR( curve.controlPoints()[1].z(), 5.0, 0.00001 );

  // Adding Z again should fail
  QVERIFY( !curve.addZValue( 10.0 ) );
}

void TestQgsNurbsCurve::addMValue()
{
  // Start with a 2D curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QVERIFY( !curve.isMeasure() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurve );

  // Add M value
  QVERIFY( curve.addMValue( 100.0 ) );
  QVERIFY( curve.isMeasure() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurveM );

  // Check control points now have M
  QGSCOMPARENEAR( curve.controlPoints()[0].m(), 100.0, 0.00001 );
  QGSCOMPARENEAR( curve.controlPoints()[1].m(), 100.0, 0.00001 );

  // Adding M again should fail
  QVERIFY( !curve.addMValue( 200.0 ) );
}

void TestQgsNurbsCurve::dropZValue()
{
  // Start with a 3D curve
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0, 5 ), QgsPoint( 10, 10, 15 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QVERIFY( curve.is3D() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurveZ );

  // Drop Z value
  QVERIFY( curve.dropZValue() );
  QVERIFY( !curve.is3D() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurve );

  // Dropping Z again should fail
  QVERIFY( !curve.dropZValue() );
}

void TestQgsNurbsCurve::dropMValue()
{
  // Start with an M curve
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE M(1, (0 0 10, 10 10 20))"_s ) );

  QVERIFY( curve.isMeasure() );
  QVERIFY( !curve.is3D() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurveM );

  // Drop M value
  QVERIFY( curve.dropMValue() );
  QVERIFY( !curve.isMeasure() );
  QCOMPARE( curve.wkbType(), Qgis::WkbType::NurbsCurve );

  // Dropping M again should fail
  QVERIFY( !curve.dropMValue() );
}

void TestQgsNurbsCurve::toFromWkt()
{
  // Basic NURBS curve
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE(2, (0 0, 5 10, 10 0))"_s ) );

  QCOMPARE( curve.degree(), 2 );
  QCOMPARE( curve.controlPoints().size(), 3 );
  QVERIFY( !curve.isEmpty() );

  QString wkt = curve.asWkt();
  QVERIFY( wkt.contains( u"NurbsCurve"_s ) );
}

void TestQgsNurbsCurve::toFromWktZ()
{
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE Z(2, (0 0 0, 5 10 5, 10 0 0))"_s ) );

  QVERIFY( curve.is3D() );
  QCOMPARE( curve.degree(), 2 );

  QString wkt = curve.asWkt();
  QVERIFY( wkt.contains( u"NurbsCurve Z"_s ) );
}

void TestQgsNurbsCurve::toFromWktM()
{
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE M(2, (0 0 10, 5 10 20, 10 0 30))"_s ) );

  QVERIFY( curve.isMeasure() );
  QVERIFY( !curve.is3D() );
  QCOMPARE( curve.degree(), 2 );

  QString wkt = curve.asWkt();
  QVERIFY( wkt.contains( u"NurbsCurve M"_s ) );
}

void TestQgsNurbsCurve::toFromWktZM()
{
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE ZM(2, (0 0 1 10, 5 10 2 20, 10 0 3 30))"_s ) );

  QVERIFY( curve.is3D() );
  QVERIFY( curve.isMeasure() );
  QCOMPARE( curve.degree(), 2 );

  QString wkt = curve.asWkt();
  QVERIFY( wkt.contains( u"NurbsCurve ZM"_s ) );
}

void TestQgsNurbsCurve::toFromWktWithWeights()
{
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE(2, (0 0, 5 10, 10 0), (1, 2, 1))"_s ) );

  QCOMPARE( curve.degree(), 2 );
  QCOMPARE( curve.controlPoints().size(), 3 );
  QVERIFY( curve.isRational() );
  QCOMPARE( curve.weights().size(), 3 );
}

void TestQgsNurbsCurve::toFromWktWithKnots()
{
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE(2, (0 0, 3 6, 6 3, 9 0), (1, 1, 1, 1), (0, 0, 0, 0.5, 1, 1, 1))"_s ) );

  QCOMPARE( curve.degree(), 2 );
  QCOMPARE( curve.controlPoints().size(), 4 );
  QCOMPARE( curve.knots().size(), 7 );
}

void TestQgsNurbsCurve::toFromWktEmpty()
{
  QgsNurbsCurve curve;
  QVERIFY( curve.fromWkt( u"NURBSCURVE EMPTY"_s ) );

  QVERIFY( curve.isEmpty() );
}

void TestQgsNurbsCurve::toFromWkb()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 5, 10 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve original( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );

  QByteArray wkb = original.asWkb();
  QVERIFY( !wkb.isEmpty() );

  QgsNurbsCurve restored;
  QgsConstWkbPtr wkbPtr( wkb );
  restored.fromWkb( wkbPtr );

  QCOMPARE( restored.degree(), original.degree() );
  QCOMPARE( restored.controlPoints().size(), original.controlPoints().size() );
  QVERIFY( restored.equals( original ) );
}

void TestQgsNurbsCurve::wkbCompatibilityWithSFCGAL()
{
  // WKB test cases compatible with SFCGAL and PostGIS
  struct TestCase
  {
      QString wkt;
      QString wkb;
  };

  QVector<TestCase> testCases = {
    { u"NURBSCURVE(1, (0 0, 10 10))"_s, u"011500000001000000020000000100000000000000000000000000000000000100000000000024400000000000002440000400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(1, (0 0, 5 5, 10 0))"_s, u"011500000001000000030000000100000000000000000000000000000000000100000000000014400000000000001440000100000000000024400000000000000000000500000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(1, (0 0, 10 10), (1, 1))"_s, u"011500000001000000020000000100000000000000000000000000000000000100000000000024400000000000002440000400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(1, (0 0, 10 10), (0.5, 2.0))"_s, u"01150000000100000002000000010000000000000000000000000000000001000000000000e03f01000000000000244000000000000024400100000000000000400400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(1, (0 0, 5 5, 10 0), (1, 1.5, 1))"_s, u"01150000000100000003000000010000000000000000000000000000000000010000000000001440000000000000144001000000000000f83f0100000000000024400000000000000000000500000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(1, (0 0, 10 10), (1, 1), (0, 0, 1, 1))"_s, u"011500000001000000020000000100000000000000000000000000000000000100000000000024400000000000002440000400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(1, (0 0, 5 5, 10 0), (1, 1, 1), (0, 0, 0.5, 1, 1))"_s, u"011500000001000000030000000100000000000000000000000000000000000100000000000014400000000000001440000100000000000024400000000000000000000500000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 5 10, 10 0))"_s, u"0115000000020000000300000001000000000000000000000000000000000001000000000000144000000000000024400001000000000000244000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (1 0, 1 1, 0 1))"_s, u"0115000000020000000300000001000000000000f03f00000000000000000001000000000000f03f000000000000f03f00010000000000000000000000000000f03f0006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (-5 -5, 0 10, 5 -5))"_s, u"011500000002000000030000000100000000000014c000000000000014c00001000000000000000000000000000024400001000000000000144000000000000014c00006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 2 8, 8 2, 10 10))"_s, u"0115000000020000000400000001000000000000000000000000000000000001000000000000004000000000000020400001000000000000204000000000000000400001000000000000244000000000000024400007000000000000000000000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 5 10, 10 0), (1, 1, 1))"_s, u"0115000000020000000300000001000000000000000000000000000000000001000000000000144000000000000024400001000000000000244000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (1 0, 1 1, 0 1), (1, 0.5, 1))"_s, u"0115000000020000000300000001000000000000f03f00000000000000000001000000000000f03f000000000000f03f01000000000000e03f010000000000000000000000000000f03f0006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 5 10, 10 0), (1, 3, 1))"_s, u"01150000000200000003000000010000000000000000000000000000000000010000000000001440000000000000244001000000000000084001000000000000244000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 5 10, 10 0), (0.5, 1, 0.5))"_s, u"01150000000200000003000000010000000000000000000000000000000001000000000000e03f010000000000001440000000000000244000010000000000002440000000000000000001000000000000e03f06000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0.5 0.25, 2.75 5.5, 5.0 0.25), (1, 1.5, 1))"_s, u"0115000000020000000300000001000000000000e03f000000000000d03f00010000000000000640000000000000164001000000000000f83f010000000000001440000000000000d03f0006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 5 10, 10 0), (1, 1, 1), (0, 0, 0, 1, 1, 1))"_s, u"0115000000020000000300000001000000000000000000000000000000000001000000000000144000000000000024400001000000000000244000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (1 0, 1 1, 0 1), (1, 0.5, 1), (0, 0, 0, 1, 1, 1))"_s, u"0115000000020000000300000001000000000000f03f00000000000000000001000000000000f03f000000000000f03f01000000000000e03f010000000000000000000000000000f03f0006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0))"_s, u"01150000000300000004000000010000000000000000000000000000000000010000000000000840000000000000244000010000000000001c40000000000000244000010000000000002440000000000000000000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 2 8, 5 12, 8 8, 10 0))"_s, u"01150000000300000005000000010000000000000000000000000000000000010000000000000040000000000000204000010000000000001440000000000000284000010000000000002040000000000000204000010000000000002440000000000000000000090000000000000000000000000000000000000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (5 0, 10 2.5, 10 7.5, 5 10, 0 7.5, 0 2.5, 5 0))"_s, u"011500000003000000070000000100000000000014400000000000000000000100000000000024400000000000000440000100000000000024400000000000001e40000100000000000014400000000000002440000100000000000000000000000000001e40000100000000000000000000000000000440000100000000000014400000000000000000000b0000000000000000000000000000000000000000000000000000000000000000000000000000000000d03f000000000000e03f000000000000e83f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 5 5, 10 0, 15 -5, 20 0))"_s, u"01150000000300000005000000010000000000000000000000000000000000010000000000001440000000000000144000010000000000002440000000000000000000010000000000002e4000000000000014c000010000000000003440000000000000000000090000000000000000000000000000000000000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0), (1, 1, 1, 1))"_s, u"01150000000300000004000000010000000000000000000000000000000000010000000000000840000000000000244000010000000000001c40000000000000244000010000000000002440000000000000000000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0), (1, 2, 2, 1))"_s, u"011500000003000000040000000100000000000000000000000000000000000100000000000008400000000000002440010000000000000040010000000000001c400000000000002440010000000000000040010000000000002440000000000000000000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0), (0.5, 1.5, 2.0, 0.8))"_s, u"01150000000300000004000000010000000000000000000000000000000001000000000000e03f010000000000000840000000000000244001000000000000f83f010000000000001c4000000000000024400100000000000000400100000000000024400000000000000000019a9999999999e93f080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 2 8, 5 12, 8 8, 10 0), (1, 3, 5, 3, 1))"_s, u"01150000000300000005000000010000000000000000000000000000000000010000000000000040000000000000204001000000000000084001000000000000144000000000000028400100000000000014400100000000000020400000000000002040010000000000000840010000000000002440000000000000000000090000000000000000000000000000000000000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0), (1, 1, 1, 1), (0, 0, 0, 0, 1, 1, 1, 1))"_s, u"01150000000300000004000000010000000000000000000000000000000000010000000000000840000000000000244000010000000000001c40000000000000244000010000000000002440000000000000000000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0), (1, 2, 2, 1), (0, 0, 0, 0, 1, 1, 1, 1))"_s, u"011500000003000000040000000100000000000000000000000000000000000100000000000008400000000000002440010000000000000040010000000000001c400000000000002440010000000000000040010000000000002440000000000000000000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(3, (0 0, 3 10, 7 10, 10 0), (1, 1, 1, 1), (0, 0, 0, 0, 0.3, 0.7, 1, 1))"_s, u"01150000000300000004000000010000000000000000000000000000000000010000000000000840000000000000244000010000000000001c40000000000000244000010000000000002440000000000000000000080000000000000000000000000000000000000000000000000000000000000000000000333333333333d33f666666666666e63f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(4, (0 0, 2 8, 5 12, 8 8, 10 0))"_s, u"011500000004000000050000000100000000000000000000000000000000000100000000000000400000000000002040000100000000000014400000000000002840000100000000000020400000000000002040000100000000000024400000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(4, (0 0, 1 5, 3 8, 7 6, 10 10, 12 0))"_s, u"0115000000040000000600000001000000000000000000000000000000000001000000000000f03f000000000000144000010000000000000840000000000000204000010000000000001c400000000000001840000100000000000024400000000000002440000100000000000028400000000000000000000b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(4, (0 0, 2 8, 5 12, 8 8, 10 0), (1, 1.5, 2, 1.5, 1))"_s, u"01150000000400000005000000010000000000000000000000000000000000010000000000000040000000000000204001000000000000f83f0100000000000014400000000000002840010000000000000040010000000000002040000000000000204001000000000000f83f0100000000000024400000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(4, (0 0, 2 8, 5 12, 8 8, 10 0), (1, 1, 1, 1, 1), (0, 0, 0, 0, 0, 1, 1, 1, 1, 1))"_s, u"011500000004000000050000000100000000000000000000000000000000000100000000000000400000000000002040000100000000000014400000000000002840000100000000000020400000000000002040000100000000000024400000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(5, (0 0, 1 2, 3 4, 5 6, 7 4, 8 2, 10 0), (1, 1, 1, 1, 1, 1, 1))"_s, u"0115000000050000000700000001000000000000000000000000000000000001000000000000f03f000000000000004000010000000000000840000000000000104000010000000000001440000000000000184000010000000000001c400000000000001040000100000000000020400000000000000040000100000000000024400000000000000000000d000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    // 3D (Z) test cases
    { u"NURBSCURVE Z(1, (0 0 0, 10 10 5))"_s, u"01fd0300000100000002000000010000000000000000000000000000000000000000000000000001000000000000244000000000000024400000000000001440000400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE Z(2, (0 0 0, 5 10 5, 10 0 0))"_s, u"01fd030000020000000300000001000000000000000000000000000000000000000000000000000100000000000014400000000000002440000000000000144000010000000000002440000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE Z(2, (0 0 0, 5 10 5, 10 0 0), (1, 2, 1))"_s, u"01fd0300000200000003000000010000000000000000000000000000000000000000000000000001000000000000144000000000000024400000000000001440010000000000000040010000000000002440000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE Z(3, (0 0 0, 3 10 3, 7 10 7, 10 0 10))"_s, u"01fd030000030000000400000001000000000000000000000000000000000000000000000000000100000000000008400000000000002440000000000000084000010000000000001c4000000000000024400000000000001c40000100000000000024400000000000000000000000000000244000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE Z(3, (0 0 0, 3 10 3, 7 10 7, 10 0 10), (1, 1.5, 1.5, 1), (0, 0, 0, 0, 1, 1, 1, 1))"_s, u"01fd030000030000000400000001000000000000000000000000000000000000000000000000000100000000000008400000000000002440000000000000084001000000000000f83f010000000000001c4000000000000024400000000000001c4001000000000000f83f0100000000000024400000000000000000000000000000244000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    // M test cases
    { u"NURBSCURVE M(1, (0 0 0, 10 10 3600))"_s, u"01e5070000010000000200000001000000000000000000000000000000000000000000000000000100000000000024400000000000002440000000000020ac40000400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE M(2, (0 0 0, 5 10 1800, 10 0 3600))"_s, u"01e50700000200000003000000010000000000000000000000000000000000000000000000000001000000000000144000000000000024400000000000209c40000100000000000024400000000000000000000000000020ac400006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE M(2, (0 0 0, 5 10 1800, 10 0 3600), (1, 2, 1))"_s, u"01e50700000200000003000000010000000000000000000000000000000000000000000000000001000000000000144000000000000024400000000000209c400100000000000000400100000000000024400000000000000000000000000020ac400006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    // ZM test cases
    { u"NURBSCURVE ZM(1, (0 0 0 0, 10 10 5 3600))"_s, u"01cd0b000001000000020000000100000000000000000000000000000000000000000000000000000000000000000001000000000000244000000000000024400000000000001440000000000020ac40000400000000000000000000000000000000000000000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE ZM(2, (0 0 0 0, 5 10 5 1800, 10 0 0 3600))"_s, u"01cd0b0000020000000300000001000000000000000000000000000000000000000000000000000000000000000000010000000000001440000000000000244000000000000014400000000000209c400001000000000000244000000000000000000000000000000000000000000020ac400006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE ZM(2, (0 0 0 0, 5 10 5 1800, 10 0 0 3600), (1, 2, 1))"_s, u"01cd0b0000020000000300000001000000000000000000000000000000000000000000000000000000000000000000010000000000001440000000000000244000000000000014400000000000209c4001000000000000004001000000000000244000000000000000000000000000000000000000000020ac400006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE ZM(3, (0 0 0 0, 3 10 3 1200, 7 10 7 2400, 10 0 10 3600), (1, 1.5, 1.5, 1), (0, 0, 0, 0, 1, 1, 1, 1))"_s, u"01cd0b0000030000000400000001000000000000000000000000000000000000000000000000000000000000000000010000000000000840000000000000244000000000000008400000000000c0924001000000000000f83f010000000000001c4000000000000024400000000000001c400000000000c0a24001000000000000f83f01000000000000244000000000000000000000000000002440000000000020ac4000080000000000000000000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f000000000000f03f"_s },
    // Additional rational curve test cases
    { u"NURBSCURVE(2, (1 0, 1 1, 0 1), (1, 0.5, 1), (0, 0, 0, 1, 1, 1))"_s, u"0115000000020000000300000001000000000000f03f00000000000000000001000000000000f03f000000000000f03f01000000000000e03f010000000000000000000000000000f03f0006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (2 0, 2 1, 0 1), (1, 0.5, 1), (0, 0, 0, 1, 1, 1))"_s, u"01150000000200000003000000010000000000000040000000000000000000010000000000000040000000000000f03f01000000000000e03f010000000000000000000000000000f03f0006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 1 1, 2 0), (1, 1, 1), (0, 0, 0, 1, 1, 1))"_s, u"0115000000020000000300000001000000000000000000000000000000000001000000000000f03f000000000000f03f0001000000000000004000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 5 5, 10 0))"_s, u"0115000000020000000300000001000000000000000000000000000000000001000000000000144000000000000014400001000000000000244000000000000000000006000000000000000000000000000000000000000000000000000000000000000000f03f000000000000f03f000000000000f03f"_s },
    { u"NURBSCURVE(2, (0 0, 1 2, 2 1, 3 3, 4 2, 5 4, 6 3, 7 5, 8 4, 9 6, 10 5))"_s, u"0115000000020000000b00000001000000000000000000000000000000000001000000000000f03f000000000000004000010000000000000040000000000000f03f00010000000000000840000000000000084000010000000000001040000000000000004000010000000000001440000000000000104000010000000000001840000000000000084000010000000000001c400000000000001440000100000000000020400000000000001040000100000000000022400000000000001840000100000000000024400000000000001440000e0000000000000000000000000000000000000000000000000000001cc7711cc771bc3f1cc7711cc771cc3f555555555555d53f1cc7711cc771dc3f721cc7711cc7e13f555555555555e53f398ee3388ee3e83f1cc7711cc771ec3f000000000000f03f000000000000f03f000000000000f03f"_s },
  };

  int wktSuccessCount = 0;
  int wkbSuccessCount = 0;

  for ( const TestCase &testCase : testCases )
  {
    // Test 1: WKT parsing
    QgsGeometry geomFromWkt = QgsGeometry::fromWkt( testCase.wkt );
    if ( !geomFromWkt.isNull() )
    {
      wktSuccessCount++;

      // Test 3: Convert to WKB and compare
      QByteArray wkbFromWkt = geomFromWkt.asWkb();
      QString wkbHex = QString::fromLatin1( wkbFromWkt.toHex() );
      QCOMPARE( wkbHex, testCase.wkb );
    }

    // Test 2: WKB parsing
    QByteArray wkbBytes = QByteArray::fromHex( testCase.wkb.toLatin1() );
    QgsGeometry geomFromWkb;
    geomFromWkb.fromWkb( wkbBytes );
    if ( !geomFromWkb.isNull() )
    {
      wkbSuccessCount++;

      // Test 4: Convert to WKB and compare (round-trip)
      QByteArray wkbRoundTrip = geomFromWkb.asWkb();
      QString wkbHexRoundTrip = QString::fromLatin1( wkbRoundTrip.toHex() );
      QCOMPARE( wkbHexRoundTrip, testCase.wkb );
    }
  }

  // All test cases should pass
  QCOMPARE( wktSuccessCount, testCases.size() );
  QCOMPARE( wkbSuccessCount, testCases.size() );
}

void TestQgsNurbsCurve::asGeometry()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QgsGeometry geom( curve.clone() );

  QVERIFY( !geom.isNull() );
  QVERIFY( geom.length() > 0.0 );
}

void TestQgsNurbsCurve::cast()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve curve( controlPoints, 1, QVector<double> { 0, 0, 1, 1 }, QVector<double> { 1, 1 } );

  QVERIFY( QgsNurbsCurve::cast( &curve ) );

  QgsLineString line;
  QVERIFY( !QgsNurbsCurve::cast( &line ) );
}

void TestQgsNurbsCurve::toFromWktInvalid()
{
  // Test parsing invalid WKT strings
  QgsNurbsCurve curve;

  // Empty/malformed WKT
  // Note: "NURBSCURVE" alone is equivalent to "NURBSCURVE EMPTY" - this is valid but empty
  QVERIFY( curve.fromWkt( u"NURBSCURVE"_s ) );
  QVERIFY( curve.isEmpty() );

  // Empty parentheses should also work and create an empty curve
  QVERIFY( curve.fromWkt( u"NURBSCURVE()"_s ) );
  QVERIFY( curve.isEmpty() );

  // Invalid type should fail
  QVERIFY( !curve.fromWkt( u"NOT_A_NURBS(1, (0 0, 10 10))"_s ) );

  // Missing degree
  QVERIFY( !curve.fromWkt( u"NURBSCURVE((0 0, 10 10))"_s ) );

  // Degree without control points
  QVERIFY( !curve.fromWkt( u"NURBSCURVE(2, ())"_s ) );

  // Too few control points for degree
  QVERIFY( !curve.fromWkt( u"NURBSCURVE(3, (0 0, 10 10))"_s ) ); // degree 3 requires >= 4 points

  // Mixed dimensions in WKT - the parser currently accepts mixed dimensions
  // Note: This is permissive behavior - points with extra coordinates
  // (like "10 10 5" in a 2D curve) will have those extra values stored
  QgsNurbsCurve mixedDimsCurve;
  bool parsedMixed = mixedDimsCurve.fromWkt( u"NURBSCURVE(1, (0 0, 10 10 5))"_s );
  // Parsing should succeed - the third coordinate on second point is ignored for 2D type
  if ( parsedMixed )
  {
    QVERIFY( !mixedDimsCurve.isEmpty() );
  }

  // Test Z type with 2D points - should add default Z
  QgsNurbsCurve zCurveWith2DPoints;
  QVERIFY( zCurveWith2DPoints.fromWkt( u"NURBSCURVEZ(1, (0 0, 10 10))"_s ) );
  QVERIFY( zCurveWith2DPoints.is3D() );
}

void TestQgsNurbsCurve::isValidTests()
{
  QString error;

  // Valid NURBS
  QVector<QgsPoint> validControlPoints { QgsPoint( 0, 0 ), QgsPoint( 5, 10 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve validCurve( validControlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 1, 1 } );
  QVERIFY( validCurve.isValid( error, Qgis::GeometryValidityFlags() ) );
  QVERIFY( error.isEmpty() );

  // Empty curve is invalid
  QgsNurbsCurve emptyCurve;
  QVERIFY( !emptyCurve.isValid( error, Qgis::GeometryValidityFlags() ) );

  // Degree > number of control points - 1
  QgsNurbsCurve invalidDegree;
  invalidDegree.setControlPoints( QVector<QgsPoint> { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) } );
  invalidDegree.setDegree( 5 ); // Degree 5 requires >= 6 control points
  invalidDegree.setKnots( QVector<double> { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 } );
  invalidDegree.setWeights( QVector<double> { 1, 1 } );
  QVERIFY( !invalidDegree.isValid( error, Qgis::GeometryValidityFlags() ) );

  // Wrong knot vector size
  QgsNurbsCurve wrongKnotSize;
  wrongKnotSize.setControlPoints( validControlPoints );
  wrongKnotSize.setDegree( 2 );
  wrongKnotSize.setKnots( QVector<double> { 0, 0, 1, 1 } ); // Should be 6 elements for 3 points + degree 2
  wrongKnotSize.setWeights( QVector<double> { 1, 1, 1 } );
  QVERIFY( !wrongKnotSize.isValid( error, Qgis::GeometryValidityFlags() ) );

  // Non-decreasing knots check (invalid)
  QgsNurbsCurve decreasingKnots;
  decreasingKnots.setControlPoints( validControlPoints );
  decreasingKnots.setDegree( 2 );
  decreasingKnots.setKnots( QVector<double> { 0, 0, 1, 0.5, 1, 1 } ); // Decreasing at index 3
  decreasingKnots.setWeights( QVector<double> { 1, 1, 1 } );
  QVERIFY( !decreasingKnots.isValid( error, Qgis::GeometryValidityFlags() ) );

  // Wrong weights vector size
  QgsNurbsCurve wrongWeightsSize;
  wrongWeightsSize.setControlPoints( validControlPoints );
  wrongWeightsSize.setDegree( 2 );
  wrongWeightsSize.setKnots( QVector<double> { 0, 0, 0, 1, 1, 1 } );
  wrongWeightsSize.setWeights( QVector<double> { 1, 1 } ); // Should be 3 elements
  QVERIFY( !wrongWeightsSize.isValid( error, Qgis::GeometryValidityFlags() ) );
}

void TestQgsNurbsCurve::weightAccessTests()
{
  QVector<QgsPoint> controlPoints { QgsPoint( 0, 0 ), QgsPoint( 5, 10 ), QgsPoint( 10, 0 ) };
  QgsNurbsCurve curve( controlPoints, 2, QVector<double> { 0, 0, 0, 1, 1, 1 }, QVector<double> { 1, 2, 1 } );

  // Valid indices
  QGSCOMPARENEAR( curve.weight( 0 ), 1.0, 0.00001 );
  QGSCOMPARENEAR( curve.weight( 1 ), 2.0, 0.00001 );
  QGSCOMPARENEAR( curve.weight( 2 ), 1.0, 0.00001 );

  // Invalid indices return 1.0
  QGSCOMPARENEAR( curve.weight( -1 ), 1.0, 0.00001 );
  QGSCOMPARENEAR( curve.weight( 3 ), 1.0, 0.00001 );
  QGSCOMPARENEAR( curve.weight( 100 ), 1.0, 0.00001 );

  // setWeight tests
  QVERIFY( curve.setWeight( 1, 3.0 ) );
  QGSCOMPARENEAR( curve.weight( 1 ), 3.0, 0.00001 );

  // Invalid index
  QVERIFY( !curve.setWeight( -1, 2.0 ) );
  QVERIFY( !curve.setWeight( 5, 2.0 ) );

  // Invalid weight (zero or negative)
  QVERIFY( !curve.setWeight( 0, 0.0 ) );
  QVERIFY( !curve.setWeight( 0, -1.0 ) );
}

void TestQgsNurbsCurve::evaluateInvalidNurbs()
{
  // Test evaluate on invalid NURBS returns empty point
  QgsNurbsCurve invalidCurve;
  invalidCurve.setControlPoints( QVector<QgsPoint> { QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) } );
  invalidCurve.setDegree( 5 ); // Invalid: degree > n-1
  invalidCurve.setKnots( QVector<double> { 0, 0, 1, 1 } );
  invalidCurve.setWeights( QVector<double> { 1, 1 } );

  QString error;
  QVERIFY( !invalidCurve.isValid( error, Qgis::GeometryValidityFlags() ) );

  QgsPoint result = invalidCurve.evaluate( 0.5 );
  QVERIFY( result.isEmpty() );
}

QGSTEST_MAIN( TestQgsNurbsCurve )
#include "testqgsnurbscurve.moc"
