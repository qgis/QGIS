/***************************************************************************
     testqgsgeometryutilsbase.cpp
     --------------------------------------
    Date                 : 2023-12-15
    Copyright            : (C) 2023 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
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
#include "qgsgeometryutils_base.h"

class TestQgsGeometryUtilsBase : public QObject
{
    Q_OBJECT

  private slots:
    void testFuzzyEqual();
    void testFuzzyDistanceEqual();
};
void TestQgsGeometryUtilsBase::testFuzzyEqual()
{
  QVERIFY( QgsGeometryUtilsBase::fuzzyEqual( 0.1, 1.0, 2.0, 1.0, 2.0 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyEqual( 1.0, 1.0, 2.0, 1.5, 2.5 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyEqual( 0.01, 1.0, 2.0, 1.001, 2.001 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyEqual( 0.1, 1.0, 2.0, 1.5, 2.0 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyEqual( 0.4, 1.0, 2.0, 1.5, 2.5 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyEqual( 0.001, 1.0, 2.0, 1.1, 2.1 ) );
}

void TestQgsGeometryUtilsBase::testFuzzyDistanceEqual()
{
  QVERIFY( QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.1, 1.0, 2.0, 1.0, 2.0 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyDistanceEqual( 2.0, 1.0, 2.0, 1.5, 2.1 ) );

  QVERIFY( QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.01, 1.0, 2.0, 1.001, 2.001 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.1, 1.0, 2.0, 1.5, 2.0 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.2, 1.0, 2.0, 1.5, 2.5 ) );

  QVERIFY( !QgsGeometryUtilsBase::fuzzyDistanceEqual( 0.001, 1.0, 2.0, 1.1, 2.1 ) );
}

QGSTEST_MAIN( TestQgsGeometryUtilsBase )
#include "testqgsgeometryutilsbase.moc"
