/***************************************************************************
     testqgsmultilinestring.cpp
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

#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"

#include "testgeometryutils.h"

class TestQgsMultiLineString: public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void addGeometry();
    void addGeometryInitialDimension();
    void addGeometryZ();
    void addGeometryM();
    void addGeometryZM();
    void insertGeometry();
    void assignment();
    void clone();
    void copy();
    void clear();
    void cast();
    void vertexIterator();
    void boundary();
    void boundaryZ();
    void segmentLength();
    void toCurveType();
    void toFromWKT();
    void toFromWKB();
    void exportImport();
};

void TestQgsMultiLineString::constructor()
{
  QgsMultiLineString mls;

  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.nCoordinates(), 0 );
  QCOMPARE( mls.ringCount(), 0 );
  QCOMPARE( mls.partCount(), 0 );
  QVERIFY( !mls.is3D() );
  QVERIFY( !mls.isMeasure() );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( mls.wktTypeStr(), QString( "MultiLineString" ) );
  QCOMPARE( mls.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( mls.dimension(), 0 );
  QVERIFY( !mls.hasCurvedSegments() );
  QCOMPARE( mls.area(), 0.0 );
  QCOMPARE( mls.perimeter(), 0.0 );
  QCOMPARE( mls.numGeometries(), 0 );
  QVERIFY( !mls.geometryN( 0 ) );
  QVERIFY( !mls.geometryN( -1 ) );
  QCOMPARE( mls.vertexCount( 0, 0 ), 0 );
  QCOMPARE( mls.vertexCount( 0, 1 ), 0 );
  QCOMPARE( mls.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiLineString::addGeometry()
{
  QgsMultiLineString mls;

  //try with nullptr
  mls.addGeometry( nullptr );
  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.nCoordinates(), 0 );
  QCOMPARE( mls.ringCount(), 0 );
  QCOMPARE( mls.partCount(), 0 );
  QCOMPARE( mls.numGeometries(), 0 );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
  QVERIFY( !mls.geometryN( 0 ) );
  QVERIFY( !mls.geometryN( -1 ) );

  // not a linestring
  QVERIFY( !mls.addGeometry( new QgsPoint() ) );
  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.nCoordinates(), 0 );
  QCOMPARE( mls.ringCount(), 0 );
  QCOMPARE( mls.partCount(), 0 );
  QCOMPARE( mls.numGeometries(), 0 );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
  QVERIFY( !mls.geometryN( 0 ) );
  QVERIFY( !mls.geometryN( -1 ) );

  //valid geometry
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) );
  mls.addGeometry( part.clone() );

  QVERIFY( !mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 1 );
  QCOMPARE( mls.nCoordinates(), 2 );
  QCOMPARE( mls.ringCount(), 1 );
  QCOMPARE( mls.partCount(), 1 );
  QVERIFY( !mls.is3D() );
  QVERIFY( !mls.isMeasure() );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( mls.wktTypeStr(), QString( "MultiLineString" ) );
  QCOMPARE( mls.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( mls.dimension(), 1 );
  QVERIFY( !mls.hasCurvedSegments() );
  QCOMPARE( mls.area(), 0.0 );
  QCOMPARE( mls.perimeter(), 0.0 );
  QVERIFY( mls.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls.geometryN( 0 ) ), part );
  QVERIFY( !mls.geometryN( 100 ) );
  QVERIFY( !mls.geometryN( -1 ) );
  QCOMPARE( mls.vertexCount( 0, 0 ), 2 );
  QCOMPARE( mls.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiLineString::addGeometryInitialDimension()
{
  QgsMultiLineString mls;

  //initial adding of geometry should set z/m type
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 ) );
  mls.addGeometry( part.clone() );

  QVERIFY( mls.is3D() );
  QVERIFY( !mls.isMeasure() );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QCOMPARE( mls.wktTypeStr(), QString( "MultiLineStringZ" ) );
  QCOMPARE( mls.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( mls.geometryN( 0 ) ) ), part );

  mls.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 ) );
  mls.addGeometry( part.clone() );

  QVERIFY( !mls.is3D() );
  QVERIFY( mls.isMeasure() );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringM );
  QCOMPARE( mls.wktTypeStr(), QString( "MultiLineStringM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( mls.geometryN( 0 ) ) ), part );

  mls.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 ) );
  mls.addGeometry( part.clone() );

  QVERIFY( mls.is3D() );
  QVERIFY( mls.isMeasure() );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( mls.wktTypeStr(), QString( "MultiLineStringZM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( mls.geometryN( 0 ) ) ), part );

  //add another part
  mls.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) );
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.vertexCount( 0, 0 ), 2 );

  part.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 ) );
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.vertexCount( 1, 0 ), 2 );
  QCOMPARE( mls.numGeometries(), 2 );
  QVERIFY( mls.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of points type
  mls.clear();
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );

  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( mls.vertexCount( 0, 0 ), 2 );
  QCOMPARE( mls.vertexCount( 1, 0 ), 2 );
  QCOMPARE( mls.vertexCount( 2, 0 ), 0 );
  QCOMPARE( mls.vertexCount( -1, 0 ), 0 );
  QCOMPARE( mls.nCoordinates(), 4 );
  QCOMPARE( mls.ringCount(), 1 );
  QCOMPARE( mls.partCount(), 2 );
  QVERIFY( !mls.is3D() );

  const QgsLineString *ls = static_cast< const QgsLineString * >( mls.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 13 ) );

  // test lineStringN by the same occasion
  QCOMPARE( static_cast< const QgsLineString * >( mls.lineStringN( 0 ) ),
            static_cast< const QgsLineString * >( mls.geometryN( 0 ) ) );

  ls = static_cast< const QgsLineString * >( mls.lineStringN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11 ) );

  QCOMPARE( static_cast< const QgsLineString * >( mls.lineStringN( 1 ) ),
            static_cast< const QgsLineString * >( mls.geometryN( 1 ) ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( mls.vertexCount( 0, 0 ), 2 );
  QCOMPARE( mls.vertexCount( 1, 0 ), 2 );
  QCOMPARE( mls.vertexCount( 2, 0 ), 2 );
  QCOMPARE( mls.nCoordinates(), 6 );
  QCOMPARE( mls.ringCount(), 1 );
  QCOMPARE( mls.partCount(), 3 );
  QVERIFY( !mls.is3D() );
  QVERIFY( !mls.isMeasure() );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 32, 41 ) );
}

void TestQgsMultiLineString::addGeometryZ()
{
  QgsMultiLineString mls;

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZ );

  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( mls.is3D() );

  const QgsLineString *ls = static_cast< const QgsLineString * >( mls.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11, 3 ) );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 31, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( mls.is3D() );
  QVERIFY( !mls.isMeasure() );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
}

void TestQgsMultiLineString::addGeometryM()
{
  QgsMultiLineString mls;

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringM );

  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringM );
  QVERIFY( mls.isMeasure() );

  const QgsLineString *ls = static_cast< const QgsLineString * >( mls.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringM );
  QVERIFY( !mls.is3D() );
  QVERIFY( mls.isMeasure() );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
}

void TestQgsMultiLineString::addGeometryZM()
{
  QgsMultiLineString mls;

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZM );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( mls.isMeasure() );
  QVERIFY( mls.is3D() );

  const QgsLineString *ls = static_cast< const QgsLineString * >( mls.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 )
                  << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( mls.is3D() );
  QVERIFY( mls.isMeasure() );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 )
                  << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 ) ) ;
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( mls.is3D() );
  QVERIFY( mls.isMeasure() );

  ls = static_cast< const QgsLineString * >( mls.geometryN( 3 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
}

void TestQgsMultiLineString::insertGeometry()
{
  QgsMultiLineString mls;
  mls.clear();

  mls.insertGeometry( nullptr, 0 );
  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 0 );

  mls.insertGeometry( nullptr, -1 );
  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 0 );

  mls.insertGeometry( nullptr, 100 );
  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 0 );

  mls.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 0 );

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  mls.insertGeometry( part.clone(), 0 );

  QVERIFY( !mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 1 );
}

void TestQgsMultiLineString::assignment()
{
  QgsMultiLineString mls1;
  QgsMultiLineString mls2;

  mls1 = mls2;
  QCOMPARE( mls1.numGeometries(), 0 );

  QgsMultiLineString mls3;
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  mls3.addGeometry( part.clone() );
  mls3.addGeometry( part.clone() );

  mls1 = mls3;
  QCOMPARE( mls1.numGeometries(), 2 );

  QCOMPARE( *static_cast< const QgsLineString * >( mls1.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( mls1.geometryN( 1 ) ), part );
}

void TestQgsMultiLineString::clone()
{
  QgsMultiLineString mls;
  std::unique_ptr< QgsMultiLineString >cloned( mls.clone() );

  QVERIFY( cloned->isEmpty() );

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  mls.addGeometry( part.clone() );
  mls.addGeometry( part.clone() );

  cloned.reset( mls.clone() );

  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( cloned->geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( cloned->geometryN( 1 ) ), part );
}

void TestQgsMultiLineString::copy()
{
  QgsMultiLineString mls1;
  QgsMultiLineString mls2( mls1 );

  QVERIFY( mls2.isEmpty() );

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  mls1.addGeometry( part.clone() );
  mls1.addGeometry( part.clone() );

  QgsMultiLineString mls3( mls1 );

  QCOMPARE( mls3.numGeometries(), 2 );
  QCOMPARE( mls3.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( *static_cast< const QgsLineString * >( mls3.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( mls3.geometryN( 1 ) ), part );
}

void TestQgsMultiLineString::clear()
{
  QgsMultiLineString mls;

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  mls.addGeometry( part.clone() );
  mls.addGeometry( part.clone() );

  QCOMPARE( mls.numGeometries(), 2 );

  mls.clear();

  QVERIFY( mls.isEmpty() );
  QCOMPARE( mls.numGeometries(), 0 );
  QCOMPARE( mls.nCoordinates(), 0 );
  QCOMPARE( mls.ringCount(), 0 );
  QCOMPARE( mls.partCount(), 0 );
  QVERIFY( !mls.is3D() );
  QVERIFY( !mls.isMeasure() );
  QCOMPARE( mls.wkbType(), QgsWkbTypes::MultiLineString );
}

void TestQgsMultiLineString::cast()
{
  QVERIFY( !QgsMultiLineString().cast( nullptr ) );

  QgsMultiLineString mls;
  QVERIFY( QgsMultiLineString().cast( &mls ) );

  mls.fromWkt( QStringLiteral( "MultiLineStringZ()" ) );
  QVERIFY( QgsMultiLineString().cast( &mls ) );
  mls.fromWkt( QStringLiteral( "MultiLineStringM()" ) );
  QVERIFY( QgsMultiLineString().cast( &mls ) );
  mls.fromWkt( QStringLiteral( "MultiLineStringZM()" ) );
  QVERIFY( QgsMultiLineString().cast( &mls ) );
}

void TestQgsMultiLineString::vertexIterator()
{
  QgsMultiLineString mls;
  QgsLineString part;

  part.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  mls.addGeometry( part.clone() );

  part.setPoints( QgsPointSequence() << QgsPoint( 10, 10 )
                  << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  mls.addGeometry( part.clone() );

  // vertex iterator: 2 linestrings with 3 points each
  QgsAbstractGeometry::vertex_iterator it = mls.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = mls.vertices_end();

  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );

  ++it;
  QCOMPARE( *it, QgsPoint( 1, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 1 ) );

  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 2 ) );

  ++it;
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );

  ++it;
  QCOMPARE( *it, QgsPoint( 11, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 1 ) );

  ++it;
  QCOMPARE( *it, QgsPoint( 11, 11 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 2 ) );

  ++it;
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &mls );

  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 11 ) );

  QVERIFY( !it2.hasNext() );
}

void TestQgsMultiLineString::boundary()
{
  QgsMultiLineString mls;
  QVERIFY( !mls.boundary() );

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  mls.addGeometry( part.clone() );

  QgsAbstractGeometry *boundary = mls.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );

  delete boundary;

  // add another linestring
  part.setPoints( QgsPointSequence() << QgsPoint( 10, 10 )
                  << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  mls.addGeometry( part.clone() );

  boundary = mls.boundary();
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
  part.setPoints( QgsPointSequence() << QgsPoint( 20, 20 ) << QgsPoint( 21, 20 )
                  << QgsPoint( 21, 21 ) << QgsPoint( 20, 20 ) );
  mls.addGeometry( part.clone() );

  boundary = mls.boundary();
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
}

void TestQgsMultiLineString::boundaryZ()
{
  QgsMultiLineString mls;
  QgsLineString part;

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                  << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  mls.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 )
                  << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
  mls.addGeometry( part.clone() );

  QgsAbstractGeometry *boundary = mls.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );

  auto pt = static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( pt->x(), 0.0 );
  QCOMPARE( pt->y(), 0.0 );
  QCOMPARE( pt->z(), 10.0 );

  pt = static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( pt->x(), 1.0 );
  QCOMPARE( pt->y(), 1.0 );
  QCOMPARE( pt->z(), 20.0 );

  pt = static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) );
  QCOMPARE( mpBoundary->geometryN( 2 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( pt->x(), 10.0 );
  QCOMPARE( pt->y(), 10.0 );
  QCOMPARE( pt->z(), 100.0 );

  pt = static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) );
  QCOMPARE( mpBoundary->geometryN( 3 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( pt->x(), 20.0 );
  QCOMPARE( pt->y(), 20.0 );
  QCOMPARE( pt->z(), 200.0 );

  delete boundary;
}

void TestQgsMultiLineString::segmentLength()
{
  QgsMultiLineString mls;

  QCOMPARE( mls.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                << QgsPoint( 111, 2 ) << QgsPoint( 11, 2 ) );
  mls.addGeometry( ls.clone() );

  QCOMPARE( mls.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, -1, 0 ) ), 10.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 1, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 1, 1 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );

  // add another line
  ls.setPoints( QgsPointSequence() << QgsPoint( 30, 6 )
                << QgsPoint( 34, 6 ) << QgsPoint( 34, 8 )
                << QgsPoint( 30, 8 ) << QgsPoint( 30, 6 ) );
  mls.addGeometry( ls.clone() );

  QCOMPARE( mls.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 0, -1, 0 ) ), 10.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, -1 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 0 ) ), 4.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 1 ) ), 2.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 2 ) ), 4.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 3 ) ), 2.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 0, 4 ) ), 0.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 1, 1 ) ), 2.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 1, 1, 2 ) ), 4.0 );
  QCOMPARE( mls.segmentLength( QgsVertexId( 2, 0, 0 ) ), 0.0 );
}

void TestQgsMultiLineString::toCurveType()
{
  QgsMultiLineString mls;
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  mls.addGeometry( part.clone() );
  mls.addGeometry( part.clone() );

  std::unique_ptr< QgsMultiCurve > curveType( mls.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( curveType->numGeometries(), 2 );

  const QgsCompoundCurve *curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 0 ) );
  QCOMPARE( curve->asWkt(), QStringLiteral( "CompoundCurveZM ((5 50 1 4, 6 61 3 5))" ) );

  curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 1 ) );
  QCOMPARE( curve->asWkt(), QStringLiteral( "CompoundCurveZM ((5 50 1 4, 6 61 3 5))" ) );
}

void TestQgsMultiLineString::toFromWKT()
{
  QgsMultiLineString mls1;
  QgsLineString part;

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) ) ;
  mls1.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) ) ;
  mls1.addGeometry( part.clone() );

  QString wkt = mls1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsMultiLineString mls2;

  QVERIFY( mls2.fromWkt( wkt ) );
  QCOMPARE( mls2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 0 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 1 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 1 ) ) );

  //bad WKT
  mls1.clear();

  QVERIFY( !mls1.fromWkt( "Point()" ) );
  QVERIFY( mls1.isEmpty() );
  QCOMPARE( mls1.numGeometries(), 0 );
  QCOMPARE( mls1.wkbType(), QgsWkbTypes::MultiLineString );
}

void TestQgsMultiLineString::toFromWKB()
{
  QgsMultiLineString mls1;

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  mls1.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 ) ) ;
  mls1.addGeometry( part.clone() );

  QByteArray wkb = mls1.asWkb();
  QgsConstWkbPtr wkbPtr1( wkb );
  QgsMultiLineString mls2;
  mls2.fromWkb( wkbPtr1 );

  QCOMPARE( mls2.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 0 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 1 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 1 ) ) );

  //parts with Z
  mls1.clear();
  mls2.clear();

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) ) ;
  mls1.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) ) ;
  mls1.addGeometry( part.clone() );

  wkb = mls1.asWkb();
  QgsConstWkbPtr wkbPtr2( wkb );
  mls2.fromWkb( wkbPtr2 );

  QCOMPARE( mls2.numGeometries(), 2 );
  QCOMPARE( mls2.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 0 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 1 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 1 ) ) );

  //parts with m
  mls1.clear();
  mls2.clear();

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 ) ) ;
  mls1.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 ) ) ;
  mls1.addGeometry( part.clone() );

  wkb = mls1.asWkb();
  QgsConstWkbPtr wkbPtr3( wkb );
  mls2.fromWkb( wkbPtr3 );

  QCOMPARE( mls2.numGeometries(), 2 );
  QCOMPARE( mls2.wkbType(), QgsWkbTypes::MultiLineStringM );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 0 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 1 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 1 ) ) );

  // parts with ZM
  mls1.clear();
  mls2.clear();

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) ) ;
  mls1.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) ) ;
  mls1.addGeometry( part.clone() );

  wkb = mls1.asWkb();
  QgsConstWkbPtr wkbPtr4( wkb );
  mls2.fromWkb( wkbPtr4 );

  QCOMPARE( mls2.numGeometries(), 2 );
  QCOMPARE( mls2.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 0 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( mls2.geometryN( 1 ) ),
            *static_cast< const QgsLineString * >( mls1.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  mls2.clear();

  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !mls2.fromWkb( nullPtr ) );
  QCOMPARE( mls2.wkbType(), QgsWkbTypes::MultiLineString );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !mls2.fromWkb( wkbPointPtr ) );
  QCOMPARE( mls2.wkbType(), QgsWkbTypes::MultiLineString );
}

void TestQgsMultiLineString::exportImport()
{
  //as JSON
  QgsMultiLineString exportC;

  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 )
                  << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  exportC.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 )
                  << QgsPoint( QgsWkbTypes::Point, 43, 43 ) ) ;
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 3,13</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 43,43</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );

  QString expectedGML2empty( QStringLiteral( "<MultiLineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiLineString().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13</posList></LineString></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43</posList></LineString></curveMember></MultiCurve>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );

  QString expectedGML3empty( QStringLiteral( "<MultiCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiLineString().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[7.0,17.0],[3.0,13.0]],[[27.0,37.0],[43.0,43.0]]],\"type\":\"MultiLineString\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiLineString exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) ) ;
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( "{\"coordinates\":[[[2.333,5.667],[0.6,4.333]],[[9.0,4.111],[1.049,1.024]]],\"type\":\"MultiLineString\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333</posList></LineString></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024</posList></LineString></curveMember></MultiCurve>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>7,17,0 3,13,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>27,37,0 43,43,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml(), expectedKml );

  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 0.6,4.333,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 1.049,1.024,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );
}


QGSTEST_MAIN( TestQgsMultiLineString )
#include "testqgsmultilinestring.moc"
