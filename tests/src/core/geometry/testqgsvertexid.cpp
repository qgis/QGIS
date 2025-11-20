/***************************************************************************
                         testqgsvertexid.cpp
                         -------------------------
    begin                : November 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgis.h"
#include "qgstest.h"

#include <QObject>


class TestQgsVertexId : public QObject
{
    Q_OBJECT
  private slots:
    void testConstructor();
    void testOperators();
    void testMethods();
};

void TestQgsVertexId::testConstructor()
{
  QVERIFY( !QgsVertexId().isValid() );
  QVERIFY( !QgsVertexId( 0, 0, -1 ).isValid() );
  QVERIFY( !QgsVertexId( 0, -1, 0 ).isValid() );
  QVERIFY( !QgsVertexId( -1, 0, 0 ).isValid() );
  QVERIFY( QgsVertexId( 0, 0, 0 ).isValid() );
}

void TestQgsVertexId::testOperators()
{
  QVERIFY( QgsVertexId() == QgsVertexId() );
  QVERIFY( QgsVertexId( 1, 2, 3 ) == QgsVertexId( 1, 2, 3 ) );
  QVERIFY( !( QgsVertexId( 1, 2, 3 ) == QgsVertexId( 0, 2, 3 ) ) );
  QVERIFY( !( QgsVertexId( 1, 2, 3 ) == QgsVertexId( 1, 0, 3 ) ) );
  QVERIFY( !( QgsVertexId( 1, 2, 3 ) == QgsVertexId( 1, 2, 0 ) ) );

  QVERIFY( !( QgsVertexId() != QgsVertexId() ) );
  QVERIFY( !( QgsVertexId( 1, 2, 3 ) != QgsVertexId( 1, 2, 3 ) ) );
  QVERIFY( QgsVertexId( 1, 2, 3 ) != QgsVertexId( 0, 2, 3 ) );
  QVERIFY( QgsVertexId( 1, 2, 3 ) != QgsVertexId( 1, 0, 3 ) );
  QVERIFY( QgsVertexId( 1, 2, 3 ) != QgsVertexId( 1, 2, 0 ) );
}

void TestQgsVertexId::testMethods()
{
  QVERIFY( !QgsVertexId().partEqual( QgsVertexId() ) );
  QVERIFY( !QgsVertexId().ringEqual( QgsVertexId() ) );
  QVERIFY( !QgsVertexId().vertexEqual( QgsVertexId() ) );

  QVERIFY( QgsVertexId( 1, 1, 1 ).partEqual( QgsVertexId( 1, 0, 0 ) ) );
  QVERIFY( QgsVertexId( 1, 1, 1 ).partEqual( QgsVertexId( 1, -1, -1 ) ) );
  QVERIFY( !QgsVertexId( 1, 1, 1 ).partEqual( QgsVertexId( 0, 1, 1 ) ) );

  QVERIFY( QgsVertexId( 1, 1, 1 ).ringEqual( QgsVertexId( 1, 1, 0 ) ) );
  QVERIFY( QgsVertexId( 1, 1, 1 ).ringEqual( QgsVertexId( 1, 1, -1 ) ) );
  QVERIFY( !QgsVertexId( 1, 1, 1 ).ringEqual( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !QgsVertexId( 1, 1, 1 ).ringEqual( QgsVertexId( -1, 1, 1 ) ) );

  QVERIFY( QgsVertexId( 1, 1, 1 ).vertexEqual( QgsVertexId( 1, 1, 1 ) ) );
  QVERIFY( !QgsVertexId( 1, 1, 1 ).vertexEqual( QgsVertexId( 1, 1, 0 ) ) );
  QVERIFY( !QgsVertexId( 1, 1, 1 ).vertexEqual( QgsVertexId( -1, 0, 1 ) ) );
  QVERIFY( !QgsVertexId( 1, 1, 1 ).vertexEqual( QgsVertexId( 0, -1, 1 ) ) );
}

QGSTEST_MAIN( TestQgsVertexId )
#include "testqgsvertexid.moc"
