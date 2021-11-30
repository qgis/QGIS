/***************************************************************************
     testqgspolygon.cpp
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
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "testgeometryutils.h"
#include "testtransformer.h"

class TestQgsPolygon: public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void altConstructor();
    void polygon25D();
    void clear();
    void equality();
    void clone();
    void copy();
    void assignment();
    void setBadExteriorRing();
    void setExteriorRing();
    void setExteriorRingZM();
    void setCurvedExteriorRing();
    void setExteriorRingChangesInteriorRings();
    void deleteExteriorRing();
    void addInteriorRing();
    void addInteriorRingZM();
    void setInteriorRings();
    void removeInteriorRing();
    void removeInteriorRings();
    void removeInvalidRings();
    void insertVertex();
    void moveVertex();
    void deleteVertex();
    void deleteVertexRemoveRing();
    void nextVertex();
    void vertexNumberFromVertexId();
    void vertexAngle();
    void adjacentVertices();
    void removeDuplicateNodes();
    void dropZValue();
    void dropMValue();
    void swapXy();
    void boundary();
    void pointDistanceToBoundary();
    void closestSegment();
    void segmentLength();
    void forceRHR();
    void boundingBoxIntersects();
    void filterVertices();
    void transformVertices();
    void transformWithClass();
    void transform2D();
    void transform3D();
    void transformReverse();
    void transformOldVersion();
    void Qtransform();
    void cast();
    void toPolygon();
    void toCurveType();
    void toFromWkb();
    void toFromWkbZM();
    void toFromWkb25D();
    void toFromWKT();
    void exportImport();
};

void TestQgsPolygon::constructor()
{
  QgsPolygon pl;

  QVERIFY( pl.isEmpty() );
  QCOMPARE( pl.numInteriorRings(), 0 );
  QCOMPARE( pl.nCoordinates(), 0 );
  QCOMPARE( pl.ringCount(), 0 );
  QCOMPARE( pl.partCount(), 0 );
  QVERIFY( !pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( pl.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( pl.geometryType(), QString( "Polygon" ) );
  QCOMPARE( pl.dimension(), 2 );
  QVERIFY( !pl.hasCurvedSegments() );
  QCOMPARE( pl.area(), 0.0 );
  QCOMPARE( pl.perimeter(), 0.0 );
  QVERIFY( !pl.exteriorRing() );
  QVERIFY( !pl.interiorRing( 0 ) );
}

void TestQgsPolygon::altConstructor()
{
  QgsLineString ext( QVector<QgsPoint>() << QgsPoint( 0, 0 )
                     << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                     << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QgsLineString ring1( QVector< QgsPoint >() << QgsPoint( 1, 1 )
                       << QgsPoint( 2, 1 ) << QgsPoint( 2, 2 )
                       << QgsPoint( 1, 2 ) << QgsPoint( 1, 1 ) );
  QgsLineString ring2( QVector< QgsPoint >() << QgsPoint( 3, 1 )
                       << QgsPoint( 4, 1 ) << QgsPoint( 4, 2 )
                       << QgsPoint( 3, 2 ) << QgsPoint( 3, 1 ) );

  QgsPolygon pl( ext.clone(), QList< QgsLineString *>() << ring1.clone() << ring2.clone() );

  QCOMPARE( pl.asWkt(), QStringLiteral( "Polygon ((0 0, 0 10, 10 10, 10 0, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1),(3 1, 4 1, 4 2, 3 2, 3 1))" ) );
}

void TestQgsPolygon::polygon25D()
{
  //test handling of 25D rings/polygons
  QgsPolygon pl;
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QVERIFY( pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon25D );

  //adding a LineStringZ, should become LineString25D
  QgsLineString *ring1 = new QgsLineString();
  ring1->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                    << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 )
                    << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                    << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 )
                    << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );

  QCOMPARE( ring1->wkbType(), QgsWkbTypes::LineStringZ );

  pl.addInteriorRing( ring1 );

  QVERIFY( pl.interiorRing( 0 ) );
  QVERIFY( pl.interiorRing( 0 )->is3D() );
  QVERIFY( !pl.interiorRing( 0 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( pl.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::Point25D, 0.1, 0.1, 1 ) );

  //add a LineStringM, should become LineString25D
  ring1 = new QgsLineString();
  ring1->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                    << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 )
                    << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                    << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 )
                    << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );

  QCOMPARE( ring1->wkbType(), QgsWkbTypes::LineStringM );

  pl.addInteriorRing( ring1 );

  QVERIFY( pl.interiorRing( 1 ) );
  QVERIFY( pl.interiorRing( 1 )->is3D() );
  QVERIFY( !pl.interiorRing( 1 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( pl.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::Point25D, 0.1, 0.1 ) );

  //add curved ring to polygon
  QgsCircularString *ring2 = new QgsCircularString();
  ring2->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                    << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                    << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );

  QVERIFY( ring2->hasCurvedSegments() );

  pl.addInteriorRing( ring2 );

  QVERIFY( pl.interiorRing( 2 ) );
  QVERIFY( !pl.interiorRing( 2 )->hasCurvedSegments() );
  QVERIFY( pl.interiorRing( 2 )->is3D() );
  QVERIFY( !pl.interiorRing( 2 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString25D );
}

void TestQgsPolygon::clear()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  pl.addInteriorRing( ring );

  QCOMPARE( pl.numInteriorRings(), 1 );

  pl.clear();

  QVERIFY( pl.isEmpty() );
  QCOMPARE( pl.numInteriorRings(), 0 );
  QCOMPARE( pl.nCoordinates(), 0 );
  QCOMPARE( pl.ringCount(), 0 );
  QCOMPARE( pl.partCount(), 0 );
  QVERIFY( !pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );
}

void TestQgsPolygon::equality()
{
  QgsPolygon pl1;
  QgsPolygon pl2;

  QVERIFY( pl1 == pl2 );
  QVERIFY( !( pl1 != pl2 ) );

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl1.setExteriorRing( ext );

  QVERIFY( !( pl1 == pl2 ) );
  QVERIFY( pl1 != pl2 );

  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl2.setExteriorRing( ext );

  QVERIFY( pl1 == pl2 );
  QVERIFY( !( pl1 != pl2 ) );

  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 0 ) << QgsPoint( 0, 0 ) );
  pl2.setExteriorRing( ext );

  QVERIFY( !( pl1 == pl2 ) );
  QVERIFY( pl1 != pl2 );

  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  pl2.setExteriorRing( ext );

  QVERIFY( !( pl1 == pl2 ) );
  QVERIFY( pl1 != pl2 );

  pl2.setExteriorRing( pl1.exteriorRing()->clone() );

  QVERIFY( pl1 == pl2 );
  QVERIFY( !( pl1 != pl2 ) );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  pl1.addInteriorRing( ring );

  QVERIFY( !( pl1 == pl2 ) );
  QVERIFY( pl1 != pl2 );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 2, 1 )
                   << QgsPoint( 2, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 2, 1 ) );
  pl2.addInteriorRing( ring );

  QVERIFY( !( pl1 == pl2 ) );
  QVERIFY( pl1 != pl2 );

  pl2.removeInteriorRing( 0 );
  pl2.addInteriorRing( pl1.interiorRing( 0 )->clone() );

  QVERIFY( pl1 == pl2 );
  QVERIFY( !( pl1 != pl2 ) );

  QgsLineString ls;
  QVERIFY( pl1 != ls );
  QVERIFY( !( pl1 == ls ) );
}

void TestQgsPolygon::clone()
{
  QgsPolygon pl;

  std::unique_ptr< QgsPolygon > cloned( pl.clone() );
  QCOMPARE( pl, *cloned );

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl.addInteriorRing( ring );

  cloned.reset( pl.clone() );
  QCOMPARE( pl, *cloned );
}

void TestQgsPolygon::copy()
{
  QgsPolygon pl1;
  QgsPolygon pl2( pl1 );

  QCOMPARE( pl1, pl2 );

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl1.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl1.addInteriorRing( ring );

  pl2 = QgsPolygon( pl1 );
  QCOMPARE( pl1, pl2 );
}

void TestQgsPolygon::assignment()
{
  QgsPolygon pl1;
  QgsPolygon pl2;

  pl1 = pl2;
  QCOMPARE( pl2, pl1 );

  QgsPolygon pl3;
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl3.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl3.addInteriorRing( ring );

  pl1 = pl3;
  QCOMPARE( pl3, pl1 );
}

void TestQgsPolygon::setBadExteriorRing()
{
  QgsPolygon pl;

  //try with no ring
  QgsLineString *ext = nullptr;
  pl.setExteriorRing( ext );

  QVERIFY( pl.isEmpty() );
  QCOMPARE( pl.numInteriorRings(), 0 );
  QCOMPARE( pl.nCoordinates(), 0 );
  QCOMPARE( pl.ringCount(), 0 );
  QCOMPARE( pl.partCount(), 0 );
  QVERIFY( !pl.exteriorRing() );
  QVERIFY( !pl.interiorRing( 0 ) );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  // empty exterior ring
  ext = new QgsLineString();
  pl.setExteriorRing( ext );

  QVERIFY( pl.isEmpty() );
  QCOMPARE( pl.numInteriorRings(), 0 );
  QCOMPARE( pl.nCoordinates(), 0 );
  QCOMPARE( pl.ringCount(), 1 );
  QCOMPARE( pl.partCount(), 1 );
  QVERIFY( pl.exteriorRing() );
  QVERIFY( !pl.interiorRing( 0 ) );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  //test that a non closed exterior ring will be automatically closed
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) );

  QVERIFY( !ext->isClosed() );

  pl.setExteriorRing( ext );

  QVERIFY( !pl.isEmpty() );
  QVERIFY( pl.exteriorRing()->isClosed() );
  QCOMPARE( pl.nCoordinates(), 5 );
}

void TestQgsPolygon::setExteriorRing()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext );

  QVERIFY( !pl.isEmpty() );
  QCOMPARE( pl.numInteriorRings(), 0 );
  QCOMPARE( pl.nCoordinates(), 5 );
  QCOMPARE( pl.ringCount(), 1 );
  QCOMPARE( pl.partCount(), 1 );
  QVERIFY( !pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( pl.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( pl.geometryType(), QString( "Polygon" ) );
  QCOMPARE( pl.dimension(), 2 );
  QVERIFY( !pl.hasCurvedSegments() );
  QCOMPARE( pl.area(), 100.0 );
  QCOMPARE( pl.perimeter(), 40.0 );
  QVERIFY( pl.exteriorRing() );
  QVERIFY( !pl.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineString * >( pl.exteriorRing() ) ), *ext );
}

void TestQgsPolygon::setExteriorRingZM()
{
  QgsPolygon pl;

  //initial setting of exterior ring should set z/m type
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QVERIFY( pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZ );
  QCOMPARE( pl.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( pl.geometryType(), QString( "Polygon" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( pl.exteriorRing() ) ), *ext );

  pl.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QVERIFY( !pl.is3D() );
  QVERIFY( pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonM );
  QCOMPARE( pl.wktTypeStr(), QString( "PolygonM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( pl.exteriorRing() ) ), *ext );

  pl.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  pl.setExteriorRing( ext );

  QVERIFY( pl.is3D() );
  QVERIFY( pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZM );
  QCOMPARE( pl.wktTypeStr(), QString( "PolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( pl.exteriorRing() ) ), *ext );

  pl.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QVERIFY( pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon25D );
  QCOMPARE( pl.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( pl.exteriorRing() ) ), *ext );
}

void TestQgsPolygon::setCurvedExteriorRing()
{
  QgsPolygon pl;

  //setting curved exterior ring should be segmentized
  QgsCircularString *cs = new QgsCircularString();
  cs->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                 << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                 << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );

  QVERIFY( cs->hasCurvedSegments() );

  pl.setExteriorRing( cs );

  QVERIFY( !pl.exteriorRing()->hasCurvedSegments() );
  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsPolygon::setExteriorRingChangesInteriorRings()
{
  QgsPolygon pl;

  //change dimensionality of interior rings using setExteriorRing
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QVector< QgsCurve * > rings;

  QgsPointSequence pts;
  pts << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( pts );

  pts = QgsPointSequence();
  pts << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.4, 2 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.4, 0.4, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.4, 0.3, 4 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3,  1 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( pts );

  pl.setInteriorRings( rings );

  QVERIFY( pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QVERIFY( pl.interiorRing( 0 )->is3D() );
  QVERIFY( !pl.interiorRing( 0 )->isMeasure() );
  QVERIFY( pl.interiorRing( 1 )->is3D() );
  QVERIFY( !pl.interiorRing( 1 )->isMeasure() );

  //reset exterior ring to 2d
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );

  pl.setExteriorRing( ext );

  QVERIFY( !pl.is3D() );
  QVERIFY( !pl.interiorRing( 0 )->is3D() ); //rings should also be made 2D
  QVERIFY( !pl.interiorRing( 1 )->is3D() );

  //reset exterior ring to LineStringM
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) );

  pl.setExteriorRing( ext );

  QVERIFY( pl.isMeasure() );
  QVERIFY( pl.interiorRing( 0 )->isMeasure() ); //rings should also gain measure
  QVERIFY( pl.interiorRing( 1 )->isMeasure() );

  //25D exterior ring
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 0 ) );

  pl.setExteriorRing( ext );

  QVERIFY( pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QVERIFY( pl.interiorRing( 0 )->is3D() ); //rings should also be made 25D
  QVERIFY( !pl.interiorRing( 0 )->isMeasure() );
  QVERIFY( pl.interiorRing( 1 )->is3D() );
  QVERIFY( !pl.interiorRing( 1 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( pl.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString25D );
}

void TestQgsPolygon::deleteExteriorRing()
{
  QgsPolygon pl;

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 )
                << QgsPoint( 7, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  // test that interior ring is "promoted" when exterior is removed
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( pl.numInteriorRings(), 1 );

  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( pl.numInteriorRings(), 1 );

  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( pl.numInteriorRings(), 1 );

  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( pl.numInteriorRings(), 0 );

  QVERIFY( pl.exteriorRing() );
}

void TestQgsPolygon::addInteriorRing()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext );

  //empty ring
  QCOMPARE( pl.numInteriorRings(), 0 );
  QVERIFY( !pl.interiorRing( -1 ) );
  QVERIFY( !pl.interiorRing( 0 ) );

  pl.addInteriorRing( nullptr );

  QCOMPARE( pl.numInteriorRings(), 0 );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  pl.addInteriorRing( ring );

  QCOMPARE( pl.numInteriorRings(), 1 );
  QCOMPARE( pl.interiorRing( 0 ), ring );
  QVERIFY( !pl.interiorRing( 1 ) );

  QgsCoordinateSequence seq = pl.coordinateSequence();
  QgsRingSequence expected;
  expected << ( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  expected << ( QgsPointSequence() << QgsPoint( 1, 1 )
                << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );

  QCOMPARE( seq, QgsCoordinateSequence() << expected );
  QCOMPARE( pl.nCoordinates(), 10 );

  //add non-closed interior ring, should be closed automatically
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
                   << QgsPoint( 0.1, 0.9 ) << QgsPoint( 0.9, 0.9 )
                   << QgsPoint( 0.9, 0.1 ) );

  QVERIFY( !ring->isClosed() );

  pl.addInteriorRing( ring );

  QCOMPARE( pl.numInteriorRings(), 2 );
  QVERIFY( pl.interiorRing( 1 )->isClosed() );
}

void TestQgsPolygon::addInteriorRingZM()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  pl.addInteriorRing( ring );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.1, 0.9 )
                   << QgsPoint( 0.9, 0.9 ) << QgsPoint( 0.9, 0.1 ) );
  pl.addInteriorRing( ring );

  //try adding an interior ring with z to a 2d polygon, z should be dropped
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  pl.addInteriorRing( ring );

  QCOMPARE( pl.numInteriorRings(), 3 );
  QVERIFY( !pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( pl.interiorRing( 2 ) );
  QVERIFY( !pl.interiorRing( 2 )->is3D() );
  QVERIFY( !pl.interiorRing( 2 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString );

  //try adding an interior ring with m to a 2d polygon, m should be dropped
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  pl.addInteriorRing( ring );

  QCOMPARE( pl.numInteriorRings(), 4 );
  QVERIFY( !pl.is3D() );
  QVERIFY( !pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( pl.interiorRing( 3 ) );
  QVERIFY( !pl.interiorRing( 3 )->is3D() );
  QVERIFY( !pl.interiorRing( 3 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 3 )->wkbType(), QgsWkbTypes::LineString );

  //addInteriorRing without z/m to PolygonZM
  pl.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 ) );
  pl.setExteriorRing( ext );

  QVERIFY( pl.is3D() );
  QVERIFY( pl.isMeasure() );
  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZM );

  //ring has no z
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 2 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 9 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 9 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 1 ) );
  pl.addInteriorRing( ring );

  QVERIFY( pl.interiorRing( 0 ) );
  QVERIFY( pl.interiorRing( 0 )->is3D() );
  QVERIFY( pl.interiorRing( 0 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( pl.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::PointZM, 1, 1, 0, 2 ) );

  //ring has no m
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  pl.addInteriorRing( ring );

  QVERIFY( pl.interiorRing( 1 ) );
  QVERIFY( pl.interiorRing( 1 )->is3D() );
  QVERIFY( pl.interiorRing( 1 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( pl.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.1, 1, 0 ) );
}

void TestQgsPolygon::setInteriorRings()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext );

  //add a list of rings with mixed types
  QVector< QgsCurve * > rings;

  QgsPointSequence pts;
  pts << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( pts );

  pts = QgsPointSequence();
  pts << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 )
      << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.4, 0, 2 )
      << QgsPoint( QgsWkbTypes::PointM, 0.4, 0.4, 0, 3 )
      << QgsPoint( QgsWkbTypes::PointM, 0.4, 0.3, 0, 4 )
      << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( pts );

  //throw an empty ring in too
  rings << 0;

  pts = QgsPointSequence();
  pts << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
      << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[3] )->setPoints( QgsPointSequence() );

  pl.setInteriorRings( rings );

  QCOMPARE( pl.numInteriorRings(), 3 );

  QVERIFY( pl.interiorRing( 0 ) );
  QVERIFY( !pl.interiorRing( 0 )->is3D() );
  QVERIFY( !pl.interiorRing( 0 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( pl.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::Point, 0.1, 0.1 ) );

  QVERIFY( pl.interiorRing( 1 ) );
  QVERIFY( !pl.interiorRing( 1 )->is3D() );
  QVERIFY( !pl.interiorRing( 1 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( pl.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::Point, 0.3, 0.3 ) );

  QVERIFY( pl.interiorRing( 2 ) );
  QVERIFY( !pl.interiorRing( 2 )->is3D() );
  QVERIFY( !pl.interiorRing( 2 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString );

  //set rings with existing
  rings.clear();

  pts = QgsPointSequence();
  pts << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( pts );

  pl.setInteriorRings( rings );

  QCOMPARE( pl.numInteriorRings(), 1 );
  QVERIFY( pl.interiorRing( 0 ) );
  QVERIFY( !pl.interiorRing( 0 )->is3D() );
  QVERIFY( !pl.interiorRing( 0 )->isMeasure() );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( pl.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ),
            QgsPoint( QgsWkbTypes::Point, 0.8, 0.8 ) );

  rings.clear();
  pl.setInteriorRings( rings );

  QCOMPARE( pl.numInteriorRings(), 0 );
}

void TestQgsPolygon::removeInteriorRing()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                  << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext );

  QVERIFY( !pl.removeInteriorRing( -1 ) );
  QVERIFY( !pl.removeInteriorRing( 0 ) );

  QVector< QgsCurve * > rings;

  QgsPointSequence pts;
  pts << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.1, 0.2 )
      << QgsPoint( 0.2, 0.2 ) << QgsPoint( 0.2, 0.1 )
      << QgsPoint( 0.1, 0.1 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( pts );

  pts = QgsPointSequence();
  pts << QgsPoint( 0.3, 0.3 ) << QgsPoint( 0.3, 0.4 )
      << QgsPoint( 0.4, 0.4 ) << QgsPoint( 0.4, 0.3 )
      << QgsPoint( 0.3, 0.3 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( pts );

  pts = QgsPointSequence();
  pts << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.8, 0.9 )
      << QgsPoint( 0.9, 0.9 ) << QgsPoint( 0.9, 0.8 )
      << QgsPoint( 0.8, 0.8 );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[2] )->setPoints( pts );

  pl.setInteriorRings( rings );

  QCOMPARE( pl.numInteriorRings(), 3 );

  QVERIFY( pl.removeInteriorRing( 0 ) );
  QCOMPARE( pl.numInteriorRings(), 2 );
  QCOMPARE( pl.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.3 ) );
  QCOMPARE( pl.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.8, 0.8 ) );

  QVERIFY( pl.removeInteriorRing( 1 ) );
  QCOMPARE( pl.numInteriorRings(), 1 );
  QCOMPARE( pl.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.3 ) );

  QVERIFY( pl.removeInteriorRing( 0 ) );
  QCOMPARE( pl.numInteriorRings(), 0 );

  QVERIFY( !pl.removeInteriorRing( 0 ) );
}

void TestQgsPolygon::removeInteriorRings()
{
  QgsPolygon pl;
  pl.removeInteriorRings();

  QgsLineString ext;
  ext.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 )
                 << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );

  pl.setExteriorRing( ext.clone() );
  pl.removeInteriorRings();

  QCOMPARE( pl.numInteriorRings(), 0 );

  // add interior rings
  QgsLineString ring1;
  ring1.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
                   << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )
                   << QgsPoint( 0.1, 0.1 ) );
  QgsLineString ring2;
  ring2.setPoints( QgsPointSequence() << QgsPoint( 0.6, 0.8 )
                   << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )
                   << QgsPoint( 0.6, 0.8 ) );

  pl.setInteriorRings( QVector< QgsCurve * >() << ring1.clone() << ring2.clone() );

  // remove ring with size filter
  pl.removeInteriorRings( 0.0075 );
  QCOMPARE( pl.numInteriorRings(), 1 );

  // remove ring with no size filter
  pl.removeInteriorRings();
  QCOMPARE( pl.numInteriorRings(), 0 );
}

void TestQgsPolygon::removeInvalidRings()
{
  QgsPolygon pl;
  pl.removeInvalidRings(); // no crash

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  pl.setExteriorRing( ls.clone() );

  pl.removeInvalidRings();
  QCOMPARE( pl.asWkt(), QStringLiteral( "PolygonZM ((11 2 3 4, 4 12 13 14, 11 12 13 14, 11 22 23 24, 11 2 3 4))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  pl.removeInvalidRings();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 4 12 13 14, 11 12 13 14, 11 22 23 24, 11 2 3 4),(1 2 5 6, 11.01 2.01 15 16, 11 2.01 25 26, 1 2 5 6))" ) );
}

void TestQgsPolygon::insertVertex()
{
  //insert vertex in empty polygon
  QgsPolygon pl;

  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( pl.isEmpty() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 )
                << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ls.clone() );

  auto ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  QVERIFY( pl.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( pl.nCoordinates(), 8 );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 0.5, 0 ) );

  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );

  // first vertex
  QVERIFY( pl.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );

  QCOMPARE( pl.nCoordinates(), 9 );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( ext->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( ext->pointN( 8 ), QgsPoint( 0, 0.1 ) );

  // last vertex
  QVERIFY( pl.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );

  QCOMPARE( pl.nCoordinates(), 10 );
  QCOMPARE( ext->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( ext->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( ext->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  // with interior ring
  pl.addInteriorRing( ls.clone() );

  auto ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  QCOMPARE( pl.nCoordinates(), 17 );

  QVERIFY( pl.insertVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 0.3, 0 ) ) );

  QCOMPARE( pl.nCoordinates(), 18 );
  QCOMPARE( ring->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 0.5, 0 ) );

  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 1, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !pl.insertVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 6.0, 7.0 ) ) );

  // first vertex in interior ring
  QVERIFY( pl.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 0, 0.1 ) ) );

  QCOMPARE( pl.nCoordinates(), 19 );
  QCOMPARE( ring->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( ring->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( ring->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( ring->pointN( 8 ), QgsPoint( 0, 0.1 ) );

  // last vertex in interior ring
  QVERIFY( pl.insertVertex( QgsVertexId( 0, 1, 9 ), QgsPoint( 0.1, 0.1 ) ) );

  QCOMPARE( pl.nCoordinates(), 20 );
  QCOMPARE( ring->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( ring->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( ring->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( ring->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );
}

void TestQgsPolygon::moveVertex()
{
  //empty polygon
  QgsPolygon pl;

  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( pl.isEmpty() );

  //valid polygon
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  pl.setExteriorRing( ls.clone() );

  auto ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  QVERIFY( pl.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( pl.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( pl.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );

  QCOMPARE( ext->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  //out of range
  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 3.0, 4.0 ) ) );

  QCOMPARE( ext->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // with interior ring
  pl.addInteriorRing( ls.clone() );

  auto ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  QVERIFY( pl.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( pl.moveVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( pl.moveVertex( QgsVertexId( 0, 1, 2 ), QgsPoint( 26.0, 27.0 ) ) );

  QCOMPARE( ring->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( ring->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !pl.moveVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 3.0, 4.0 ) ) );
}

void TestQgsPolygon::deleteVertex()
{
  //empty polygon
  QgsPolygon pl;

  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( pl.isEmpty() );

  //valid polygon
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 )
                << QgsPoint( 7, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  pl.setExteriorRing( ls.clone() );

  auto ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  //out of range vertices
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  //valid vertices
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );

  QCOMPARE( ext->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ext->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );

  QCOMPARE( ext->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( ext->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );

  QCOMPARE( ext->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( ext->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( ext->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ext->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QVERIFY( !pl.exteriorRing() );

  // with interior ring
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  auto ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  //out of range vertices
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 1, -1 ) ) );
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 1, 100 ) ) );
  QVERIFY( !pl.deleteVertex( QgsVertexId( 0, 2, 1 ) ) );

  //valid vertices
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  QCOMPARE( ring->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( ring->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ring->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );

  QCOMPARE( ring->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ring->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( ring->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 1, 4 ) ) );

  QCOMPARE( ring->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( ring->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( ring->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( ring->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( pl.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  QCOMPARE( pl.numInteriorRings(), 0 );
  QVERIFY( pl.exteriorRing() );
}

void TestQgsPolygon::deleteVertexRemoveRing()
{
  //removing the fourth to last vertex removes the whole ring
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                  << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );

  pl.setExteriorRing( ext );
  QVERIFY( pl.exteriorRing() );

  pl.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !pl.exteriorRing() );
}

void TestQgsPolygon::nextVertex()
{
  QgsPolygon pl;

  QgsPoint pt;
  QgsVertexId v;
  QVERIFY( !pl.nextVertex( v, pt ) );

  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !pl.nextVertex( v, pt ) );

  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !pl.nextVertex( v, pt ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  pl.setExteriorRing( ls.clone() );

  v = QgsVertexId( 0, 0, 4 ); //out of range
  QVERIFY( !pl.nextVertex( v, pt ) );

  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( pl.nextVertex( v, pt ) );

  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );

  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );

  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );

  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );

  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( !pl.nextVertex( v, pt ) );

  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );

  // add interior ring
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 )
                << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  pl.addInteriorRing( ls.clone() );

  v = QgsVertexId( 0, 1, 4 ); //out of range
  QVERIFY( !pl.nextVertex( v, pt ) );

  v = QgsVertexId( 0, 1, -5 );
  QVERIFY( pl.nextVertex( v, pt ) );

  v = QgsVertexId( 0, 1, -1 );
  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );

  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );

  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );

  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );

  v = QgsVertexId( 0, 2, 0 );
  QVERIFY( !pl.nextVertex( v, pt ) );

  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( pl.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
}

void TestQgsPolygon::vertexNumberFromVertexId()
{
  QgsLineString ls;

  QCOMPARE( ls.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );

  QgsPolygon pl;
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 )
                << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  pl.setExteriorRing( ls.clone() );

  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), 4 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 5 ) ), -1 );

  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), 4 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 0, 5 ) ), -1 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), 5 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 1 ) ), 6 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 2 ) ), 7 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 3 ) ), 8 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 4 ) ), 9 );
  QCOMPARE( pl.vertexNumberFromVertexId( QgsVertexId( 0, 1, 5 ) ), -1 );
}

void TestQgsPolygon::vertexAngle()
{
  QgsPolygon pl;

  ( void )pl.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )pl.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )pl.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 )
                << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ls.clone() );

  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );

  pl.addInteriorRing( ls.clone() );

  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( pl.vertexAngle( QgsVertexId( 0, 1, 6 ) ), 2.35619, 0.00001 );
}

void TestQgsPolygon::adjacentVertices()
{
  // test adjacent vertices - should wrap around!
  QgsPolygon pl;
  QgsVertexId previous( 1, 2, 3 );
  QgsVertexId next( 4, 5, 6 );

  pl.adjacentVertices( QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  pl.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  pl.adjacentVertices( QgsVertexId( 0, 0, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  QgsLineString *closedRing1 = new QgsLineString();
  closedRing1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                          << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 )
                          << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  pl.setExteriorRing( closedRing1 );

  pl.adjacentVertices( QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );

  pl.adjacentVertices( QgsVertexId( 0, 0, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );

  pl.adjacentVertices( QgsVertexId( 0, 0, 2 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 3 ) );

  pl.adjacentVertices( QgsVertexId( 0, 0, 3 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );

  pl.adjacentVertices( QgsVertexId( 0, 0, 4 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );

  pl.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  // part number should be retained
  pl.adjacentVertices( QgsVertexId( 1, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 1 ) );

  // interior ring
  pl.addInteriorRing( closedRing1->clone() );

  pl.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 1 ) );

  pl.adjacentVertices( QgsVertexId( 0, 1, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 2 ) );

  pl.adjacentVertices( QgsVertexId( 0, 1, 2 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 3 ) );

  pl.adjacentVertices( QgsVertexId( 0, 1, 3 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 4 ) );

  pl.adjacentVertices( QgsVertexId( 0, 1, 4 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 1 ) );

  pl.adjacentVertices( QgsVertexId( 0, 2, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
}

void TestQgsPolygon::removeDuplicateNodes()
{
  QgsPolygon pl;

  QVERIFY( !pl.removeDuplicateNodes() );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 11, 22 ) << QgsPoint( 11, 2 ) );
  pl.setExteriorRing( ls.clone() );

  QVERIFY( !pl.removeDuplicateNodes() );
  QCOMPARE( pl.asWkt(), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 )
                << QgsPoint( 11.01, 21.99 ) << QgsPoint( 10.99, 1.99 )
                << QgsPoint( 11, 2 ) );
  pl.setExteriorRing( ls.clone() );

  QVERIFY( pl.removeDuplicateNodes( 0.02 ) );
  QVERIFY( !pl.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 )
                << QgsPoint( 11.01, 21.99 ) << QgsPoint( 10.99, 1.99 )
                << QgsPoint( 11, 2 ) );
  pl.setExteriorRing( ls.clone() );

  QVERIFY( !pl.removeDuplicateNodes() );
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11.01 1.99, 11.02 2.01, 11 12, 11 22, 11.01 21.99, 10.99 1.99, 11 2))" ) );

  // don't create degenerate rings
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 2.01 )
                << QgsPoint( 11, 2.01 ) << QgsPoint( 11, 2 ) );
  pl.addInteriorRing( ls.clone() );

  QVERIFY( pl.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2),(11 2, 11.01 2.01, 11 2.01, 11 2))" ) );
}

void TestQgsPolygon::dropZValue()
{
  QgsPolygon pl;

  pl.dropZValue();
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  pl.dropZValue(); // not z
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with z
  pl.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 )
                << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZ );

  pl.dropZValue();
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with zm
  pl.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 )
                << QgsPoint( 11, 12, 13, 14 )
                << QgsPoint( 1, 12, 23, 24 )
                << QgsPoint( 1, 2, 3, 4 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZM );

  pl.dropZValue();
  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonM );

  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( pl.exteriorRing() )->pointN( 0 ),
            QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( pl.interiorRing( 0 ) )->pointN( 0 ),
            QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
}

void TestQgsPolygon::dropMValue()
{
  QgsPolygon pl;

  pl.dropMValue();
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 )
                << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  pl.dropMValue(); // not zm
  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );

  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with m
  pl.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 )
                << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 )
                << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonM );
  pl.dropMValue();

  QCOMPARE( pl.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( pl.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );

  // with zm
  pl.clear();
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 )
                << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZM );
  pl.dropMValue();

  QCOMPARE( pl.wkbType(), QgsWkbTypes::PolygonZ );
  QCOMPARE( pl.exteriorRing()->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( pl.exteriorRing() )->pointN( 0 ),
            QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  QCOMPARE( pl.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( pl.interiorRing( 0 ) )->pointN( 0 ),
            QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
}

void TestQgsPolygon::swapXy()
{
  QgsPolygon pl;
  pl.swapXy(); //no crash

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  pl.setExteriorRing( ls.clone() );

  pl.swapXy();
  QCOMPARE( pl.asWkt(), QStringLiteral( "PolygonZM ((2 11 3 4, 12 11 13 14, 22 11 23 24, 2 11 3 4))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  pl.swapXy();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4),(2 1 5 6, 2.01 11.01 15 16, 2.01 11 25 26, 2 11 5 6, 2 1 5 6))" ) );
}

void TestQgsPolygon::boundary()
{
  QgsPolygon pl;
  QVERIFY( !pl.boundary() );

  QgsLineString ext;
  ext.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext.clone() );

  QgsAbstractGeometry *boundary = pl.boundary();
  QgsLineString *lineBoundary = dynamic_cast< QgsLineString * >( boundary );

  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.0 );

  delete boundary;

  // add interior rings
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
                  << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )
                  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString ring2;
  ring2.setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
                   << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )
                   << QgsPoint( 0.8, 0.8 ) );

  pl.setInteriorRings( QVector< QgsCurve * >() << ring.clone() << ring2.clone() );

  boundary = pl.boundary();
  QgsMultiLineString *multiLineBoundary = dynamic_cast< QgsMultiLineString * >( boundary );

  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 3 );

  lineBoundary = qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.0 );

  lineBoundary = qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.1 );
  QCOMPARE( lineBoundary->xAt( 1 ), 0.2 );
  QCOMPARE( lineBoundary->xAt( 2 ), 0.2 );
  QCOMPARE( lineBoundary->xAt( 3 ), 0.1 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.1 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.1 );
  QCOMPARE( lineBoundary->yAt( 2 ), 0.2 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.1 );

  lineBoundary = qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.8 );
  QCOMPARE( lineBoundary->xAt( 1 ), 0.9 );
  QCOMPARE( lineBoundary->xAt( 2 ), 0.9 );
  QCOMPARE( lineBoundary->xAt( 3 ), 0.8 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.8 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.8 );
  QCOMPARE( lineBoundary->yAt( 2 ), 0.9 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.8 );

  pl.setInteriorRings( QVector< QgsCurve * >() );
  delete boundary;

  //test boundary with z
  ext.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 )
                 << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  pl.setExteriorRing( ext.clone() );

  boundary = pl.boundary();
  lineBoundary = qgis::down_cast< QgsLineString * >( boundary );

  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( lineBoundary->zAt( 0 ), 10.0 );
  QCOMPARE( lineBoundary->zAt( 1 ), 15.0 );
  QCOMPARE( lineBoundary->zAt( 2 ), 20.0 );
  QCOMPARE( lineBoundary->zAt( 3 ), 10.0 );

  delete boundary;
}

void TestQgsPolygon::pointDistanceToBoundary()
{
  QgsPolygon pl;
  // no meaning, but let's not crash
  ( void )pl.pointDistanceToBoundary( 0, 0 );

  QgsLineString ext;
  ext.setPoints( QgsPointSequence() << QgsPoint( 0, 0 )
                 << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )
                 << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  pl.setExteriorRing( ext.clone() );

  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0.1, 0.5 ), 0.1, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );

  // with a ring
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
                  << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.6 )
                  << QgsPoint( 0.1, 0.6 ) << QgsPoint( 0.1, 0.1 ) );
  pl.setInteriorRings( QVector< QgsCurve * >() << ring.clone() );

  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0.1, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0.01, 0.5 ), 0.01, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0.08, 0.5 ), 0.02, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( 0.12, 0.5 ), -0.02, 0.0000000001 );
  QGSCOMPARENEAR( pl.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );
}

void TestQgsPolygon::closestSegment()
{
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;

  QgsPolygon pl;
  ( void )pl.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty polygon, just want no crash

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 )
                << QgsPoint( 5, 10 ) );
  pl.setExteriorRing( ls.clone() );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( pl.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );

  // with interior ring
  ls.setPoints( QgsPointSequence() << QgsPoint( 6, 11.5 ) << QgsPoint( 6.5, 12 )
                << QgsPoint( 6, 13 ) << QgsPoint( 6, 11.5 ) );
  pl.addInteriorRing( ls.clone() );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 6, 11.4 ), pt, v, &leftOf ), 0.01, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.5, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  QGSCOMPARENEAR( pl.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  // point directly on segment
  QCOMPARE( pl.closestSegment( QgsPoint( 6, 13 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 6, 13 ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( leftOf, 0 );
}

void TestQgsPolygon::segmentLength()
{
  QgsPolygon pl;

  QCOMPARE( pl.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2 )
                << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                << QgsPoint( 111, 2 ) << QgsPoint( 11, 2 ) );
  pl.setExteriorRing( ls.clone() );

  QCOMPARE( pl.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 1 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 1, 1, 1 ) ), 0.0 );

  // add interior ring
  ls.setPoints( QgsPointSequence() << QgsPoint( 30, 6 )
                << QgsPoint( 34, 6 ) << QgsPoint( 34, 8 )
                << QgsPoint( 30, 8 ) << QgsPoint( 30, 6 ) );
  pl.addInteriorRing( ls.clone() );

  QCOMPARE( pl.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, -1 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 0 ) ), 4.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 1 ) ), 2.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 2 ) ), 4.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 3 ) ), 2.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 0, 1, 4 ) ), 0.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( pl.segmentLength( QgsVertexId( 1, 1, 1 ) ), 2.0 );
}

void TestQgsPolygon::forceRHR()
{
  QgsPolygon pl;
  pl.forceRHR(); // no crash

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  pl.forceRHR();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 23 24, 0 100 23 24, 0 0 3 4))" ) );
  pl.forceRHR();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 23 24, 100 0 13 14, 0 0 3 4))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  pl.forceRHR();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 13 14, 0 100 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  pl.forceRHR();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 13 14, 100 0 13 14, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 10 20 3 4, 20 10 6 8, 10 10 1 2))" ) );
  pl.forceRHR();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 6 8, 10 20 3 4, 10 10 1 2))" ) );

  // force cw (same as force RHR)
  pl = QgsPolygon();
  pl.forceClockwise(); // no crash

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  pl.forceClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 23 24, 0 100 23 24, 0 0 3 4))" ) );
  pl.forceClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 23 24, 100 0 13 14, 0 0 3 4))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  pl.forceClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 13 14, 0 100 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  pl.forceClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 13 14, 100 0 13 14, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 10 20 3 4, 20 10 6 8, 10 10 1 2))" ) );
  pl.forceClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 6 8, 10 20 3 4, 10 10 1 2))" ) );

  // force ccw
  pl = QgsPolygon();
  pl.forceCounterClockwise(); // no crash

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  pl.forceCounterClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 23 24, 100 100 13 14, 0 100 13 14, 0 0 3 4))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 23 24, 0 100 23 24, 0 0 3 4))" ) );
  pl.forceCounterClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 23 24, 0 100 23 24, 0 0 3 4))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  pl.forceCounterClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 23 24, 100 100 13 14, 0 100 13 14, 0 0 3 4),(10 10 1 2, 10 20 6 8, 20 10 3 4, 10 10 1 2))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 13 14, 0 100 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  pl.forceCounterClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 13 14, 0 100 23 24, 0 0 3 4),(10 10 1 2, 10 20 6 8, 20 10 3 4, 10 10 1 2))" ) );

  pl.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 10 20 3 4, 20 10 6 8, 10 10 1 2))" ) );
  pl.forceCounterClockwise();
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 23 24, 100 100 13 14, 0 100 13 14, 0 0 3 4),(10 10 1 2, 10 20 3 4, 20 10 6 8, 10 10 1 2))" ) );
}

void TestQgsPolygon::boundingBoxIntersects()
{
  QgsPolygon pl;
  QVERIFY( !pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 4, 2 )
                << QgsPoint( 4, 4 ) << QgsPoint( 0, 2 ) );
  pl.setExteriorRing( ls.clone() );

  QVERIFY( pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );

  // double test because of cache
  QVERIFY( pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !pl.boundingBoxIntersects( QgsRectangle( 11, 3, 6, 9 ) ) );
  QVERIFY( !pl.boundingBoxIntersects( QgsRectangle( 11, 3, 6, 9 ) ) );
  QCOMPARE( pl.boundingBox(), QgsRectangle( 0, 2, 4, 4 ) );

  // clear cache
  ls.setPoints( QgsPointSequence() << QgsPoint( 10, 2 ) << QgsPoint( 14, 2 )
                << QgsPoint( 15, 4 ) << QgsPoint( 10, 2 ) );
  pl.setExteriorRing( ls.clone() );

  QVERIFY( !pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( !pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QCOMPARE( pl.boundingBox(), QgsRectangle( 10, 2, 15, 4 ) );
  QVERIFY( pl.boundingBoxIntersects( QgsRectangle( 11, 3, 16, 9 ) ) );

  // technically invalid -- the interior ring is outside the exterior, but we want boundingBoxIntersects to be tolerant to
  // cases like this!
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 4, 2 )
                << QgsPoint( 5, 4 ) << QgsPoint( 1, 2 ) );
  pl.addInteriorRing( ls.clone() );

  QVERIFY( pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( pl.boundingBoxIntersects( QgsRectangle( 1, 3, 6, 9 ) ) );
  QVERIFY( pl.boundingBoxIntersects( QgsRectangle( 11, 3, 16, 9 ) ) );
  QVERIFY( !pl.boundingBoxIntersects( QgsRectangle( 21, 3, 26, 9 ) ) );
  QCOMPARE( pl.boundingBox(), QgsRectangle( 10, 2, 15, 4 ) );
}

void TestQgsPolygon::filterVertices()
{
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };

  QgsPolygon pl;
  pl.filterVertices( filter ); // no crash

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  pl.setExteriorRing( ls.clone() );

  pl.filterVertices( filter );

  QCOMPARE( pl.asWkt(), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  pl.filterVertices( filter );

  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4),(10 2 5 6, 11.01 2.01 15 16, 11 2.01 25 26, 11 2 5 6, 10 2 5 6),(11.01 2.01 15 16, 11 2.01 25 26))" ) );
}

void TestQgsPolygon::transformVertices()
{
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };

  QgsPolygon pl;
  pl.transformVertices( transform ); // no crash

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  pl.setExteriorRing( ls.clone() );

  pl.transformVertices( transform );
  QCOMPARE( pl.asWkt(), QStringLiteral( "PolygonZM ((13 5 7 9, 6 15 17 19, 13 15 17 19, 13 25 27 29, 13 5 7 9))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  pl.transformVertices( transform );
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((15 8 11 14, 8 18 21 24, 15 18 21 24, 15 28 31 34, 15 8 11 14),(12 5 9 11, 6 15 17 19, 13.01 5.01 19 21, 13 5.01 29 31, 13 5 9 11, 12 5 9 11),(3 5 9 11, 13.01 5.01 19 21, 13 5.01 29 31, 3 5 9 11))" ) );
}

void TestQgsPolygon::transformWithClass()
{
  QgsPolygon pl;
  TestTransformer transformer;

  // no crash
  QVERIFY( pl.transform( &transformer ) );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  pl.setExteriorRing( ls.clone() );

  QVERIFY( pl.transform( &transformer ) );
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((33 16 8 3, 12 26 18 13, 33 26 18 13, 33 36 28 23, 33 16 8 3))" ) );

  ls.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM )
                << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM )
                << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM )
                << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  pl.addInteriorRing( ls.clone() );

  QVERIFY( pl.transform( &transformer ) );
  QCOMPARE( pl.asWkt( 2 ), QStringLiteral( "PolygonZM ((99 30 13 2, 36 40 23 12, 99 40 23 12, 99 50 33 22, 99 30 13 2),(30 16 10 5, 12 26 18 13, 33.03 16.01 20 15, 33 16.01 30 25, 33 16 10 5, 30 16 10 5),(3 16 10 5, 33.03 16.01 20 15, 33 16.01 30 25, 3 16 10 5))" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !pl.transform( &failTransformer ) );
}

void TestQgsPolygon::transform2D()
{
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  QgsPolygon pl;

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                << QgsPoint( 6274985, -3526584 )
                << QgsPoint( 6474985, -3526584 )
                << QgsPoint( 6374985, -3626584 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  pl.transform( tr, Qgis::TransformDirection::Forward );
  const QgsLineString *ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  QGSCOMPARENEAR( ext->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).y(), -39.724, 0.001 );

  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMaximum(), -38.7999, 0.001 );

  const QgsLineString *ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  QGSCOMPARENEAR( ring->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).y(), -39.724, 0.001 );

  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().yMaximum(), -38.7999, 0.001 );
}

void TestQgsPolygon::transform3D()
{
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  QgsPolygon pl;

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  pl.transform( tr, Qgis::TransformDirection::Forward );

  const QgsLineString *ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  QGSCOMPARENEAR( ext->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).m(), 2.0, 0.001 );

  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMaximum(), -38.7999, 0.001 );

  const QgsLineString *ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  QGSCOMPARENEAR( ring->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).m(), 2.0, 0.001 );

  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pl.interiorRing( 0 )->boundingBox().yMaximum(), -38.7999, 0.001 );

}

void TestQgsPolygon::transformReverse()
{
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  QgsPolygon pl;

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  pl.transform( tr, Qgis::TransformDirection::Forward );
  pl.transform( tr, Qgis::TransformDirection::Reverse );

  const QgsLineString *ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  QGSCOMPARENEAR( ext->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( ext->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( ext->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( ext->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( ext->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( ext->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( ext->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( ext->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( ext->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMaximum(), -3526584, 100 );

  const QgsLineString *ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  QGSCOMPARENEAR( ring->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( ring->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( ring->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( ring->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( ring->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( ring->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( ring->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( ring->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( ring->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( ring->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( ring->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( ring->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( ring->boundingBox().yMaximum(), -3526584, 100 );
}

void TestQgsPolygon::transformOldVersion()
{
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  QgsPolygon pl;

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
  //z value transform
  pl.transform( tr, Qgis::TransformDirection::Forward, true );
  const QgsLineString *ext = static_cast< const QgsLineString * >( pl.exteriorRing() );
  QGSCOMPARENEAR( ext->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).z(), -19.249066, 0.001 );
  const QgsLineString *ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );
  QGSCOMPARENEAR( ring->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).z(), -19.249066, 0.001 );
  pl.transform( tr, Qgis::TransformDirection::Reverse, true );
  ext = static_cast< const QgsLineString * >( pl.exteriorRing() );
  QGSCOMPARENEAR( ext->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).z(), 1, 0.001 );
  ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );
  QGSCOMPARENEAR( ring->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).z(), 1, 0.001 );
#endif
}

void TestQgsPolygon::Qtransform()
{
  QTransform qtr = QTransform::fromScale( 2, 3 );

  QgsPolygon pl;
  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 12, 23, 24 )
                << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  pl.setExteriorRing( ls.clone() );
  pl.addInteriorRing( ls.clone() );

  pl.transform( qtr, 2, 3, 4, 5 );

  const QgsLineString *ext = static_cast< const QgsLineString * >( pl.exteriorRing() );

  QGSCOMPARENEAR( ext->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( ext->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( ext->pointN( 0 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 0 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( ext->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( ext->pointN( 1 ).z(), 41.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 1 ).m(), 74.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( ext->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( ext->pointN( 2 ).z(), 71.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 2 ).m(), 124.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( ext->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( ext->pointN( 3 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( ext->pointN( 3 ).m(), 24.0, 0.001 );

  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( pl.exteriorRing()->boundingBox().yMaximum(), 36, 0.001 );

  const QgsLineString *ring = static_cast< const QgsLineString * >( pl.interiorRing( 0 ) );

  QGSCOMPARENEAR( ring->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( ring->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( ring->pointN( 0 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 0 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( ring->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( ring->pointN( 1 ).z(), 41.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 1 ).m(), 74.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( ring->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( ring->pointN( 2 ).z(), 71.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 2 ).m(), 124.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( ring->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( ring->pointN( 3 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( ring->pointN( 3 ).m(), 24.0, 0.001 );

  QGSCOMPARENEAR( ring->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( ring->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( ring->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( ring->boundingBox().yMaximum(), 36, 0.001 );
}

void TestQgsPolygon::cast()
{
  QVERIFY( !QgsPolygon().cast( nullptr ) );

  QgsPolygon pl;
  QVERIFY( QgsPolygon().cast( &pl ) );

  pl.fromWkt( QStringLiteral( "PolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsPolygon().cast( &pl ) );

  pl.fromWkt( QStringLiteral( "PolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsPolygon().cast( &pl ) );

  pl.fromWkt( QStringLiteral( "PolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsPolygon().cast( &pl ) );
}

void TestQgsPolygon::toPolygon()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl.addInteriorRing( ring );

  //surfaceToPolygon - should be identical given polygon has no curves
  std::unique_ptr< QgsPolygon > surface( pl.surfaceToPolygon() );
  QCOMPARE( *surface, pl );

  //toPolygon - should be identical given polygon has no curves
  std::unique_ptr< QgsPolygon > toP( pl.toPolygon() );
  QCOMPARE( *toP, pl );
}

void TestQgsPolygon::toCurveType()
{
  QgsPolygon pl;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl.addInteriorRing( ring );

  std::unique_ptr< QgsCurvePolygon > curveType( pl.toCurveType() );

  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygonZM );

  const QgsLineString *exteriorRing = static_cast< const QgsLineString * >( curveType->exteriorRing() );
  QCOMPARE( exteriorRing->numPoints(), 5 );
  QCOMPARE( exteriorRing->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( exteriorRing->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) );
  QCOMPARE( exteriorRing->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  QCOMPARE( exteriorRing->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( exteriorRing->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );

  QCOMPARE( curveType->numInteriorRings(), 1 );

  const QgsLineString *interiorRing = static_cast< const QgsLineString * >( curveType->interiorRing( 0 ) );
  QCOMPARE( interiorRing->numPoints(), 5 );
  QCOMPARE( interiorRing->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 ) );
  QCOMPARE( interiorRing->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) );
  QCOMPARE( interiorRing->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 ) );
  QCOMPARE( interiorRing->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( interiorRing->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
}

void TestQgsPolygon::toFromWkb()
{
  QgsPolygon pl1;
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  pl1.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  pl1.addInteriorRing( ring );

  QByteArray wkb = pl1.asWkb();
  QCOMPARE( wkb.size(), pl1.wkbSize() );

  QgsPolygon pl2;
  QgsConstWkbPtr wkbPtr( wkb );
  pl2.fromWkb( wkbPtr );

  QCOMPARE( pl1, pl2 );
}

void TestQgsPolygon::toFromWkbZM()
{
  //PolygonZ
  QgsPolygon pl1;
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  pl1.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  pl1.addInteriorRing( ring );

  QByteArray wkb = pl1.asWkb();
  QCOMPARE( wkb.size(), pl1.wkbSize() );

  QgsPolygon pl2;
  QgsConstWkbPtr wkbPtr1( wkb );
  pl2.fromWkb( wkbPtr1 );
  QCOMPARE( pl1, pl2 );

  //PolygonM
  pl1.clear();
  pl2.clear();

  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10,  0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0,  0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  pl1.setExteriorRing( ext );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 9, 0, 2 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 9, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 ) );
  pl1.addInteriorRing( ring );

  wkb = pl1.asWkb();
  QgsConstWkbPtr wkbPtr2( wkb );
  pl2.fromWkb( wkbPtr2 );

  QCOMPARE( pl1, pl2 );

  //PolygonZM
  pl1.clear();
  pl2.clear();

  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl1.setExteriorRing( ext );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl1.addInteriorRing( ring );

  wkb = pl1.asWkb();
  QgsConstWkbPtr wkbPtr3( wkb );
  pl2.fromWkb( wkbPtr3 );

  QCOMPARE( pl1, pl2 );
}

void TestQgsPolygon::toFromWkb25D()
{
  QgsPolygon pl1;
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  pl1.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point25D, 1, 9, 2 )
                   << QgsPoint( QgsWkbTypes::Point25D, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::Point25D, 9, 1, 4 )
                   << QgsPoint( QgsWkbTypes::Point25D, 1, 1, 1 ) );
  pl1.addInteriorRing( ring );

  QgsPolygon pl2;
  QByteArray wkb = pl1.asWkb();
  QgsConstWkbPtr wkbPtr( wkb );
  pl2.fromWkb( wkbPtr );

  QCOMPARE( pl1, pl2 );

  //bad WKB - check for no crash
  pl2.clear();

  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !pl2.fromWkb( nullPtr ) );
  QCOMPARE( pl2.wkbType(), QgsWkbTypes::Polygon );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !pl2.fromWkb( wkbPointPtr ) );
  QCOMPARE( pl2.wkbType(), QgsWkbTypes::Polygon );
}

void TestQgsPolygon::toFromWKT()
{
  QgsPolygon pl1;

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  pl1.setExteriorRing( ext );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  pl1.addInteriorRing( ring );

  QString wkt = pl1.asWkt();
  QVERIFY( !wkt.isEmpty() );

  QgsPolygon pl2;
  QVERIFY( pl2.fromWkt( wkt ) );
  QCOMPARE( pl1, pl2 );

  //bad WKT
  QVERIFY( !pl2.fromWkt( "Point()" ) );
  QVERIFY( pl2.isEmpty() );
  QVERIFY( !pl2.exteriorRing() );
  QCOMPARE( pl2.numInteriorRings(), 0 );
  QVERIFY( !pl2.is3D() );
  QVERIFY( !pl2.isMeasure() );
  QCOMPARE( pl2.wkbType(), QgsWkbTypes::Polygon );
}

void TestQgsPolygon::exportImport()
{
  //as JSON
  QgsPolygon exportPolygon;
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  exportPolygon.setExteriorRing( ext );

  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportPolygon.asGml2( doc ) ), expectedSimpleGML2 );

  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior></Polygon>" ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );

  QString expectedGML3empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( QStringLiteral( "{\"coordinates\":[[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]]],\"type\":\"Polygon\"}" ) );
  QCOMPARE( exportPolygon.asJson(), expectedSimpleJson );

  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( QStringLiteral( "{\"coordinates\":[[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]],[[1.0,1.0],[1.0,9.0],[9.0,9.0],[9.0,1.0],[1.0,1.0]]],\"type\":\"Polygon\"}" ) );
  QCOMPARE( exportPolygon.asJson(), expectedJson );

  QgsPolygon exportPolygonFloat;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 100 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 10 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 ) );
  exportPolygonFloat.setExteriorRing( ext );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 4 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 4 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) );
  exportPolygonFloat.addInteriorRing( ring );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[[1.111,1.111],[1.111,11.111],[11.111,11.111],[11.111,1.111],[1.111,1.111]],[[0.667,0.667],[0.667,1.333],[1.333,1.333],[1.333,0.667],[0.667,0.667]]],\"type\":\"Polygon\"}" ) );
  QCOMPARE( exportPolygonFloat.asJson( 3 ), expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2 += QLatin1String( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,1 1,9 9,9 9,1 1,1</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QGSCOMPAREGML( elemToString( exportPolygon.asGml2( doc ) ), expectedGML2 );

  QString expectedGML2prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2prec3 += QLatin1String( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QGSCOMPAREGML( elemToString( exportPolygonFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior>" ) );
  expectedGML3 += QLatin1String( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 1 1 9 9 9 9 1 1 1</posList></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedGML3 );

  QString expectedGML3prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.111 1.111 1.111 11.111 11.111 11.111 11.111 1.111 1.111 1.111</posList></LinearRing></exterior>" ) );
  expectedGML3prec3 += QLatin1String( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.667 0.667 0.667 1.333 1.333 1.333 1.333 0.667 0.667 0.667</posList></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0,0,0 0,10,0 10,10,0 10,0,0 0,0,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1,1,0 1,9,0 9,9,0 9,1,0 1,1,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  QCOMPARE( exportPolygon.asKml(), expectedKml );

  QString expectedKmlPrec3( QStringLiteral( "<Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1.111,1.111,0 1.111,11.111,0 11.111,11.111,0 11.111,1.111,0 1.111,1.111,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0.667,0.667,0 0.667,1.333,0 1.333,1.333,0 1.333,0.667,0 0.667,0.667,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  QCOMPARE( exportPolygonFloat.asKml( 3 ), expectedKmlPrec3 );
}

QGSTEST_MAIN( TestQgsPolygon )
#include "testqgspolygon.moc"
