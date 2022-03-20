/***************************************************************************
     testqgscoordinateutils.cpp
     --------------------------------------
    Date                 : March 2022
    Copyright            : (C) 2022 Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include "qgstest.h"
#include "qgscoordinateutils.h"

class TestQgsCoordinateUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testDegreeWithSuffix();
};


void TestQgsCoordinateUtils::testDegreeWithSuffix()
{
  bool ok = false ;
  bool isEasting = false;
  double value = 0.0;

  value = QgsCoordinateUtils::degreeToDecimal( QStringLiteral( "1.234W" ), &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, true );
  QCOMPARE( value, -1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( QStringLiteral( "-1.234 w" ), &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, true );
  QCOMPARE( value, -1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( QStringLiteral( "1.234s" ), &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, false );
  QCOMPARE( value, -1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( QStringLiteral( "1.234N" ), &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, false );
  QCOMPARE( value, 1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( QStringLiteral( "1.234 e" ), &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, true );
  QCOMPARE( value, 1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( QStringLiteral( "bad string" ), &ok, &isEasting );
  QCOMPARE( ok, false );
  QCOMPARE( value, 0.0 );
}

QGSTEST_MAIN( TestQgsCoordinateUtils )
#include "testqgscoordinateutils.moc"
