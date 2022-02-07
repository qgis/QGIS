/***************************************************************************
  qgstest - %{Cpp:License:ClassName}

 ---------------------
 begin                : 5.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTEST_H
#define QGSTEST_H

#include <QtTest/QTest>
#include "qgsapplication.h"

#include "qgsabstractgeometry.h"
#include "qgscurve.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgslinestring.h"
#include "qgsgeometrycollection.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultisurface.h"
#include "qgsmultipolygon.h"
#include "qgspoint.h"
#include "qgssurface.h"
#include "qgscurvepolygon.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgsrectangle.h"
#include "qgsregularpolygon.h"


#define QGSTEST_MAIN(TestObject) \
  QT_BEGIN_NAMESPACE \
  QT_END_NAMESPACE \
  int main(int argc, char *argv[]) \
  { \
    QgsApplication app(argc, argv, false); \
    app.init(); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
  }


#define QGSCOMPARENEAR(value,expected,epsilon) { \
    bool _xxxresult = qgsDoubleNear( value, expected, epsilon ); \
    if ( !_xxxresult  ) \
    { \
      qDebug( "Expecting %.10f got %.10f (diff %.10f > %.10f)", static_cast< double >( expected ), static_cast< double >( value ), std::fabs( static_cast< double >( expected ) - value ), static_cast< double >( epsilon ) ); \
    } \
    QVERIFY( qgsDoubleNear( value, expected, epsilon ) ); \
  }(void)(0)

#define QGSCOMPARENOTNEAR(value,not_expected,epsilon) { \
    bool _xxxresult = qgsDoubleNear( value, not_expected, epsilon ); \
    if ( _xxxresult  ) \
    { \
      qDebug( "Expecting %f to be differerent from %f (diff %f > %f)", static_cast< double >( value ), static_cast< double >( not_expected ), std::fabs( static_cast< double >( not_expected ) - value ), static_cast< double >( epsilon ) ); \
    } \
    QVERIFY( !qgsDoubleNear( value, not_expected, epsilon ) ); \
  }(void)(0)

#define QGSCOMPARENEARPOINT(point1,point2,epsilon) { \
    QGSCOMPARENEAR( point1.x(), point2.x(), epsilon ); \
    QGSCOMPARENEAR( point1.y(), point2.y(), epsilon ); \
  }(void)(0)

#define QGSCOMPARENEARRECTANGLE(rectangle1,rectangle2,epsilon) { \
    QGSCOMPARENEAR( rectangle1.xMinimum(), rectangle2.xMinimum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.xMaximum(), rectangle2.xMaximum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.yMinimum(), rectangle2.yMinimum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.yMaximum(), rectangle2.yMaximum(), epsilon ); \
  }(void)(0)

//sometimes GML attributes are in a different order - but that's ok
#define QGSCOMPAREGML(result,expected) { \
    QCOMPARE( result.replace( QStringLiteral("ts=\" \" cs=\",\""), QStringLiteral("cs=\",\" ts=\" \"") ), expected ); \
  }(void)(0)

/**
 * QGIS unit test utilities.
 * \since QGIS 3.0
 */
namespace QgsTest
{

  //! Returns TRUE if test is running on a CI infrastructure
  bool isCIRun()
  {
    return qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" );
  }

  bool runFlakyTests()
  {
    return qgetenv( "RUN_FLAKY_TESTS" ) == QStringLiteral( "true" );
  }
}

/**
 * For QCOMPARE pretty printing
 */
char *toString( const QgsAbstractGeometry &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCurve &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCircularString &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCompoundCurve &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsLineString &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsGeometryCollection &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiCurve &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiLineString &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiPoint &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiSurface &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiPolygon &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsPoint &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsPointXY &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsSurface &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCurvePolygon &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsPolygon &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsRegularPolygon &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsTriangle &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsRectangle &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsEllipse &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsCircle &geom )
{
  return QTest::toString( geom.toString() );
}

#endif // QGSTEST_H
