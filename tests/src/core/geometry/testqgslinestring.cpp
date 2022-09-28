/***************************************************************************
     testqgslinestring.cpp
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
                           (C) 2021 by Antoine Facchini
                           (C) 2021 by Benoit De Mezzo Facchini
    Email                : loic dot bartoletti at oslandia dot com
                           antoine dot facchini at oslandia dot com
                           benoit dot de dot mezzo at oslandia dot com
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
#include "qgsfeedback.h"
#include "qgsgeometryutils.h"
#include "qgslinesegment.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"
#include "testtransformer.h"


class TestQgsLineString: public QObject
{
    Q_OBJECT
  private slots:
    void constructorEmpty();
    void constructorFromArrayZ();
    void constructorFromArrayM();
    void constructorFromArrayZM();
    void constructorFromArray();
    void constructorFromQVector();
    void constructorFromQgsPointSequence();
    void constructorFrom2Pts();
    void constructorFromLineSegment();
    void addVertex();
    void clear();
    void setPoints();
    void pointN();
    void gettersSetters();
    void appendWithZM();
    void append();
    void equality();
    void close();
    void asQPolygonF();
    void clone();
    void toWkbFromWkb();
    void toWktFromWkt();
    void exportAs();
    void length();
    void startEndPoint();
    void length3D();
    void curveToLine();
    void points();
    void CRSTransform();
    void QTransformation();
    void insertVertex();
    void moveVertex();
    void deleteVertex();
    void reversed();
    void addZValue();
    void addMValue();
    void dropZValue();
    void dropMValue();
    void convertTo();
    void isRing();
    void coordinateSequence();
    void nextVertex();
    void vertexIterator();
    void vertexAtPointAt();
    void vertexAtPointAtZ();
    void vertexAtPointAtM();
    void vertexAtPointAtZM();
    void vertexAtPointAt25D();
    void centroid();
    void closestSegment();
    void sumUpArea();
    void boundingBox();
    void boundingBox3D();
    void angle();
    void removingVertexRemoveLine();
    void boundary();
    void extend();
    void addToPainterPath();
    void toCurveType();
    void adjacentVertices();
    void vertexNumberFromVertexId();
    void segmentLength();
    void boundingBoxIntersects();
    void orientation();
    void collectDuplicateNodes();
    void removeDuplicateNodes();
    void swapXy();
    void filterVertices();
    void transformVertices();
    void curveSubstring();
    void interpolatePoint();
    void visitPoints();
    void setPointsFromData();
};

void TestQgsLineString::constructorEmpty()
{
  QgsLineString ls;

  QVERIFY( ls.isEmpty() );
  QCOMPARE( ls.numPoints(), 0 );
  QCOMPARE( ls.vertexCount(), 0 );
  QCOMPARE( ls.nCoordinates(), 0 );
  QCOMPARE( ls.ringCount(), 0 );
  QCOMPARE( ls.partCount(), 0 );
  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.wktTypeStr(), QString( "LineString" ) );
  QCOMPARE( ls.geometryType(), QString( "LineString" ) );
  QCOMPARE( ls.dimension(), 1 );
  QVERIFY( !ls.hasCurvedSegments() );
  QCOMPARE( ls.area(), 0.0 );
  QCOMPARE( ls.perimeter(), 0.0 );
}

void TestQgsLineString::constructorFromArrayZ()
{
  QVector< double > xx( {1, 2, 3} );
  QVector< double > yy( {11, 12, 13} );
  QVector< double > zz( {21, 22, 23} );

  QgsLineString ls( xx, yy, zz );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.zAt( 0 ), 21.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.zAt( 1 ), 22.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( ls.zAt( 2 ), 23.0 );

  // LineString25D
  ls = QgsLineString( xx, yy, zz, QVector< double >(), true );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.zAt( 0 ), 21.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.zAt( 1 ), 22.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( ls.zAt( 2 ), 23.0 );

  // unbalanced -> z ignored
  zz = QVector< double >( {21, 22} );
  ls = QgsLineString( xx, yy, zz );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );

  // unbalanced -> z truncated
  zz = QVector< double >( {21, 22, 23, 24} );
  ls = QgsLineString( xx, yy, zz );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.zAt( 0 ), 21.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.zAt( 1 ), 22.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( ls.zAt( 2 ), 23.0 );
}

void TestQgsLineString::constructorFromArrayM()
{
  QVector< double > xx( {1, 2, 3} );
  QVector< double > yy( {11, 12, 13} );
  QVector< double > mm( {21, 22, 23} );
  QgsLineString ls( xx, yy, QVector< double >(), mm );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.mAt( 0 ), 21.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.mAt( 1 ), 22.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( ls.mAt( 2 ), 23.0 );

  // unbalanced -> m ignored
  mm = QVector< double >( {21, 22} );
  ls = QgsLineString( xx, yy, QVector< double >(), mm );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );

  // unbalanced -> m truncated
  mm = QVector< double >( {21, 22, 23, 24} );;
  ls = QgsLineString( xx, yy, QVector< double >(), mm );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.mAt( 0 ), 21.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.mAt( 1 ), 22.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( ls.mAt( 2 ), 23.0 );
}

void TestQgsLineString::constructorFromArrayZM()
{
  QVector< double > xx( {1, 2, 3} );
  QVector< double > yy( {11, 12, 13} );
  QVector< double > zz( {21, 22, 23} );
  QVector< double > mm( {31, 32, 33} );
  QgsLineString ls( xx, yy, zz, mm );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.zAt( 0 ), 21.0 );
  QCOMPARE( ls.mAt( 0 ), 31.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.zAt( 1 ), 22.0 );
  QCOMPARE( ls.mAt( 1 ), 32.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( ls.zAt( 2 ), 23.0 );
  QCOMPARE( ls.mAt( 2 ), 33.0 );

  QCOMPARE( *ls.xData(), 1.0 );
  QCOMPARE( *( ls.xData() + 1 ), 2.0 );
  QCOMPARE( *( ls.xData() + 2 ), 3.0 );
  QCOMPARE( *ls.yData(), 11.0 );
  QCOMPARE( *( ls.yData() + 1 ), 12.0 );
  QCOMPARE( *( ls.yData() + 2 ), 13.0 );
  QCOMPARE( *ls.zData(), 21.0 );
  QCOMPARE( *( ls.zData() + 1 ), 22.0 );
  QCOMPARE( *( ls.zData() + 2 ), 23.0 );
  QCOMPARE( *ls.mData(), 31.0 );
  QCOMPARE( *( ls.mData() + 1 ), 32.0 );
  QCOMPARE( *( ls.mData() + 2 ), 33.0 );
}

void TestQgsLineString::constructorFromArray()
{
  QVector< double > xx( {1, 2, 3} );
  QVector< double > yy( {11, 12, 13} );
  QgsLineString ls( xx, yy );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.xAt( 2 ), 3.0 );
  QCOMPARE( ls.yAt( 2 ), 13.0 );
  QCOMPARE( *ls.xData(), 1.0 );
  QCOMPARE( *( ls.xData() + 1 ), 2.0 );
  QCOMPARE( *( ls.xData() + 2 ), 3.0 );
  QCOMPARE( *ls.yData(), 11.0 );
  QCOMPARE( *( ls.yData() + 1 ), 12.0 );
  QCOMPARE( *( ls.yData() + 2 ), 13.0 );

  // unbalanced
  xx = QVector< double >( {1, 2} );
  yy = QVector< double >( {11, 12, 13} );
  ls = QgsLineString( xx, yy );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );

  xx = QVector< double >( {1, 2, 3} );
  yy = QVector< double >( {11, 12} );
  ls = QgsLineString( xx, yy );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 11.0 );
  QCOMPARE( ls.xAt( 1 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
}

void TestQgsLineString::constructorFromQVector()
{
  QVector<QgsPointXY> pts;
  pts << QgsPointXY( 1, 2 ) << QgsPointXY( 11, 12 ) << QgsPointXY( 21, 22 );
  QgsLineString ls( pts );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.xAt( 1 ), 11.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.xAt( 2 ), 21.0 );
  QCOMPARE( ls.yAt( 2 ), 22.0 );
}

void TestQgsLineString::constructorFromQgsPointSequence()
{
  QgsLineString ls = QgsLineString( QgsPointSequence() );
  QVERIFY( ls.isEmpty() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );

  QgsPointSequence pts;
  pts << QgsPoint( 10, 20 ) << QgsPoint( 30, 40 );
  ls = QgsLineString( pts );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 10.0 );
  QCOMPARE( ls.yAt( 0 ), 20.0 );
  QCOMPARE( ls.xAt( 1 ), 30.0 );
  QCOMPARE( ls.yAt( 1 ), 40.0 );

  // with z
  QgsPointSequence pts3D;
  pts3D << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 30, 40, 200 );
  ls = QgsLineString( pts3D );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 10.0 );
  QCOMPARE( ls.yAt( 0 ), 20.0 );
  QCOMPARE( ls.zAt( 0 ), 100.0 );
  QCOMPARE( ls.xAt( 1 ), 30.0 );
  QCOMPARE( ls.yAt( 1 ), 40.0 );
  QCOMPARE( ls.zAt( 1 ), 200.0 );

  // with m
  ls = QgsLineString( QgsPointSequence()  << QgsPoint( 1, 2, 0, 4, QgsWkbTypes::PointM ) );
  QCOMPARE( ls.numPoints(), 1 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
}

void TestQgsLineString::constructorFrom2Pts()
{
  QgsLineString ls( QgsPoint( 1, 2 ), QgsPoint( 21, 22 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.xAt( 1 ), 21.0 );
  QCOMPARE( ls.yAt( 1 ), 22.0 );

  ls = QgsLineString( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 23 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.zAt( 0 ), 3.0 );
  QCOMPARE( ls.xAt( 1 ), 21.0 );
  QCOMPARE( ls.yAt( 1 ), 22.0 );
  QCOMPARE( ls.zAt( 1 ), 23.0 );

  ls = QgsLineString( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 23 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.mAt( 0 ), 3.0 );
  QCOMPARE( ls.xAt( 1 ), 21.0 );
  QCOMPARE( ls.yAt( 1 ), 22.0 );
  QCOMPARE( ls.mAt( 1 ), 23.0 );

  ls = QgsLineString( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.zAt( 0 ), 3.0 );
  QCOMPARE( ls.mAt( 0 ), 4.0 );
  QCOMPARE( ls.xAt( 1 ), 21.0 );
  QCOMPARE( ls.yAt( 1 ), 22.0 );
  QCOMPARE( ls.zAt( 1 ), 23.0 );
  QCOMPARE( ls.mAt( 1 ), 24.0 );
}

void TestQgsLineString::constructorFromLineSegment()
{
  QgsLineString ls( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.xAt( 1 ), 3.0 );
  QCOMPARE( ls.yAt( 1 ), 4.0 );
}

void TestQgsLineString::addVertex()
{
  QgsLineString ls;
  ls.addVertex( QgsPoint( 1.0, 2.0 ) );

  QVERIFY( !ls.isEmpty() );
  QCOMPARE( ls.numPoints(), 1 );
  QCOMPARE( ls.vertexCount(), 1 );
  QCOMPARE( ls.nCoordinates(), 1 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !ls.hasCurvedSegments() );
  QCOMPARE( ls.area(), 0.0 );
  QCOMPARE( ls.perimeter(), 0.0 );

  //adding first vertex should set linestring z/m type
  ls = QgsLineString();
  ls.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QVERIFY( !ls.isEmpty() );
  QVERIFY( ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.wktTypeStr(), QString( "LineStringZ" ) );

  ls = QgsLineString();
  ls.addVertex( QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  QVERIFY( !ls.isEmpty() );
  QVERIFY( !ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.wktTypeStr(), QString( "LineStringM" ) );

  ls = QgsLineString();
  ls.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  QVERIFY( !ls.isEmpty() );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.wktTypeStr(), QString( "LineStringZM" ) );

  ls = QgsLineString();
  ls.addVertex( QgsPoint( QgsWkbTypes::Point25D, 1.0, 2.0, 3.0 ) );

  QVERIFY( !ls.isEmpty() );
  QVERIFY( ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.wktTypeStr(), QString( "LineStringZ" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  ls = QgsLineString();
  ls.addVertex( QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  ls.addVertex( QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point

  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.vertexCount(), 2 );
  QCOMPARE( ls.nCoordinates(), 2 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString ); //should still be 2d
  QVERIFY( !ls.is3D() );
  QCOMPARE( ls.area(), 0.0 );
  QCOMPARE( ls.perimeter(), 0.0 );

  ls = QgsLineString();
  ls.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  ls.addVertex( QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) ); //add 2d point

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ ); //should still be 3d
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0 ) );
  QVERIFY( ls.is3D() );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.vertexCount(), 2 );
  QCOMPARE( ls.nCoordinates(), 2 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
}

void TestQgsLineString::clear()
{
  QgsPointSequence pts3D;
  pts3D << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) << QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 );
  QgsLineString ls( pts3D );

  ls.clear();

  QVERIFY( ls.isEmpty() );
  QCOMPARE( ls.numPoints(), 0 );
  QCOMPARE( ls.vertexCount(), 0 );
  QCOMPARE( ls.nCoordinates(), 0 );
  QCOMPARE( ls.ringCount(), 0 );
  QCOMPARE( ls.partCount(), 0 );
  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
}

void TestQgsLineString::setPoints()
{
  QgsLineString ls;
  QgsPointSequence pts;

  pts = QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 );
  ls.setPoints( pts );

  QVERIFY( !ls.isEmpty() );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.vertexCount(), 3 );
  QCOMPARE( ls.nCoordinates(), 3 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !ls.hasCurvedSegments() );

  QgsPointSequence expectedPts;
  ls.points( expectedPts );

  QCOMPARE( expectedPts, pts );
  QCOMPARE( *ls.xData(), 1.0 );
  QCOMPARE( *( ls.xData() + 1 ), 2.0 );
  QCOMPARE( *( ls.xData() + 2 ), 3.0 );
  QCOMPARE( *ls.yData(), 2.0 );
  QCOMPARE( *( ls.yData() + 1 ), 3.0 );
  QCOMPARE( *( ls.yData() + 2 ), 4.0 );

  //setPoints with empty list, should clear linestring
  ls.setPoints( QgsPointSequence() );

  QVERIFY( ls.isEmpty() );
  QCOMPARE( ls.numPoints(), 0 );
  QCOMPARE( ls.vertexCount(), 0 );
  QCOMPARE( ls.nCoordinates(), 0 );
  QCOMPARE( ls.ringCount(), 0 );
  QCOMPARE( ls.partCount(), 0 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );

  ls.points( expectedPts );
  QVERIFY( expectedPts.isEmpty() );

  //setPoints with z
  pts = QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
        << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 );
  ls.setPoints( pts );

  QCOMPARE( ls.numPoints(), 2 );
  QVERIFY( ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );

  ls.points( expectedPts );
  QCOMPARE( expectedPts, pts );

  //setPoints with 25d
  pts = QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 )
        << QgsPoint( QgsWkbTypes::Point25D, 2, 3, 4 );
  ls.setPoints( pts );

  QCOMPARE( ls.numPoints(), 2 );
  QVERIFY( ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) );

  ls.points( expectedPts );
  QCOMPARE( expectedPts, pts );

  //setPoints with m
  pts = QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
        << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 );
  ls.setPoints( pts );

  QCOMPARE( ls.numPoints(), 2 );
  QVERIFY( !ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );

  ls.points( expectedPts );
  QCOMPARE( expectedPts, pts );

  //setPoints with zm
  pts = QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
        << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 );
  ls.setPoints( pts );

  QCOMPARE( ls.numPoints(), 2 );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );

  ls.points( expectedPts );
  QCOMPARE( expectedPts, pts );

  //setPoints with MIXED dimensionality of points
  pts = QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
        << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 );
  ls.setPoints( pts );

  QCOMPARE( ls.numPoints(), 2 );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );

  ls.points( expectedPts );
  QCOMPARE( expectedPts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );;
}

void TestQgsLineString::pointN()
{
  QgsLineString ls( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 )
                    << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPoint bad = ls.pointN( -1 );
  bad = ls.pointN( 100 );
}

void TestQgsLineString::gettersSetters()
{
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );

  QCOMPARE( ls.xAt( 0 ), 1.0 );
  QCOMPARE( ls.xAt( 1 ), 11.0 );
  QCOMPARE( ls.xAt( 2 ), 21.0 );
  QCOMPARE( ls.xAt( -1 ), 0.0 ); //out of range
  QCOMPARE( ls.xAt( 11 ), 0.0 ); //out of range

  ls.setXAt( 0, 51.0 );
  QCOMPARE( ls.xAt( 0 ), 51.0 );

  ls.setXAt( 1, 61.0 );
  QCOMPARE( ls.xAt( 1 ), 61.0 );

  ls.setXAt( -1, 51.0 ); //out of range
  ls.setXAt( 11, 51.0 ); //out of range
  QCOMPARE( ls.yAt( 0 ), 2.0 );
  QCOMPARE( ls.yAt( 1 ), 12.0 );
  QCOMPARE( ls.yAt( 2 ), 22.0 );
  QCOMPARE( ls.yAt( -1 ), 0.0 ); //out of range
  QCOMPARE( ls.yAt( 11 ), 0.0 ); //out of range

  ls.setYAt( 0, 52.0 );
  QCOMPARE( ls.yAt( 0 ), 52.0 );

  ls.setYAt( 1, 62.0 );
  QCOMPARE( ls.yAt( 1 ), 62.0 );

  ls.setYAt( -1, 52.0 ); //out of range
  ls.setYAt( 11, 52.0 ); //out of range
  QCOMPARE( ls.zAt( 0 ), 3.0 );
  QCOMPARE( ls.zAt( 1 ), 13.0 );
  QCOMPARE( ls.zAt( 2 ), 23.0 );
  QVERIFY( std::isnan( ls.zAt( -1 ) ) ); //out of range
  QVERIFY( std::isnan( ls.zAt( 11 ) ) ); //out of range

  ls.setZAt( 0, 53.0 );
  QCOMPARE( ls.zAt( 0 ), 53.0 );

  ls.setZAt( 1, 63.0 );
  QCOMPARE( ls.zAt( 1 ), 63.0 );

  ls.setZAt( -1, 53.0 ); //out of range
  ls.setZAt( 11, 53.0 ); //out of range
  QCOMPARE( ls.mAt( 0 ), 4.0 );
  QCOMPARE( ls.mAt( 1 ), 14.0 );
  QCOMPARE( ls.mAt( 2 ), 24.0 );
  QVERIFY( std::isnan( ls.mAt( -1 ) ) ); //out of range
  QVERIFY( std::isnan( ls.mAt( 11 ) ) ); //out of range

  ls.setMAt( 0, 54.0 );
  QCOMPARE( ls.mAt( 0 ), 54.0 );

  ls.setMAt( 1, 64.0 );
  QCOMPARE( ls.mAt( 1 ), 64.0 );

  ls.setMAt( -1, 54.0 ); //out of range
  ls.setMAt( 11, 54.0 ); //out of range

  //check zAt/setZAt with non-3d linestring
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( ls.zAt( 0 ) ) );
  QVERIFY( std::isnan( ls.zAt( 1 ) ) );
  ls.setZAt( 0, 53.0 );
  ls.setZAt( 1, 63.0 );

  //check mAt/setMAt with non-measure linestring
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( ls.mAt( 0 ) ) );
  QVERIFY( std::isnan( ls.mAt( 1 ) ) );
  ls.setMAt( 0, 53.0 );
  ls.setMAt( 1, 63.0 );
}

void TestQgsLineString::appendWithZM()
{
  QgsLineString ls;
  std::unique_ptr<QgsLineString> toAppend( new QgsLineString() );

  //check dimensionality is inherited from append line if initially empty
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  ls.append( toAppend.get() );

  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( ls.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( ls.pointN( 2 ), toAppend->pointN( 2 ) );

  //append points with z to non z linestring
  ls.clear();
  ls.addVertex( QgsPoint( 1.0, 2.0 ) );

  QVERIFY( !ls.is3D() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );

  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  ls.append( toAppend.get() );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( 31, 32 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( 41, 42 ) );
  QCOMPARE( ls.pointN( 3 ), QgsPoint( 51, 52 ) );

  //append points without z/m to linestring with z & m
  ls.clear();
  ls.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );

  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  ls.append( toAppend.get() );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 41, 42 ) );
  QCOMPARE( ls.pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 51, 52 ) );

  //25d append
  ls.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 )
                       << QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );
  ls.append( toAppend.get() );

  QVERIFY( ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );

  ls.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );

  ls.append( toAppend.get() );
  QVERIFY( ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );
}

void TestQgsLineString::append()
{
  //append to empty
  QgsLineString ls;
  ls.append( nullptr );
  QVERIFY( ls.isEmpty() );
  QCOMPARE( ls.numPoints(), 0 );

  std::unique_ptr<QgsLineString> toAppend( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 12 )
                       << QgsPoint( 21, 22 ) );
  ls.append( toAppend.get() );

  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.vertexCount(), 3 );
  QCOMPARE( ls.nCoordinates(), 3 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( ls.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( ls.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  ls.append( toAppend.get() );

  QCOMPARE( ls.numPoints(), 6 );
  QCOMPARE( ls.vertexCount(), 6 );
  QCOMPARE( ls.nCoordinates(), 6 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QCOMPARE( ls.pointN( 3 ), toAppend->pointN( 0 ) );
  QCOMPARE( ls.pointN( 4 ), toAppend->pointN( 1 ) );
  QCOMPARE( ls.pointN( 5 ), toAppend->pointN( 2 ) );

  //append another line the closes the original geometry.
  //Make sure there are not duplicit points except start and end point
  ls.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 1, 1 )
                       << QgsPoint( 5, 5 )
                       << QgsPoint( 10, 1 ) );
  ls.append( toAppend.get() );

  QCOMPARE( ls.numPoints(), 3 );
  QCOMPARE( ls.vertexCount(), 3 );

  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 10, 1 )
                       << QgsPoint( 1, 1 ) );
  ls.append( toAppend.get() );

  QVERIFY( ls.isClosed() );
  QCOMPARE( ls.numPoints(), 4 );
  QCOMPARE( ls.vertexCount(), 4 );
}

void TestQgsLineString::equality()
{
  QgsLineString ls1;
  QgsLineString ls2;

  QVERIFY( ls1 == ls2 );
  QVERIFY( !( ls1 != ls2 ) );

  //different number of vertices
  ls1.addVertex( QgsPoint( 1, 2 ) );
  QVERIFY( !( ls1 == ls2 ) );
  QVERIFY( ls1 != ls2 );

  ls2.addVertex( QgsPoint( 1, 2 ) );
  QVERIFY( ls1 == ls2 );
  QVERIFY( !( ls1 != ls2 ) );

  //check non-integer equality
  ls1.addVertex( QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  ls2.addVertex( QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( ls1 == ls2 );
  QVERIFY( !( ls1 != ls2 ) );

  //different coordinates
  ls1.addVertex( QgsPoint( 7, 8 ) );
  ls2.addVertex( QgsPoint( 6, 9 ) );
  QVERIFY( !( ls1 == ls2 ) );
  QVERIFY( ls1 != ls2 );

  //different dimension
  QgsLineString ls3;
  ls3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );

  QVERIFY( !( ls1 == ls3 ) );
  QVERIFY( ls1 != ls3 );

  //different z coordinates
  QgsLineString ls4;
  ls4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );

  QVERIFY( !( ls3 == ls4 ) );
  QVERIFY( ls3 != ls4 );

  //different m values
  QgsLineString ls5;
  ls5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsLineString ls6;
  ls6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );

  QVERIFY( !( ls5 == ls6 ) );
  QVERIFY( ls5 != ls6 );

  //different type
  QVERIFY( ls6 != QgsCircularString() );

  QgsPoint p1;
  QVERIFY( !( ls6 == p1 ) );
  QVERIFY( ls6 != p1 );
  QVERIFY( ls6 == ls6 );
}

void TestQgsLineString::close()
{
  QgsLineString ls;
  QVERIFY( !ls.isClosed() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 2 )
                << QgsPoint( 11, 22 )
                << QgsPoint( 1, 22 ) );
  QVERIFY( !ls.isClosed() );
  QCOMPARE( ls.numPoints(), 4 );
  QCOMPARE( ls.area(), 0.0 );
  QCOMPARE( ls.perimeter(), 0.0 );

  ls.close();
  QVERIFY( ls.isClosed() );
  QCOMPARE( ls.numPoints(), 5 );
  QCOMPARE( ls.vertexCount(), 5 );
  QCOMPARE( ls.nCoordinates(), 5 );
  QCOMPARE( ls.ringCount(), 1 );
  QCOMPARE( ls.partCount(), 1 );
  QCOMPARE( ls.pointN( 4 ), QgsPoint( 1, 2 ) );
  QCOMPARE( ls.area(), 0.0 );
  QCOMPARE( ls.perimeter(), 0.0 );

  //try closing already closed line, should be no change
  ls.close();
  QVERIFY( ls.isClosed() );
  QCOMPARE( ls.numPoints(), 5 );
  QCOMPARE( ls.pointN( 4 ), QgsPoint( 1, 2 ) );

  // tiny differences
  ls.setPoints( QgsPointSequence() << QgsPoint( 0.000000000000001, 0.000000000000002 )
                << QgsPoint( 0.000000000000011, 0.000000000000002 )
                << QgsPoint( 0.000000000000011, 0.000000000000022 )
                << QgsPoint( 0.000000000000001, 0.000000000000022 ) );
  QVERIFY( !ls.isClosed() );

  ls.close();
  QVERIFY( ls.isClosed() );
  QCOMPARE( ls.numPoints(), 5 );
  QGSCOMPARENEAR( ls.pointN( 4 ).x(), 0.000000000000001, 0.00000000000000001 );
  QGSCOMPARENEAR( ls.pointN( 4 ).y(), 0.000000000000002, 0.00000000000000001 );

  //test that m values aren't considered when testing for closedness
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  QVERIFY( ls.isClosed() );

  //close with z and m
  ls = QgsLineString();
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  ls.close();
  QCOMPARE( ls.pointN( 4 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
}

void TestQgsLineString::asQPolygonF()
{
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QPolygonF poly = ls.asQPolygonF();
  QCOMPARE( poly.count(), 4 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 1 ).x(), 11.0 );
  QCOMPARE( poly.at( 1 ).y(), 2.0 );
  QCOMPARE( poly.at( 2 ).x(), 11.0 );
  QCOMPARE( poly.at( 2 ).y(), 22.0 );
  QCOMPARE( poly.at( 3 ).x(), 1.0 );
  QCOMPARE( poly.at( 3 ).y(), 22.0 );
}

void TestQgsLineString::clone()
{
  // At the same time, check segmentize as the result should
  // be equal to a clone for LineStrings
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 2 )
                << QgsPoint( 11, 22 )
                << QgsPoint( 1, 22 ) );
  std::unique_ptr<QgsLineString> cloned( ls.clone() );

  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), ls.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), ls.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), ls.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), ls.pointN( 3 ) );

  std::unique_ptr< QgsLineString > segmentized( static_cast< QgsLineString * >( ls.segmentize() ) );

  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), ls.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), ls.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), ls.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), ls.pointN( 3 ) );

  //clone with Z/M
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( ls.clone() );

  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( cloned->is3D() );
  QVERIFY( cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), ls.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), ls.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), ls.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), ls.pointN( 3 ) );

  segmentized.reset( static_cast< QgsLineString * >( ls.segmentize() ) );

  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), ls.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), ls.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), ls.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), ls.pointN( 3 ) );

  //clone an empty line
  ls.clear();
  cloned.reset( ls.clone() );

  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineString );

  segmentized.reset( static_cast< QgsLineString * >( ls.segmentize() ) );

  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsLineString::toWkbFromWkb()
{
  QgsLineString ls1;
  ls1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QByteArray wkb1 = ls1.asWkb();
  QCOMPARE( wkb1.size(), ls1.wkbSize() );

  QgsLineString ls2;
  QgsConstWkbPtr wkb1ptr( wkb1 );
  ls2.fromWkb( wkb1ptr );

  QCOMPARE( ls2.numPoints(), 4 );
  QCOMPARE( ls2.vertexCount(), 4 );
  QCOMPARE( ls2.nCoordinates(), 4 );
  QCOMPARE( ls2.ringCount(), 1 );
  QCOMPARE( ls2.partCount(), 1 );
  QCOMPARE( ls2.wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( ls2.is3D() );
  QVERIFY( ls2.isMeasure() );
  QCOMPARE( ls2.pointN( 0 ), ls1.pointN( 0 ) );
  QCOMPARE( ls2.pointN( 1 ), ls1.pointN( 1 ) );
  QCOMPARE( ls2.pointN( 2 ), ls1.pointN( 2 ) );
  QCOMPARE( ls2.pointN( 3 ), ls1.pointN( 3 ) );

  //bad WKB - check for no crash
  ls2.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !ls2.fromWkb( nullPtr ) );
  QCOMPARE( ls2.wkbType(), QgsWkbTypes::LineString );

  QgsPoint point( 1, 2 );
  QByteArray wkb2 = point.asWkb();
  QgsConstWkbPtr wkb2ptr( wkb2 );

  QVERIFY( !ls2.fromWkb( wkb2ptr ) );
  QCOMPARE( ls2.wkbType(), QgsWkbTypes::LineString );
}

void TestQgsLineString::toWktFromWkt()
{
  QgsLineString ls1;
  ls1.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QString wkt = ls1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsLineString ls2;

  QVERIFY( ls2.fromWkt( wkt ) );
  QCOMPARE( ls2.numPoints(), 4 );
  QCOMPARE( ls2.wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( ls2.is3D() );
  QVERIFY( ls2.isMeasure() );
  QCOMPARE( ls2.pointN( 0 ), ls1.pointN( 0 ) );
  QCOMPARE( ls2.pointN( 1 ), ls1.pointN( 1 ) );
  QCOMPARE( ls2.pointN( 2 ), ls1.pointN( 2 ) );
  QCOMPARE( ls2.pointN( 3 ), ls1.pointN( 3 ) );

  //bad WKT
  QVERIFY( !ls2.fromWkt( "Polygon()" ) );
  QVERIFY( ls2.isEmpty() );
  QCOMPARE( ls2.numPoints(), 0 );
  QVERIFY( !ls2.is3D() );
  QVERIFY( !ls2.isMeasure() );
  QCOMPARE( ls2.wkbType(), QgsWkbTypes::LineString );
}

void TestQgsLineString::exportAs()
{
  //asGML2
  QgsLineString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );

  QgsLineString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );

  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLine.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLineFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsLineString().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLine.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsLineString().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( "{\"coordinates\":[[31.0,32.0],[41.0,42.0],[51.0,52.0]],\"type\":\"LineString\"}" );
  QCOMPARE( exportLine.asJson(), expectedJson );
  QString expectedJsonPrec3( "{\"coordinates\":[[0.333,0.667],[1.333,1.667],[2.333,2.667]],\"type\":\"LineString\"}" );
  QCOMPARE( exportLineFloat.asJson( 3 ), expectedJsonPrec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>31,32,0 41,42,0 51,52,0</coordinates></LineString>" ) );
  QCOMPARE( exportLine.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<LineString><altitudeMode>clampToGround</altitudeMode><coordinates>0.333,0.667,0 1.333,1.667,0 2.333,2.667,0</coordinates></LineString>" ) );
  QCOMPARE( exportLineFloat.asKml( 3 ), expectedKmlPrec3 );
}

void TestQgsLineString::length()
{
  QgsLineString ls;
  QCOMPARE( ls.length(), 0.0 );

  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( ls.length(), 23.0 );
}

void TestQgsLineString::startEndPoint()
{
  QgsLineString ls( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                    << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                    << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QCOMPARE( ls.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( ls.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  ls.clear();
  QVERIFY( ls.startPoint().isEmpty() );
  QVERIFY( ls.endPoint().isEmpty() );
}

void TestQgsLineString::length3D()
{
  // without vertices
  QgsLineString ls;
  QCOMPARE( ls.length3D(), 0.0 );

  // without Z
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                << QgsPoint( QgsWkbTypes::Point, 3, 4 )
                << QgsPoint( QgsWkbTypes::Point, 8, 16 ) );
  QCOMPARE( ls.length3D(), 18.0 );

  // with z
  ls.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                << QgsPoint( QgsWkbTypes::PointZ, 4, 6, 2 ) );
  QCOMPARE( ls.length3D(), 8.0 );

  ls.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 0 ) );
  QCOMPARE( ls.length3D(), 0.0 );

  // with z and m
  ls.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 0, 0 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 13 )
                << QgsPoint( QgsWkbTypes::PointZM, 4, 6, 2, 7 ) );
  QCOMPARE( ls.length3D(), 8.0 );
}

void TestQgsLineString::curveToLine()
{
  //no segmentation required, so should return a clone
  QgsLineString ls( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                    << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                    << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  std::unique_ptr< QgsLineString > segmentized( static_cast< QgsLineString * >( ls.curveToLine() ) );

  QCOMPARE( segmentized->numPoints(), 3 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), ls.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), ls.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), ls.pointN( 2 ) );
}

void TestQgsLineString::points()
{
  QgsLineString ls;
  QgsPointSequence points;

  ls.points( points );
  QVERIFY( ls.isEmpty() );

  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  ls.points( points );

  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
}

void TestQgsLineString::CRSTransform()
{
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) );// want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                << QgsPoint( 6474985, -3526584 ) );
  ls.transform( tr, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( ls.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ls.boundingBox().xMinimum(), 175.771, 0.001 );
  QGSCOMPARENEAR( ls.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( ls.boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( ls.boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform without considering Z
  ls = QgsLineString( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                      << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  ls.transform( tr, Qgis::TransformDirection::Forward );

  QGSCOMPARENEAR( ls.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 0 ).z(), 1.0, 0.001 );
  QCOMPARE( ls.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( ls.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).z(), 3.0, 0.001 );
  QCOMPARE( ls.pointN( 1 ).m(), 4.0 );

  //3d CRS transform with Z
  ls = QgsLineString( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                      << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  ls.transform( tr, Qgis::TransformDirection::Forward, true );

  QGSCOMPARENEAR( ls.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 0 ).z(), 1.0, 0.001 );
  QCOMPARE( ls.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( ls.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).z(), 3.0, 0.001 );
  QCOMPARE( ls.pointN( 1 ).m(), 4.0 );

  //reverse transform
  ls.transform( tr, Qgis::TransformDirection::Reverse );

  QGSCOMPARENEAR( ls.pointN( 0 ).x(), 6374985, 0.01 );
  QGSCOMPARENEAR( ls.pointN( 0 ).y(), -3626584, 0.01 );
  QGSCOMPARENEAR( ls.pointN( 0 ).z(), 1, 0.001 );
  QCOMPARE( ls.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( ls.pointN( 1 ).x(), 6474985, 0.01 );
  QGSCOMPARENEAR( ls.pointN( 1 ).y(), -3526584, 0.01 );
  QGSCOMPARENEAR( ls.pointN( 1 ).z(), 3, 0.001 );
  QCOMPARE( ls.pointN( 1 ).m(), 4.0 );

#if 0 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  ls.transform( tr, Qgis::TransformDirection::Forward, true );

  QGSCOMPARENEAR( ls.pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).z(), -21.092128, 0.001 );

  ls.transform( tr, Qgis::TransformDirection::Reverse, true );

  QGSCOMPARENEAR( ls.pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ls.pointN( 1 ).z(), 3.0, 0.001 );
#endif
}

void TestQgsLineString::QTransformation()
{
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  ls.transform( qtr );

  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 22, 36, 13, 14 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 2, 6, 22, 36 ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  ls.transform( QTransform::fromScale( 1, 1 ), 3, 2, 4, 3 );

  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 9, 16 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 29, 46 ) );
}

void TestQgsLineString::insertVertex()
{
  //insert vertex in empty line
  QgsLineString ls;

  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( ls.numPoints(), 1 );
  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );

  //insert 4d vertex in empty line, should set line to 4d
  ls.clear();

  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) ) );
  QCOMPARE( ls.numPoints(), 1 );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) );

  //2d line
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );

  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( ls.numPoints(), 4 );
  QVERIFY( !ls.is3D() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 19.0 ) ) );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( 8.0, 9.0 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( 18.0, 19.0 ) );
  QCOMPARE( ls.pointN( 3 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( ls.pointN( 4 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ls.pointN( 5 ), QgsPoint( 21.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 6 ), QgsPoint( 31.0, 32.0 ) ) );
  QCOMPARE( ls.pointN( 6 ), QgsPoint( 31.0, 32.0 ) );
  QCOMPARE( ls.numPoints(), 7 );

  //insert vertex past end
  QVERIFY( !ls.insertVertex( QgsVertexId( 0, 0, 8 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( ls.numPoints(), 7 );

  //insert vertex before start
  QVERIFY( !ls.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( ls.numPoints(), 7 );

  //insert 4d vertex in 4d line
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( ls.numPoints(), 4 );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( ls.numPoints(), 5 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );

  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102, 103, 104 ) ) );
  QCOMPARE( ls.numPoints(), 4 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 101, 102 ) );

  //insert first vertex as Point25D
  ls.clear();

  QVERIFY( ls.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::Point25D, 101, 102, 103 ) ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 101, 102, 103 ) );
}

void TestQgsLineString::moveVertex()
{
  //empty line
  QgsLineString ls;
  QVERIFY( !ls.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( ls.isEmpty() );

  //valid line
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );

  QVERIFY( ls.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( ls.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( ls.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !ls.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !ls.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  QVERIFY( ls.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( ls.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );

  QVERIFY( ls.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 3, 4 ) );
}

void TestQgsLineString::deleteVertex()
{
  //empty line
  QgsLineString ls;

  QVERIFY( !ls.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( ls.isEmpty() );

  //valid line
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );

  //out of range vertices
  QVERIFY( !ls.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !ls.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( ls.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( ls.numPoints(), 2 );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );

  //removing the second to last vertex removes both remaining vertices
  QVERIFY( ls.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( ls.numPoints(), 0 );
  QVERIFY( !ls.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( ls.isEmpty() );
}

void TestQgsLineString::reversed()
{
  QgsLineString ls;
  std::unique_ptr< QgsLineString > reversed( ls.reversed() );

  QVERIFY( reversed->isEmpty() );

  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( ls.reversed() );

  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
}

void TestQgsLineString::addZValue()
{
  QgsLineString ls;

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( ls.addZValue() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );

  ls.clear();

  QVERIFY( ls.addZValue() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );

  //2d line
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( ls.addZValue( 2 ) );
  QVERIFY( ls.is3D() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  QVERIFY( !ls.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );

  //linestring with m
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );

  QVERIFY( ls.addZValue( 5 ) );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );

  //linestring25d
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( !ls.addZValue( 5 ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
}

void TestQgsLineString::addMValue()
{
  QgsLineString ls;

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( ls.addMValue() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );

  ls.clear();

  QVERIFY( ls.addMValue() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );

  //2d line
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( ls.addMValue( 2 ) );
  QVERIFY( !ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  QVERIFY( !ls.addMValue( 4 ) ); //already has m value, test that existing m is unchanged
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );

  //linestring with z
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );

  QVERIFY( ls.addMValue( 5 ) );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );

  //linestring25d, should become LineStringZM
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( ls.addMValue( 5 ) );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
}

void TestQgsLineString::dropZValue()
{
  QgsLineString ls;
  QVERIFY( !ls.dropZValue() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !ls.dropZValue() );

  ls.addZValue( 1.0 );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QVERIFY( ls.is3D() );
  QVERIFY( ls.dropZValue() );
  QVERIFY( !ls.is3D() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !ls.dropZValue() ); //already dropped

  //linestring with m
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );

  QVERIFY( ls.dropZValue() );
  QVERIFY( !ls.is3D() );
  QVERIFY( ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );

  //linestring25d
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( ls.dropZValue() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
}

void TestQgsLineString::dropMValue()
{
  QgsLineString ls( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !ls.dropMValue() );

  ls.addMValue( 1.0 );

  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QVERIFY( ls.isMeasure() );
  QVERIFY( ls.dropMValue() );
  QVERIFY( !ls.isMeasure() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !ls.dropMValue() ); //already dropped

  //linestring with z
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );

  QVERIFY( ls.dropMValue() );
  QVERIFY( !ls.isMeasure() );
  QVERIFY( ls.is3D() );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3, 0 ) );
}

void TestQgsLineString::convertTo()
{
  QgsLineString ls( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  QVERIFY( ls.convertTo( QgsWkbTypes::LineString ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( ls.convertTo( QgsWkbTypes::LineStringZ ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );

  ls.setZAt( 0, 5.0 );

  QVERIFY( ls.convertTo( QgsWkbTypes::LineString25D ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 5.0 ) );
  QVERIFY( ls.convertTo( QgsWkbTypes::LineStringZM ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );

  ls.setMAt( 0, 6.0 );

  QVERIFY( ls.convertTo( QgsWkbTypes::LineStringM ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( ls.convertTo( QgsWkbTypes::LineString ) );
  QCOMPARE( ls.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( ls.pointN( 0 ), QgsPoint( 1, 2 ) );
  QVERIFY( !ls.convertTo( QgsWkbTypes::Polygon ) );
}

void TestQgsLineString::isRing()
{
  QgsLineString ls;
  QVERIFY( !ls.isRing() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  QVERIFY( !ls.isRing() ); //<4 points

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  QVERIFY( !ls.isRing() ); //not closed

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  QVERIFY( ls.isRing() );
}

void TestQgsLineString::coordinateSequence()
{
  QgsLineString ls;
  QgsCoordinateSequence coords = ls.coordinateSequence();

  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );

  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  coords = ls.coordinateSequence();

  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 3 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
}

void TestQgsLineString::nextVertex()
{
  QgsLineString ls;
  QgsVertexId v;
  QgsPoint p;

  QVERIFY( !ls.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !ls.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !ls.nextVertex( v, p ) );

  //LineString
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  v = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !ls.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( ls.nextVertex( v, p ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QVERIFY( !ls.nextVertex( v, p ) );

  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );

  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );

  //LineStringZ
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !ls.nextVertex( v, p ) );

  //LineStringM
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !ls.nextVertex( v, p ) );

  //LineStringZM
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !ls.nextVertex( v, p ) );

  //LineString25D
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QVERIFY( ls.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( !ls.nextVertex( v, p ) );
}

void TestQgsLineString::vertexIterator()
{
  QgsLineString ls;

  // vertex iterator on empty linestring
  QgsAbstractGeometry::vertex_iterator it = ls.vertices_begin();
  QCOMPARE( it, ls.vertices_end() );

  // Java-style iterator on empty linetring
  QgsVertexIterator itx( &ls );
  QVERIFY( !itx.hasNext() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );

  // vertex iterator
  it = ls.vertices_begin();
  QCOMPARE( *it, QgsPoint( 1, 2 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 12 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it;
  QCOMPARE( it, ls.vertices_end() );

  // Java-style iterator
  itx = QgsVertexIterator( &ls );
  QVERIFY( itx.hasNext() );
  QCOMPARE( itx.next(), QgsPoint( 1, 2 ) );
  QVERIFY( itx.hasNext() );
  QCOMPARE( itx.next(), QgsPoint( 11, 12 ) );
  QVERIFY( !itx.hasNext() );
}

void TestQgsLineString::vertexAtPointAt()
{
  QgsLineString ls;
  QgsVertexId v;
  QgsPoint p;
  Qgis::VertexType type;

  ls.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  ls.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QVERIFY( !ls.pointAt( -10, p, type ) );
  QVERIFY( !ls.pointAt( 10, p, type ) );

  //LineString
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  ls.vertexAt( QgsVertexId( 0, 0, -10 ) );
  ls.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash

  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QVERIFY( !ls.pointAt( -10, p, type ) );
  QVERIFY( !ls.pointAt( 10, p, type ) );
  QVERIFY( ls.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( ls.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsLineString::vertexAtPointAtZ()
{
  QgsLineString ls( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                    << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QgsVertexId v;
  QgsPoint p;
  Qgis::VertexType type;

  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( ls.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( ls.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsLineString::vertexAtPointAtM()
{
  QgsLineString ls( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                    << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QgsVertexId v;
  QgsPoint p;
  Qgis::VertexType type;

  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( ls.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( ls.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsLineString::vertexAtPointAtZM()
{
  //LineStringZM
  QgsLineString ls( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                    << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QgsVertexId v;
  QgsPoint p;
  Qgis::VertexType type;

  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( ls.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( ls.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsLineString::vertexAtPointAt25D()
{
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 )
                << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );

  QgsVertexId v;
  QgsPoint p;
  Qgis::VertexType type;

  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( ls.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( ls.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
  QVERIFY( ls.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( type, Qgis::VertexType::Segment );
}

void TestQgsLineString::centroid()
{
  QgsLineString ls;
  QVERIFY( ls.centroid().isEmpty() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QCOMPARE( ls.centroid(), QgsPoint( 5, 10 ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) );
  QCOMPARE( ls.centroid(), QgsPoint( 10, 5 ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 )
                << QgsPoint( 2, 9 ) << QgsPoint( 2, 0 ) );
  QCOMPARE( ls.centroid(), QgsPoint( 1, 4.95 ) );

  //linestring with 0 length segment
  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 )
                << QgsPoint( 2, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 0 ) );
  QCOMPARE( ls.centroid(), QgsPoint( 1, 4.95 ) );

  //linestring with 0 total length segment
  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 4 )
                << QgsPoint( 5, 4 ) << QgsPoint( 5, 4 ) );
  QCOMPARE( ls.centroid(), QgsPoint( 5, 4 ) );
}

void TestQgsLineString::closestSegment()
{
  QgsLineString ls;
  int leftOf = 0;
  QgsPoint p( 0, 0 ); // reset all coords to zero
  QgsVertexId v;

  ( void )ls.closestSegment( QgsPoint( 1, 2 ), p, v ); //empty line, just want no crash
  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );

  QVERIFY( ls.closestSegment( QgsPoint( 5, 10 ), p, v ) < 0 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 8, 9 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 11, 9 ), p, v, &leftOf ), 2.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 10, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                << QgsPoint( 10, 10 )
                << QgsPoint( 10, 15 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 11, 12 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 10, 12 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                << QgsPoint( 6, 4 )
                << QgsPoint( 4, 4 )
                << QgsPoint( 5, 5 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 2.35, 4 ), p, v, &leftOf ), 2.7225, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                << QgsPoint( 4, 4 )
                << QgsPoint( 6, 4 )
                << QgsPoint( 5, 5 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 2.35, 4 ), p, v, &leftOf ), 2.7225, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                << QgsPoint( 6, 4 )
                << QgsPoint( 4, 4 )
                << QgsPoint( 5, 5 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 3.5, 2 ), p, v, &leftOf ), 4.250000, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                << QgsPoint( 4, 4 )
                << QgsPoint( 6, 4 )
                << QgsPoint( 5, 5 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 3.5, 2 ), p, v, &leftOf ), 4.250000, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                << QgsPoint( 1, 4 )
                << QgsPoint( 2, 2 )
                << QgsPoint( 1, 1 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 1, 0 ), p, v, &leftOf ), 1, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 1, 1 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                << QgsPoint( 2, 2 )
                << QgsPoint( 1, 4 )
                << QgsPoint( 1, 1 ) );

  QGSCOMPARENEAR( ls.closestSegment( QgsPoint( 1, 0 ), p, v, &leftOf ), 1, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 1, 1 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
}

void TestQgsLineString::sumUpArea()
{
  QgsLineString ls;

  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  ls.sumUpArea( area );
  QCOMPARE( area, 1.0 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  ls.sumUpArea( area );
  QCOMPARE( area, 1.0 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  ls.sumUpArea( area );
  QGSCOMPARENEAR( area, -24, 4 * std::numeric_limits<double>::epsilon() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  ls.sumUpArea( area );
  QGSCOMPARENEAR( area, -22, 4 * std::numeric_limits<double>::epsilon() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 )
                << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  ls.sumUpArea( area );
  QGSCOMPARENEAR( area, -18, 4 * std::numeric_limits<double>::epsilon() );
}

void TestQgsLineString::boundingBox()
{
  //test that bounding box is updated after every modification to the line string
  QgsLineString ls;
  QVERIFY( ls.boundingBox().isNull() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( -5, -10 )
                << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -6, -10, -5, -9 ) );

  //setXAt
  ls.setXAt( 2, -4 );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -6, -10, -4, -9 ) );

  //setYAt
  ls.setYAt( 1, -15 );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -6, -15, -4, -9 ) );

  //append
  std::unique_ptr<QgsLineString> toAppend( new QgsLineString() );

  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 4, 0 ) );
  ls.append( toAppend.get() );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -6, -15, 4, 2 ) );

  ls.addVertex( QgsPoint( 6, 3 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -6, -15, 6, 3 ) );

  ls.clear();
  QVERIFY( ls.boundingBox().isNull() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QByteArray wkbToAppend = toAppend->asWkb();
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  ls.fromWkb( wkbToAppendPtr );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 1, 0, 4, 2 ) );

  ls.fromWkt( QStringLiteral( "LineString( 1 5, 3 4, 6 3 )" ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );

  ls.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -1, 3, 6, 7 ) );

  ls.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( -3, 3, 6, 10 ) );

  ls.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );
}

void TestQgsLineString::boundingBox3D()
{
  // boundingBox - test 3D boundingbox
  QgsLineString bb3d;
  bb3d.setPoints( QgsPointSequence() << QgsPoint( -1.0, -1.0, -1.0 )
                  << QgsPoint( -2.0, -1.0, -1.0 )
                  << QgsPoint( 1.0, 2.0, -1.0 )
                  << QgsPoint( 1.0, 2.0, 2.0 ) );
  QCOMPARE( bb3d.calculateBoundingBox3d(), QgsBox3d( QgsPoint( -2.0, -1.0, -1.0 ), QgsPoint( 1.0, 2.0, 2.0 ) ) );
  // retrieve again, should use cached values
  QCOMPARE( bb3d.calculateBoundingBox3d(), QgsBox3d( QgsPoint( -2.0, -1.0, -1.0 ), QgsPoint( 1.0, 2.0, 2.0 ) ) );

  // linestring with z
  bb3d.setPoints( QgsPointSequence() << QgsPoint( -1.0, -1.0 )
                  << QgsPoint( -2.0, -1.0 )
                  << QgsPoint( 1.0, 2.0 )
                  << QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( bb3d.calculateBoundingBox3d(), QgsBox3d( QgsPoint( -2.0, -1, std::numeric_limits< double >::quiet_NaN() ), QgsPoint( 1.0, 2.0, std::numeric_limits< double >::quiet_NaN() ) ) );
}

void TestQgsLineString::angle()
{
  QgsLineString ls;

  ( void )ls.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  ( void )ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  ( void )ls.vertexAngle( QgsVertexId( 0, 0, 2 ) ); //no crash

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.0, 4 * std::numeric_limits<double>::epsilon() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.71239, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 4.71239, 0.0001 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 3.1416, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.1416, 0.0001 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.7854, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0.0, 0.0001 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) );
  ( void )ls.vertexAngle( QgsVertexId( 0, 0, 20 ) );

  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 4.71239, 0.00001 );

  //closed line string
  ls.close();
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( ls.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
}

void TestQgsLineString::removingVertexRemoveLine()
{
  //removing the second to last vertex should remove the whole line
  QgsLineString ls;

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) );
  QVERIFY( ls.numPoints() == 2 );

  ls.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QVERIFY( ls.numPoints() == 0 );
}

void TestQgsLineString::boundary()
{
  QgsLineString ls;
  QVERIFY( !ls.boundary() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = ls.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );

  delete boundary;

  // closed string = no boundary
  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !ls.boundary() );
  \

  //boundary with z
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = ls.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );

  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );

  delete boundary;
}

void TestQgsLineString::extend()
{
  QgsLineString ls;
  ls.extend( 10, 10 ); //test no crash

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  ls.extend( 1, 2 );

  QCOMPARE( ls.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, -1, 0 ) );
  QCOMPARE( ls.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 1, 0 ) );
  QCOMPARE( ls.pointN( 2 ), QgsPoint( QgsWkbTypes::Point, 1, 3 ) );
}

void TestQgsLineString::addToPainterPath()
{
  // note most tests are in test_qgsgeometry.py
  QgsLineString path;
  QPainterPath pPath;

  path.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );

  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  path.addToPainterPath( pPath );

  QVERIFY( !pPath.isEmpty() );
}

void TestQgsLineString::toCurveType()
{
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  std::unique_ptr< QgsCompoundCurve > curveType( ls.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 2 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
}

void TestQgsLineString::adjacentVertices()
{
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );

  ls.adjacentVertices( QgsVertexId( 0, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  ls.adjacentVertices( QgsVertexId( 0, 0, 3 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  ls.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );

  ls.adjacentVertices( QgsVertexId( 0, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );

  ls.adjacentVertices( QgsVertexId( 0, 0, 2 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId() );

  // ring, part should be maintained
  ls.adjacentVertices( QgsVertexId( 1, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 2 ) );

  ls.adjacentVertices( QgsVertexId( 1, 2, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 2, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 2, 2 ) );

  // closed ring
  ls.close();
  ls.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );

  ls.adjacentVertices( QgsVertexId( 0, 0, 3 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId() );
}

void TestQgsLineString::vertexNumberFromVertexId()
{
  QgsLineString ls;

  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );

  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
}

void TestQgsLineString::segmentLength()
{
  QgsLineString ls;

  QCOMPARE( ls.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );

  QCOMPARE( ls.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( ls.segmentLength( QgsVertexId( 1, 1, 1 ) ), 100.0 );
}

void TestQgsLineString::collectDuplicateNodes()
{
  QgsLineString ls;

  // with only one point
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( ls.collectDuplicateNodes( 0.0, false ).isEmpty() );


  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 )
                << QgsPoint( 1, 3, 3 ) << QgsPoint( 1, 3, 5 )
                << QgsPoint( 2, 4, 3 ) << QgsPoint( 2, 4, 3 )
                << QgsPoint( 4, 5, 6 ) );

  // without considering Z
  QVector< QgsVertexId > duplicateNodes = ls.collectDuplicateNodes( 0.1, false );

  QCOMPARE( duplicateNodes.size(), 2 );
  QCOMPARE( duplicateNodes[0], QgsVertexId( -1, -1, 2 ) );
  QCOMPARE( duplicateNodes[1], QgsVertexId( -1, -1, 4 ) );

  // considering Z
  duplicateNodes = ls.collectDuplicateNodes( 0.1, true );

  QCOMPARE( duplicateNodes.size(), 1 );
  QCOMPARE( duplicateNodes[0], QgsVertexId( -1, -1, 4 ) );
}

void TestQgsLineString::removeDuplicateNodes()
{
  QgsLineString ls;
  QVERIFY( !ls.removeDuplicateNodes() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );

  QVERIFY( !ls.removeDuplicateNodes() );
  QCOMPARE( ls.asWkt(), QStringLiteral( "LineString (11 2, 11 12, 111 12)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 )
                << QgsPoint( 11.02, 2.01 ) << QgsPoint( 11, 12 )
                << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );

  QVERIFY( ls.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !ls.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( ls.asWkt(), QStringLiteral( "LineString (11 2, 11 12, 111 12)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 )
                << QgsPoint( 11.02, 2.01 ) << QgsPoint( 11, 12 )
                << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );

  QVERIFY( !ls.removeDuplicateNodes() );
  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99)" ) );

  // don't create degenerate lines
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) );

  QVERIFY( !ls.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineString (11 2)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) );

  QVERIFY( !ls.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineString (11 2, 11.01 1.99)" ) );

  // with z
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 )
                << QgsPoint( 11.02, 2.01, 3 ) << QgsPoint( 11, 12, 4 )
                << QgsPoint( 111, 12, 5 ) << QgsPoint( 111.01, 11.99, 6 ) );

  QVERIFY( ls.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( ls.asWkt(), QStringLiteral( "LineStringZ (11 2 1, 11 12 4, 111 12 5)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 )
                << QgsPoint( 11.02, 2.01, 3 ) << QgsPoint( 11, 12, 4 )
                << QgsPoint( 111, 12, 5 ) << QgsPoint( 111.01, 11.99, 6 ) );

  QVERIFY( !ls.removeDuplicateNodes( 0.02, true ) );
  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineStringZ (11 2 1, 11.01 1.99 2, 11.02 2.01 3, 11 12 4, 111 12 5, 111.01 11.99 6)" ) );
}

void TestQgsLineString::swapXy()
{
  QgsLineString ls;
  ls.swapXy(); // no crash
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  ls.swapXy();

  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24)" ) );
}

void TestQgsLineString::filterVertices()
{
  QgsLineString ls;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };

  ls.filterVertices( filter ); // no crash

  ls.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  ls.filterVertices( filter );

  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineStringZM (1 2 3 4, 4 12 13 14)" ) );

}

void TestQgsLineString::transformVertices()
{
  QgsLineString ls;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 5, point.y() + 6, point.z() + 7, point.m() + 8 );
  };

  ls.transformVertices( transform ); // no crash

  ls.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  ls.transformVertices( transform );

  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineStringZM (16 8 10 12, 6 8 10 12, 9 18 20 22, 116 18 30 32)" ) );

  // transform using class
  ls = QgsLineString();
  TestTransformer transformer;

  // no crash
  QVERIFY( !ls.transform( nullptr ) );
  QVERIFY( ls.transform( &transformer ) );

  ls.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );

  QVERIFY( ls.transform( &transformer ) );
  QCOMPARE( ls.asWkt( 2 ), QStringLiteral( "LineStringZM (33 16 8 3, 3 16 8 3, 12 26 18 13, 333 26 28 23)" ) );

  QgsFeedback feedback;
  feedback.cancel();
  QVERIFY( !ls.transform( &transformer, &feedback ) );

  TestFailTransformer failTransformer;
  QVERIFY( !ls.transform( &failTransformer ) );
}

void TestQgsLineString::curveSubstring()
{
  QgsLineString ls;

  std::unique_ptr< QgsLineString > substringResult( ls.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  substringResult.reset( ls.curveSubstring( 0, 0 ) );

  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 2 3 4)" ) );

  substringResult.reset( ls.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );

  substringResult.reset( ls.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );

  substringResult.reset( ls.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 3 4 5)" ) );

  substringResult.reset( ls.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 3 4 5)" ) );

  substringResult.reset( ls.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24)" ) );

  substringResult.reset( ls.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 12 13 14, 111 12 23 24)" ) );

  substringResult.reset( ls.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 12 13 14, 21 12 14 15)" ) );

  substringResult.reset( ls.curveSubstring(
                           QgsGeometryUtils::distanceToVertex( ls, QgsVertexId( 0, 0, 1 ) ),
                           QgsGeometryUtils::distanceToVertex( ls, QgsVertexId( 0, 0, 2 ) ) ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 12 13 14, 111 12 23 24)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 0, QgsWkbTypes::PointZ )
                << QgsPoint( 11, 12, 13, 0, QgsWkbTypes::PointZ )
                << QgsPoint( 111, 12, 23, 0, QgsWkbTypes::PointZ ) );
  substringResult.reset( ls.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZ (11 3 4, 11 12 13, 21 12 14)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 0, 3, QgsWkbTypes::PointM )
                << QgsPoint( 11, 12, 0, 13, QgsWkbTypes::PointM )
                << QgsPoint( 111, 12, 0, 23, QgsWkbTypes::PointM ) );
  substringResult.reset( ls.curveSubstring( 1, 20 ) );

  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringM (11 3 4, 11 12 13, 21 12 14)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  substringResult.reset( ls.curveSubstring( 1, 20 ) );

  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineString (11 3, 11 12, 21 12)" ) );
}

void TestQgsLineString::interpolatePoint()
{
  QgsLineString ls;

  std::unique_ptr< QgsPoint > interpolateResult( ls.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );

  interpolateResult.reset( ls.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11 2 3 4)" ) );

  interpolateResult.reset( ls.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );

  interpolateResult.reset( ls.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );

  interpolateResult.reset( ls.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11 3 4 5)" ) );

  interpolateResult.reset( ls.interpolatePoint( 20 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (21 12 14 15)" ) );

  interpolateResult.reset( ls.interpolatePoint( 110 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (111 12 23 24)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 0, QgsWkbTypes::PointZ )
                << QgsPoint( 11, 12, 13, 0, QgsWkbTypes::PointZ )
                << QgsPoint( 111, 12, 23, 0, QgsWkbTypes::PointZ ) );

  interpolateResult.reset( ls.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (11 3 4)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 0, 3, QgsWkbTypes::PointM )
                << QgsPoint( 11, 12, 0, 13, QgsWkbTypes::PointM )
                << QgsPoint( 111, 12, 0, 23, QgsWkbTypes::PointM ) );

  interpolateResult.reset( ls.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (11 3 4)" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );

  interpolateResult.reset( ls.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (11 3)" ) );
}

void TestQgsLineString::visitPoints()
{
  QgsLineString ls;
  ls.visitPointsByRegularDistance( 1, [ = ]( double, double, double, double, double, double, double, double, double, double, double, double )->bool
  {
    return true;
  } ); // no crash

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  int visitCount = 0;
  QVector< double > xx;
  QVector< double > yy;
  QVector< double > zz;
  QVector< double > mm;
  QVector< double > pX, pY, pZ, pM, nX, nY, nZ, nM;

  auto visitor = [ & ]( double x, double y, double z, double m, double ppx, double ppy, double ppz, double ppm, double nnx, double nny, double nnz, double nnm )->bool
  {
    xx << x;
    yy << y;
    zz << z;
    mm << m;
    pX << ppx;
    pY << ppy;
    pZ << ppz;
    pM << ppm;
    nX << nnx;
    nY << nny;
    nZ << nnz;
    nM << nnm;
    visitCount++;
    return true;
  };
  ls.visitPointsByRegularDistance( 0, visitor );

  QCOMPARE( visitCount, 1 );
  QCOMPARE( xx.at( 0 ), 11.0 );
  QCOMPARE( yy.at( 0 ), 2.0 );
  QCOMPARE( zz.at( 0 ), 3.0 );
  QCOMPARE( mm.at( 0 ), 4.0 );

  xx.clear();
  yy.clear();
  zz.clear();
  mm.clear();
  pX.clear();
  pY.clear();
  pZ.clear();
  pM.clear();
  nX.clear();
  nY.clear();
  nZ.clear();
  nM.clear();
  visitCount = 0;

  ls.visitPointsByRegularDistance( -1, visitor );
  QCOMPARE( visitCount, 0 );

  ls.visitPointsByRegularDistance( 10000, visitor );
  QCOMPARE( visitCount, 0 );

  ls.visitPointsByRegularDistance( 30, visitor );
  QCOMPARE( visitCount, 3 );

  QCOMPARE( xx.at( 0 ), 31.0 );
  QCOMPARE( yy.at( 0 ), 12.0 );
  QCOMPARE( zz.at( 0 ), 15.0 );
  QCOMPARE( mm.at( 0 ), 16.0 );
  QCOMPARE( pX.at( 0 ), 11.0 );
  QCOMPARE( pY.at( 0 ), 12.0 );
  QCOMPARE( pZ.at( 0 ), 13.0 );
  QCOMPARE( pM.at( 0 ), 14.0 );
  QCOMPARE( nX.at( 0 ), 111.0 );
  QCOMPARE( nY.at( 0 ), 12.0 );
  QCOMPARE( nZ.at( 0 ), 23.0 );
  QCOMPARE( nM.at( 0 ), 24.0 );
  QCOMPARE( xx.at( 1 ), 61.0 );
  QCOMPARE( yy.at( 1 ), 12.0 );
  QCOMPARE( zz.at( 1 ), 18.0 );
  QCOMPARE( mm.at( 1 ), 19.0 );
  QCOMPARE( pX.at( 1 ), 11.0 );
  QCOMPARE( pY.at( 1 ), 12.0 );
  QCOMPARE( pZ.at( 1 ), 13.0 );
  QCOMPARE( pM.at( 1 ), 14.0 );
  QCOMPARE( nX.at( 1 ), 111.0 );
  QCOMPARE( nY.at( 1 ), 12.0 );
  QCOMPARE( nZ.at( 1 ), 23.0 );
  QCOMPARE( nM.at( 1 ), 24.0 );
  QCOMPARE( xx.at( 2 ), 91.0 );
  QCOMPARE( yy.at( 2 ), 12.0 );
  QCOMPARE( zz.at( 2 ), 21.0 );
  QCOMPARE( mm.at( 2 ), 22.0 );
  QCOMPARE( pX.at( 2 ), 11.0 );
  QCOMPARE( pY.at( 2 ), 12.0 );
  QCOMPARE( pZ.at( 2 ), 13.0 );
  QCOMPARE( pM.at( 2 ), 14.0 );
  QCOMPARE( nX.at( 2 ), 111.0 );
  QCOMPARE( nY.at( 2 ), 12.0 );
  QCOMPARE( nZ.at( 2 ), 23.0 );
  QCOMPARE( nM.at( 2 ), 24.0 );
}

void TestQgsLineString::orientation()
{
  QgsLineString ls;

  ( void )ls.orientation(); // no crash

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 )
                << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );

  QCOMPARE( ls.orientation(), Qgis::AngularDirection::Clockwise );

  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )
                << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );

  QCOMPARE( ls.orientation(), Qgis::AngularDirection::CounterClockwise );
}

void TestQgsLineString::boundingBoxIntersects()
{
  QgsLineString ls;
  QVERIFY( !ls.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 13, -10 ) );

  QVERIFY( ls.boundingBoxIntersects( QgsRectangle( 12, 3, 16, 9 ) ) );

  // double test because of cache
  QVERIFY( ls.boundingBoxIntersects( QgsRectangle( 12, 3, 16, 9 ) ) );
  QVERIFY( !ls.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !ls.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 11, -10, 13, 12 ) );

  // clear cache
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 13, -10 ) );

  QVERIFY( !ls.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !ls.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QCOMPARE( ls.boundingBox(), QgsRectangle( 11, -10, 13, 12 ) );
  QVERIFY( ls.boundingBoxIntersects( QgsRectangle( 12, 3, 16, 9 ) ) );
}


void TestQgsLineString::setPointsFromData()
{
  //setPoints
  QgsLineString l8;
  double x [] = {1, 2, 3};
  double y [] = {2, 3, 4};
  l8.setPoints( 3, x, y );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QCOMPARE( l8.vertexCount(), 3 );
  QCOMPARE( l8.nCoordinates(), 3 );
  QCOMPARE( l8.ringCount(), 1 );
  QCOMPARE( l8.partCount(), 1 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !l8.hasCurvedSegments() );
  QgsPointSequence pts;
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QCOMPARE( *l8.xData(), 1.0 );
  QCOMPARE( *( l8.xData() + 1 ), 2.0 );
  QCOMPARE( *( l8.xData() + 2 ), 3.0 );
  QCOMPARE( *l8.yData(), 2.0 );
  QCOMPARE( *( l8.yData() + 1 ), 3.0 );
  QCOMPARE( *( l8.yData() + 2 ), 4.0 );

  //setPoints with empty list, should clear linestring
  l8.setPoints( 0, nullptr, nullptr );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );
  l8.points( pts );
  QVERIFY( pts.isEmpty() );

  //setPoints with z
  double x3 [] = {1, 2};
  double y3 [] = {2, 3};
  double z3 [] = {3, 4};
  l8.setPoints( 2, x3, y3, z3 );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZ );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //setPoints with m
  double x4 [] = {1, 2};
  double y4 [] = {2, 3};
  double m4 [] = {3, 4};
  l8.setPoints( 2, x4, y4, nullptr, m4 );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //setPoints with zm
  double x5 [] = {1, 2};
  double y5 [] = {2, 3};
  double z5 [] = {4, 4};
  double m5 [] = {5, 5};
  l8.setPoints( 2, x5, y5, z5, m5 );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

}

QGSTEST_MAIN( TestQgsLineString )
#include "testqgslinestring.moc"
