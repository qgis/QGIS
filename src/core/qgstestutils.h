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

/**
 * Assorted helper methods for unit testing.
 * @note added in QGIS 2.16
 */

#define QGSCOMPARENEAR(a,b,epsilon) { \
  bool _xxxresult = qgsDoubleNear( a, b, epsilon ); \
  if ( !_xxxresult  ) \
  { \
    qDebug( "Expecting %f got %f (diff %f > %f)", static_cast< double >( a ), static_cast< double >( b ), qAbs( static_cast< double >( a ) - b ), static_cast< double >( epsilon ) ); \
  } \
  QVERIFY( qgsDoubleNear( a, b, epsilon ) ); \
}


#endif // QGSTESTUTILS_H
