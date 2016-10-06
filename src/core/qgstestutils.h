/***************************************************************************
    qgstestutils.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyalld dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTESTUTILS_H
#define QGSTESTUTILS_H

#include "qgis.h"
#include "QtTest/qtestcase.h"

/** \ingroup core
 * Assorted helper methods for unit testing.
 * @note added in QGIS 2.16
 */

#define QGSCOMPARENEAR(value,expected,epsilon) { \
    bool _xxxresult = qgsDoubleNear( value, expected, epsilon ); \
    if ( !_xxxresult  ) \
    { \
      qDebug( "Expecting %f got %f (diff %f > %f)", static_cast< double >( expected ), static_cast< double >( value ), qAbs( static_cast< double >( expected ) - value ), static_cast< double >( epsilon ) ); \
    } \
    QVERIFY( qgsDoubleNear( value, expected, epsilon ) ); \
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

#endif // QGSTESTUTILS_H
