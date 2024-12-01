/***************************************************************************
     testqgsmultipoint.cpp
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

#include "qgslinestring.h"
#include "qgsmultipoint.h"
#include "qgspoint.h"
#include "qgsvertexid.h"

#include "testgeometryutils.h"

class TestQgsMultiPoint : public QObject
{
    Q_OBJECT
  private slots:
    void constructor();
    void copyConstructor();
    void addGeometryWithNullptr();
    void addGeometryWithNotAPoint();
    void addGeometry();
    void addGeometryWithZM();
    void addGeometryDimensionPreservation();
    void addGeometryDimensionPreservationZ();
    void addGeometryDimensionPreservationM();
    void addGeometryDimensionPreservationZM();
    void cordinateSequenceWithMultiPart();
    void insertGeometry();
    void clone();
    void clear();
    void assignment();
    void cast();
    void isValid();
    void toCurveType();
    void toFromWKB();
    void toFromWKBWithZ();
    void toFromWKBWithM();
    void toFromWKBWithZM();
    void toFromBadWKB();
    void toFromWKT();
    void exportImport();
    void vertexNumberFromVertexId();
    void adjacentVertices();
    void filterVertices();
    void vertexIterator();
    void removeDuplicateNodes();
    void closestSegment();
    void boundingBox();
    void boundary();
    void segmentLength();
};

void TestQgsMultiPoint::constructor()
{
  QgsMultiPoint mp;

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPoint" ) );
  QCOMPARE( mp.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( mp.dimension(), 0 );
  QVERIFY( !mp.hasCurvedSegments() );
  QCOMPARE( mp.area(), 0.0 );
  QCOMPARE( mp.perimeter(), 0.0 );
  QCOMPARE( mp.numGeometries(), 0 );
  QVERIFY( !mp.geometryN( 0 ) );
  QVERIFY( !mp.geometryN( -1 ) );
  QCOMPARE( mp.vertexCount( 0, 0 ), 0 );
  QCOMPARE( mp.vertexCount( 0, 1 ), 0 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 0 );

  // constructor using vector of QgsPoint
  QgsMultiPoint mp2( QVector<QgsPoint> {} );
  QVERIFY( mp2.isEmpty() );
  QCOMPARE( mp2.nCoordinates(), 0 );
  QVERIFY( !mp2.is3D() );
  QVERIFY( !mp2.isMeasure() );
  QCOMPARE( mp2.wkbType(), Qgis::WkbType::MultiPoint );
  // vector of 2d points
  QgsMultiPoint mp3( QVector<QgsPoint> { QgsPoint( 1, 2 ), QgsPoint( 3, 4 ) } );
  QVERIFY( !mp3.isEmpty() );
  QCOMPARE( mp3.nCoordinates(), 2 );
  QVERIFY( !mp3.is3D() );
  QVERIFY( !mp3.isMeasure() );
  QCOMPARE( mp3.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp3.pointN( 0 )->x(), 1 );
  QCOMPARE( mp3.pointN( 0 )->y(), 2 );
  QCOMPARE( mp3.pointN( 1 )->x(), 3 );
  QCOMPARE( mp3.pointN( 1 )->y(), 4 );
  // vector of 3d points
  QgsMultiPoint mp4( QVector<QgsPoint> { QgsPoint( 1, 2, 11 ), QgsPoint( 3, 4, 12 ) } );
  QVERIFY( !mp4.isEmpty() );
  QCOMPARE( mp4.nCoordinates(), 2 );
  QVERIFY( mp4.is3D() );
  QVERIFY( !mp4.isMeasure() );
  QCOMPARE( mp4.wkbType(), Qgis::WkbType::MultiPointZ );
  QCOMPARE( mp4.pointN( 0 )->x(), 1 );
  QCOMPARE( mp4.pointN( 0 )->y(), 2 );
  QCOMPARE( mp4.pointN( 0 )->z(), 11 );
  QCOMPARE( mp4.pointN( 1 )->x(), 3 );
  QCOMPARE( mp4.pointN( 1 )->y(), 4 );
  QCOMPARE( mp4.pointN( 1 )->z(), 12 );
  // vector of 4d points
  QgsMultiPoint mp5( QVector<QgsPoint> { QgsPoint( 1, 2, 11, 21 ), QgsPoint( 3, 4, 12, 22 ) } );
  QVERIFY( !mp5.isEmpty() );
  QCOMPARE( mp5.nCoordinates(), 2 );
  QVERIFY( mp5.is3D() );
  QVERIFY( mp5.isMeasure() );
  QCOMPARE( mp5.wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( mp5.pointN( 0 )->x(), 1 );
  QCOMPARE( mp5.pointN( 0 )->y(), 2 );
  QCOMPARE( mp5.pointN( 0 )->z(), 11 );
  QCOMPARE( mp5.pointN( 0 )->m(), 21 );
  QCOMPARE( mp5.pointN( 1 )->x(), 3 );
  QCOMPARE( mp5.pointN( 1 )->y(), 4 );
  QCOMPARE( mp5.pointN( 1 )->z(), 12 );
  QCOMPARE( mp5.pointN( 1 )->m(), 22 );
  // vector of pointm
  QgsMultiPoint mp6( QVector<QgsPoint> { QgsPoint( 1, 2, std::numeric_limits<double>::quiet_NaN(), 21 ), QgsPoint( 3, 4, std::numeric_limits<double>::quiet_NaN(), 22 ) } );
  QVERIFY( !mp6.isEmpty() );
  QCOMPARE( mp6.nCoordinates(), 2 );
  QVERIFY( !mp6.is3D() );
  QVERIFY( mp6.isMeasure() );
  QCOMPARE( mp6.wkbType(), Qgis::WkbType::MultiPointM );
  QCOMPARE( mp6.pointN( 0 )->x(), 1 );
  QCOMPARE( mp6.pointN( 0 )->y(), 2 );
  QCOMPARE( mp6.pointN( 0 )->m(), 21 );
  QCOMPARE( mp6.pointN( 1 )->x(), 3 );
  QCOMPARE( mp6.pointN( 1 )->y(), 4 );
  QCOMPARE( mp6.pointN( 1 )->m(), 22 );
  // vector of points with mismatched dimensions
  QgsMultiPoint mp7( QVector<QgsPoint> { QgsPoint( 1, 2 ), QgsPoint( 3, 4, std::numeric_limits<double>::quiet_NaN(), 22 ) } );
  QVERIFY( !mp7.isEmpty() );
  QCOMPARE( mp7.nCoordinates(), 2 );
  QVERIFY( !mp7.is3D() );
  QVERIFY( !mp7.isMeasure() );
  QCOMPARE( mp7.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp7.pointN( 0 )->x(), 1 );
  QCOMPARE( mp7.pointN( 0 )->y(), 2 );
  QCOMPARE( mp7.pointN( 1 )->x(), 3 );
  QCOMPARE( mp7.pointN( 1 )->y(), 4 );

  // constructor using vector of QgsPoint*
  QgsMultiPoint mp2a( QVector<QgsPoint *> {} );
  QVERIFY( mp2a.isEmpty() );
  QCOMPARE( mp2a.nCoordinates(), 0 );
  QVERIFY( !mp2a.is3D() );
  QVERIFY( !mp2a.isMeasure() );
  QCOMPARE( mp2a.wkbType(), Qgis::WkbType::MultiPoint );
  // vector of 2d points
  QgsMultiPoint mp3a( QVector<QgsPoint *> { new QgsPoint( 1, 2 ), new QgsPoint( 3, 4 ) } );
  QVERIFY( !mp3a.isEmpty() );
  QCOMPARE( mp3a.nCoordinates(), 2 );
  QVERIFY( !mp3a.is3D() );
  QVERIFY( !mp3a.isMeasure() );
  QCOMPARE( mp3a.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp3a.pointN( 0 )->x(), 1 );
  QCOMPARE( mp3a.pointN( 0 )->y(), 2 );
  QCOMPARE( mp3a.pointN( 1 )->x(), 3 );
  QCOMPARE( mp3a.pointN( 1 )->y(), 4 );
  // vector of 3d points
  QgsMultiPoint mp4a( QVector<QgsPoint *> { new QgsPoint( 1, 2, 11 ), new QgsPoint( 3, 4, 12 ) } );
  QVERIFY( !mp4a.isEmpty() );
  QCOMPARE( mp4a.nCoordinates(), 2 );
  QVERIFY( mp4a.is3D() );
  QVERIFY( !mp4a.isMeasure() );
  QCOMPARE( mp4a.wkbType(), Qgis::WkbType::MultiPointZ );
  QCOMPARE( mp4a.pointN( 0 )->x(), 1 );
  QCOMPARE( mp4a.pointN( 0 )->y(), 2 );
  QCOMPARE( mp4a.pointN( 0 )->z(), 11 );
  QCOMPARE( mp4a.pointN( 1 )->x(), 3 );
  QCOMPARE( mp4a.pointN( 1 )->y(), 4 );
  QCOMPARE( mp4a.pointN( 1 )->z(), 12 );
  // vector of 4d points
  QgsMultiPoint mp5a( QVector<QgsPoint *> { new QgsPoint( 1, 2, 11, 21 ), new QgsPoint( 3, 4, 12, 22 ) } );
  QVERIFY( !mp5a.isEmpty() );
  QCOMPARE( mp5a.nCoordinates(), 2 );
  QVERIFY( mp5a.is3D() );
  QVERIFY( mp5a.isMeasure() );
  QCOMPARE( mp5a.wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( mp5a.pointN( 0 )->x(), 1 );
  QCOMPARE( mp5a.pointN( 0 )->y(), 2 );
  QCOMPARE( mp5a.pointN( 0 )->z(), 11 );
  QCOMPARE( mp5a.pointN( 0 )->m(), 21 );
  QCOMPARE( mp5a.pointN( 1 )->x(), 3 );
  QCOMPARE( mp5a.pointN( 1 )->y(), 4 );
  QCOMPARE( mp5a.pointN( 1 )->z(), 12 );
  QCOMPARE( mp5a.pointN( 1 )->m(), 22 );
  // vector of pointm
  QgsMultiPoint mp6a( QVector<QgsPoint *> { new QgsPoint( 1, 2, std::numeric_limits<double>::quiet_NaN(), 21 ), new QgsPoint( 3, 4, std::numeric_limits<double>::quiet_NaN(), 22 ) } );
  QVERIFY( !mp6a.isEmpty() );
  QCOMPARE( mp6a.nCoordinates(), 2 );
  QVERIFY( !mp6a.is3D() );
  QVERIFY( mp6a.isMeasure() );
  QCOMPARE( mp6a.wkbType(), Qgis::WkbType::MultiPointM );
  QCOMPARE( mp6a.pointN( 0 )->x(), 1 );
  QCOMPARE( mp6a.pointN( 0 )->y(), 2 );
  QCOMPARE( mp6a.pointN( 0 )->m(), 21 );
  QCOMPARE( mp6a.pointN( 1 )->x(), 3 );
  QCOMPARE( mp6a.pointN( 1 )->y(), 4 );
  QCOMPARE( mp6a.pointN( 1 )->m(), 22 );
  // vector of points with mismatched dimensions
  QgsMultiPoint mp7a( QVector<QgsPoint *> { new QgsPoint( 1, 2 ), new QgsPoint( 3, 4, std::numeric_limits<double>::quiet_NaN(), 22 ) } );
  QVERIFY( !mp7a.isEmpty() );
  QCOMPARE( mp7a.nCoordinates(), 2 );
  QVERIFY( !mp7a.is3D() );
  QVERIFY( !mp7a.isMeasure() );
  QCOMPARE( mp7a.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp7a.pointN( 0 )->x(), 1 );
  QCOMPARE( mp7a.pointN( 0 )->y(), 2 );
  QCOMPARE( mp7a.pointN( 1 )->x(), 3 );
  QCOMPARE( mp7a.pointN( 1 )->y(), 4 );

  // constructor using vector of QgsPointXY
  QgsMultiPoint mp8( QVector<QgsPointXY> {} );
  QVERIFY( mp8.isEmpty() );
  QCOMPARE( mp8.nCoordinates(), 0 );
  QVERIFY( !mp8.is3D() );
  QVERIFY( !mp8.isMeasure() );
  QCOMPARE( mp8.wkbType(), Qgis::WkbType::MultiPoint );
  // vector of 2d points
  QgsMultiPoint mp9( QVector<QgsPointXY> { QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) } );
  QVERIFY( !mp9.isEmpty() );
  QCOMPARE( mp9.nCoordinates(), 2 );
  QVERIFY( !mp9.is3D() );
  QVERIFY( !mp9.isMeasure() );
  QCOMPARE( mp9.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp9.pointN( 0 )->x(), 1 );
  QCOMPARE( mp9.pointN( 0 )->y(), 2 );
  QCOMPARE( mp9.pointN( 1 )->x(), 3 );
  QCOMPARE( mp9.pointN( 1 )->y(), 4 );

  // using separate vectors of coordinates
  QgsMultiPoint mp10( QVector<double> {}, QVector<double> {} );
  QVERIFY( mp10.isEmpty() );
  QCOMPARE( mp10.nCoordinates(), 0 );
  QVERIFY( !mp10.is3D() );
  QVERIFY( !mp10.isMeasure() );
  QCOMPARE( mp10.wkbType(), Qgis::WkbType::MultiPoint );

  QgsMultiPoint mp11( QVector<double> { 1, 2 }, QVector<double> { 3, 4 } );
  QVERIFY( !mp11.isEmpty() );
  QCOMPARE( mp11.nCoordinates(), 2 );
  QVERIFY( !mp11.is3D() );
  QVERIFY( !mp11.isMeasure() );
  QCOMPARE( mp11.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp11.pointN( 0 )->x(), 1 );
  QCOMPARE( mp11.pointN( 0 )->y(), 3 );
  QCOMPARE( mp11.pointN( 1 )->x(), 2 );
  QCOMPARE( mp11.pointN( 1 )->y(), 4 );

  QgsMultiPoint mp12( QVector<double> { 1, 2 }, QVector<double> { 3, 4 }, QVector<double> { 13, 14 } );
  QVERIFY( !mp12.isEmpty() );
  QCOMPARE( mp12.nCoordinates(), 2 );
  QVERIFY( mp12.is3D() );
  QVERIFY( !mp12.isMeasure() );
  QCOMPARE( mp12.wkbType(), Qgis::WkbType::MultiPointZ );
  QCOMPARE( mp12.pointN( 0 )->x(), 1 );
  QCOMPARE( mp12.pointN( 0 )->y(), 3 );
  QCOMPARE( mp12.pointN( 0 )->z(), 13 );
  QCOMPARE( mp12.pointN( 1 )->x(), 2 );
  QCOMPARE( mp12.pointN( 1 )->y(), 4 );
  QCOMPARE( mp12.pointN( 1 )->z(), 14 );

  QgsMultiPoint mp13( QVector<double> { 1, 2 }, QVector<double> { 3, 4 }, QVector<double> { 13, 14 }, QVector<double> { 15, 16 } );
  QVERIFY( !mp13.isEmpty() );
  QCOMPARE( mp13.nCoordinates(), 2 );
  QVERIFY( mp13.is3D() );
  QVERIFY( mp13.isMeasure() );
  QCOMPARE( mp13.wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( mp13.pointN( 0 )->x(), 1 );
  QCOMPARE( mp13.pointN( 0 )->y(), 3 );
  QCOMPARE( mp13.pointN( 0 )->z(), 13 );
  QCOMPARE( mp13.pointN( 0 )->m(), 15 );
  QCOMPARE( mp13.pointN( 1 )->x(), 2 );
  QCOMPARE( mp13.pointN( 1 )->y(), 4 );
  QCOMPARE( mp13.pointN( 1 )->z(), 14 );
  QCOMPARE( mp13.pointN( 1 )->m(), 16 );

  QgsMultiPoint mp14( QVector<double> { 1, 2 }, QVector<double> { 3, 4 }, QVector<double> {}, QVector<double> { 15, 16 } );
  QVERIFY( !mp14.isEmpty() );
  QCOMPARE( mp14.nCoordinates(), 2 );
  QVERIFY( !mp14.is3D() );
  QVERIFY( mp14.isMeasure() );
  QCOMPARE( mp14.wkbType(), Qgis::WkbType::MultiPointM );
  QCOMPARE( mp14.pointN( 0 )->x(), 1 );
  QCOMPARE( mp14.pointN( 0 )->y(), 3 );
  QCOMPARE( mp14.pointN( 0 )->m(), 15 );
  QCOMPARE( mp14.pointN( 1 )->x(), 2 );
  QCOMPARE( mp14.pointN( 1 )->y(), 4 );
  QCOMPARE( mp14.pointN( 1 )->m(), 16 );

  // mismatched sizes
  QgsMultiPoint mp15( QVector<double> { 1, 2, 5 }, QVector<double> { 3, 4 } );
  QVERIFY( !mp15.isEmpty() );
  QCOMPARE( mp15.nCoordinates(), 2 );
  QVERIFY( !mp15.is3D() );
  QVERIFY( !mp15.isMeasure() );
  QCOMPARE( mp15.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp15.pointN( 0 )->x(), 1 );
  QCOMPARE( mp15.pointN( 0 )->y(), 3 );
  QCOMPARE( mp15.pointN( 1 )->x(), 2 );
  QCOMPARE( mp15.pointN( 1 )->y(), 4 );
  QgsMultiPoint mp16( QVector<double> { 1, 2 }, QVector<double> { 3, 4, 5 } );
  QVERIFY( !mp16.isEmpty() );
  QCOMPARE( mp16.nCoordinates(), 2 );
  QVERIFY( !mp16.is3D() );
  QVERIFY( !mp16.isMeasure() );
  QCOMPARE( mp16.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp16.pointN( 0 )->x(), 1 );
  QCOMPARE( mp16.pointN( 0 )->y(), 3 );
  QCOMPARE( mp16.pointN( 1 )->x(), 2 );
  QCOMPARE( mp16.pointN( 1 )->y(), 4 );
}

void TestQgsMultiPoint::copyConstructor()
{
  QgsMultiPoint mp1;
  QgsMultiPoint mp2( mp1 );

  QVERIFY( mp2.isEmpty() );

  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 20, 10, 14, 18 ) );
  QgsMultiPoint mp3( mp1 );

  QCOMPARE( mp3.numGeometries(), 2 );
  QCOMPARE( mp3.wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( *static_cast<const QgsPoint *>( mp3.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp3.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointZM, 20, 10, 14, 18 ) );
}

void TestQgsMultiPoint::addGeometryWithNullptr()
{
  QgsMultiPoint mp;
  mp.addGeometry( nullptr );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
  QVERIFY( !mp.geometryN( 0 ) );
  QVERIFY( !mp.geometryN( -1 ) );
}

void TestQgsMultiPoint::addGeometryWithNotAPoint()
{
  QgsMultiPoint mp;
  QVERIFY( !mp.addGeometry( new QgsLineString() ) );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
  QVERIFY( !mp.geometryN( 0 ) );
  QVERIFY( !mp.geometryN( -1 ) );
}

void TestQgsMultiPoint::addGeometry()
{
  QgsMultiPoint mp;
  QgsPoint part( 1, 10 );
  mp.addGeometry( part.clone() );

  QVERIFY( !mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 1 );
  QCOMPARE( mp.nCoordinates(), 1 );
  QCOMPARE( mp.ringCount(), 1 );
  QCOMPARE( mp.partCount(), 1 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPoint" ) );
  QCOMPARE( mp.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( mp.dimension(), 0 );
  QVERIFY( !mp.hasCurvedSegments() );
  QCOMPARE( mp.area(), 0.0 );
  QCOMPARE( mp.perimeter(), 0.0 );
  QVERIFY( mp.geometryN( 0 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp.geometryN( 0 ) ), part );
  QVERIFY( !mp.geometryN( 100 ) );
  QVERIFY( !mp.geometryN( -1 ) );
  QCOMPARE( mp.vertexCount( 0, 0 ), 1 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 0 );
}

void TestQgsMultiPoint::addGeometryWithZM()
{
  //initial adding of geometry should set z/m type
  QgsMultiPoint mp;
  QgsPoint part( Qgis::WkbType::PointZ, 10, 11, 1 );
  mp.addGeometry( part.clone() );

  QVERIFY( mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZ );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPoint Z" ) );
  QCOMPARE( mp.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 0 ) ) ), part );

  mp.clear();
  part = QgsPoint( Qgis::WkbType::PointM, 10, 10, 0, 3 );
  mp.addGeometry( part.clone() );

  QVERIFY( !mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointM );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPoint M" ) );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 0 ) ) ), part );

  mp.clear();
  part = QgsPoint( Qgis::WkbType::PointZM, 10, 10, 5, 3 );
  mp.addGeometry( part.clone() );

  QVERIFY( mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( mp.wktTypeStr(), QString( "MultiPoint ZM" ) );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 0 ) ) ), part );
}

void TestQgsMultiPoint::addGeometryDimensionPreservation()
{
  //adding subsequent points should not alter z/m type, regardless of points type
  QgsMultiPoint mp;
  QgsPoint part( 9, 1 );

  mp.addGeometry( part.clone() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 1.0, 2.0, 3 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp.vertexCount( 0, 0 ), 1 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 1 );
  QCOMPARE( mp.vertexCount( 2, 0 ), 0 );
  QCOMPARE( mp.vertexCount( -1, 0 ), 0 );
  QCOMPARE( mp.nCoordinates(), 2 );
  QCOMPARE( mp.ringCount(), 1 );
  QCOMPARE( mp.partCount(), 2 );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint ); //should still be 2d
  QVERIFY( !mp.is3D() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 1 ) ) ), QgsPoint( 1, 2 ) );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 11.0, 12.0, 0, 3 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
  QCOMPARE( mp.vertexCount( 0, 0 ), 1 );
  QCOMPARE( mp.vertexCount( 1, 0 ), 1 );
  QCOMPARE( mp.vertexCount( 2, 0 ), 1 );
  QCOMPARE( mp.nCoordinates(), 3 );
  QCOMPARE( mp.ringCount(), 1 );
  QCOMPARE( mp.partCount(), 3 );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint ); //should still be 2d
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 2 ) ) ), QgsPoint( 11, 12 ) );
}

void TestQgsMultiPoint::addGeometryDimensionPreservationZ()
{
  QgsMultiPoint mp;

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 1.0, 2.0, 3 ) );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZ );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::Point, 11.0, 12.0 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZ );
  QVERIFY( mp.is3D() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 0 ) ) ), QgsPoint( Qgis::WkbType::PointZ, 1, 2, 3 ) );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 1 ) ) ), QgsPoint( Qgis::WkbType::PointZ, 11, 12, 0 ) );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 21.0, 22.0, 0, 3 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZ );
  QVERIFY( mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 2 ) ) ), QgsPoint( Qgis::WkbType::PointZ, 21, 22, 0 ) );
}

void TestQgsMultiPoint::addGeometryDimensionPreservationM()
{
  QgsMultiPoint mp;

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 1.0, 2.0, 0, 3 ) );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointM );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::Point, 11.0, 12.0 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointM );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 0 ) ) ), QgsPoint( Qgis::WkbType::PointM, 1, 2, 0, 3 ) );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 1 ) ) ), QgsPoint( Qgis::WkbType::PointM, 11, 12, 0, 0 ) );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 21.0, 22.0, 3 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointM );
  QVERIFY( !mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 2 ) ) ), QgsPoint( Qgis::WkbType::PointM, 21, 22, 0, 0 ) );
}

void TestQgsMultiPoint::addGeometryDimensionPreservationZM()
{
  QgsMultiPoint mp;

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 1.0, 2.0, 4, 3 ) );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZM );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::Point, 11.0, 12.0 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZM );
  QVERIFY( mp.isMeasure() );
  QVERIFY( mp.is3D() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 0 ) ) ), QgsPoint( Qgis::WkbType::PointZM, 1, 2, 4, 3 ) );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 1 ) ) ), QgsPoint( Qgis::WkbType::PointZM, 11, 12, 0, 0 ) );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 21.0, 22.0, 3 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZM );
  QVERIFY( mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 2 ) ) ), QgsPoint( Qgis::WkbType::PointZM, 21, 22, 3, 0 ) );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 31.0, 32.0, 0, 4 ) );

  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPointZM );
  QVERIFY( mp.is3D() );
  QVERIFY( mp.isMeasure() );
  QCOMPARE( *( static_cast<const QgsPoint *>( mp.geometryN( 3 ) ) ), QgsPoint( Qgis::WkbType::PointZM, 31, 32, 0, 4 ) );
}

void TestQgsMultiPoint::cordinateSequenceWithMultiPart()
{
  QgsMultiPoint mp;
  QgsPoint part( 10, 11 );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.vertexCount( 0, 0 ), 1 );

  part = QgsPoint( 9, 1 );
  mp.addGeometry( part.clone() );

  QCOMPARE( mp.vertexCount( 1, 0 ), 1 );
  QCOMPARE( mp.numGeometries(), 2 );
  QVERIFY( mp.geometryN( 0 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp.geometryN( 1 ) ), part );

  QgsCoordinateSequence seq = mp.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 10, 11 ) ) ) << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 9, 1 ) ) ) );
  QCOMPARE( mp.nCoordinates(), 2 );
}

void TestQgsMultiPoint::insertGeometry()
{
  QgsMultiPoint mp;
  mp.insertGeometry( nullptr, 0 );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( nullptr, -1 );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( nullptr, 100 );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( new QgsLineString(), 0 );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );

  mp.insertGeometry( new QgsPoint(), 0 );

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 1 );

  mp.insertGeometry( new QgsPoint( 0, 0 ), 0 );

  QVERIFY( !mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 2 );
}

void TestQgsMultiPoint::clone()
{
  QgsMultiPoint mp;
  std::unique_ptr<QgsMultiPoint> cloned( mp.clone() );

  QVERIFY( cloned->isEmpty() );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 0, 0, 1, 5 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 1, 2, 3, 4 ) );
  cloned.reset( mp.clone() );

  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast<const QgsPoint *>( cloned->geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( cloned->geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointZM, 1, 2, 3, 4 ) );
}

void TestQgsMultiPoint::clear()
{
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 0, 10, 2 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 11, 12, 3 ) );

  QCOMPARE( mp.numGeometries(), 2 );

  mp.clear();

  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.nCoordinates(), 0 );
  QCOMPARE( mp.ringCount(), 0 );
  QCOMPARE( mp.partCount(), 0 );
  QVERIFY( !mp.is3D() );
  QVERIFY( !mp.isMeasure() );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
}

void TestQgsMultiPoint::assignment()
{
  QgsMultiPoint mp1;

  QgsMultiPoint mp2;
  mp1 = mp2;
  QCOMPARE( mp1.numGeometries(), 0 );

  QgsMultiPoint mp3;
  mp3.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp3.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 20, 10, 14, 18 ) );
  mp1 = mp3;
  QCOMPARE( mp1.numGeometries(), 2 );
  QCOMPARE( *static_cast<const QgsPoint *>( mp1.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp1.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointZM, 20, 10, 14, 18 ) );
}

void TestQgsMultiPoint::cast()
{
  QVERIFY( !QgsMultiPoint().cast( nullptr ) );

  QgsMultiPoint mp;
  QVERIFY( QgsMultiPoint().cast( &mp ) );

  mp.clear();
  mp.fromWkt( QStringLiteral( "MultiPointZ(PointZ(0 1 1))" ) );
  QVERIFY( QgsMultiPoint().cast( &mp ) );

  mp.fromWkt( QStringLiteral( "MultiPointM(PointM(0 1 1))" ) );
  QVERIFY( QgsMultiPoint().cast( &mp ) );

  mp.fromWkt( QStringLiteral( "MultiPointZM(PointZM(0 1 1 2))" ) );
  QVERIFY( QgsMultiPoint().cast( &mp ) );
}

void TestQgsMultiPoint::isValid()
{
  QString error;

  QgsMultiPoint mp;
  QVERIFY( mp.isValid( error ) );

  mp.addGeometry( new QgsPoint() );
  QVERIFY( mp.isValid( error ) );

  mp.addGeometry( new QgsPoint( 0, 0 ) );
  QVERIFY( mp.isValid( error ) );
}

void TestQgsMultiPoint::toCurveType()
{
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 20, 10, 14, 18 ) );

  std::unique_ptr<QgsMultiPoint> curveType( mp.toCurveType() );

  QCOMPARE( curveType->wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( curveType->numGeometries(), 2 );

  const QgsPoint *curve = static_cast<const QgsPoint *>( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );

  curve = static_cast<const QgsPoint *>( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, QgsPoint( Qgis::WkbType::PointZM, 20, 10, 14, 18 ) );
}

void TestQgsMultiPoint::toFromWKB()
{
  QgsMultiPoint mp1;
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::Point, 10, 11 ) );
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::Point, 20, 21 ) );

  QByteArray wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr( wkb );

  QgsMultiPoint mp2;
  mp2.fromWkb( wkbPtr );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::Point, 10, 11 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::Point, 20, 21 ) );
}

void TestQgsMultiPoint::toFromWKBWithZ()
{
  QgsMultiPoint mp1;
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 10, 0, 4 ) );
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 9, 1, 4 ) );

  QByteArray wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr( wkb );

  QgsMultiPoint mp2;
  mp2.fromWkb( wkbPtr );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( mp2.wkbType(), Qgis::WkbType::MultiPointZ );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointZ, 10, 0, 4 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointZ, 9, 1, 4 ) );
}

void TestQgsMultiPoint::toFromWKBWithM()
{
  QgsMultiPoint mp1;
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 10, 0, 0, 4 ) );
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 9, 1, 0, 4 ) );

  QByteArray wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr( wkb );

  QgsMultiPoint mp2;
  mp2.fromWkb( wkbPtr );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( mp2.wkbType(), Qgis::WkbType::MultiPointM );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointM, 10, 0, 0, 4 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointM, 9, 1, 0, 4 ) );
}

void TestQgsMultiPoint::toFromWKBWithZM()
{
  QgsMultiPoint mp1;
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 70, 4 ) );
  mp1.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 9, 1, 3, 4 ) );

  QByteArray wkb = mp1.asWkb();
  QgsConstWkbPtr wkbPtr( wkb );

  QgsMultiPoint mp2;
  mp2.fromWkb( wkbPtr );

  QCOMPARE( mp2.numGeometries(), 2 );
  QCOMPARE( mp2.wkbType(), Qgis::WkbType::MultiPointZM );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 70, 4 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp2.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointZM, 9, 1, 3, 4 ) );
}

void TestQgsMultiPoint::toFromBadWKB()
{
  // check for no crash
  QgsMultiPoint mp;
  QgsConstWkbPtr nullPtr( nullptr, 0 );

  QVERIFY( !mp.fromWkb( nullPtr ) );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );

  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );

  QVERIFY( !mp.fromWkb( wkbPointPtr ) );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
}

void TestQgsMultiPoint::toFromWKT()
{
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 9, 1, 4, 4 ) );

  QString wkt = mp.asWkt();
  QVERIFY( !wkt.isEmpty() );

  mp.clear();
  QVERIFY( mp.fromWkt( wkt ) );
  QCOMPARE( mp.numGeometries(), 2 );
  QCOMPARE( *static_cast<const QgsPoint *>( mp.geometryN( 0 ) ), QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast<const QgsPoint *>( mp.geometryN( 1 ) ), QgsPoint( Qgis::WkbType::PointZM, 9, 1, 4, 4 ) );

  //bad WKT
  mp.clear();
  QVERIFY( !mp.fromWkt( "Point()" ) );
  QVERIFY( mp.isEmpty() );
  QCOMPARE( mp.numGeometries(), 0 );
  QCOMPARE( mp.wkbType(), Qgis::WkbType::MultiPoint );
}

void TestQgsMultiPoint::exportImport()
{
  QgsMultiPoint exportC;
  exportC.addGeometry( new QgsPoint( Qgis::WkbType::Point, 0, 10 ) );
  exportC.addGeometry( new QgsPoint( Qgis::WkbType::Point, 10, 0 ) );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,10</coordinates></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">10,0</coordinates></Point></pointMember></MultiPoint>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPoint xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPoint().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0 10</pos></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">10 0</pos></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiPoint xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPoint().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[0.0,10.0],[10.0,0.0]],\"type\":\"MultiPoint\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiPoint exportFloat;
  exportFloat.addGeometry( new QgsPoint( Qgis::WkbType::Point, 10 / 9.0, 100 / 9.0 ) );
  exportFloat.addGeometry( new QgsPoint( Qgis::WkbType::Point, 4 / 3.0, 2 / 3.0 ) );


  QString expectedJsonPrec3( "{\"coordinates\":[[1.111,11.111],[1.333,0.667]],\"type\":\"MultiPoint\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,11.111</coordinates></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.333,0.667</coordinates></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1.111 11.111</pos></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1.333 0.667</pos></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><Point><coordinates>0,10</coordinates></Point><Point><coordinates>10,0</coordinates></Point></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><Point><coordinates>1.111,11.111</coordinates></Point><Point><coordinates>1.333,0.667</coordinates></Point></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );

  QgsMultiPoint exportZ;
  exportZ.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 0, 10, 0 ) );
  exportZ.addGeometry( new QgsPoint( Qgis::WkbType::PointZ, 10, 0, 1 ) );

  QString expectedJsonZ( "{\"coordinates\":[[0.0,10.0,0.0],[10.0,0.0,1.0]],\"type\":\"MultiPoint\"}" );
  res = exportZ.asJson();
  QCOMPARE( res, expectedJsonZ );

  QgsMultiPoint exportM;
  exportM.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 0, 10, 0 ) );
  exportM.addGeometry( new QgsPoint( Qgis::WkbType::PointM, 10, 0, 1 ) );

  QString expectedJsonM( "{\"coordinates\":[[0.0,10.0],[10.0,0.0]],\"type\":\"MultiPoint\"}" );
  res = exportM.asJson();
  QCOMPARE( res, expectedJsonM );
}

void TestQgsMultiPoint::vertexNumberFromVertexId()
{
  QgsMultiPoint mp;

  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 9, 1, 4, 4 ) );

  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), -1 );
  QCOMPARE( mp.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
}

void TestQgsMultiPoint::adjacentVertices()
{
  //both should be invalid
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 9, 1, 4, 4 ) );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );

  mp.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  mp.adjacentVertices( QgsVertexId( 1, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
}

void TestQgsMultiPoint::filterVertices()
{
  QgsMultiPoint mp;
  auto filter = []( const QgsPoint &point ) -> bool {
    return point.x() < 5;
  };
  mp.filterVertices( filter ); // no crash
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 3, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 1, 0, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 11, 0, 4, 8 ) );
  mp.filterVertices( filter );

  QCOMPARE( mp.asWkt( 2 ), QStringLiteral( "MultiPoint ZM ((3 0 4 8),(1 0 4 8))" ) );
}

void TestQgsMultiPoint::vertexIterator()
{
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( 0, 0 ) );
  mp.addGeometry( new QgsPoint( 1, 1 ) );

  QgsAbstractGeometry::vertex_iterator it = mp.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = mp.vertices_end();

  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &mp );

  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( !it2.hasNext() );
}

void TestQgsMultiPoint::removeDuplicateNodes()
{
  // multipoints should not be affected by removeDuplicatePoints
  QgsMultiPoint mp;

  QVERIFY( !mp.removeDuplicateNodes() );

  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 1, 4, 8 ) );
  mp.addGeometry( new QgsPoint( Qgis::WkbType::PointZM, 10, 1, 4, 8 ) );

  QVERIFY( !mp.removeDuplicateNodes() );
  QCOMPARE( mp.numGeometries(), 2 );
}

void TestQgsMultiPoint::closestSegment()
{
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( 0, 0 ) );
  mp.addGeometry( new QgsPoint( 1, 1 ) );

  QgsPoint closest;
  QgsVertexId after;

  // return error - points have no segments
  QVERIFY( mp.closestSegment( QgsPoint( 0.5, 0.5 ), closest, after ) < 0 );
}

void TestQgsMultiPoint::boundingBox()
{
  QgsMultiPoint mp;
  mp.addGeometry( new QgsPoint( 0, 0 ) );
  QCOMPARE( mp.boundingBox(), QgsRectangle( 0, 0, 0, 0 ) );

  mp.addGeometry( new QgsPoint( 1, 2 ) );
  QCOMPARE( mp.boundingBox(), QgsRectangle( 0, 0, 1, 2 ) );

  mp.clear();
  QCOMPARE( mp.boundingBox(), QgsRectangle() );

  mp.addGeometry( new QgsPoint( 1, 2 ) );
  QCOMPARE( mp.boundingBox(), QgsRectangle( 1, 2, 1, 2 ) );

  mp.addGeometry( new QgsPoint( 10, 3 ) );
  QCOMPARE( mp.boundingBox(), QgsRectangle( 1, 2, 10, 3 ) );

  mp.addGeometry( new QgsPoint( 0, 0 ) );
  QCOMPARE( mp.boundingBox(), QgsRectangle( 0, 0, 10, 3 ) );
}

void TestQgsMultiPoint::boundary()
{
  //multipoints have no boundary defined
  QgsMultiPoint mp;
  QVERIFY( !mp.boundary() );

  // add some points and retest, should still be undefined
  mp.addGeometry( new QgsPoint( 0, 0 ) );
  mp.addGeometry( new QgsPoint( 1, 1 ) );

  QVERIFY( !mp.boundary() );
}

void TestQgsMultiPoint::segmentLength()
{
  QgsMultiPoint mp;
  QCOMPARE( mp.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );

  mp.addGeometry( new QgsPoint( 0, 0 ) );
  mp.addGeometry( new QgsPoint( 1, 1 ) );

  QCOMPARE( mp.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
}

QGSTEST_MAIN( TestQgsMultiPoint )
#include "testqgsmultipoint.moc"
