/***************************************************************************
     testqgsmulticurve.cpp
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
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
#include <QString>

#include "qgscircularstring.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"

#include "testgeometryutils.h"

class TestQgsMultiCurve: public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void addGeometry();
    void addGeometryZM();
    void addGeometryDimensionPreservation();
    void addGeometryDimensionPreservationZ();
    void addGeometryDimensionPreservationM();
    void addGeometryDimensionPreservationZM();
    void insertGeometry();
    void curveN();
    void clear();
    void clone();
    void copy();
    void assignment();
    void cast();
    void boundary();
    void reversed();
    void segmentize();
    void toCurveType();
    void toFromWKB();
    void toFromWKBWithZM();
    void toFromWKT();
    void exportImport();
};

void TestQgsMultiCurve::constructor()
{
  QgsMultiCurve mc;

  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.nCoordinates(), 0 );
  QCOMPARE( mc.ringCount(), 0 );
  QCOMPARE( mc.partCount(), 0 );
  QVERIFY( !mc.is3D() );
  QVERIFY( !mc.isMeasure() );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( mc.wktTypeStr(), QString( "MultiCurve" ) );
  QCOMPARE( mc.geometryType(), QString( "MultiCurve" ) );
  QCOMPARE( mc.dimension(), 0 );
  QVERIFY( !mc.hasCurvedSegments() );
  QCOMPARE( mc.area(), 0.0 );
  QCOMPARE( mc.perimeter(), 0.0 );
  QCOMPARE( mc.numGeometries(), 0 );
  QVERIFY( !mc.geometryN( 0 ) );
  QVERIFY( !mc.geometryN( -1 ) );
  QCOMPARE( mc.vertexCount( 0, 0 ), 0 );
  QCOMPARE( mc.vertexCount( 0, 1 ), 0 );
  QCOMPARE( mc.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiCurve::addGeometry()
{
  QgsMultiCurve mc;

  //try with nullptr
  mc.addGeometry( nullptr );

  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.nCoordinates(), 0 );
  QCOMPARE( mc.ringCount(), 0 );
  QCOMPARE( mc.partCount(), 0 );
  QCOMPARE( mc.numGeometries(), 0 );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
  QVERIFY( !mc.geometryN( 0 ) );
  QVERIFY( !mc.geometryN( -1 ) );

  // not a curve
  QVERIFY( !mc.addGeometry( new QgsPoint() ) );

  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.nCoordinates(), 0 );
  QCOMPARE( mc.ringCount(), 0 );
  QCOMPARE( mc.partCount(), 0 );
  QCOMPARE( mc.numGeometries(), 0 );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
  QVERIFY( !mc.geometryN( 0 ) );
  QVERIFY( !mc.geometryN( -1 ) );

  //valid geometry
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 )
                  << QgsPoint( 2, 11 ) << QgsPoint( 1, 12 ) );
  mc.addGeometry( part.clone() );

  QVERIFY( !mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 1 );
  QCOMPARE( mc.nCoordinates(), 3 );
  QCOMPARE( mc.ringCount(), 1 );
  QCOMPARE( mc.partCount(), 1 );
  QVERIFY( !mc.is3D() );
  QVERIFY( !mc.isMeasure() );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( mc.wktTypeStr(), QString( "MultiCurve" ) );
  QCOMPARE( mc.geometryType(), QString( "MultiCurve" ) );
  QCOMPARE( mc.dimension(), 1 );
  QVERIFY( mc.hasCurvedSegments() );
  QCOMPARE( mc.area(), 0.0 );
  QCOMPARE( mc.perimeter(), 0.0 );
  QVERIFY( mc.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc.geometryN( 0 ) ), part );
  QVERIFY( !mc.geometryN( 100 ) );
  QVERIFY( !mc.geometryN( -1 ) );
  QCOMPARE( mc.vertexCount( 0, 0 ), 3 );
  QCOMPARE( mc.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiCurve::addGeometryZM()
{
  //initial adding of geometry should set z/m type
  QgsMultiCurve mc;
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 31, 3 ) );
  mc.addGeometry( part.clone() );

  QVERIFY( mc.is3D() );
  QVERIFY( !mc.isMeasure() );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZ );
  QCOMPARE( mc.wktTypeStr(), QString( "MultiCurveZ" ) );
  QCOMPARE( mc.geometryType(), QString( "MultiCurve" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( mc.geometryN( 0 ) ) ), part );

  mc.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 31, 0, 3 ) );
  mc.addGeometry( part.clone() );

  QVERIFY( !mc.is3D() );
  QVERIFY( mc.isMeasure() );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveM );
  QCOMPARE( mc.wktTypeStr(), QString( "MultiCurveM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( mc.geometryN( 0 ) ) ), part );

  mc.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 31, 4, 5 ) );
  mc.addGeometry( part.clone() );

  QVERIFY( mc.is3D() );
  QVERIFY( mc.isMeasure() );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( mc.wktTypeStr(), QString( "MultiCurveZM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( mc.geometryN( 0 ) ) ), part );

  //add another part
  mc.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 )
                  << QgsPoint( 2, 11 ) << QgsPoint( 1, 20 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.vertexCount( 0, 0 ), 3 );

  part.setPoints( QgsPointSequence() << QgsPoint( 9, 12 )
                  << QgsPoint( 3, 13 ) << QgsPoint( 9, 20 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.vertexCount( 1, 0 ), 3 );
  QCOMPARE( mc.numGeometries(), 2 );
  QVERIFY( mc.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc.geometryN( 1 ) ), part );
}

void TestQgsMultiCurve::addGeometryDimensionPreservation()
{
  //adding subsequent points should not alter z/m type, regardless of parts type
  QgsMultiCurve mc;
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 9, 12 )
                  << QgsPoint( 3, 13 ) << QgsPoint( 9, 20 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );

  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 )
                  << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 20, 4 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( mc.vertexCount( 0, 0 ), 3 );
  QCOMPARE( mc.vertexCount( 1, 0 ), 3 );
  QCOMPARE( mc.vertexCount( 2, 0 ), 0 );
  QCOMPARE( mc.vertexCount( -1, 0 ), 0 );
  QCOMPARE( mc.nCoordinates(), 6 );
  QCOMPARE( mc.ringCount(), 1 );
  QCOMPARE( mc.partCount(), 2 );
  QVERIFY( !mc.is3D() );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( mc.geometryN( 0 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 9, 20 ) );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 1 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 1, 20 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 51, 0, 4 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( mc.vertexCount( 0, 0 ), 3 );
  QCOMPARE( mc.vertexCount( 1, 0 ), 3 );
  QCOMPARE( mc.vertexCount( 2, 0 ), 3 );
  QCOMPARE( mc.nCoordinates(), 9 );
  QCOMPARE( mc.ringCount(), 1 );
  QCOMPARE( mc.partCount(), 3 );
  QVERIFY( !mc.is3D() );
  QVERIFY( !mc.isMeasure() );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 2 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 21, 51 ) );
}

void TestQgsMultiCurve::addGeometryDimensionPreservationZ()
{
  QgsMultiCurve mc;
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 )
                  << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 21, 4 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZ );

  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 )
                  << QgsPoint( 3, 31 ) << QgsPoint( 2, 41 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZ );
  QVERIFY( mc.is3D() );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( mc.geometryN( 0 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 1, 21, 4 ) );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 1 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 2, 41, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 71, 0, 6 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZ );
  QVERIFY( mc.is3D() );
  QVERIFY( !mc.isMeasure() );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 2 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 5, 71, 0 ) );
}

void TestQgsMultiCurve::addGeometryDimensionPreservationM()
{
  QgsMultiCurve mc;

  QgsCircularString part;
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 71, 0, 5 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveM );

  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 )
                  << QgsPoint( 3, 31 ) << QgsPoint( 2, 41 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveM );
  QVERIFY( mc.isMeasure() );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( mc.geometryN( 0 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 5, 71, 0, 5 ) );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 1 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 2, 41, 0, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 )
                  << QgsPoint( 14, 15, 16 ) << QgsPoint( 11, 25, 17 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveM );
  QVERIFY( !mc.is3D() );
  QVERIFY( mc.isMeasure() );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 2 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 11, 25, 0, 0 ) );
}

void TestQgsMultiCurve::addGeometryDimensionPreservationZM()
{
  QgsMultiCurve mc;
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZM );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 11 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZM );
  QVERIFY( mc.isMeasure() );
  QVERIFY( mc.is3D() );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( mc.geometryN( 0 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 1 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 11, 0, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 )
                  << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 77, 81, 9 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZM );
  QVERIFY( mc.is3D() );
  QVERIFY( mc.isMeasure() );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 2 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 77, 81, 9, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 )
                  << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 177, 181, 0, 13 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurveZM );
  QVERIFY( mc.is3D() );
  QVERIFY( mc.isMeasure() );

  ls = static_cast< const QgsCircularString * >( mc.geometryN( 3 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 177, 181, 0, 13 ) );
}

void TestQgsMultiCurve::insertGeometry()
{
  QgsMultiCurve mc;

  mc.insertGeometry( nullptr, 0 );
  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 0 );

  mc.insertGeometry( nullptr, -1 );
  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 0 );

  mc.insertGeometry( nullptr, 100 );
  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 0 );

  mc.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 0 );

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) );
  mc.insertGeometry( part.clone(), 0 );

  QVERIFY( !mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 1 );
}

void TestQgsMultiCurve::curveN()
{
  QgsMultiCurve mc;

  QVERIFY( !mc.curveN( 0 ) );
  QVERIFY( !std::as_const( mc ).curveN( 0 ) );

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 7, 17 )
                  << QgsPoint( 3, 13 ) << QgsPoint( 7, 11 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( *mc.curveN( 0 ), part );
  QCOMPARE( *std::as_const( mc ).curveN( 0 ), part );

  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 )
                  << QgsPoint( 3, 31 ) << QgsPoint( 2, 41 ) );
  mc.addGeometry( part.clone() );

  QCOMPARE( *mc.curveN( 1 ), part );
  QCOMPARE( *std::as_const( mc ).curveN( 1 ), part );

  QVERIFY( !mc.curveN( 3 ) );
  QVERIFY( !std::as_const( mc ).curveN( 3 ) );

  QVERIFY( !mc.curveN( -1 ) );
  QVERIFY( !std::as_const( mc ).curveN( -1 ) );
}

void TestQgsMultiCurve::clear()
{
  QgsMultiCurve mc;

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  mc.addGeometry( part.clone() );
  mc.addGeometry( part.clone() );

  QCOMPARE( mc.numGeometries(), 2 );

  mc.clear();

  QVERIFY( mc.isEmpty() );
  QCOMPARE( mc.numGeometries(), 0 );
  QCOMPARE( mc.nCoordinates(), 0 );
  QCOMPARE( mc.ringCount(), 0 );
  QCOMPARE( mc.partCount(), 0 );
  QVERIFY( !mc.is3D() );
  QVERIFY( !mc.isMeasure() );
  QCOMPARE( mc.wkbType(), QgsWkbTypes::MultiCurve );
}

void TestQgsMultiCurve::clone()
{
  QgsMultiCurve mc;
  std::unique_ptr< QgsMultiCurve > cloned( mc.clone() );

  QVERIFY( cloned->isEmpty() );

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  mc.addGeometry( part.clone() );
  mc.addGeometry( part.clone() );

  cloned.reset( mc.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( cloned->geometryN( 0 ) );

  QCOMPARE( *ls, part );

  ls = static_cast< const QgsCircularString * >( cloned->geometryN( 1 ) );

  QCOMPARE( *ls, part );
}

void TestQgsMultiCurve::copy()
{
  QgsMultiCurve mc1;
  QgsMultiCurve mc2( mc1 );

  QVERIFY( mc2.isEmpty() );

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  mc1.addGeometry( part.clone() );
  mc1.addGeometry( part.clone() );

  mc2 = QgsMultiCurve( mc1 );

  QCOMPARE( mc2.numGeometries(), 2 );
  QCOMPARE( mc2.wkbType(), QgsWkbTypes::MultiCurveZM );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( mc2.geometryN( 0 ) );

  QCOMPARE( *ls, part );

  ls = static_cast< const QgsCircularString * >( mc2.geometryN( 1 ) );

  QCOMPARE( *ls, part );
}

void TestQgsMultiCurve::assignment()
{
  QgsMultiCurve mc1;
  QgsMultiCurve mc2;

  mc1 = mc2;
  QCOMPARE( mc1.numGeometries(), 0 );

  mc2.clear();
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  mc2.addGeometry( part.clone() );
  mc2.addGeometry( part.clone() );

  mc1 = mc2;
  QCOMPARE( mc1.numGeometries(), 2 );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( mc1.geometryN( 0 ) );
  QCOMPARE( *ls, part );

  ls = static_cast< const QgsCircularString * >( mc1.geometryN( 1 ) );
  QCOMPARE( *ls, part );
}

void TestQgsMultiCurve::cast()
{
  QVERIFY( !QgsMultiCurve().cast( nullptr ) );

  QgsMultiCurve mc1;
  QVERIFY( QgsMultiCurve().cast( &mc1 ) );

  QgsMultiCurve mc2;
  mc2.fromWkt( QStringLiteral( "MultiCurveZ()" ) );
  QVERIFY( QgsMultiCurve().cast( &mc2 ) );

  mc2.fromWkt( QStringLiteral( "MultiCurveM()" ) );
  QVERIFY( QgsMultiCurve().cast( &mc2 ) );

  mc2.fromWkt( QStringLiteral( "MultiCurveZM()" ) );
  QVERIFY( QgsMultiCurve().cast( &mc2 ) );
}

void TestQgsMultiCurve::boundary()
{
  QgsMultiCurve mc;

  QVERIFY( !mc.boundary() );

  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  mc.addGeometry( cs.clone() );

  QgsAbstractGeometry *boundary = mc.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // add another QgsCircularString
  cs.setPoints( QgsPointSequence() << QgsPoint( 10, 10 )
                << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  mc.addGeometry( cs.clone() );

  boundary = mc.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 11.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 11.0 );
  delete boundary;

  // add a closed string = no boundary
  cs.setPoints( QgsPointSequence() << QgsPoint( 20, 20 )
                << QgsPoint( 21, 20 ) <<  QgsPoint( 20, 20 ) );
  mc.addGeometry( cs.clone() );

  boundary = mc.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );

  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 11.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 11.0 );
  delete boundary;

  //boundary with z
  mc.clear();
  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  mc.addGeometry( cs.clone() );

  cs.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 )
                << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 )
                << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
  mc.addGeometry( cs.clone() );

  boundary = mc.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );

  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );

  QCOMPARE( mpBoundary->geometryN( 2 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->z(), 100.0 );

  QCOMPARE( mpBoundary->geometryN( 3 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 20.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 20.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->z(), 200.0 );

  delete boundary;
}

void TestQgsMultiCurve::reversed()
{
  QgsMultiCurve mc;

  std::unique_ptr< QgsMultiCurve > reversed( mc.reversed() );
  QVERIFY( reversed->isEmpty() );

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) );
  mc.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) );
  mc.addGeometry( part.clone() );

  reversed.reset( mc.reversed() );
  QVERIFY( !reversed->isEmpty() );

  const QgsCircularString *ls = static_cast< const QgsCircularString * >( reversed->geometryN( 0 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) );

  ls = static_cast< const QgsCircularString * >( reversed->geometryN( 1 ) );

  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) );
}

void TestQgsMultiCurve::segmentize()
{
  QgsCircularString cs;
  cs.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );

  QgsCompoundCurve cc;
  cc.addCurve( cs.clone() );

  QgsMultiCurve mc;
  mc.addGeometry( cc.clone() );

  QgsMultiLineString *segmentized2 = static_cast<QgsMultiLineString *>( mc.segmentize() );

  QCOMPARE( segmentized2->vertexCount(), 156 );
  QCOMPARE( segmentized2->partCount(), 1 );
  QVERIFY( !segmentized2->is3D() );
  QVERIFY( !segmentized2->isMeasure() );
  QCOMPARE( segmentized2->wkbType(),  QgsWkbTypes::Type::MultiLineString );
}

void TestQgsMultiCurve::toCurveType()
{
  QgsMultiCurve mc;
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  mc.addGeometry( part.clone() );
  mc.addGeometry( part.clone() );

  std::unique_ptr< QgsMultiCurve > curveType( mc.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( curveType->numGeometries(), 2 );

  const QgsCircularString *curve = static_cast< const QgsCircularString * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, *static_cast< const QgsCircularString * >( mc.geometryN( 0 ) ) );

  curve = static_cast< const QgsCircularString * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, *static_cast< const QgsCircularString * >( mc.geometryN( 1 ) ) );
}

void TestQgsMultiCurve::toFromWKB()
{
  QgsMultiCurve mc1;

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 11 ) );
  mc1.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 27, 54 ) );
  mc1.addGeometry( part.clone() );

  QByteArray wkb = mc1.asWkb();
  QgsMultiCurve mc2;
  QgsConstWkbPtr wkbPtr( wkb );
  mc2.fromWkb( wkbPtr );

  QCOMPARE( mc2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 0 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 1 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  mc1.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !mc1.fromWkb( nullPtr ) );
  QCOMPARE( mc1.wkbType(), QgsWkbTypes::MultiCurve );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !mc1.fromWkb( wkbPointPtr ) );
  QCOMPARE( mc1.wkbType(), QgsWkbTypes::MultiCurve );
}

void TestQgsMultiCurve::toFromWKBWithZM()
{
  //parts with Z
  QgsMultiCurve mc1;

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 7, 11, 3 ) );
  mc1.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 )
                  << QgsPoint( QgsWkbTypes::PointZ, 27, 53, 6 ) );
  mc1.addGeometry( part.clone() );

  QByteArray wkb = mc1.asWkb();
  QgsConstWkbPtr wkbPtr( wkb );
  QgsMultiCurve mc2;
  mc2.fromWkb( wkbPtr );

  QCOMPARE( mc2.numGeometries(), 2 );
  QCOMPARE( mc2.wkbType(), QgsWkbTypes::MultiCurveZ );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 0 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 1 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 1 ) ) );

  //parts with m
  mc1.clear();
  mc2.clear();

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 7, 11, 0, 5 ) );
  mc1.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 27, 53, 0, 7 ) );
  mc1.addGeometry( part.clone() );

  wkb = mc1.asWkb();
  wkbPtr = QgsConstWkbPtr( wkb );
  mc2.fromWkb( wkbPtr );

  QCOMPARE( mc2.numGeometries(), 2 );
  QCOMPARE( mc2.wkbType(), QgsWkbTypes::MultiCurveM );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 0 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 1 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 1 ) ) );

  // parts with ZM
  mc1.clear();
  mc2.clear();

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 5 ) );
  mc1.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 47, 12, 15 ) );
  mc1.addGeometry( part.clone() );

  wkb = mc1.asWkb();
  wkbPtr = QgsConstWkbPtr( wkb );
  mc2.fromWkb( wkbPtr );

  QCOMPARE( mc2.numGeometries(), 2 );
  QCOMPARE( mc2.wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 0 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 1 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 1 ) ) );
}

void TestQgsMultiCurve::toFromWKT()
{
  QgsMultiCurve mc1;

  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) );
  mc1.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) );
  mc1.addGeometry( part.clone() );

  QString wkt = mc1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsMultiCurve mc2;
  QVERIFY( mc2.fromWkt( wkt ) );
  QCOMPARE( mc2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 0 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( mc2.geometryN( 1 ) ),
            *static_cast< const QgsCircularString * >( mc1.geometryN( 1 ) ) );

  //bad WKT
  mc1 = QgsMultiCurve();
  QVERIFY( !mc1.fromWkt( "Point()" ) );
  QVERIFY( mc1.isEmpty() );
  QCOMPARE( mc1.numGeometries(), 0 );
  QCOMPARE( mc1.wkbType(), QgsWkbTypes::MultiCurve );
}

void TestQgsMultiCurve::exportImport()
{
  QgsCircularString part;

  //as JSON
  QgsMultiCurve exportC;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 11 ) );

  exportC.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 27, 47 ) );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 6.9,17 6.9,17 6.8,17 6.8,17.1 6.7,17.1 6.7,17.1 6.6,17.1 6.6,17.1 6.5,17.1 6.5,17.1 6.4,17.1 6.4,17.1 6.3,17.1 6.2,17.2 6.2,17.2 6.1,17.2 6.1,17.2 6,17.2 6,17.2 5.9,17.2 5.9,17.2 5.8,17.2 5.7,17.2 5.7,17.1 5.6,17.1 5.6,17.1 5.5,17.1 5.5,17.1 5.4,17.1 5.4,17.1 5.3,17.1 5.3,17.1 5.2,17.1 5.2,17 5.1,17 5,17 5,17 4.9,17 4.9,17 4.8,16.9 4.8,16.9 4.7,16.9 4.7,16.9 4.6,16.9 4.6,16.8 4.5,16.8 4.5,16.8 4.4,16.8 4.4,16.7 4.3,16.7 4.3,16.7 4.3,16.6 4.2,16.6 4.2,16.6 4.1,16.5 4.1,16.5 4,16.5 4,16.4 3.9,16.4 3.9,16.4 3.9,16.3 3.8,16.3 3.8,16.3 3.7,16.2 3.7,16.2 3.7,16.1 3.6,16.1 3.6,16.1 3.6,16 3.5,16 3.5,15.9 3.5,15.9 3.4,15.8 3.4,15.8 3.4,15.7 3.3,15.7 3.3,15.7 3.3,15.6 3.2,15.6 3.2,15.5 3.2,15.5 3.2,15.4 3.1,15.4 3.1,15.3 3.1,15.3 3.1,15.2 3.1,15.2 3,15.1 3,15.1 3,15 3,15 3,14.9 3,14.8 2.9,14.8 2.9,14.7 2.9,14.7 2.9,14.6 2.9,14.6 2.9,14.5 2.9,14.5 2.9,14.4 2.9,14.4 2.9,14.3 2.8,14.2 2.8,14.2 2.8,14.1 2.8,14.1 2.8,14 2.8,14 2.8,13.9 2.8,13.9 2.8,13.8 2.8,13.8 2.9,13.7 2.9,13.6 2.9,13.6 2.9,13.5 2.9,13.5 2.9,13.4 2.9,13.4 2.9,13.3 2.9,13.3 2.9,13.2 3,13.2 3,13.1 3,13 3,13 3,12.9 3,12.9 3.1,12.8 3.1,12.8 3.1,12.7 3.1,12.7 3.1,12.6 3.2,12.6 3.2,12.5 3.2,12.5 3.2,12.4 3.3,12.4 3.3,12.3 3.3,12.3 3.4,12.3 3.4,12.2 3.4,12.2 3.5,12.1 3.5,12.1 3.5,12 3.6,12 3.6,11.9 3.6,11.9 3.7,11.9 3.7,11.8 3.7,11.8 3.8,11.7 3.8,11.7 3.9,11.7 3.9,11.6 3.9,11.6 4,11.6 4,11.5 4.1,11.5 4.1,11.5 4.2,11.4 4.2,11.4 4.3,11.4 4.3,11.3 4.3,11.3 4.4,11.3 4.4,11.2 4.5,11.2 4.5,11.2 4.6,11.2 4.6,11.1 4.7,11.1 4.7,11.1 4.8,11.1 4.8,11.1 4.9,11 4.9,11 5,11 5,11 5.1,11 5.2,11 5.2,10.9 5.3,10.9 5.3,10.9 5.4,10.9 5.4,10.9 5.5,10.9 5.5,10.9 5.6,10.9 5.6,10.9 5.7,10.9 5.7,10.8 5.8,10.8 5.9,10.8 5.9,10.8 6,10.8 6,10.8 6.1,10.8 6.1,10.8 6.2,10.8 6.2,10.8 6.3,10.9 6.4,10.9 6.4,10.9 6.5,10.9 6.5,10.9 6.6,10.9 6.6,10.9 6.7,10.9 6.7,10.9 6.8,10.9 6.8,11 6.9,11 6.9,11 7,11</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 27.1,36.9 27.2,36.8 27.3,36.6 27.4,36.5 27.5,36.4 27.6,36.3 27.7,36.2 27.8,36 27.9,35.9 28,35.8 28.1,35.7 28.2,35.6 28.3,35.5 28.4,35.4 28.5,35.3 28.7,35.2 28.8,35.1 28.9,35 29,34.9 29.1,34.8 29.3,34.7 29.4,34.6 29.5,34.6 29.7,34.5 29.8,34.4 29.9,34.3 30.1,34.3 30.2,34.2 30.3,34.1 30.5,34 30.6,34 30.7,33.9 30.9,33.9 31,33.8 31.2,33.7 31.3,33.7 31.5,33.6 31.6,33.6 31.8,33.6 31.9,33.5 32.1,33.5 32.2,33.4 32.4,33.4 32.5,33.4 32.7,33.3 32.8,33.3 33,33.3 33.1,33.3 33.3,33.2 33.4,33.2 33.6,33.2 33.7,33.2 33.9,33.2 34,33.2 34.2,33.2 34.3,33.2 34.5,33.2 34.6,33.2 34.8,33.2 34.9,33.2 35.1,33.2 35.3,33.3 35.4,33.3 35.6,33.3 35.7,33.3 35.9,33.3 36,33.4 36.2,33.4 36.3,33.4 36.5,33.5 36.6,33.5 36.8,33.6 36.9,33.6 37.1,33.7 37.2,33.7 37.3,33.8 37.5,33.8 37.6,33.9 37.8,33.9 37.9,34 38,34.1 38.2,34.1 38.3,34.2 38.5,34.3 38.6,34.3 38.7,34.4 38.9,34.5 39,34.6 39.1,34.7 39.2,34.7 39.4,34.8 39.5,34.9 39.6,35 39.7,35.1 39.9,35.2 40,35.3 40.1,35.4 40.2,35.5 40.3,35.6 40.4,35.7 40.5,35.8 40.6,35.9 40.7,36.1 40.8,36.2 40.9,36.3 41,36.4 41.1,36.5 41.2,36.6 41.3,36.8 41.4,36.9 41.5,37 41.6,37.1 41.7,37.3 41.8,37.4 41.8,37.5 41.9,37.7 42,37.8 42.1,37.9 42.1,38.1 42.2,38.2 42.3,38.3 42.3,38.5 42.4,38.6 42.4,38.8 42.5,38.9 42.6,39.1 42.6,39.2 42.6,39.4 42.7,39.5 42.7,39.6 42.8,39.8 42.8,39.9 42.8,40.1 42.9,40.2 42.9,40.4 42.9,40.5 43,40.7 43,40.9 43,41 43,41.2 43,41.3 43,41.5 43,41.6 43.1,41.8 43.1,41.9 43.1,42.1 43.1,42.2 43,42.4 43,42.5 43,42.7 43,42.8 43,43 43,43.1 43,43.3 42.9,43.5 42.9,43.6 42.9,43.8 42.8,43.9 42.8,44.1 42.8,44.2 42.7,44.4 42.7,44.5 42.6,44.6 42.6,44.8 42.6,44.9 42.5,45.1 42.4,45.2 42.4,45.4 42.3,45.5 42.3,45.7 42.2,45.8 42.1,45.9 42.1,46.1 42,46.2 41.9,46.3 41.8,46.5 41.8,46.6 41.7,46.7 41.6,46.9 41.5,47 41.4,47.1 41.3,47.2 41.2,47.4 41.1,47.5 41,47.6 40.9,47.7 40.8,47.8 40.7,47.9 40.6,48.1 40.5,48.2 40.4,48.3 40.3,48.4 40.2,48.5 40.1,48.6 40,48.7 39.9,48.8 39.7,48.9 39.6,49 39.5,49.1 39.4,49.2 39.2,49.3 39.1,49.3 39,49.4 38.9,49.5 38.7,49.6 38.6,49.7 38.5,49.7 38.3,49.8 38.2,49.9 38,49.9 37.9,50 37.8,50.1 37.6,50.1 37.5,50.2 37.3,50.2 37.2,50.3 37.1,50.3 36.9,50.4 36.8,50.4 36.6,50.5 36.5,50.5 36.3,50.6 36.2,50.6 36,50.6 35.9,50.7 35.7,50.7 35.6,50.7 35.4,50.7 35.3,50.7 35.1,50.8 34.9,50.8 34.8,50.8 34.6,50.8 34.5,50.8 34.3,50.8 34.2,50.8 34,50.8 33.9,50.8 33.7,50.8 33.6,50.8 33.4,50.8 33.3,50.8 33.1,50.7 33,50.7 32.8,50.7 32.7,50.7 32.5,50.6 32.4,50.6 32.2,50.6 32.1,50.5 31.9,50.5 31.8,50.4 31.6,50.4 31.5,50.4 31.3,50.3 31.2,50.3 31,50.2 30.9,50.1 30.7,50.1 30.6,50 30.5,50 30.3,49.9 30.2,49.8 30.1,49.7 29.9,49.7 29.8,49.6 29.7,49.5 29.5,49.4 29.4,49.4 29.3,49.3 29.1,49.2 29,49.1 28.9,49 28.8,48.9 28.7,48.8 28.5,48.7 28.4,48.6 28.3,48.5 28.2,48.4 28.1,48.3 28,48.2 27.9,48.1 27.8,48 27.7,47.8 27.6,47.7 27.5,47.6 27.4,47.5 27.3,47.4 27.2,47.2 27.1,47.1 27,47</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );

  QString expectedGML2empty( QStringLiteral( "<MultiLineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiCurve().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13 7 11</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43 27 47</posList></ArcString></segments></Curve></curveMember></MultiCurve>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );

  QString expectedGML3empty( QStringLiteral( "<MultiCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiCurve().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[7.0,17.0],[6.9,17.0],[6.9,17.0],[6.8,17.0],[6.8,17.1],[6.7,17.1],[6.7,17.1],[6.6,17.1],[6.6,17.1],[6.5,17.1],[6.5,17.1],[6.4,17.1],[6.4,17.1],[6.3,17.1],[6.2,17.2],[6.2,17.2],[6.1,17.2],[6.1,17.2],[6.0,17.2],[6.0,17.2],[5.9,17.2],[5.9,17.2],[5.8,17.2],[5.7,17.2],[5.7,17.1],[5.6,17.1],[5.6,17.1],[5.5,17.1],[5.5,17.1],[5.4,17.1],[5.4,17.1],[5.3,17.1],[5.3,17.1],[5.2,17.1],[5.2,17.0],[5.1,17.0],[5.0,17.0],[5.0,17.0],[4.9,17.0],[4.9,17.0],[4.8,16.9],[4.8,16.9],[4.7,16.9],[4.7,16.9],[4.6,16.9],[4.6,16.8],[4.5,16.8],[4.5,16.8],[4.4,16.8],[4.4,16.7],[4.3,16.7],[4.3,16.7],[4.3,16.6],[4.2,16.6],[4.2,16.6],[4.1,16.5],[4.1,16.5],[4.0,16.5],[4.0,16.4],[3.9,16.4],[3.9,16.4],[3.9,16.3],[3.8,16.3],[3.8,16.3],[3.7,16.2],[3.7,16.2],[3.7,16.1],[3.6,16.1],[3.6,16.1],[3.6,16.0],[3.5,16.0],[3.5,15.9],[3.5,15.9],[3.4,15.8],[3.4,15.8],[3.4,15.7],[3.3,15.7],[3.3,15.7],[3.3,15.6],[3.2,15.6],[3.2,15.5],[3.2,15.5],[3.2,15.4],[3.1,15.4],[3.1,15.3],[3.1,15.3],[3.1,15.2],[3.1,15.2],[3.0,15.1],[3.0,15.1],[3.0,15.0],[3.0,15.0],[3.0,14.9],[3.0,14.8],[2.9,14.8],[2.9,14.7],[2.9,14.7],[2.9,14.6],[2.9,14.6],[2.9,14.5],[2.9,14.5],[2.9,14.4],[2.9,14.4],[2.9,14.3],[2.8,14.2],[2.8,14.2],[2.8,14.1],[2.8,14.1],[2.8,14.0],[2.8,14.0],[2.8,13.9],[2.8,13.9],[2.8,13.8],[2.8,13.8],[2.9,13.7],[2.9,13.6],[2.9,13.6],[2.9,13.5],[2.9,13.5],[2.9,13.4],[2.9,13.4],[2.9,13.3],[2.9,13.3],[2.9,13.2],[3.0,13.2],[3.0,13.1],[3.0,13.0],[3.0,13.0],[3.0,12.9],[3.0,12.9],[3.1,12.8],[3.1,12.8],[3.1,12.7],[3.1,12.7],[3.1,12.6],[3.2,12.6],[3.2,12.5],[3.2,12.5],[3.2,12.4],[3.3,12.4],[3.3,12.3],[3.3,12.3],[3.4,12.3],[3.4,12.2],[3.4,12.2],[3.5,12.1],[3.5,12.1],[3.5,12.0],[3.6,12.0],[3.6,11.9],[3.6,11.9],[3.7,11.9],[3.7,11.8],[3.7,11.8],[3.8,11.7],[3.8,11.7],[3.9,11.7],[3.9,11.6],[3.9,11.6],[4.0,11.6],[4.0,11.5],[4.1,11.5],[4.1,11.5],[4.2,11.4],[4.2,11.4],[4.3,11.4],[4.3,11.3],[4.3,11.3],[4.4,11.3],[4.4,11.2],[4.5,11.2],[4.5,11.2],[4.6,11.2],[4.6,11.1],[4.7,11.1],[4.7,11.1],[4.8,11.1],[4.8,11.1],[4.9,11.0],[4.9,11.0],[5.0,11.0],[5.0,11.0],[5.1,11.0],[5.2,11.0],[5.2,10.9],[5.3,10.9],[5.3,10.9],[5.4,10.9],[5.4,10.9],[5.5,10.9],[5.5,10.9],[5.6,10.9],[5.6,10.9],[5.7,10.9],[5.7,10.8],[5.8,10.8],[5.9,10.8],[5.9,10.8],[6.0,10.8],[6.0,10.8],[6.1,10.8],[6.1,10.8],[6.2,10.8],[6.2,10.8],[6.3,10.9],[6.4,10.9],[6.4,10.9],[6.5,10.9],[6.5,10.9],[6.6,10.9],[6.6,10.9],[6.7,10.9],[6.7,10.9],[6.8,10.9],[6.8,11.0],[6.9,11.0],[6.9,11.0],[7.0,11.0]],[[27.0,37.0],[27.1,36.9],[27.2,36.8],[27.3,36.6],[27.4,36.5],[27.5,36.4],[27.6,36.3],[27.7,36.2],[27.8,36.0],[27.9,35.9],[28.0,35.8],[28.1,35.7],[28.2,35.6],[28.3,35.5],[28.4,35.4],[28.5,35.3],[28.7,35.2],[28.8,35.1],[28.9,35.0],[29.0,34.9],[29.1,34.8],[29.3,34.7],[29.4,34.6],[29.5,34.6],[29.7,34.5],[29.8,34.4],[29.9,34.3],[30.1,34.3],[30.2,34.2],[30.3,34.1],[30.5,34.0],[30.6,34.0],[30.7,33.9],[30.9,33.9],[31.0,33.8],[31.2,33.7],[31.3,33.7],[31.5,33.6],[31.6,33.6],[31.8,33.6],[31.9,33.5],[32.1,33.5],[32.2,33.4],[32.4,33.4],[32.5,33.4],[32.7,33.3],[32.8,33.3],[33.0,33.3],[33.1,33.3],[33.3,33.2],[33.4,33.2],[33.6,33.2],[33.7,33.2],[33.9,33.2],[34.0,33.2],[34.2,33.2],[34.3,33.2],[34.5,33.2],[34.6,33.2],[34.8,33.2],[34.9,33.2],[35.1,33.2],[35.3,33.3],[35.4,33.3],[35.6,33.3],[35.7,33.3],[35.9,33.3],[36.0,33.4],[36.2,33.4],[36.3,33.4],[36.5,33.5],[36.6,33.5],[36.8,33.6],[36.9,33.6],[37.1,33.7],[37.2,33.7],[37.3,33.8],[37.5,33.8],[37.6,33.9],[37.8,33.9],[37.9,34.0],[38.0,34.1],[38.2,34.1],[38.3,34.2],[38.5,34.3],[38.6,34.3],[38.7,34.4],[38.9,34.5],[39.0,34.6],[39.1,34.7],[39.2,34.7],[39.4,34.8],[39.5,34.9],[39.6,35.0],[39.7,35.1],[39.9,35.2],[40.0,35.3],[40.1,35.4],[40.2,35.5],[40.3,35.6],[40.4,35.7],[40.5,35.8],[40.6,35.9],[40.7,36.1],[40.8,36.2],[40.9,36.3],[41.0,36.4],[41.1,36.5],[41.2,36.6],[41.3,36.8],[41.4,36.9],[41.5,37.0],[41.6,37.1],[41.7,37.3],[41.8,37.4],[41.8,37.5],[41.9,37.7],[42.0,37.8],[42.1,37.9],[42.1,38.1],[42.2,38.2],[42.3,38.3],[42.3,38.5],[42.4,38.6],[42.4,38.8],[42.5,38.9],[42.6,39.1],[42.6,39.2],[42.6,39.4],[42.7,39.5],[42.7,39.6],[42.8,39.8],[42.8,39.9],[42.8,40.1],[42.9,40.2],[42.9,40.4],[42.9,40.5],[43.0,40.7],[43.0,40.9],[43.0,41.0],[43.0,41.2],[43.0,41.3],[43.0,41.5],[43.0,41.6],[43.1,41.8],[43.1,41.9],[43.1,42.1],[43.1,42.2],[43.0,42.4],[43.0,42.5],[43.0,42.7],[43.0,42.8],[43.0,43.0],[43.0,43.1],[43.0,43.3],[42.9,43.5],[42.9,43.6],[42.9,43.8],[42.8,43.9],[42.8,44.1],[42.8,44.2],[42.7,44.4],[42.7,44.5],[42.6,44.6],[42.6,44.8],[42.6,44.9],[42.5,45.1],[42.4,45.2],[42.4,45.4],[42.3,45.5],[42.3,45.7],[42.2,45.8],[42.1,45.9],[42.1,46.1],[42.0,46.2],[41.9,46.3],[41.8,46.5],[41.8,46.6],[41.7,46.7],[41.6,46.9],[41.5,47.0],[41.4,47.1],[41.3,47.2],[41.2,47.4],[41.1,47.5],[41.0,47.6],[40.9,47.7],[40.8,47.8],[40.7,47.9],[40.6,48.1],[40.5,48.2],[40.4,48.3],[40.3,48.4],[40.2,48.5],[40.1,48.6],[40.0,48.7],[39.9,48.8],[39.7,48.9],[39.6,49.0],[39.5,49.1],[39.4,49.2],[39.2,49.3],[39.1,49.3],[39.0,49.4],[38.9,49.5],[38.7,49.6],[38.6,49.7],[38.5,49.7],[38.3,49.8],[38.2,49.9],[38.0,49.9],[37.9,50.0],[37.8,50.1],[37.6,50.1],[37.5,50.2],[37.3,50.2],[37.2,50.3],[37.1,50.3],[36.9,50.4],[36.8,50.4],[36.6,50.5],[36.5,50.5],[36.3,50.6],[36.2,50.6],[36.0,50.6],[35.9,50.7],[35.7,50.7],[35.6,50.7],[35.4,50.7],[35.3,50.7],[35.1,50.8],[34.9,50.8],[34.8,50.8],[34.6,50.8],[34.5,50.8],[34.3,50.8],[34.2,50.8],[34.0,50.8],[33.9,50.8],[33.7,50.8],[33.6,50.8],[33.4,50.8],[33.3,50.8],[33.1,50.7],[33.0,50.7],[32.8,50.7],[32.7,50.7],[32.5,50.6],[32.4,50.6],[32.2,50.6],[32.1,50.5],[31.9,50.5],[31.8,50.4],[31.6,50.4],[31.5,50.4],[31.3,50.3],[31.2,50.3],[31.0,50.2],[30.9,50.1],[30.7,50.1],[30.6,50.0],[30.5,50.0],[30.3,49.9],[30.2,49.8],[30.1,49.7],[29.9,49.7],[29.8,49.6],[29.7,49.5],[29.5,49.4],[29.4,49.4],[29.3,49.3],[29.1,49.2],[29.0,49.1],[28.9,49.0],[28.8,48.9],[28.7,48.8],[28.5,48.7],[28.4,48.6],[28.3,48.5],[28.2,48.4],[28.1,48.3],[28.0,48.2],[27.9,48.1],[27.8,48.0],[27.7,47.8],[27.6,47.7],[27.5,47.6],[27.4,47.5],[27.3,47.4],[27.2,47.2],[27.1,47.1],[27.0,47.0]]],\"type\":\"MultiLineString\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiCurve exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 11 / 3.0 ) );
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 )
                  << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 1 / 3.0 ) );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[[2.333,5.667],[2.316,5.677],[2.298,5.687],[2.28,5.697],[2.262,5.707],[2.244,5.716],[2.226,5.725],[2.207,5.734],[2.188,5.742],[2.17,5.75],[2.151,5.757],[2.131,5.765],[2.112,5.772],[2.093,5.778],[2.074,5.785],[2.054,5.79],[2.034,5.796],[2.015,5.801],[1.995,5.806],[1.975,5.811],[1.955,5.815],[1.935,5.819],[1.915,5.822],[1.894,5.826],[1.874,5.828],[1.854,5.831],[1.834,5.833],[1.813,5.835],[1.793,5.836],[1.773,5.837],[1.752,5.838],[1.732,5.838],[1.711,5.838],[1.691,5.838],[1.67,5.837],[1.65,5.836],[1.63,5.834],[1.609,5.833],[1.589,5.83],[1.569,5.828],[1.548,5.825],[1.528,5.822],[1.508,5.818],[1.488,5.814],[1.468,5.81],[1.448,5.805],[1.428,5.801],[1.409,5.795],[1.389,5.79],[1.37,5.784],[1.35,5.777],[1.331,5.771],[1.312,5.764],[1.293,5.756],[1.274,5.749],[1.255,5.741],[1.236,5.732],[1.218,5.724],[1.199,5.715],[1.181,5.705],[1.163,5.696],[1.145,5.686],[1.128,5.676],[1.11,5.665],[1.093,5.654],[1.076,5.643],[1.059,5.632],[1.042,5.62],[1.025,5.608],[1.009,5.596],[0.993,5.583],[0.977,5.57],[0.962,5.557],[0.946,5.543],[0.931,5.53],[0.916,5.516],[0.901,5.502],[0.887,5.487],[0.873,5.473],[0.859,5.458],[0.845,5.442],[0.832,5.427],[0.819,5.411],[0.806,5.395],[0.793,5.379],[0.781,5.363],[0.769,5.346],[0.757,5.33],[0.746,5.313],[0.735,5.296],[0.724,5.278],[0.713,5.261],[0.703,5.243],[0.693,5.225],[0.684,5.207],[0.674,5.189],[0.666,5.171],[0.657,5.152],[0.649,5.133],[0.641,5.115],[0.633,5.096],[0.626,5.077],[0.619,5.057],[0.612,5.038],[0.606,5.019],[0.6,4.999],[0.594,4.979],[0.589,4.96],[0.584,4.94],[0.579,4.92],[0.575,4.9],[0.571,4.88],[0.568,4.86],[0.564,4.84],[0.562,4.819],[0.559,4.799],[0.557,4.779],[0.555,4.759],[0.554,4.738],[0.553,4.718],[0.552,4.697],[0.552,4.677],[0.552,4.656],[0.552,4.636],[0.553,4.616],[0.554,4.595],[0.555,4.575],[0.557,4.554],[0.559,4.534],[0.562,4.514],[0.564,4.494],[0.568,4.473],[0.571,4.453],[0.575,4.433],[0.579,4.413],[0.584,4.393],[0.589,4.374],[0.594,4.354],[0.6,4.334],[0.606,4.315],[0.612,4.295],[0.619,4.276],[0.626,4.257],[0.633,4.238],[0.641,4.219],[0.649,4.2],[0.657,4.181],[0.666,4.163],[0.674,4.144],[0.684,4.126],[0.693,4.108],[0.703,4.09],[0.713,4.073],[0.724,4.055],[0.735,4.038],[0.746,4.021],[0.757,4.004],[0.769,3.987],[0.781,3.97],[0.793,3.954],[0.806,3.938],[0.819,3.922],[0.832,3.906],[0.845,3.891],[0.859,3.876],[0.873,3.861],[0.887,3.846],[0.901,3.832],[0.916,3.817],[0.931,3.804],[0.946,3.79],[0.962,3.776],[0.977,3.763],[0.993,3.75],[1.009,3.738],[1.025,3.726],[1.042,3.713],[1.059,3.702],[1.076,3.69],[1.093,3.679],[1.11,3.668],[1.128,3.658],[1.145,3.648],[1.163,3.638],[1.181,3.628],[1.199,3.619],[1.218,3.61],[1.236,3.601],[1.255,3.593],[1.274,3.585],[1.293,3.577],[1.312,3.57],[1.331,3.563],[1.35,3.556],[1.37,3.55],[1.389,3.544],[1.409,3.538],[1.428,3.533],[1.448,3.528],[1.468,3.523],[1.488,3.519],[1.508,3.515],[1.528,3.511],[1.548,3.508],[1.569,3.505],[1.589,3.503],[1.609,3.501],[1.63,3.499],[1.65,3.497],[1.67,3.496],[1.691,3.496],[1.711,3.495],[1.732,3.495],[1.752,3.496],[1.773,3.496],[1.793,3.497],[1.813,3.499],[1.834,3.5],[1.854,3.503],[1.874,3.505],[1.894,3.508],[1.915,3.511],[1.935,3.514],[1.955,3.518],[1.975,3.523],[1.995,3.527],[2.015,3.532],[2.034,3.537],[2.054,3.543],[2.074,3.549],[2.093,3.555],[2.112,3.562],[2.131,3.569],[2.151,3.576],[2.17,3.584],[2.188,3.592],[2.207,3.6],[2.226,3.608],[2.244,3.617],[2.262,3.627],[2.28,3.636],[2.298,3.646],[2.316,3.656],[2.333,3.667]],[[9.0,4.111],[8.966,4.178],[8.932,4.244],[8.896,4.309],[8.859,4.374],[8.821,4.438],[8.782,4.502],[8.742,4.565],[8.7,4.627],[8.658,4.688],[8.614,4.749],[8.57,4.809],[8.524,4.868],[8.477,4.926],[8.43,4.983],[8.381,5.04],[8.331,5.096],[8.281,5.151],[8.229,5.205],[8.177,5.258],[8.124,5.31],[8.069,5.361],[8.014,5.411],[7.958,5.461],[7.901,5.509],[7.844,5.556],[7.785,5.603],[7.726,5.648],[7.666,5.692],[7.605,5.735],[7.543,5.777],[7.481,5.818],[7.418,5.858],[7.354,5.897],[7.29,5.935],[7.225,5.971],[7.159,6.007],[7.093,6.041],[7.026,6.074],[6.958,6.106],[6.89,6.137],[6.822,6.167],[6.753,6.195],[6.683,6.222],[6.613,6.248],[6.543,6.273],[6.472,6.296],[6.401,6.319],[6.329,6.34],[6.257,6.36],[6.185,6.378],[6.112,6.395],[6.04,6.411],[5.966,6.426],[5.893,6.44],[5.819,6.452],[5.746,6.463],[5.672,6.472],[5.597,6.48],[5.523,6.487],[5.449,6.493],[5.374,6.498],[5.3,6.501],[5.225,6.503],[5.15,6.503],[5.076,6.502],[5.001,6.5],[4.927,6.497],[4.852,6.492],[4.778,6.486],[4.704,6.479],[4.629,6.47],[4.555,6.46],[4.482,6.449],[4.408,6.437],[4.335,6.423],[4.262,6.408],[4.189,6.392],[4.116,6.374],[4.044,6.355],[3.972,6.335],[3.901,6.314],[3.83,6.292],[3.759,6.268],[3.688,6.243],[3.619,6.217],[3.549,6.189],[3.48,6.16],[3.412,6.131],[3.344,6.1],[3.277,6.067],[3.21,6.034],[3.144,5.999],[3.078,5.964],[3.013,5.927],[2.949,5.889],[2.886,5.85],[2.823,5.81],[2.761,5.768],[2.699,5.726],[2.638,5.683],[2.578,5.638],[2.519,5.593],[2.461,5.546],[2.403,5.499],[2.347,5.45],[2.291,5.401],[2.236,5.35],[2.182,5.299],[2.129,5.246],[2.076,5.193],[2.025,5.139],[1.975,5.084],[1.925,5.028],[1.877,4.971],[1.829,4.914],[1.783,4.855],[1.738,4.796],[1.693,4.736],[1.65,4.675],[1.608,4.614],[1.567,4.551],[1.527,4.488],[1.488,4.425],[1.45,4.36],[1.413,4.295],[1.378,4.23],[1.343,4.164],[1.31,4.097],[1.278,4.029],[1.247,3.961],[1.217,3.893],[1.189,3.824],[1.161,3.755],[1.135,3.685],[1.11,3.614],[1.087,3.544],[1.064,3.472],[1.043,3.401],[1.023,3.329],[1.004,3.257],[0.987,3.184],[0.971,3.111],[0.956,3.038],[0.942,2.965],[0.93,2.891],[0.919,2.817],[0.909,2.743],[0.901,2.669],[0.894,2.595],[0.888,2.52],[0.883,2.446],[0.88,2.371],[0.878,2.297],[0.878,2.222],[0.878,2.148],[0.88,2.073],[0.883,1.998],[0.888,1.924],[0.894,1.85],[0.901,1.775],[0.909,1.701],[0.919,1.627],[0.93,1.553],[0.942,1.48],[0.956,1.406],[0.971,1.333],[0.987,1.26],[1.004,1.188],[1.023,1.116],[1.043,1.044],[1.064,0.972],[1.087,0.901],[1.11,0.83],[1.135,0.76],[1.161,0.69],[1.189,0.62],[1.217,0.551],[1.247,0.483],[1.278,0.415],[1.31,0.348],[1.343,0.281],[1.378,0.215],[1.413,0.149],[1.45,0.084],[1.488,0.02],[1.527,-0.044],[1.567,-0.107],[1.608,-0.169],[1.65,-0.231],[1.693,-0.291],[1.738,-0.351],[1.783,-0.411],[1.829,-0.469],[1.877,-0.527],[1.925,-0.584],[1.975,-0.639],[2.025,-0.694],[2.076,-0.749],[2.129,-0.802],[2.182,-0.854],[2.236,-0.906],[2.291,-0.956],[2.347,-1.006],[2.403,-1.054],[2.461,-1.102],[2.519,-1.148],[2.578,-1.194],[2.638,-1.238],[2.699,-1.282],[2.761,-1.324],[2.823,-1.365],[2.886,-1.405],[2.949,-1.444],[3.013,-1.482],[3.078,-1.519],[3.144,-1.555],[3.21,-1.589],[3.277,-1.623],[3.344,-1.655],[3.412,-1.686],[3.48,-1.716],[3.549,-1.745],[3.619,-1.772],[3.688,-1.798],[3.759,-1.823],[3.83,-1.847],[3.901,-1.87],[3.972,-1.891],[4.044,-1.911],[4.116,-1.93],[4.189,-1.947],[4.262,-1.964],[4.335,-1.979],[4.408,-1.992],[4.482,-2.005],[4.555,-2.016],[4.629,-2.026],[4.704,-2.034],[4.778,-2.042],[4.852,-2.048],[4.927,-2.052],[5.001,-2.056],[5.076,-2.058],[5.15,-2.059],[5.225,-2.058],[5.3,-2.056],[5.374,-2.053],[5.449,-2.049],[5.523,-2.043],[5.597,-2.036],[5.672,-2.028],[5.746,-2.018],[5.819,-2.007],[5.893,-1.995],[5.966,-1.982],[6.04,-1.967],[6.112,-1.951],[6.185,-1.934],[6.257,-1.915],[6.329,-1.895],[6.401,-1.874],[6.472,-1.852],[6.543,-1.829],[6.613,-1.804],[6.683,-1.778],[6.753,-1.751],[6.822,-1.722],[6.89,-1.693],[6.958,-1.662],[7.026,-1.63],[7.093,-1.597],[7.159,-1.562],[7.225,-1.527],[7.29,-1.49],[7.354,-1.453],[7.418,-1.414],[7.481,-1.374],[7.543,-1.333],[7.605,-1.291],[7.666,-1.248],[7.726,-1.203],[7.785,-1.158],[7.844,-1.112],[7.901,-1.065],[7.958,-1.016],[8.014,-0.967],[8.069,-0.917],[8.124,-0.865],[8.177,-0.813],[8.229,-0.76],[8.281,-0.706],[8.331,-0.651],[8.381,-0.596],[8.43,-0.539],[8.477,-0.482],[8.524,-0.423],[8.57,-0.364],[8.614,-0.304],[8.658,-0.244],[8.7,-0.182],[8.742,-0.12],[8.782,-0.057],[8.821,0.006],[8.859,0.07],[8.896,0.135],[8.932,0.201],[8.966,0.267],[9.0,0.333]]],\"type\":\"MultiLineString\"}" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 2.316,5.677 2.298,5.687 2.28,5.697 2.262,5.707 2.244,5.716 2.226,5.725 2.207,5.734 2.188,5.742 2.17,5.75 2.151,5.757 2.131,5.765 2.112,5.772 2.093,5.778 2.074,5.785 2.054,5.79 2.034,5.796 2.015,5.801 1.995,5.806 1.975,5.811 1.955,5.815 1.935,5.819 1.915,5.822 1.894,5.826 1.874,5.828 1.854,5.831 1.834,5.833 1.813,5.835 1.793,5.836 1.773,5.837 1.752,5.838 1.732,5.838 1.711,5.838 1.691,5.838 1.67,5.837 1.65,5.836 1.63,5.834 1.609,5.833 1.589,5.83 1.569,5.828 1.548,5.825 1.528,5.822 1.508,5.818 1.488,5.814 1.468,5.81 1.448,5.805 1.428,5.801 1.409,5.795 1.389,5.79 1.37,5.784 1.35,5.777 1.331,5.771 1.312,5.764 1.293,5.756 1.274,5.749 1.255,5.741 1.236,5.732 1.218,5.724 1.199,5.715 1.181,5.705 1.163,5.696 1.145,5.686 1.128,5.676 1.11,5.665 1.093,5.654 1.076,5.643 1.059,5.632 1.042,5.62 1.025,5.608 1.009,5.596 0.993,5.583 0.977,5.57 0.962,5.557 0.946,5.543 0.931,5.53 0.916,5.516 0.901,5.502 0.887,5.487 0.873,5.473 0.859,5.458 0.845,5.442 0.832,5.427 0.819,5.411 0.806,5.395 0.793,5.379 0.781,5.363 0.769,5.346 0.757,5.33 0.746,5.313 0.735,5.296 0.724,5.278 0.713,5.261 0.703,5.243 0.693,5.225 0.684,5.207 0.674,5.189 0.666,5.171 0.657,5.152 0.649,5.133 0.641,5.115 0.633,5.096 0.626,5.077 0.619,5.057 0.612,5.038 0.606,5.019 0.6,4.999 0.594,4.979 0.589,4.96 0.584,4.94 0.579,4.92 0.575,4.9 0.571,4.88 0.568,4.86 0.564,4.84 0.562,4.819 0.559,4.799 0.557,4.779 0.555,4.759 0.554,4.738 0.553,4.718 0.552,4.697 0.552,4.677 0.552,4.656 0.552,4.636 0.553,4.616 0.554,4.595 0.555,4.575 0.557,4.554 0.559,4.534 0.562,4.514 0.564,4.494 0.568,4.473 0.571,4.453 0.575,4.433 0.579,4.413 0.584,4.393 0.589,4.374 0.594,4.354 0.6,4.334 0.606,4.315 0.612,4.295 0.619,4.276 0.626,4.257 0.633,4.238 0.641,4.219 0.649,4.2 0.657,4.181 0.666,4.163 0.674,4.144 0.684,4.126 0.693,4.108 0.703,4.09 0.713,4.073 0.724,4.055 0.735,4.038 0.746,4.021 0.757,4.004 0.769,3.987 0.781,3.97 0.793,3.954 0.806,3.938 0.819,3.922 0.832,3.906 0.845,3.891 0.859,3.876 0.873,3.861 0.887,3.846 0.901,3.832 0.916,3.817 0.931,3.804 0.946,3.79 0.962,3.776 0.977,3.763 0.993,3.75 1.009,3.738 1.025,3.726 1.042,3.713 1.059,3.702 1.076,3.69 1.093,3.679 1.11,3.668 1.128,3.658 1.145,3.648 1.163,3.638 1.181,3.628 1.199,3.619 1.218,3.61 1.236,3.601 1.255,3.593 1.274,3.585 1.293,3.577 1.312,3.57 1.331,3.563 1.35,3.556 1.37,3.55 1.389,3.544 1.409,3.538 1.428,3.533 1.448,3.528 1.468,3.523 1.488,3.519 1.508,3.515 1.528,3.511 1.548,3.508 1.569,3.505 1.589,3.503 1.609,3.501 1.63,3.499 1.65,3.497 1.67,3.496 1.691,3.496 1.711,3.495 1.732,3.495 1.752,3.496 1.773,3.496 1.793,3.497 1.813,3.499 1.834,3.5 1.854,3.503 1.874,3.505 1.894,3.508 1.915,3.511 1.935,3.514 1.955,3.518 1.975,3.523 1.995,3.527 2.015,3.532 2.034,3.537 2.054,3.543 2.074,3.549 2.093,3.555 2.112,3.562 2.131,3.569 2.151,3.576 2.17,3.584 2.188,3.592 2.207,3.6 2.226,3.608 2.244,3.617 2.262,3.627 2.28,3.636 2.298,3.646 2.316,3.656 2.333,3.667</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 8.966,4.178 8.932,4.244 8.896,4.309 8.859,4.374 8.821,4.438 8.782,4.502 8.742,4.565 8.7,4.627 8.658,4.688 8.614,4.749 8.57,4.809 8.524,4.868 8.477,4.926 8.43,4.983 8.381,5.04 8.331,5.096 8.281,5.151 8.229,5.205 8.177,5.258 8.124,5.31 8.069,5.361 8.014,5.411 7.958,5.461 7.901,5.509 7.844,5.556 7.785,5.603 7.726,5.648 7.666,5.692 7.605,5.735 7.543,5.777 7.481,5.818 7.418,5.858 7.354,5.897 7.29,5.935 7.225,5.971 7.159,6.007 7.093,6.041 7.026,6.074 6.958,6.106 6.89,6.137 6.822,6.167 6.753,6.195 6.683,6.222 6.613,6.248 6.543,6.273 6.472,6.296 6.401,6.319 6.329,6.34 6.257,6.36 6.185,6.378 6.112,6.395 6.04,6.411 5.966,6.426 5.893,6.44 5.819,6.452 5.746,6.463 5.672,6.472 5.597,6.48 5.523,6.487 5.449,6.493 5.374,6.498 5.3,6.501 5.225,6.503 5.15,6.503 5.076,6.502 5.001,6.5 4.927,6.497 4.852,6.492 4.778,6.486 4.704,6.479 4.629,6.47 4.555,6.46 4.482,6.449 4.408,6.437 4.335,6.423 4.262,6.408 4.189,6.392 4.116,6.374 4.044,6.355 3.972,6.335 3.901,6.314 3.83,6.292 3.759,6.268 3.688,6.243 3.619,6.217 3.549,6.189 3.48,6.16 3.412,6.131 3.344,6.1 3.277,6.067 3.21,6.034 3.144,5.999 3.078,5.964 3.013,5.927 2.949,5.889 2.886,5.85 2.823,5.81 2.761,5.768 2.699,5.726 2.638,5.683 2.578,5.638 2.519,5.593 2.461,5.546 2.403,5.499 2.347,5.45 2.291,5.401 2.236,5.35 2.182,5.299 2.129,5.246 2.076,5.193 2.025,5.139 1.975,5.084 1.925,5.028 1.877,4.971 1.829,4.914 1.783,4.855 1.738,4.796 1.693,4.736 1.65,4.675 1.608,4.614 1.567,4.551 1.527,4.488 1.488,4.425 1.45,4.36 1.413,4.295 1.378,4.23 1.343,4.164 1.31,4.097 1.278,4.029 1.247,3.961 1.217,3.893 1.189,3.824 1.161,3.755 1.135,3.685 1.11,3.614 1.087,3.544 1.064,3.472 1.043,3.401 1.023,3.329 1.004,3.257 0.987,3.184 0.971,3.111 0.956,3.038 0.942,2.965 0.93,2.891 0.919,2.817 0.909,2.743 0.901,2.669 0.894,2.595 0.888,2.52 0.883,2.446 0.88,2.371 0.878,2.297 0.878,2.222 0.878,2.148 0.88,2.073 0.883,1.998 0.888,1.924 0.894,1.85 0.901,1.775 0.909,1.701 0.919,1.627 0.93,1.553 0.942,1.48 0.956,1.406 0.971,1.333 0.987,1.26 1.004,1.188 1.023,1.116 1.043,1.044 1.064,0.972 1.087,0.901 1.11,0.83 1.135,0.76 1.161,0.69 1.189,0.62 1.217,0.551 1.247,0.483 1.278,0.415 1.31,0.348 1.343,0.281 1.378,0.215 1.413,0.149 1.45,0.084 1.488,0.02 1.527,-0.044 1.567,-0.107 1.608,-0.169 1.65,-0.231 1.693,-0.291 1.738,-0.351 1.783,-0.411 1.829,-0.469 1.877,-0.527 1.925,-0.584 1.975,-0.639 2.025,-0.694 2.076,-0.749 2.129,-0.802 2.182,-0.854 2.236,-0.906 2.291,-0.956 2.347,-1.006 2.403,-1.054 2.461,-1.102 2.519,-1.148 2.578,-1.194 2.638,-1.238 2.699,-1.282 2.761,-1.324 2.823,-1.365 2.886,-1.405 2.949,-1.444 3.013,-1.482 3.078,-1.519 3.144,-1.555 3.21,-1.589 3.277,-1.623 3.344,-1.655 3.412,-1.686 3.48,-1.716 3.549,-1.745 3.619,-1.772 3.688,-1.798 3.759,-1.823 3.83,-1.847 3.901,-1.87 3.972,-1.891 4.044,-1.911 4.116,-1.93 4.189,-1.947 4.262,-1.964 4.335,-1.979 4.408,-1.992 4.482,-2.005 4.555,-2.016 4.629,-2.026 4.704,-2.034 4.778,-2.042 4.852,-2.048 4.927,-2.052 5.001,-2.056 5.076,-2.058 5.15,-2.059 5.225,-2.058 5.3,-2.056 5.374,-2.053 5.449,-2.049 5.523,-2.043 5.597,-2.036 5.672,-2.028 5.746,-2.018 5.819,-2.007 5.893,-1.995 5.966,-1.982 6.04,-1.967 6.112,-1.951 6.185,-1.934 6.257,-1.915 6.329,-1.895 6.401,-1.874 6.472,-1.852 6.543,-1.829 6.613,-1.804 6.683,-1.778 6.753,-1.751 6.822,-1.722 6.89,-1.693 6.958,-1.662 7.026,-1.63 7.093,-1.597 7.159,-1.562 7.225,-1.527 7.29,-1.49 7.354,-1.453 7.418,-1.414 7.481,-1.374 7.543,-1.333 7.605,-1.291 7.666,-1.248 7.726,-1.203 7.785,-1.158 7.844,-1.112 7.901,-1.065 7.958,-1.016 8.014,-0.967 8.069,-0.917 8.124,-0.865 8.177,-0.813 8.229,-0.76 8.281,-0.706 8.331,-0.651 8.381,-0.596 8.43,-0.539 8.477,-0.482 8.524,-0.423 8.57,-0.364 8.614,-0.304 8.658,-0.244 8.7,-0.182 8.742,-0.12 8.782,-0.057 8.821,0.006 8.859,0.07 8.896,0.135 8.932,0.201 8.966,0.267 9,0.333</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.333 3.667</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 0.333</posList></ArcString></segments></Curve></curveMember></MultiCurve>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>7,17,0 6.9,17,0 6.9,17,0 6.8,17,0 6.8,17.1,0 6.7,17.1,0 6.7,17.1,0 6.6,17.1,0 6.6,17.1,0 6.5,17.1,0 6.5,17.1,0 6.4,17.1,0 6.4,17.1,0 6.3,17.1,0 6.2,17.2,0 6.2,17.2,0 6.1,17.2,0 6.1,17.2,0 6,17.2,0 6,17.2,0 5.9,17.2,0 5.9,17.2,0 5.8,17.2,0 5.7,17.2,0 5.7,17.1,0 5.6,17.1,0 5.6,17.1,0 5.5,17.1,0 5.5,17.1,0 5.4,17.1,0 5.4,17.1,0 5.3,17.1,0 5.3,17.1,0 5.2,17.1,0 5.2,17,0 5.1,17,0 5,17,0 5,17,0 4.9,17,0 4.9,17,0 4.8,16.9,0 4.8,16.9,0 4.7,16.9,0 4.7,16.9,0 4.6,16.9,0 4.6,16.8,0 4.5,16.8,0 4.5,16.8,0 4.4,16.8,0 4.4,16.7,0 4.3,16.7,0 4.3,16.7,0 4.3,16.6,0 4.2,16.6,0 4.2,16.6,0 4.1,16.5,0 4.1,16.5,0 4,16.5,0 4,16.4,0 3.9,16.4,0 3.9,16.4,0 3.9,16.3,0 3.8,16.3,0 3.8,16.3,0 3.7,16.2,0 3.7,16.2,0 3.7,16.1,0 3.6,16.1,0 3.6,16.1,0 3.6,16,0 3.5,16,0 3.5,15.9,0 3.5,15.9,0 3.4,15.8,0 3.4,15.8,0 3.4,15.7,0 3.3,15.7,0 3.3,15.7,0 3.3,15.6,0 3.2,15.6,0 3.2,15.5,0 3.2,15.5,0 3.2,15.4,0 3.1,15.4,0 3.1,15.3,0 3.1,15.3,0 3.1,15.2,0 3.1,15.2,0 3,15.1,0 3,15.1,0 3,15,0 3,15,0 3,14.9,0 3,14.8,0 2.9,14.8,0 2.9,14.7,0 2.9,14.7,0 2.9,14.6,0 2.9,14.6,0 2.9,14.5,0 2.9,14.5,0 2.9,14.4,0 2.9,14.4,0 2.9,14.3,0 2.8,14.2,0 2.8,14.2,0 2.8,14.1,0 2.8,14.1,0 2.8,14,0 2.8,14,0 2.8,13.9,0 2.8,13.9,0 2.8,13.8,0 2.8,13.8,0 2.9,13.7,0 2.9,13.6,0 2.9,13.6,0 2.9,13.5,0 2.9,13.5,0 2.9,13.4,0 2.9,13.4,0 2.9,13.3,0 2.9,13.3,0 2.9,13.2,0 3,13.2,0 3,13.1,0 3,13,0 3,13,0 3,12.9,0 3,12.9,0 3.1,12.8,0 3.1,12.8,0 3.1,12.7,0 3.1,12.7,0 3.1,12.6,0 3.2,12.6,0 3.2,12.5,0 3.2,12.5,0 3.2,12.4,0 3.3,12.4,0 3.3,12.3,0 3.3,12.3,0 3.4,12.3,0 3.4,12.2,0 3.4,12.2,0 3.5,12.1,0 3.5,12.1,0 3.5,12,0 3.6,12,0 3.6,11.9,0 3.6,11.9,0 3.7,11.9,0 3.7,11.8,0 3.7,11.8,0 3.8,11.7,0 3.8,11.7,0 3.9,11.7,0 3.9,11.6,0 3.9,11.6,0 4,11.6,0 4,11.5,0 4.1,11.5,0 4.1,11.5,0 4.2,11.4,0 4.2,11.4,0 4.3,11.4,0 4.3,11.3,0 4.3,11.3,0 4.4,11.3,0 4.4,11.2,0 4.5,11.2,0 4.5,11.2,0 4.6,11.2,0 4.6,11.1,0 4.7,11.1,0 4.7,11.1,0 4.8,11.1,0 4.8,11.1,0 4.9,11,0 4.9,11,0 5,11,0 5,11,0 5.1,11,0 5.2,11,0 5.2,10.9,0 5.3,10.9,0 5.3,10.9,0 5.4,10.9,0 5.4,10.9,0 5.5,10.9,0 5.5,10.9,0 5.6,10.9,0 5.6,10.9,0 5.7,10.9,0 5.7,10.8,0 5.8,10.8,0 5.9,10.8,0 5.9,10.8,0 6,10.8,0 6,10.8,0 6.1,10.8,0 6.1,10.8,0 6.2,10.8,0 6.2,10.8,0 6.3,10.9,0 6.4,10.9,0 6.4,10.9,0 6.5,10.9,0 6.5,10.9,0 6.6,10.9,0 6.6,10.9,0 6.7,10.9,0 6.7,10.9,0 6.8,10.9,0 6.8,11,0 6.9,11,0 6.9,11,0 7,11,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>27,37,0 27.1,36.9,0 27.2,36.8,0 27.3,36.6,0 27.4,36.5,0 27.5,36.4,0 27.6,36.3,0 27.7,36.2,0 27.8,36,0 27.9,35.9,0 28,35.8,0 28.1,35.7,0 28.2,35.6,0 28.3,35.5,0 28.4,35.4,0 28.5,35.3,0 28.7,35.2,0 28.8,35.1,0 28.9,35,0 29,34.9,0 29.1,34.8,0 29.3,34.7,0 29.4,34.6,0 29.5,34.6,0 29.7,34.5,0 29.8,34.4,0 29.9,34.3,0 30.1,34.3,0 30.2,34.2,0 30.3,34.1,0 30.5,34,0 30.6,34,0 30.7,33.9,0 30.9,33.9,0 31,33.8,0 31.2,33.7,0 31.3,33.7,0 31.5,33.6,0 31.6,33.6,0 31.8,33.6,0 31.9,33.5,0 32.1,33.5,0 32.2,33.4,0 32.4,33.4,0 32.5,33.4,0 32.7,33.3,0 32.8,33.3,0 33,33.3,0 33.1,33.3,0 33.3,33.2,0 33.4,33.2,0 33.6,33.2,0 33.7,33.2,0 33.9,33.2,0 34,33.2,0 34.2,33.2,0 34.3,33.2,0 34.5,33.2,0 34.6,33.2,0 34.8,33.2,0 34.9,33.2,0 35.1,33.2,0 35.3,33.3,0 35.4,33.3,0 35.6,33.3,0 35.7,33.3,0 35.9,33.3,0 36,33.4,0 36.2,33.4,0 36.3,33.4,0 36.5,33.5,0 36.6,33.5,0 36.8,33.6,0 36.9,33.6,0 37.1,33.7,0 37.2,33.7,0 37.3,33.8,0 37.5,33.8,0 37.6,33.9,0 37.8,33.9,0 37.9,34,0 38,34.1,0 38.2,34.1,0 38.3,34.2,0 38.5,34.3,0 38.6,34.3,0 38.7,34.4,0 38.9,34.5,0 39,34.6,0 39.1,34.7,0 39.2,34.7,0 39.4,34.8,0 39.5,34.9,0 39.6,35,0 39.7,35.1,0 39.9,35.2,0 40,35.3,0 40.1,35.4,0 40.2,35.5,0 40.3,35.6,0 40.4,35.7,0 40.5,35.8,0 40.6,35.9,0 40.7,36.1,0 40.8,36.2,0 40.9,36.3,0 41,36.4,0 41.1,36.5,0 41.2,36.6,0 41.3,36.8,0 41.4,36.9,0 41.5,37,0 41.6,37.1,0 41.7,37.3,0 41.8,37.4,0 41.8,37.5,0 41.9,37.7,0 42,37.8,0 42.1,37.9,0 42.1,38.1,0 42.2,38.2,0 42.3,38.3,0 42.3,38.5,0 42.4,38.6,0 42.4,38.8,0 42.5,38.9,0 42.6,39.1,0 42.6,39.2,0 42.6,39.4,0 42.7,39.5,0 42.7,39.6,0 42.8,39.8,0 42.8,39.9,0 42.8,40.1,0 42.9,40.2,0 42.9,40.4,0 42.9,40.5,0 43,40.7,0 43,40.9,0 43,41,0 43,41.2,0 43,41.3,0 43,41.5,0 43,41.6,0 43.1,41.8,0 43.1,41.9,0 43.1,42.1,0 43.1,42.2,0 43,42.4,0 43,42.5,0 43,42.7,0 43,42.8,0 43,43,0 43,43.1,0 43,43.3,0 42.9,43.5,0 42.9,43.6,0 42.9,43.8,0 42.8,43.9,0 42.8,44.1,0 42.8,44.2,0 42.7,44.4,0 42.7,44.5,0 42.6,44.6,0 42.6,44.8,0 42.6,44.9,0 42.5,45.1,0 42.4,45.2,0 42.4,45.4,0 42.3,45.5,0 42.3,45.7,0 42.2,45.8,0 42.1,45.9,0 42.1,46.1,0 42,46.2,0 41.9,46.3,0 41.8,46.5,0 41.8,46.6,0 41.7,46.7,0 41.6,46.9,0 41.5,47,0 41.4,47.1,0 41.3,47.2,0 41.2,47.4,0 41.1,47.5,0 41,47.6,0 40.9,47.7,0 40.8,47.8,0 40.7,47.9,0 40.6,48.1,0 40.5,48.2,0 40.4,48.3,0 40.3,48.4,0 40.2,48.5,0 40.1,48.6,0 40,48.7,0 39.9,48.8,0 39.7,48.9,0 39.6,49,0 39.5,49.1,0 39.4,49.2,0 39.2,49.3,0 39.1,49.3,0 39,49.4,0 38.9,49.5,0 38.7,49.6,0 38.6,49.7,0 38.5,49.7,0 38.3,49.8,0 38.2,49.9,0 38,49.9,0 37.9,50,0 37.8,50.1,0 37.6,50.1,0 37.5,50.2,0 37.3,50.2,0 37.2,50.3,0 37.1,50.3,0 36.9,50.4,0 36.8,50.4,0 36.6,50.5,0 36.5,50.5,0 36.3,50.6,0 36.2,50.6,0 36,50.6,0 35.9,50.7,0 35.7,50.7,0 35.6,50.7,0 35.4,50.7,0 35.3,50.7,0 35.1,50.8,0 34.9,50.8,0 34.8,50.8,0 34.6,50.8,0 34.5,50.8,0 34.3,50.8,0 34.2,50.8,0 34,50.8,0 33.9,50.8,0 33.7,50.8,0 33.6,50.8,0 33.4,50.8,0 33.3,50.8,0 33.1,50.7,0 33,50.7,0 32.8,50.7,0 32.7,50.7,0 32.5,50.6,0 32.4,50.6,0 32.2,50.6,0 32.1,50.5,0 31.9,50.5,0 31.8,50.4,0 31.6,50.4,0 31.5,50.4,0 31.3,50.3,0 31.2,50.3,0 31,50.2,0 30.9,50.1,0 30.7,50.1,0 30.6,50,0 30.5,50,0 30.3,49.9,0 30.2,49.8,0 30.1,49.7,0 29.9,49.7,0 29.8,49.6,0 29.7,49.5,0 29.5,49.4,0 29.4,49.4,0 29.3,49.3,0 29.1,49.2,0 29,49.1,0 28.9,49,0 28.8,48.9,0 28.7,48.8,0 28.5,48.7,0 28.4,48.6,0 28.3,48.5,0 28.2,48.4,0 28.1,48.3,0 28,48.2,0 27.9,48.1,0 27.8,48,0 27.7,47.8,0 27.6,47.7,0 27.5,47.6,0 27.4,47.5,0 27.3,47.4,0 27.2,47.2,0 27.1,47.1,0 27,47,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml( 1 ), expectedKml );

  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 2.316,5.677,0 2.298,5.687,0 2.28,5.697,0 2.262,5.707,0 2.244,5.716,0 2.226,5.725,0 2.207,5.734,0 2.188,5.742,0 2.17,5.75,0 2.151,5.757,0 2.131,5.765,0 2.112,5.772,0 2.093,5.778,0 2.074,5.785,0 2.054,5.79,0 2.034,5.796,0 2.015,5.801,0 1.995,5.806,0 1.975,5.811,0 1.955,5.815,0 1.935,5.819,0 1.915,5.822,0 1.894,5.826,0 1.874,5.828,0 1.854,5.831,0 1.834,5.833,0 1.813,5.835,0 1.793,5.836,0 1.773,5.837,0 1.752,5.838,0 1.732,5.838,0 1.711,5.838,0 1.691,5.838,0 1.67,5.837,0 1.65,5.836,0 1.63,5.834,0 1.609,5.833,0 1.589,5.83,0 1.569,5.828,0 1.548,5.825,0 1.528,5.822,0 1.508,5.818,0 1.488,5.814,0 1.468,5.81,0 1.448,5.805,0 1.428,5.801,0 1.409,5.795,0 1.389,5.79,0 1.37,5.784,0 1.35,5.777,0 1.331,5.771,0 1.312,5.764,0 1.293,5.756,0 1.274,5.749,0 1.255,5.741,0 1.236,5.732,0 1.218,5.724,0 1.199,5.715,0 1.181,5.705,0 1.163,5.696,0 1.145,5.686,0 1.128,5.676,0 1.11,5.665,0 1.093,5.654,0 1.076,5.643,0 1.059,5.632,0 1.042,5.62,0 1.025,5.608,0 1.009,5.596,0 0.993,5.583,0 0.977,5.57,0 0.962,5.557,0 0.946,5.543,0 0.931,5.53,0 0.916,5.516,0 0.901,5.502,0 0.887,5.487,0 0.873,5.473,0 0.859,5.458,0 0.845,5.442,0 0.832,5.427,0 0.819,5.411,0 0.806,5.395,0 0.793,5.379,0 0.781,5.363,0 0.769,5.346,0 0.757,5.33,0 0.746,5.313,0 0.735,5.296,0 0.724,5.278,0 0.713,5.261,0 0.703,5.243,0 0.693,5.225,0 0.684,5.207,0 0.674,5.189,0 0.666,5.171,0 0.657,5.152,0 0.649,5.133,0 0.641,5.115,0 0.633,5.096,0 0.626,5.077,0 0.619,5.057,0 0.612,5.038,0 0.606,5.019,0 0.6,4.999,0 0.594,4.979,0 0.589,4.96,0 0.584,4.94,0 0.579,4.92,0 0.575,4.9,0 0.571,4.88,0 0.568,4.86,0 0.564,4.84,0 0.562,4.819,0 0.559,4.799,0 0.557,4.779,0 0.555,4.759,0 0.554,4.738,0 0.553,4.718,0 0.552,4.697,0 0.552,4.677,0 0.552,4.656,0 0.552,4.636,0 0.553,4.616,0 0.554,4.595,0 0.555,4.575,0 0.557,4.554,0 0.559,4.534,0 0.562,4.514,0 0.564,4.494,0 0.568,4.473,0 0.571,4.453,0 0.575,4.433,0 0.579,4.413,0 0.584,4.393,0 0.589,4.374,0 0.594,4.354,0 0.6,4.334,0 0.606,4.315,0 0.612,4.295,0 0.619,4.276,0 0.626,4.257,0 0.633,4.238,0 0.641,4.219,0 0.649,4.2,0 0.657,4.181,0 0.666,4.163,0 0.674,4.144,0 0.684,4.126,0 0.693,4.108,0 0.703,4.09,0 0.713,4.073,0 0.724,4.055,0 0.735,4.038,0 0.746,4.021,0 0.757,4.004,0 0.769,3.987,0 0.781,3.97,0 0.793,3.954,0 0.806,3.938,0 0.819,3.922,0 0.832,3.906,0 0.845,3.891,0 0.859,3.876,0 0.873,3.861,0 0.887,3.846,0 0.901,3.832,0 0.916,3.817,0 0.931,3.804,0 0.946,3.79,0 0.962,3.776,0 0.977,3.763,0 0.993,3.75,0 1.009,3.738,0 1.025,3.726,0 1.042,3.713,0 1.059,3.702,0 1.076,3.69,0 1.093,3.679,0 1.11,3.668,0 1.128,3.658,0 1.145,3.648,0 1.163,3.638,0 1.181,3.628,0 1.199,3.619,0 1.218,3.61,0 1.236,3.601,0 1.255,3.593,0 1.274,3.585,0 1.293,3.577,0 1.312,3.57,0 1.331,3.563,0 1.35,3.556,0 1.37,3.55,0 1.389,3.544,0 1.409,3.538,0 1.428,3.533,0 1.448,3.528,0 1.468,3.523,0 1.488,3.519,0 1.508,3.515,0 1.528,3.511,0 1.548,3.508,0 1.569,3.505,0 1.589,3.503,0 1.609,3.501,0 1.63,3.499,0 1.65,3.497,0 1.67,3.496,0 1.691,3.496,0 1.711,3.495,0 1.732,3.495,0 1.752,3.496,0 1.773,3.496,0 1.793,3.497,0 1.813,3.499,0 1.834,3.5,0 1.854,3.503,0 1.874,3.505,0 1.894,3.508,0 1.915,3.511,0 1.935,3.514,0 1.955,3.518,0 1.975,3.523,0 1.995,3.527,0 2.015,3.532,0 2.034,3.537,0 2.054,3.543,0 2.074,3.549,0 2.093,3.555,0 2.112,3.562,0 2.131,3.569,0 2.151,3.576,0 2.17,3.584,0 2.188,3.592,0 2.207,3.6,0 2.226,3.608,0 2.244,3.617,0 2.262,3.627,0 2.28,3.636,0 2.298,3.646,0 2.316,3.656,0 2.333,3.667,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 8.966,4.178,0 8.932,4.244,0 8.896,4.309,0 8.859,4.374,0 8.821,4.438,0 8.782,4.502,0 8.742,4.565,0 8.7,4.627,0 8.658,4.688,0 8.614,4.749,0 8.57,4.809,0 8.524,4.868,0 8.477,4.926,0 8.43,4.983,0 8.381,5.04,0 8.331,5.096,0 8.281,5.151,0 8.229,5.205,0 8.177,5.258,0 8.124,5.31,0 8.069,5.361,0 8.014,5.411,0 7.958,5.461,0 7.901,5.509,0 7.844,5.556,0 7.785,5.603,0 7.726,5.648,0 7.666,5.692,0 7.605,5.735,0 7.543,5.777,0 7.481,5.818,0 7.418,5.858,0 7.354,5.897,0 7.29,5.935,0 7.225,5.971,0 7.159,6.007,0 7.093,6.041,0 7.026,6.074,0 6.958,6.106,0 6.89,6.137,0 6.822,6.167,0 6.753,6.195,0 6.683,6.222,0 6.613,6.248,0 6.543,6.273,0 6.472,6.296,0 6.401,6.319,0 6.329,6.34,0 6.257,6.36,0 6.185,6.378,0 6.112,6.395,0 6.04,6.411,0 5.966,6.426,0 5.893,6.44,0 5.819,6.452,0 5.746,6.463,0 5.672,6.472,0 5.597,6.48,0 5.523,6.487,0 5.449,6.493,0 5.374,6.498,0 5.3,6.501,0 5.225,6.503,0 5.15,6.503,0 5.076,6.502,0 5.001,6.5,0 4.927,6.497,0 4.852,6.492,0 4.778,6.486,0 4.704,6.479,0 4.629,6.47,0 4.555,6.46,0 4.482,6.449,0 4.408,6.437,0 4.335,6.423,0 4.262,6.408,0 4.189,6.392,0 4.116,6.374,0 4.044,6.355,0 3.972,6.335,0 3.901,6.314,0 3.83,6.292,0 3.759,6.268,0 3.688,6.243,0 3.619,6.217,0 3.549,6.189,0 3.48,6.16,0 3.412,6.131,0 3.344,6.1,0 3.277,6.067,0 3.21,6.034,0 3.144,5.999,0 3.078,5.964,0 3.013,5.927,0 2.949,5.889,0 2.886,5.85,0 2.823,5.81,0 2.761,5.768,0 2.699,5.726,0 2.638,5.683,0 2.578,5.638,0 2.519,5.593,0 2.461,5.546,0 2.403,5.499,0 2.347,5.45,0 2.291,5.401,0 2.236,5.35,0 2.182,5.299,0 2.129,5.246,0 2.076,5.193,0 2.025,5.139,0 1.975,5.084,0 1.925,5.028,0 1.877,4.971,0 1.829,4.914,0 1.783,4.855,0 1.738,4.796,0 1.693,4.736,0 1.65,4.675,0 1.608,4.614,0 1.567,4.551,0 1.527,4.488,0 1.488,4.425,0 1.45,4.36,0 1.413,4.295,0 1.378,4.23,0 1.343,4.164,0 1.31,4.097,0 1.278,4.029,0 1.247,3.961,0 1.217,3.893,0 1.189,3.824,0 1.161,3.755,0 1.135,3.685,0 1.11,3.614,0 1.087,3.544,0 1.064,3.472,0 1.043,3.401,0 1.023,3.329,0 1.004,3.257,0 0.987,3.184,0 0.971,3.111,0 0.956,3.038,0 0.942,2.965,0 0.93,2.891,0 0.919,2.817,0 0.909,2.743,0 0.901,2.669,0 0.894,2.595,0 0.888,2.52,0 0.883,2.446,0 0.88,2.371,0 0.878,2.297,0 0.878,2.222,0 0.878,2.148,0 0.88,2.073,0 0.883,1.998,0 0.888,1.924,0 0.894,1.85,0 0.901,1.775,0 0.909,1.701,0 0.919,1.627,0 0.93,1.553,0 0.942,1.48,0 0.956,1.406,0 0.971,1.333,0 0.987,1.26,0 1.004,1.188,0 1.023,1.116,0 1.043,1.044,0 1.064,0.972,0 1.087,0.901,0 1.11,0.83,0 1.135,0.76,0 1.161,0.69,0 1.189,0.62,0 1.217,0.551,0 1.247,0.483,0 1.278,0.415,0 1.31,0.348,0 1.343,0.281,0 1.378,0.215,0 1.413,0.149,0 1.45,0.084,0 1.488,0.02,0 1.527,-0.044,0 1.567,-0.107,0 1.608,-0.169,0 1.65,-0.231,0 1.693,-0.291,0 1.738,-0.351,0 1.783,-0.411,0 1.829,-0.469,0 1.877,-0.527,0 1.925,-0.584,0 1.975,-0.639,0 2.025,-0.694,0 2.076,-0.749,0 2.129,-0.802,0 2.182,-0.854,0 2.236,-0.906,0 2.291,-0.956,0 2.347,-1.006,0 2.403,-1.054,0 2.461,-1.102,0 2.519,-1.148,0 2.578,-1.194,0 2.638,-1.238,0 2.699,-1.282,0 2.761,-1.324,0 2.823,-1.365,0 2.886,-1.405,0 2.949,-1.444,0 3.013,-1.482,0 3.078,-1.519,0 3.144,-1.555,0 3.21,-1.589,0 3.277,-1.623,0 3.344,-1.655,0 3.412,-1.686,0 3.48,-1.716,0 3.549,-1.745,0 3.619,-1.772,0 3.688,-1.798,0 3.759,-1.823,0 3.83,-1.847,0 3.901,-1.87,0 3.972,-1.891,0 4.044,-1.911,0 4.116,-1.93,0 4.189,-1.947,0 4.262,-1.964,0 4.335,-1.979,0 4.408,-1.992,0 4.482,-2.005,0 4.555,-2.016,0 4.629,-2.026,0 4.704,-2.034,0 4.778,-2.042,0 4.852,-2.048,0 4.927,-2.052,0 5.001,-2.056,0 5.076,-2.058,0 5.15,-2.059,0 5.225,-2.058,0 5.3,-2.056,0 5.374,-2.053,0 5.449,-2.049,0 5.523,-2.043,0 5.597,-2.036,0 5.672,-2.028,0 5.746,-2.018,0 5.819,-2.007,0 5.893,-1.995,0 5.966,-1.982,0 6.04,-1.967,0 6.112,-1.951,0 6.185,-1.934,0 6.257,-1.915,0 6.329,-1.895,0 6.401,-1.874,0 6.472,-1.852,0 6.543,-1.829,0 6.613,-1.804,0 6.683,-1.778,0 6.753,-1.751,0 6.822,-1.722,0 6.89,-1.693,0 6.958,-1.662,0 7.026,-1.63,0 7.093,-1.597,0 7.159,-1.562,0 7.225,-1.527,0 7.29,-1.49,0 7.354,-1.453,0 7.418,-1.414,0 7.481,-1.374,0 7.543,-1.333,0 7.605,-1.291,0 7.666,-1.248,0 7.726,-1.203,0 7.785,-1.158,0 7.844,-1.112,0 7.901,-1.065,0 7.958,-1.016,0 8.014,-0.967,0 8.069,-0.917,0 8.124,-0.865,0 8.177,-0.813,0 8.229,-0.76,0 8.281,-0.706,0 8.331,-0.651,0 8.381,-0.596,0 8.43,-0.539,0 8.477,-0.482,0 8.524,-0.423,0 8.57,-0.364,0 8.614,-0.304,0 8.658,-0.244,0 8.7,-0.182,0 8.742,-0.12,0 8.782,-0.057,0 8.821,0.006,0 8.859,0.07,0 8.896,0.135,0 8.932,0.201,0 8.966,0.267,0 9,0.333,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );
}


QGSTEST_MAIN( TestQgsMultiCurve )
#include "testqgsmulticurve.moc"
