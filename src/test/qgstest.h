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

#include <QtTest/QtTest>
#include "qgsapplication.h"

#define QGSTEST_MAIN(TestObject) \
  QT_BEGIN_NAMESPACE \
  QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
  QT_END_NAMESPACE \
  int main(int argc, char *argv[]) \
  { \
    QgsApplication app(argc, argv, false); \
    app.init(); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
  }


#define QGSCOMPARENEAR(value,expected,epsilon) { \
    bool _xxxresult = qgsDoubleNear( value, expected, epsilon ); \
    if ( !_xxxresult  ) \
    { \
      qDebug( "Expecting %f got %f (diff %f > %f)", static_cast< double >( expected ), static_cast< double >( value ), std::fabs( static_cast< double >( expected ) - value ), static_cast< double >( epsilon ) ); \
    } \
    QVERIFY( qgsDoubleNear( value, expected, epsilon ) ); \
  }

#define QGSCOMPARENOTNEAR(value,not_expected,epsilon) { \
    bool _xxxresult = qgsDoubleNear( value, not_expected, epsilon ); \
    if ( _xxxresult  ) \
    { \
      qDebug( "Expecting %f to be differerent from %f (diff %f > %f)", static_cast< double >( value ), static_cast< double >( not_expected ), std::fabs( static_cast< double >( not_expected ) - value ), static_cast< double >( epsilon ) ); \
    } \
    QVERIFY( !qgsDoubleNear( value, not_expected, epsilon ) ); \
  }

#define QGSCOMPARENEARPOINT(point1,point2,epsilon) { \
    QGSCOMPARENEAR( point1.x(), point2.x(), epsilon ); \
    QGSCOMPARENEAR( point1.y(), point2.y(), epsilon ); \
  }

#define QGSCOMPARENEARRECTANGLE(rectangle1,rectangle2,epsilon) { \
    QGSCOMPARENEAR( rectangle1.xMinimum(), rectangle2.xMinimum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.xMaximum(), rectangle2.xMaximum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.yMinimum(), rectangle2.yMinimum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.yMaximum(), rectangle2.yMaximum(), epsilon ); \
  }

//sometimes GML attributes are in a different order - but that's ok
#define QGSCOMPAREGML(result,expected) { \
    QCOMPARE( result.replace( QStringLiteral("ts=\" \" cs=\",\""), QStringLiteral("cs=\",\" ts=\" \"") ), expected ); \
  }

/**
 * QGIS unit test utilities.
 * \since QGIS 3.0
 */
namespace QgsTest
{

  //! Returns TRUE if test is running on Travis infrastructure
  bool isTravis()
  {
    return qgetenv( "TRAVIS" ) == QStringLiteral( "true" );
  }

  bool runFlakyTests()
  {
    return qgetenv( "RUN_FLAKY_TESTS" ) == QStringLiteral( "true" );
  }
}

#endif // QGSTEST_H
